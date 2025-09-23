#include <QHBoxLayout>

#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPNGReader.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTexture.h>
#include <vtkTextureMapToPlane.h>
#include <QFile>
#include <QVTKOpenGLNativeWidget.h>

#include <kcl/model.h>
#include <magicenum/magic_enum.hpp>

#include "modelview.h"
#include "uiconstants.h"

using namespace Frontend;
using namespace Eigen;

using namespace Constants::Colors;

vtkSmartPointer<vtkTexture> readTexture(QString const& pathFile);

ModelViewOptions::ModelViewOptions()
{
    constexpr auto types = magic_enum::enum_values<KCL::ElementType>();

    // Color scheme
    sceneColor = vtkColors->GetColor3d("white");
    edgeColor = vtkColors->GetColor3d("gainsboro");
    for (auto type : types)
        elementColors[type] = vtkColors->GetColor3d("black");
    elementColors[KCL::BI] = vtkColors->GetColor3d("gold");
    elementColors[KCL::DB] = vtkColors->GetColor3d("gold");
    elementColors[KCL::BK] = vtkColors->GetColor3d("yellow");
    elementColors[KCL::PN] = vtkColors->GetColor3d("paleturquoise");
    elementColors[KCL::OP] = vtkColors->GetColor3d("turquoise");

    // Elements
    for (auto type : types)
        maskElements[type] = true;

    // Dimensions
    edgeOpacity = 0.5;
    beamLineWidth = 2.0f;
    massSize = 0.01;
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

    // Load the textures
    loadTextures();
}

void ModelView::loadTextures()
{
    mTextures["mass"] = readTexture(":/textures/mass.png");
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
    vtkColors->GetColor("Carrot", rgba);
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
            bool isMass = type == KCL::M3 || type == KCL::SM;
            if (isBeam)
            {
                drawBeams(transform, elements, elementColor);
                if (isSymmetry)
                    drawBeams(reflectTransform, elements, elementColor);
            }
            else if (isPanel)
            {
                drawPanels(transform, elements, elementColor);
                if (isSymmetry)
                    drawPanels(reflectTransform, elements, elementColor);
            }
            else if (isMass)
            {
                drawMasses(transform, elements);
                if (isSymmetry)
                    drawMasses(reflectTransform, elements);
            }
        }
    }
}

//! Render beam elements
void ModelView::drawBeams(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color)
{
    int const kNumCellPoints = 2;

    // Check if there are any elements to render
    if (elements.empty())
        return;

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
    actor->GetProperty()->SetLineWidth(mOptions.beamLineWidth);

    // Add the actor to the scene
    mRenderer->AddActor(actor);
}

//! Render panel elements
void ModelView::drawPanels(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color)
{
    int const kNumCellPoints = 4;

    // Check if there are any elements to render
    if (elements.empty())
        return;

    // Allocate the points and their connectivity list
    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> polygons;

    // Process all the elements
    int iPoint = 0;
    int numElements = elements.size();
    for (int i = 0; i != numElements; ++i)
    {
        KCL::AbstractElement const* pElement = elements[i];

        // Slice element coordinates
        KCL::VecN elementData = pElement->get();

        // Set points and connections between them
        int iData = 1;
        vtkNew<vtkPolygon> polygon;
        for (int j = 0; j != kNumCellPoints; ++j)
        {
            auto position = transform * Vector3d(elementData[iData], 0.0, elementData[iData + 1]);
            points->InsertNextPoint(position[0], position[1], position[2]);
            polygon->GetPointIds()->InsertNextId(iPoint);
            ++iPoint;
            iData += 2;
        }
        polygons->InsertNextCell(polygon);
    }

    // Create the polygons
    vtkNew<vtkPolyData> data;
    data->SetPoints(points);
    data->SetPolys(polygons);

    // Build the mapper
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(data);

    // Create the actor and add to the scene
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(color.GetData());
    actor->GetProperty()->SetEdgeColor(mOptions.edgeColor.GetData());
    actor->GetProperty()->SetEdgeOpacity(mOptions.edgeOpacity);
    actor->GetProperty()->EdgeVisibilityOn();

    // Add the actor to the scene
    mRenderer->AddActor(actor);
}

//! Represent point masses
void ModelView::drawMasses(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements)
{
    // Check if there are any elements to render
    if (elements.empty())
        return;

    // Get the visualization texture
    vtkSmartPointer<vtkTexture> texture = mTextures["mass"];
    double w = mOptions.massSize * getMaximumDimension();

    // Process all the elements
    int numElements = elements.size();
    for (int i = 0; i != numElements; ++i)
    {
        KCL::AbstractElement const* pBaseElement = elements[i];

        // Slice element coordinates
        Vector3d position;
        switch (pBaseElement->type())
        {
        case KCL::SM:
        {
            auto pElement = (KCL::PointMass1 const*) pBaseElement;
            position = {pElement->coords[0], 0.0, pElement->coords[1]};
            break;
        }
        case KCL::M3:
        {
            auto pElement = (KCL::PointMass3 const*) pBaseElement;
            position = {pElement->coords[0], pElement->coords[1], pElement->coords[2]};
            break;
        }
        default:
            continue;
        }
        position = transform * position;

        // Create and position the source
        vtkNew<vtkPlaneSource> source;
        double x = position[0];
        double y = position[1];
        double z = position[2];
        source->SetOrigin(x - w, y - w, z);
        source->SetPoint1(x + w, y - w, z);
        source->SetPoint2(x - w, y + w, z);

        // Create the mapper for the texture
        vtkNew<vtkTextureMapToPlane> mapTexture;
        mapTexture->SetInputConnection(source->GetOutputPort());

        // Map the resulting polygons
        vtkNew<vtkPolyDataMapper> mapPolygons;
        mapPolygons->SetInputConnection(mapTexture->GetOutputPort());

        // Create the actor
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapPolygons);
        actor->SetTexture(texture);

        // Add the actor to the scene
        mRenderer->AddActor(actor);
    }
}

//! Set the isometric view
void ModelView::setIsometricView()
{
    vtkSmartPointer<vtkCamera> camera = mRenderer->GetActiveCamera();
    camera->SetPosition(-1, 1, 1);
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

//! Get maximum view dimension based on already rendered objects
double ModelView::getMaximumDimension()
{
    double result = 0.0;
    double* dimensions = mRenderer->ComputeVisiblePropBounds();
    result = std::max(result, std::abs(dimensions[1] - dimensions[0]));
    result = std::max(result, std::abs(dimensions[3] - dimensions[2]));
    result = std::max(result, std::abs(dimensions[5] - dimensions[4]));
    return result;
}

//! Helper function to read texture from a file
vtkSmartPointer<vtkTexture> readTexture(QString const& pathFile)
{
    // Open the file for reading
    QFile file(pathFile);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    file.close();

    // Set the image reader
    vtkNew<vtkPNGReader> reader;
    reader->SetMemoryBuffer(data.constData());
    reader->SetMemoryBufferLength(data.size());
    reader->Update();

    // Create the texture
    vtkNew<vtkTexture> texture;
    texture->SetInputConnection(reader->GetOutputPort());

    return texture;
}
