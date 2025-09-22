#include <QHBoxLayout>

#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <QVTKOpenGLNativeWidget.h>

#include <kcl/model.h>
#include <magicenum/magic_enum.hpp>

#include "modelview.h"

using namespace Frontend;
using namespace Eigen;

ModelViewOptions::ModelViewOptions()
    : availableColors(vtkNamedColors::New())
{
    constexpr auto types = magic_enum::enum_values<KCL::ElementType>();

    // Color scheme
    sceneColor = availableColors->GetColor3d("WhiteSmoke");
    symmetryColor = availableColors->GetColor3d("Gray");
    for (auto type : types)
        elementColors[type] = availableColors->GetColor3d("Black");
    elementColors[KCL::BI] = availableColors->GetColor3d("Blue");
    elementColors[KCL::DB] = availableColors->GetColor3d("Blue");
    elementColors[KCL::BK] = availableColors->GetColor3d("Green");
    elementColors[KCL::PN] = availableColors->GetColor3d("Indigo");
    elementColors[KCL::OP] = availableColors->GetColor3d("Indigo");

    // Elements
    for (auto type : types)
        maskElements[type] = true;
}

ModelView::ModelView(KCL::Model const& model, ModelViewOptions const& options)
    : mModel(model)
    , mOptions(options)
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
    mOrientationWidget = vtkOrientationMarkerWidget::New();

    // Set up the scence
    mRenderer = vtkRenderer::New();
    mRenderer->SetBackground(mOptions.sceneColor.GetData());

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
    mOptions.availableColors->GetColor("Carrot", rgba);
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
            KCL::ElementType type = types[iType];

            // Check if the element is enabled for rendering
            if (!mOptions.maskElements[type])
                continue;
            vtkColor3d elementColor = mOptions.elementColors[type];

            // Obtain the list of elements of the same type
            std::vector<KCL::AbstractElement const*> elements = surface.elements(type);

            // Render the elements
            bool isBeam = type == KCL::BI || type == KCL::BK || type == KCL::DB;
            bool isPanel = type == KCL::PN || type == KCL::OP;
            if (isBeam)
            {
                drawBeams(transform, elements, elementColor);
                if (isSymmetry)
                    drawBeams(reflectTransform, elements, mOptions.symmetryColor);
            }
            else if (isPanel)
            {
                drawPanels(transform, elements, elementColor);
                if (isSymmetry)
                    drawPanels(reflectTransform, elements, mOptions.symmetryColor);
            }
        }
    }
}

//! Render beam elements
void ModelView::drawBeams(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color)
{
    int const kNumCellPoints = 2;

    // Allocate the points and their connectivity list
    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> indices;

    // Process all the elements
    int iPoint = 0;
    int numElements = elements.size();
    for (int i = 0; i != numElements; ++i)
    {
        KCL::AbstractElement const* pElement = elements[i];

        // Slice element coordinates
        KCL::VecN elementData = pElement->get();
        KCL::Vec2 startCoords = {elementData[0], elementData[1]};
        KCL::Vec2 endCoords = {elementData[2], elementData[3]};

        // Transform the coordinates to global coordinate system
        auto startPosition = transform * Vector3d(startCoords[0], 0.0, startCoords[1]);
        auto endPosition = transform * Vector3d(endCoords[0], 0.0, endCoords[1]);

        // Set the points
        points->InsertNextPoint(startPosition[0], startPosition[1], startPosition[2]);
        points->InsertNextPoint(endPosition[0], endPosition[1], endPosition[2]);

        // Set the connectivity indices
        indices->InsertNextCell(kNumCellPoints);
        indices->InsertCellPoint(iPoint);
        indices->InsertCellPoint(iPoint + 1);

        // Increase the counter
        iPoint += kNumCellPoints;
    }

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

//! Render panel elements
void ModelView::drawPanels(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color)
{
    // Allocate the points and their connectivity list
    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> indices;

    // Process all the elements
    int iPoint = 0;
    int numElements = elements.size();
    for (int i = 0; i != numElements; ++i)
    {
        // TODO
    }
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
