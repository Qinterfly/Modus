#include <QHBoxLayout>

#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <QVTKOpenGLNativeWidget.h>

#include <kcl/model.h>

#include "modelview.h"

using namespace Frontend;
using namespace Eigen;

ModelView::ModelView(KCL::Model const& model)
    : mModel(model)
{
    createContent();
    initialize();
}

ModelView::~ModelView()
{
}

void ModelView::clear()
{
    auto actors = mRenderer->GetActors();
    while (actors->GetLastActor())
        mRenderer->RemoveActor(actors->GetLastActor());
}

void ModelView::refresh()
{
    clear();
    drawAxes();
    drawModel();
    mRenderer->ResetCamera();
    mRenderWindow->Render();
}

IView::Type ModelView::type() const
{
    return IView::kModel;
}

void ModelView::initialize()
{
    mColors = vtkNamedColors::New();
    mOrientationWidget = vtkOrientationMarkerWidget::New();

    // Set up the scence
    mRenderer = vtkRenderer::New();
    mRenderer->SetBackground(mColors->GetColor3d("WhiteSmoke").GetData());

    // Create the window
    mRenderWindow = vtkGenericOpenGLRenderWindow::New();
    mRenderWindow->AddRenderer(mRenderer);
    mRenderWidget->setRenderWindow(mRenderWindow);
}

//! Create all the widgets and corresponding actions
void ModelView::createContent()
{
    QHBoxLayout* pLayout = new QHBoxLayout;

    // Create the VTK widget
    mRenderWidget = new QVTKOpenGLNativeWidget;

    // Combine the widgets
    pLayout->addWidget(mRenderWidget);
    setLayout(pLayout);
}

//! Represent the coordinate system
void ModelView::drawAxes()
{
    vtkNew<vtkAxesActor> axes;

    double rgba[4]{0.0, 0.0, 0.0, 0.0};
    mColors->GetColor("Carrot", rgba);
    mOrientationWidget->SetOutlineColor(rgba[0], rgba[1], rgba[2]);
    mOrientationWidget->SetOrientationMarker(axes);
    mOrientationWidget->SetInteractor(mRenderWindow->GetInteractor());
    mOrientationWidget->SetViewport(0.8, 0.0, 1.0, 0.4);
    mOrientationWidget->SetEnabled(1);
    mOrientationWidget->InteractiveOn();
}

//! Represent the model entities
void ModelView::drawModel()
{
    int numSurfaces = mModel.surfaces.size();
    for (int iSurface = 0; iSurface != numSurfaces; ++iSurface)
    {
        KCL::ElasticSurface const& surface = mModel.surfaces[iSurface];

        // Retrieve the surface data
        if (!surface.containsElement(KCL::OD))
            continue;
        auto pData = (KCL::GeneralData*) surface.element(KCL::OD);
        bool isSymmetry = pData->iSymmetry == 0;

        // Build up the transformation
        auto transform = Transformation::Identity();
        transform.translate(Vector3d(pData->coords[0], pData->coords[1], pData->coords[2]));
        transform.rotate(AngleAxisd(qDegreesToRadians(pData->sweepAngle), Vector3d::UnitY()));
        transform.rotate(AngleAxisd(qDegreesToRadians(pData->dihedralAngle), -Vector3d::UnitX()));

        // Reflect the transformation about the XOY plane
        Matrix4d reflectMatrix = Matrix4d::Identity();
        reflectMatrix(2, 2) = -1.0;
        auto reflectTransform = Transformation(reflectMatrix * transform.matrix());

        // Loop through all the element types
        std::vector<KCL::ElementType> const types = surface.types();
        int numTypes = types.size();
        for (int iType = 0; iType != numTypes; ++iType)
        {
            // Loop through all the elements of the same
            KCL::ElementType type = types[iType];
            int numElements = surface.numElements(type);
            for (int iElement = 0; iElement != numElements; ++iElement)
            {
                KCL::AbstractElement const* pBaseElement = surface.element(type, iElement);
                switch (type)
                {
                case KCL::BI:
                {
                    auto pElement = (KCL::BendingBeam*) pBaseElement;
                    drawBeam(transform, pElement->startCoords, pElement->endCoords, mColors->GetColor3d("Blue"));
                    if (isSymmetry)
                        drawBeam(reflectTransform, pElement->startCoords, pElement->endCoords, mColors->GetColor3d("Gray"));
                    break;
                }
                default:
                    break;
                }
            }
        }
    }
}

//! Represent the beam element
void ModelView::drawBeam(Transformation const& transform, KCL::Vec2 const& startCoords, KCL::Vec2 const& endCoords, vtkColor3d color)
{
    // Compute the coordinates of both ends
    auto startPosition = transform * Vector3d(startCoords[0], 0.0, startCoords[1]);
    auto endPosition = transform * Vector3d(endCoords[0], 0.0, endCoords[1]);

    // Set the points
    vtkNew<vtkPoints> points;
    points->InsertNextPoint(startPosition[0], startPosition[1], startPosition[2]);
    points->InsertNextPoint(endPosition[0], endPosition[1], endPosition[2]);

    // Set the connectivity indices
    vtkNew<vtkCellArray> indices;
    indices->InsertNextCell(2);
    indices->InsertCellPoint(0);
    indices->InsertCellPoint(1);

    // Create the polygons
    vtkNew<vtkPolyData> data;
    data->SetPoints(points);
    data->SetLines(indices);

    // Build the mapper
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(data);

    // Create the actor and add to the scene
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(color.GetData());

    // Add the actor to the scene
    mRenderer->AddActor(actor);
}

//! Set the isometric view
void ModelView::setIsometricView()
{
    vtkSmartPointer<vtkCamera> camera = mRenderer->GetActiveCamera();
    camera->SetPosition(1, 1, 1);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(0, 1, 0);
    mRenderer->ResetCamera();
    mRenderWindow->Render();
}

//! Set view perpendicular to one of the planes
void ModelView::setPlaneView(Axis axis, bool isReverse)
{
    int const kNumDirections = 3;
    vtkSmartPointer<vtkCamera> camera = mRenderer->GetActiveCamera();
    double position[kNumDirections];
    for (int i = 0; i != kNumDirections; ++i)
        position[i] = 0.0;
    int sign = isReverse ? -1 : 1;
    position[(int) axis] = 1.0 * sign;
    camera->SetPosition(position);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(0, -1, 0);
    mRenderer->ResetCamera();
    mRenderWindow->Render();
}
