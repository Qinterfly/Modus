#include <QFile>
#include <QHBoxLayout>

#include <vtkAxesActor.h>
#include <vtkCallbackCommand.h>
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
#include <QVTKOpenGLNativeWidget.h>

#include <kcl/model.h>
#include <magicenum/magic_enum.hpp>

#include "modelview.h"
#include "uiconstants.h"
#include "uiutility.h"

using namespace Frontend;
using namespace Eigen;
using namespace Constants::Colors;

constexpr auto skAllTypes = magic_enum::enum_values<KCL::ElementType>();
auto const skBeamTypes = Utility::beamTypes();
auto const skPanelTypes = Utility::panelTypes();
auto const skAeroPanelTypes = Utility::aeroPanelsTypes();
auto const skMassTypes = Utility::massTypes();
auto const skSpringTypes = Utility::springTypes();

Transformation computeTransformation(KCL::Vec3 const& coords, double dihedralAngle, double sweepAngle, double zAngle);
Transformation reflectTransformation(Transformation const& transform);
vtkSmartPointer<vtkTexture> readTexture(QString const& pathFile);

//! Helper class to update plane textures, so that they point to the camera
class PlaneFollowerCallback : public vtkCallbackCommand
{
public:
    static PlaneFollowerCallback* New()
    {
        return new PlaneFollowerCallback;
    }

    void Execute(vtkObject* caller, unsigned long evId, void*) override
    {
        // Retrieve the view vectors
        double normal[3];
        double up[3];
        double right[3];
        camera->GetViewPlaneNormal(normal);
        camera->GetViewUp(up);
        vtkMath::Normalize(normal);
        vtkMath::Normalize(up);
        vtkMath::Cross(normal, up, right);

        // Process all the sources
        double origin[3];
        double point[3];
        int numSources = sources.size();
        for (int i = 0; i != numSources; ++i)
        {
            auto source = sources[i];
            source->GetOrigin(origin);

            // Set the point along width
            vtkMath::Assign(right, point);
            vtkMath::MultiplyScalar(point, scale);
            vtkMath::Add(origin, point, point);
            source->SetPoint1(point);

            // Set the point along height
            vtkMath::Assign(up, point);
            vtkMath::MultiplyScalar(point, scale);
            vtkMath::Add(origin, point, point);
            source->SetPoint2(point);
        }
    }

    double scale;
    QList<vtkSmartPointer<vtkPlaneSource>> sources;
    vtkCamera* camera;
};

ModelViewOptions::ModelViewOptions()
{
    // Color scheme
    sceneColor = vtkColors->GetColor3d("aliceblue");
    sceneColor2 = vtkColors->GetColor3d("white");
    edgeColor = vtkColors->GetColor3d("gainsboro");
    for (auto type : skAllTypes)
        elementColors[type] = vtkColors->GetColor3d("black");
    for (auto type : skBeamTypes)
        elementColors[type] = vtkColors->GetColor3d("gold");
    for (auto type : skPanelTypes)
        elementColors[type] = vtkColors->GetColor3d("lightseagreen");
    for (auto type : skAeroPanelTypes)
        elementColors[type] = vtkColors->GetColor3d("purple");
    for (auto type : skSpringTypes)
        elementColors[type] = vtkColors->GetColor3d("red");

    // Elements
    for (auto type : skAllTypes)
        maskElements[type] = true;

    // Dimensions
    edgeOpacity = 0.5;
    beamLineWidth = 2.0f;
    springLineWidth = 2.0f;
    massScale = 0.005;
    springScale = 0.005;
    pointScale = 0.003;
    beamScale = 0.003;

    // Flags
    showThickness = false;
    showWireframe = false;
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
    mRenderer->SetBackground2(mOptions.sceneColor2.GetData());
    mRenderer->GradientBackgroundOn();

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
    // Check if the model is suitable for drawing
    if (mModel.isEmpty())
        return;

    // Loop through all the elastic surfaces
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
        auto transform = computeTransformation(pData->coords, pData->dihedralAngle, pData->sweepAngle, pData->zAngle);
        auto aeroTransform = computeTransformation(pData->coords, pData->dihedralAngle, 0.0, pData->zAngle);

        // Reflect the transformation about the XOY plane
        auto reflectTransform = reflectTransformation(transform);
        auto reflectAeroTransform = reflectTransformation(aeroTransform);

        // Draw the aero panels
        for (auto type : skAeroPanelTypes)
        {
            auto elements = surface.elements(type);
            auto color = mOptions.elementColors[type];
            drawAeroPanels(aeroTransform, elements, color);
            if (isSymmetry)
                drawAeroPanels(reflectAeroTransform, elements, color);
        }

        // Draw the panels
        for (auto type : skPanelTypes)
        {
            auto elements = surface.elements(type);
            auto color = mOptions.elementColors[type];
            if (mOptions.showThickness)
            {
                drawPanels3D(transform, elements, color);
                if (isSymmetry)
                    drawPanels3D(reflectTransform, elements, color);
            }
            else
            {
                drawPanels2D(transform, elements, color);
                if (isSymmetry)
                    drawPanels2D(reflectTransform, elements, color);
            }
        }

        // Draw the beams
        for (auto type : skBeamTypes)
        {
            auto elements = surface.elements(type);
            auto color = mOptions.elementColors[type];
            if (mOptions.showThickness)
            {
                drawBeams3D(transform, elements, color);
                if (isSymmetry)
                    drawBeams3D(reflectTransform, elements, color);
            }
            else
            {
                drawBeams2D(transform, elements, color);
                if (isSymmetry)
                    drawBeams2D(reflectTransform, elements, color);
            }
        }

        // Draw the masses
        for (auto type : skMassTypes)
        {
            auto elements = surface.elements(type);
            drawMasses(transform, elements);
            if (isSymmetry)
                drawMasses(reflectTransform, elements);
        }
    }

    // Process the special surface
    for (auto type : skSpringTypes)
    {
        auto elements = mModel.specialSurface.elements(type);
        auto color = mOptions.elementColors[type];
        drawSprings(elements, false, color);
        drawSprings(elements, true, color);
    }
}

//! Render beam elements as lines
void ModelView::drawBeams2D(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color)
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
    for (int iElement = 0; iElement != numElements; ++iElement)
    {
        KCL::AbstractElement const* pElement = elements[iElement];
        if (!mOptions.maskElements[pElement->type()])
            continue;

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

//! Draw beams as cylinders
void ModelView::drawBeams3D(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color)
{
    // Constants
    int kResolution = 8;

    // Check if there are any elements to render
    if (elements.empty())
        return;

    // Compute the cyliner radius
    double radius = mOptions.beamScale * getMaximumDimension();

    // Process all the elements
    int numElements = elements.size();
    for (int iElement = 0; iElement != numElements; ++iElement)
    {
        KCL::AbstractElement const* pElement = elements[iElement];
        if (!mOptions.maskElements[pElement->type()])
            continue;

        // Slice element coordinates
        KCL::VecN elementData = pElement->get();
        KCL::Vec2 startCoords = {elementData[0], elementData[1]};
        KCL::Vec2 endCoords = {elementData[2], elementData[3]};

        // Transform the coordinates to global coordinate system
        Vector3d startPosition = transform * Vector3d(startCoords[0], 0.0, startCoords[1]);
        Vector3d endPosition = transform * Vector3d(endCoords[0], 0.0, endCoords[1]);

        // Create the cylinder actor
        auto actor = Utility::createCylinderActor(startPosition, endPosition, radius, kResolution);
        actor->GetProperty()->SetColor(color.GetData());
        if (mOptions.showWireframe)
            actor->GetProperty()->SetRepresentationToWireframe();

        // Add the actor to the scene
        mRenderer->AddActor(actor);
    }
}

//! Render panel elements as planes
void ModelView::drawPanels2D(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color)
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
    for (int iElement = 0; iElement != numElements; ++iElement)
    {
        KCL::AbstractElement const* pElement = elements[iElement];
        if (!mOptions.maskElements[pElement->type()])
            continue;

        // Slice element coordinates
        KCL::VecN elementData = pElement->get();

        // Set points and connections between them
        int iData = 1;
        vtkNew<vtkPolygon> polygon;
        for (int iPosition = 0; iPosition != kNumCellPoints; ++iPosition)
        {
            auto position = transform * Vector3d(elementData[iData], 0.0, elementData[iData + 1]);
            points->InsertNextPoint(position[0], position[1], position[2]);
            polygon->GetPointIds()->InsertNextId(iPoint);
            ++iPoint;
            iData += 2;
        }
        polygons->InsertNextCell(polygon);
    }

    // Create the polygon objects
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
    if (mOptions.showWireframe)
        actor->GetProperty()->SetRepresentationToWireframe();

    // Add the actor to the scene
    mRenderer->AddActor(actor);
}

//! Render panel elements as hexahedrons
void ModelView::drawPanels3D(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color)
{
    int const kNumVertices = 4;

    // Check if there are any elements to render
    if (elements.empty())
        return;

    // Process all the elements
    int numElements = elements.size();
    for (int iElement = 0; iElement != numElements; ++iElement)
    {
        KCL::AbstractElement const* pElement = elements[iElement];
        auto type = pElement->type();
        if (!mOptions.maskElements[type])
            continue;

        // Get element data
        KCL::VecN elementData = pElement->get();

        // Get the plane coordinates
        int iData = 0;
        double thickness = elementData[iData++];
        Matrix42d coords;
        for (int iVertex = 0; iVertex != kNumVertices; ++iVertex)
        {
            coords(iVertex, 0) = elementData[iData];
            coords(iVertex, 1) = elementData[iData + 1];
            iData += 2;
        }

        // Get the depths
        Vector4d depths;
        int numDepths = depths.size();
        for (int iDepth = 0; iDepth != numDepths; ++iDepth)
        {
            depths[iDepth] = elementData[iData];
            ++iData;
        }

        // Evaluate the depth at the last point
        if (type != KCL::P4)
            Utility::setLastDepth(coords, depths);

        // Create the shell actor
        auto actor = Utility::createShellActor(transform, coords, depths, thickness);
        actor->GetProperty()->SetColor(color.GetData());
        actor->GetProperty()->SetEdgeColor(mOptions.edgeColor.GetData());
        actor->GetProperty()->SetEdgeOpacity(mOptions.edgeOpacity);
        actor->GetProperty()->EdgeVisibilityOn();
        if (mOptions.showWireframe)
            actor->GetProperty()->SetRepresentationToWireframe();

        // Add the actor to the scene
        mRenderer->AddActor(actor);
    }
}

//! Render aerodynamic panel elements
void ModelView::drawAeroPanels(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color)
{
    double const kOpacity = 0.5;
    double const kPolyOffset = 0.01;
    double const kPolyUnits = 10;

    // Check if there are any elements to render
    if (elements.empty())
        return;

    // Process all the elements
    int numElements = elements.size();
    for (int i = 0; i != numElements; ++i)
    {
        KCL::AbstractElement const* pElement = elements[i];
        if (!mOptions.maskElements[pElement->type()])
            continue;
        if (pElement->subType() == KCL::ElementSubType::AE1)
            continue;
        bool isVertical = pElement->type() == KCL::ElementType::DA;

        // Slice element parameters
        KCL::VecN data = pElement->get();
        KCL::Vec2 coords0 = {data[0], data[1]};
        KCL::Vec2 coords1 = {data[2], data[3]};
        KCL::Vec2 coords2 = {data[4], data[5]};
        int numStrips = data[6];
        int numPanels = data[7];

        // Combine the vertex coordinates
        Vector2d A = {coords0[0], coords0[1]}; // Bottom left
        Vector2d B = {coords2[0], coords0[1]}; // Bottom right
        Vector2d C = {coords2[1], coords1[1]}; // Top right
        Vector2d D = {coords1[0], coords1[1]}; // Top left

        // Allocate the points and their connectivity list
        vtkNew<vtkPoints> points;
        vtkNew<vtkCellArray> polygons;

        // Create the grid of points
        for (int s = 0; s <= numPanels; ++s)
        {
            double u = (double) s / numPanels;
            for (int r = 0; r <= numStrips; ++r)
            {
                double v = (double) r / numStrips;
                double x = (1.0 - v) * ((1.0 - u) * A[0] + u * B[0]) + v * ((1.0 - u) * D[0] + u * C[0]);
                double z = (1.0 - v) * ((1.0 - u) * A[1] + u * B[1]) + v * ((1.0 - u) * D[1] + u * C[1]);
                auto position = isVertical ? Vector3d(x, z, 0) : Vector3d(x, 0, z);
                position = transform * position;
                points->InsertNextPoint(position[0], position[1], position[2]);
            }
        }

        // Set the polygon data
        for (int s = 0; s != numPanels; ++s)
        {
            for (int r = 0; r != numStrips; ++r)
            {
                vtkNew<vtkPolygon> polygon;
                polygon->GetPointIds()->InsertNextId(s * (numStrips + 1) + r);
                polygon->GetPointIds()->InsertNextId(s * (numStrips + 1) + r + 1);
                polygon->GetPointIds()->InsertNextId((s + 1) * (numStrips + 1) + r + 1);
                polygon->GetPointIds()->InsertNextId((s + 1) * (numStrips + 1) + r);
                polygons->InsertNextCell(polygon);
            }
        }

        // Create the polygon objects
        vtkNew<vtkPolyData> polyData;
        polyData->SetPoints(points);
        polyData->SetPolys(polygons);

        // Build the mapper
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputData(polyData);
        mapper->SetRelativeCoincidentTopologyPolygonOffsetParameters(kPolyOffset, kPolyUnits);
        mapper->SetResolveCoincidentTopologyToPolygonOffset();

        // Create the actor and add to the scene
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(color.GetData());
        actor->GetProperty()->SetOpacity(kOpacity);
        actor->GetProperty()->SetEdgeColor(mOptions.edgeColor.GetData());
        actor->GetProperty()->SetEdgeOpacity(mOptions.edgeOpacity);
        actor->GetProperty()->EdgeVisibilityOn();
        if (mOptions.showWireframe)
            actor->GetProperty()->SetRepresentationToWireframe();

        // Add the actor to the scene
        mRenderer->AddActor(actor);
    }
}

//! Represent point masses
void ModelView::drawMasses(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements)
{
    double const kPolyOffset = -1;
    double const kPolyUnits = -66000;
    vtkColor3d kRodColor = vtkColors->GetColor3d("red");

    // Check if there are any elements to render
    if (elements.empty())
        return;

    // Get the visualization texture
    vtkSmartPointer<vtkTexture> texture = mTextures["mass"];
    double w = mOptions.massScale * getMaximumDimension();

    // Process all the elements
    int numElements = elements.size();
    QList<vtkSmartPointer<vtkPlaneSource>> sources;
    for (int i = 0; i != numElements; ++i)
    {
        KCL::AbstractElement const* pBaseElement = elements[i];
        if (!mOptions.maskElements[pBaseElement->type()])
            continue;

        // Slice element coordinates
        Vector3d startPosition;
        double lengthRod = 0.0;
        double angleRodZ = 0.0;
        switch (pBaseElement->type())
        {
        case KCL::SM:
        {
            auto pElement = (KCL::PointMass1 const*) pBaseElement;
            startPosition = {pElement->coords[0], 0.0, pElement->coords[1]};
            lengthRod = pElement->lengthRod;
            angleRodZ = pElement->angleRodZ;
            break;
        }
        case KCL::M3:
        {
            auto pElement = (KCL::PointMass3 const*) pBaseElement;
            startPosition = {pElement->coords[0], pElement->coords[1], pElement->coords[2]};
            lengthRod = pElement->lengthRod;
            angleRodZ = pElement->angleRodZ;
            break;
        }
        default:
            continue;
        }
        startPosition = transform * startPosition;
        auto endPosition = startPosition;

        // Build up the additional line which connects the mass to the elastic surface
        if (lengthRod > 0.0)
        {
            // Transform the coordiantes to the global coordinate system
            auto addTransform = Transformation::Identity();
            addTransform.translate(Vector3d(0, 0, lengthRod));
            addTransform.rotate(AngleAxisd(qDegreesToRadians(angleRodZ), Vector3d::UnitY()));
            endPosition = addTransform * endPosition;

            // Create the points and connectivity list
            vtkNew<vtkPoints> points;
            points->InsertNextPoint(startPosition[0], startPosition[1], startPosition[2]);
            points->InsertNextPoint(endPosition[0], endPosition[1], endPosition[2]);
            vtkNew<vtkCellArray> indices;
            indices->InsertNextCell(2);
            indices->InsertCellPoint(0);
            indices->InsertCellPoint(1);

            // Set the polygonal data
            vtkNew<vtkPolyData> data;
            data->SetPoints(points);
            data->SetLines(indices);

            // Build the mapper
            vtkNew<vtkPolyDataMapper> mapper;
            mapper->SetInputData(data);

            // Create the actor and add to the scene
            vtkNew<vtkActor> actor;
            actor->SetMapper(mapper);
            actor->GetProperty()->SetColor(kRodColor.GetData());

            // Add the actor to the scene
            mRenderer->AddActor(actor);
        }

        // Create and position the source
        vtkNew<vtkPlaneSource> source;
        double x = endPosition[0];
        double y = endPosition[1];
        double z = endPosition[2];
        source->SetOrigin(x - w, y - w, z);
        source->SetPoint1(x + w, y - w, z);
        source->SetPoint2(x - w, y + w, z);
        source->SetResolution(1, 1);
        sources.push_back(source);

        // Map the resulting polygons
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(source->GetOutputPort());
        mapper->SetRelativeCoincidentTopologyPolygonOffsetParameters(kPolyOffset, kPolyUnits);
        mapper->SetResolveCoincidentTopologyToPolygonOffset();

        // Create the actor
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->SetTexture(texture);

        // Add the actor to the scene
        mRenderer->AddActor(actor);
    }
    if (!sources.empty())
    {
        // Create the plane follower event
        vtkNew<PlaneFollowerCallback> callback;
        callback->scale = 2.0 * w;
        callback->sources = sources;
        callback->camera = mRenderer->GetActiveCamera();

        // Attach the follower event to the interactor
        auto interactor = mRenderWindow->GetInteractor();
        interactor->AddObserver(vtkCommand::EndInteractionEvent, callback);
    }
}

//! Represent springs
void ModelView::drawSprings(std::vector<KCL::AbstractElement const*> const& elements, bool isReflect, vtkColor3d color)
{
    int const kNumTurns = 6;
    int const kResolution = 30;

    // Check if there are any elements to render
    if (elements.empty())
        return;

    // Retrieve the scene parameters
    double maxDimension = getMaximumDimension();

    // Process all the elements
    int numElements = elements.size();
    for (int i = 0; i != numElements; ++i)
    {
        KCL::AbstractElement const* pBaseElement = elements[i];
        if (!mOptions.maskElements[pBaseElement->type()])
            continue;
        if (pBaseElement->type() != KCL::PR)
            continue;
        auto pElement = (KCL::SpringDamper const*) pBaseElement;

        // Process the first elastic surface
        auto firstSurface = mModel.surfaces[pElement->iFirstSurface - 1];
        auto pFirstData = (KCL::GeneralData*) firstSurface.element(KCL::OD);
        auto firstTransform = computeTransformation(pFirstData->coords, pFirstData->dihedralAngle, pFirstData->sweepAngle, pFirstData->zAngle);
        if (pFirstData->iSymmetry != 0 && isReflect)
            continue;
        if (isReflect)
            firstTransform = reflectTransformation(firstTransform);
        Vector3d firstPosition = firstTransform * Vector3d(pElement->coordsFirstRod[0], 0.0, pElement->coordsFirstRod[1]);

        // Process the second elastic surface
        Vector3d secondPosition = {0.0, 0.0, 0.0};
        if (pElement->iSecondSurface > 0)
        {
            auto secondSurface = mModel.surfaces[pElement->iSecondSurface - 1];
            auto pSecondData = (KCL::GeneralData*) secondSurface.element(KCL::OD);
            auto secondTransform = computeTransformation(pSecondData->coords, pSecondData->dihedralAngle, pSecondData->sweepAngle,
                                                         pSecondData->zAngle);
            if (pSecondData->iSymmetry != 0 && isReflect)
                continue;
            if (isReflect)
                secondTransform = reflectTransformation(secondTransform);
            secondPosition = secondTransform * Vector3d(pElement->coordsSecondRod[0], 0.0, pElement->coordsSecondRod[1]);
        }
        else
        {
            KCL::Vec3 addCoords = {0, 0, pElement->lengthFirstRod};
            auto addTransform = computeTransformation(addCoords, pElement->anglesFirstRod[0], pElement->anglesFirstRod[1], 0.0);
            secondPosition = addTransform * firstPosition;
        }

        // Create the helix between two points
        double lengthHelix = (secondPosition - firstPosition).norm();
        double radiusHelix = mOptions.springScale * maxDimension * lengthHelix;
        auto actorHelix = Utility::createHelixActor(firstPosition, secondPosition, radiusHelix, kNumTurns, kResolution);
        actorHelix->GetProperty()->SetColor(color.GetData());
        actorHelix->GetProperty()->SetLineWidth(mOptions.springLineWidth);

        // Set the point renderer
        double radiusPoints = mOptions.pointScale * maxDimension;
        auto actorPoints = Utility::createPointsActor({firstPosition, secondPosition}, radiusPoints);
        actorPoints->GetProperty()->SetColor(color.GetData());

        // Add the actors to the scene
        mRenderer->AddActor(actorPoints);
        mRenderer->AddActor(actorHelix);
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

//! Helper function to build up the transformation for the elastic surface using its local coordinate
Transformation computeTransformation(KCL::Vec3 const& coords, double dihedralAngle, double sweepAngle, double zAngle)
{
    Transformation result = Transformation::Identity();
    result.translate(Vector3d(coords[0], coords[1], coords[2]));
    result.rotate(AngleAxisd(-qDegreesToRadians(dihedralAngle), Vector3d::UnitX()));
    result.rotate(AngleAxisd(qDegreesToRadians(sweepAngle), Vector3d::UnitY()));
    result.rotate(AngleAxisd(qDegreesToRadians(zAngle), Vector3d::UnitZ()));
    return result;
}

//! Helper function to reflect the current transformation about the XOY plane
Transformation reflectTransformation(Transformation const& transform)
{
    Matrix4d matrix = Matrix4d::Identity();
    matrix(2, 2) = -1.0;
    return Transformation(matrix * transform.matrix());
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
