#include <QFile>
#include <QHBoxLayout>

#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCellPicker.h>
#include <vtkDataSetMapper.h>
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
#include "selectionset.h"
#include "uiconstants.h"
#include "uiutility.h"

using namespace Frontend;
using namespace Eigen;
using namespace Backend;
using namespace Constants::Colors;

// Macros
vtkStandardNewMacro(PlaneFollowerCallback);
vtkStandardNewMacro(InteractorStyle);

// Constants
constexpr auto skAllTypes = magic_enum::enum_values<KCL::ElementType>();
auto const skBeamTypes = Utility::beamTypes();
auto const skPanelTypes = Utility::panelTypes();
auto const skAeroPanelTypes = Utility::aeroPanelsTypes();
auto const skMassTypes = Utility::massTypes();
auto const skSpringTypes = Utility::springTypes();

// Helper functions
Transformation computeTransformation(KCL::Vec3 const& coords, double dihedralAngle, double sweepAngle, double zAngle);
Transformation reflectTransformation(Transformation const& transform);
vtkSmartPointer<vtkTexture> readTexture(QString const& pathFile);

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
        elementColors[type] = vtkColors->GetColor3d("chocolate");

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

    // Tolerance
    pickTolerance = 0.005;
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
    mSelector.clear();
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

ModelViewSelector& ModelView::selector()
{
    return mSelector;
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
    mOrientationWidget->SetViewport(0.8, 0.6, 1.0, 1.0);
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
            drawAeroPanels(aeroTransform, iSurface, type);
            if (isSymmetry)
                drawAeroPanels(reflectAeroTransform, iSurface, type);
        }

        // Draw the panels
        for (auto type : skPanelTypes)
        {
            if (mOptions.showThickness)
            {
                drawPanels3D(transform, iSurface, type);
                if (isSymmetry)
                    drawPanels3D(reflectTransform, iSurface, type);
            }
            else
            {
                drawPanels2D(transform, iSurface, type);
                if (isSymmetry)
                    drawPanels2D(reflectTransform, iSurface, type);
            }
        }

        // Draw the beams
        for (auto type : skBeamTypes)
        {
            if (mOptions.showThickness)
            {
                drawBeams3D(transform, iSurface, type);
                if (isSymmetry)
                    drawBeams3D(reflectTransform, iSurface, type);
            }
            else
            {
                drawBeams2D(transform, iSurface, type);
                if (isSymmetry)
                    drawBeams2D(reflectTransform, iSurface, type);
            }
        }

        // Draw the masses
        for (auto type : skMassTypes)
        {
            drawMasses(transform, iSurface, type);
            if (isSymmetry)
                drawMasses(reflectTransform, iSurface, type);
        }
    }

    // Process the special surface
    for (auto type : skSpringTypes)
    {
        drawSprings(false, type);
        drawSprings(true, type);
    }

    // Set the custom stype to use for interaction
    auto interactor = mRenderWindow->GetInteractor();
    vtkNew<InteractorStyle> style;
    style->SetDefaultRenderer(mRenderer);
    style->selector = &mSelector;
    style->pickTolerance = mOptions.pickTolerance;
    interactor->SetInteractorStyle(style);
}

//! Render beam elements as lines
void ModelView::drawBeams2D(Transformation const& transform, int iSurface, KCL::ElementType type)
{
    int const kNumCellPoints = 2;

    // Slice the elements for rendering
    std::vector<KCL::AbstractElement const*> elements = mModel.surfaces[iSurface].elements(type);
    if (elements.empty() || !mOptions.maskElements[type])
        return;
    vtkColor3d color = mOptions.elementColors[type];

    // Process all the elements
    int numElements = elements.size();
    for (int iElement = 0; iElement != numElements; ++iElement)
    {
        KCL::AbstractElement const* pElement = elements[iElement];

        // Slice element coordinates
        KCL::VecN elementData = pElement->get();
        KCL::Vec2 startCoords = {elementData[0], elementData[1]};
        KCL::Vec2 endCoords = {elementData[2], elementData[3]};

        // Transform the coordinates to global coordinate system
        auto startPosition = transform * Vector3d(startCoords[0], 0.0, startCoords[1]);
        auto endPosition = transform * Vector3d(endCoords[0], 0.0, endCoords[1]);

        // Set the points
        vtkNew<vtkPoints> points;
        points->InsertNextPoint(startPosition[0], startPosition[1], startPosition[2]);
        points->InsertNextPoint(endPosition[0], endPosition[1], endPosition[2]);

        // Set the connectivity indices
        vtkNew<vtkCellArray> indices;
        indices->InsertNextCell(kNumCellPoints);
        indices->InsertCellPoint(0);
        indices->InsertCellPoint(1);

        // Create the polygons
        vtkNew<vtkPolyData> polyData;
        polyData->SetPoints(points);
        polyData->SetLines(indices);

        // Build the mapper
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputData(polyData);

        // Create the actor and add to the scene
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(color.GetData());
        actor->GetProperty()->SetLineWidth(mOptions.beamLineWidth);

        // Register the actor
        mSelector.registerActor(Core::Selection(iSurface, type, iElement), actor);

        // Add the actor to the scene
        mRenderer->AddActor(actor);
    }
}

//! Draw beams as cylinders
void ModelView::drawBeams3D(Transformation const& transform, int iSurface, KCL::ElementType type)
{
    // Constants
    int kResolution = 8;

    // Slice the elements for rendering
    std::vector<KCL::AbstractElement const*> elements = mModel.surfaces[iSurface].elements(type);
    if (elements.empty() || !mOptions.maskElements[type])
        return;
    vtkColor3d color = mOptions.elementColors[type];

    // Compute the cyliner radius
    double radius = mOptions.beamScale * getMaximumDimension();

    // Process all the elements
    int numElements = elements.size();
    for (int iElement = 0; iElement != numElements; ++iElement)
    {
        KCL::AbstractElement const* pElement = elements[iElement];

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

        // Register the actor
        mSelector.registerActor(Core::Selection(iSurface, type, iElement), actor);

        // Add the actor to the scene
        mRenderer->AddActor(actor);
    }
}

//! Render panel elements as planes
void ModelView::drawPanels2D(Transformation const& transform, int iSurface, KCL::ElementType type)
{
    int const kNumCellPoints = 4;

    // Slice the elements for rendering
    std::vector<KCL::AbstractElement const*> elements = mModel.surfaces[iSurface].elements(type);
    if (elements.empty() || !mOptions.maskElements[type])
        return;
    vtkColor3d color = mOptions.elementColors[type];

    // Process all the elements
    int numElements = elements.size();
    for (int iElement = 0; iElement != numElements; ++iElement)
    {
        KCL::AbstractElement const* pElement = elements[iElement];

        // Slice element coordinates
        KCL::VecN elementData = pElement->get();

        // Set points and connections between them
        int iData = 1;
        vtkNew<vtkPoints> points;
        vtkNew<vtkPolygon> polygon;
        vtkNew<vtkCellArray> polygons;
        for (int iPosition = 0; iPosition != kNumCellPoints; ++iPosition)
        {
            auto position = transform * Vector3d(elementData[iData], 0.0, elementData[iData + 1]);
            points->InsertNextPoint(position[0], position[1], position[2]);
            polygon->GetPointIds()->InsertNextId(iPosition);
            iData += 2;
        }
        polygons->InsertNextCell(polygon);

        // Create the polygon objects
        vtkNew<vtkPolyData> polyData;
        polyData->SetPoints(points);
        polyData->SetPolys(polygons);

        // Build the mapper
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputData(polyData);

        // Create the actor and add to the scene
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(color.GetData());
        actor->GetProperty()->SetEdgeColor(mOptions.edgeColor.GetData());
        actor->GetProperty()->SetEdgeOpacity(mOptions.edgeOpacity);
        actor->GetProperty()->EdgeVisibilityOn();
        if (mOptions.showWireframe)
            actor->GetProperty()->SetRepresentationToWireframe();

        // Register the actor
        mSelector.registerActor(Core::Selection(iSurface, type, iElement), actor);

        // Add the actor to the scene
        mRenderer->AddActor(actor);
    }
}

//! Render panel elements as hexahedrons
void ModelView::drawPanels3D(Transformation const& transform, int iSurface, KCL::ElementType type)
{
    int const kNumVertices = 4;

    // Slice the elements for rendering
    std::vector<KCL::AbstractElement const*> elements = mModel.surfaces[iSurface].elements(type);
    if (elements.empty() || !mOptions.maskElements[type])
        return;
    vtkColor3d color = mOptions.elementColors[type];

    // Process all the elements
    int numElements = elements.size();
    for (int iElement = 0; iElement != numElements; ++iElement)
    {
        KCL::AbstractElement const* pElement = elements[iElement];

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

        // Register the actor
        mSelector.registerActor(Core::Selection(iSurface, type, iElement), actor);

        // Add the actor to the scene
        mRenderer->AddActor(actor);
    }
}

//! Render aerodynamic panel elements
void ModelView::drawAeroPanels(Transformation const& transform, int iSurface, KCL::ElementType type)
{
    double const kOpacity = 0.5;
    double const kPolyOffset = 0.01;
    double const kPolyUnits = 10;

    // Slice the elements for rendering
    std::vector<KCL::AbstractElement const*> elements = mModel.surfaces[iSurface].elements(type);
    if (elements.empty() || !mOptions.maskElements[type])
        return;
    vtkColor3d color = mOptions.elementColors[type];

    // Process all the elements
    int numElements = elements.size();
    bool isVertical = type == KCL::ElementType::DA;
    for (int iElement = 0; iElement != numElements; ++iElement)
    {
        KCL::AbstractElement const* pElement = elements[iElement];
        if (pElement->subType() == KCL::ElementSubType::AE1)
            continue;

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

        // Register the actor
        mSelector.registerActor(Core::Selection(iSurface, type, iElement), actor);

        // Add the actor to the scene
        mRenderer->AddActor(actor);
    }
}

//! Represent point masses
void ModelView::drawMasses(Transformation const& transform, int iSurface, KCL::ElementType type)
{
    double const kPolyOffset = -1;
    double const kPolyUnits = -66000;
    vtkColor3d kRodColor = vtkColors->GetColor3d("red");

    // Slice the elements for rendering
    std::vector<KCL::AbstractElement const*> elements = mModel.surfaces[iSurface].elements(type);
    if (elements.empty() || !mOptions.maskElements[type])
        return;
    vtkColor3d color = mOptions.elementColors[type];

    // Get the visualization texture
    vtkSmartPointer<vtkTexture> texture = mTextures["mass"];
    double w = mOptions.massScale * getMaximumDimension();

    // Process all the elements
    int numElements = elements.size();
    QList<vtkPlaneSource*> sources;
    for (int iElement = 0; iElement != numElements; ++iElement)
    {
        KCL::AbstractElement const* pBaseElement = elements[iElement];

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

        // Register the actor
        mSelector.registerActor(Core::Selection(iSurface, type, iElement), actor);

        // Add the actor to the scene
        mRenderer->AddActor(actor);
    }

    // Set the callback functions
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
void ModelView::drawSprings(bool isReflect, KCL::ElementType type)
{
    int const kNumTurns = 6;
    int const kResolution = 30;

    // Slice the elements for rendering
    std::vector<KCL::AbstractElement const*> elements = mModel.specialSurface.elements(type);
    if (elements.empty() || !mOptions.maskElements[type])
        return;
    vtkColor3d color = mOptions.elementColors[type];

    // Retrieve the scene parameters
    double maxDimension = getMaximumDimension();

    // Process all the elements
    int numElements = elements.size();
    for (int iElement = 0; iElement != numElements; ++iElement)
    {
        KCL::AbstractElement const* pBaseElement = elements[iElement];
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

        // Register the actor
        mSelector.registerActor(Core::Selection(type, iElement), actorHelix);

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

ModelViewSelector::ModelViewSelector(State aState)
    : state(aState)
{
}

int ModelViewSelector::numSelected() const
{
    return mSelection.count();
}

bool ModelViewSelector::isEmpty() const
{
    return numSelected() == 0;
}

bool ModelViewSelector::isSelected(vtkActor* actor) const
{
    return mSelection.contains(actor);
}

//! Add the actor to the selection set
void ModelViewSelector::select(vtkActor* actor)
{
    // Check if the selection is enabled
    if (state.testFlag(kNone) || !actor)
        return;

    // Deselect the actor on the second click
    if (isSelected(actor))
    {
        deselect(actor);
        return;
    }

    // Deselect all actors for the single selection mode
    if (state.testFlag(kSingleSelection))
        deselectAll();

    // Change the visual representation of the actor
    vtkNew<vtkProperty> property;
    property->DeepCopy(actor->GetProperty());
    actor->GetProperty()->SetColor(vtkColors->GetColor3d("Red").GetData());
    actor->GetProperty()->SetDiffuse(1.0);
    actor->GetProperty()->SetSpecular(0.0);
    actor->GetProperty()->EdgeVisibilityOn();

    // Save the original property
    mSelection[actor] = property;

    // Display the information
    if (state.testFlag(kVerbose))
    {
        Core::Selection selection = find(actor);
        QString typeName = magic_enum::enum_name(selection.type).data();
        qInfo() << QObject::tr("Element %1:%2 on ES:%3 was selected").arg(typeName).arg(1 + selection.iElement).arg(1 + selection.iSurface);
    }
}

//! Remove the actor from the selection set
void ModelViewSelector::deselect(vtkActor* actor)
{
    // Check if there is such actor on the scene
    if (!mSelection.contains(actor))
        return;

    // Set the original properties
    actor->GetProperty()->DeepCopy(mSelection[actor]);

    // Display the information
    if (state.testFlag(kVerbose))
    {
        Core::Selection selection = find(actor);
        QString typeName = magic_enum::enum_name(selection.type).data();
        qInfo() << QObject::tr("Element %1:%2 on ES:%3 was deselected").arg(typeName).arg(1 + selection.iElement).arg(1 + selection.iSurface);
    }

    // Remove the actor from the selection
    mSelection.remove(actor);
}

//! Select all the actors associated with a model entity
void ModelViewSelector::select(Backend::Core::Selection key)
{
    if (mActors.contains(key))
        return;
    QList<vtkActor*> values = mActors[key];
    int numValues = values.size();
    for (int i = 0; i != numValues; ++i)
        select(values[i]);
}

//! Deselect all the actors associated with a model entity
void ModelViewSelector::deselect(Backend::Core::Selection key)
{
    if (mActors.contains(key))
        return;
    QList<vtkActor*> values = mActors[key];
    int numValues = values.size();
    for (int i = 0; i != numValues; ++i)
        deselect(values[i]);
}

//! Remove all the actors from the selection set
void ModelViewSelector::deselectAll()
{
    if (isEmpty())
        return;
    QList<Core::Selection> const keys = mActors.keys();
    int numKeys = keys.size();
    for (int iKey = 0; iKey != numKeys; ++iKey)
    {
        QList<vtkActor*> values = mActors[keys[iKey]];
        int numValues = values.size();
        for (int iValue = 0; iValue != numValues; ++iValue)
            deselect(values[iValue]);
    }
}

//! Construct the reference from the model selection to the actor on the scene
void ModelViewSelector::registerActor(Backend::Core::Selection const& key, vtkActor* value)
{
    mActors[key].push_back(value);
}

//! Find a selection by actor
Core::Selection ModelViewSelector::find(vtkActor* actor) const
{
    Core::Selection result;
    for (auto const [key, values] : mActors.asKeyValueRange())
    {
        int iFound = values.indexOf(actor);
        if (iFound >= 0)
            result = key;
    }
    return result;
}

//! Drop all the actor pointers
void ModelViewSelector::clear()
{
    mActors.clear();
}

void PlaneFollowerCallback::Execute(vtkObject* caller, unsigned long evId, void*)
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

InteractorStyle::InteractorStyle()
    : selector(nullptr)
{
}

void InteractorStyle::OnLeftButtonDown()
{
    // Get the window interactor
    vtkRenderWindowInteractor* interactor = GetInteractor();

    // Get the location of the click (in window coordinates)
    int* position = interactor->GetEventPosition();

    // Construct the picker
    vtkNew<vtkCellPicker> picker;
    picker->SetTolerance(pickTolerance);

    // Pick from this location
    picker->Pick(position[0], position[1], 0, GetDefaultRenderer());

    // Highlight the last actor
    vtkActorCollection* actors = picker->GetActors();
    actors->InitTraversal();
    if (actors->GetNumberOfItems() > 0)
        selector->select(actors->GetLastActor());
    else
        selector->deselectAll();

    // Forward events
    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

void InteractorStyle::OnKeyPress()
{
    // Get the pressed key
    std::string key = GetInteractor()->GetKeySym();

    // Process the press
    if (key == "Escape")
        selector->deselectAll();

    // Update the selector state
    updateSelectorState();

    // Forward events
    vtkInteractorStyleTrackballCamera::OnKeyPress();
}

void InteractorStyle::OnKeyUp()
{
    updateSelectorState();
    vtkInteractorStyleTrackballCamera::OnKeyUp();
}

void InteractorStyle::updateSelectorState()
{
    if (!selector->state.testFlag(ModelViewSelector::kNone))
    {
        if (GetInteractor()->GetControlKey())
            selector->state = ModelViewSelector::State(ModelViewSelector::kMultipleSelection);
        else
            selector->state = ModelViewSelector::State(ModelViewSelector::kSingleSelection);
    }
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
