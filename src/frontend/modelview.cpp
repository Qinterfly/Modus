#include <QColorDialog>
#include <QDialog>
#include <QFile>
#include <QListWidget>
#include <QMenu>
#include <QToolBar>
#include <QVBoxLayout>

#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCameraOrientationWidget.h>
#include <vtkCellPicker.h>
#include <vtkDataSetMapper.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkGeometryFilter.h>
#include <vtkObjectFactory.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPNGReader.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataSilhouette.h>
#include <vtkPolygon.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTexture.h>
#include <vtkTransform.h>
#include <vtkUnstructuredGrid.h>
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
auto const skAeroTrapeziumTypes = Utility::aeroTrapeziumTypes();
auto const skMassTypes = Utility::massTypes();
auto const skSpringTypes = Utility::springTypes();

// Helper functions
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
    for (auto type : skAeroTrapeziumTypes)
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
    axesScale = 0.03;

    // Flags
    showThickness = false;
    showSymmetry = true;
    showWireframe = false;
    showLocalAxes = true;

    // Tolerance
    pickTolerance = 0.005;
}

ModelView::ModelView(KCL::Model const& model, ModelViewOptions const& options)
    : mModel(model)
    , mOptions(options)
{
    createContent();
    initialize();
    createConnections();
}

ModelView::~ModelView()
{
    ModelView::clear();
}

//! Clear all the items from the scene
void ModelView::clear()
{
    auto interactor = mRenderWindow->GetInteractor();

    // Remove the obsertvers
    int numObservers = mObserverTags.size();
    for (int i = 0; i != numObservers; ++i)
        interactor->RemoveObserver(mObserverTags[i]);
    mObserverTags.clear();

    // Clean up the style
    mSelector.clear();
    mStyle->clear();

    // Remove the actors
    auto actors = mRenderer->GetActors();
    while (actors->GetLastActor())
        mRenderer->RemoveActor(actors->GetLastActor());

    // Remove the view properties
    auto props = mRenderer->GetViewProps();
    while (props->GetLastProp())
        mRenderer->RemoveViewProp(props->GetLastProp());
}

//! Draw the scene
void ModelView::plot()
{
    // Add the model to the scene to compute visible boundaries, if neceassary
    bool isEmptyScene = mRenderer->GetActors()->GetNumberOfItems() == 0;
    if (isEmptyScene)
        drawModel();

    // Compute the model maximum dimension
    mMaximumDimension = Utility::getMaximumDimension(mRenderer);
    if (mMaximumDimension < std::numeric_limits<double>::epsilon())
        mMaximumDimension = 1.0;

    // Clear the scene
    clear();

    // Add the model to the scene
    drawModel();

    // Render the model
    mRenderWindow->Render();
}

//! Update the scene
void ModelView::refresh()
{
    mRenderWindow->Render();
}

//! Get the view type
IView::Type ModelView::type() const
{
    return IView::kModel;
}

//! Retrieve the model instance
KCL::Model const& ModelView::model()
{
    return mModel;
}

//! Get the view options
ModelViewOptions& ModelView::options()
{
    return mOptions;
}

//! Retrieve the selector instance
ModelViewSelector& ModelView::selector()
{
    return mSelector;
}

//! Set the initial state of widgets
void ModelView::initialize()
{
    int const kNumAnimationFrames = 15;

    // Load the textures
    loadTextures();

    // Set up the scence
    mRenderer = vtkRenderer::New();
    mRenderer->SetBackground(mOptions.sceneColor.GetData());
    mRenderer->SetBackground2(mOptions.sceneColor2.GetData());
    mRenderer->GradientBackgroundOn();
    mRenderer->ResetCamera();

    // Create the window
    mRenderWindow = vtkGenericOpenGLRenderWindow::New();
    mRenderWindow->AddRenderer(mRenderer);
    mRenderWidget->setRenderWindow(mRenderWindow);

    // Create the orientation widget
    mOrientationWidget = vtkCameraOrientationWidget::New();
    mOrientationWidget->SetParentRenderer(mRenderer);
    mOrientationWidget->On();
    mOrientationWidget->SetAnimatorTotalFrames(kNumAnimationFrames);

    // Set the custom style to use for interaction
    auto interactor = mRenderWindow->GetInteractor();
    mStyle = InteractorStyle::New();
    mStyle->SetDefaultRenderer(mRenderer);
    mStyle->selector = &mSelector;
    mStyle->pickTolerance = mOptions.pickTolerance;
    mStyle->handler = new InteractorHandler(this);
    interactor->SetInteractorStyle(mStyle);

    // Set the maximum dimension
    mMaximumDimension = 0.0;
}

//! Load the textures from the resource file
void ModelView::loadTextures()
{
    mTextures["mass"] = readTexture(":/textures/mass.png");
}

//! Create all the widgets and corresponding actions
void ModelView::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;

    // Create the VTK widget
    mRenderWidget = new QVTKOpenGLNativeWidget;

    // Create auxiliary function
    auto createShowAction = [this](QIcon const& icon, QString const& name, bool& option)
    {
        QAction* pAction = new QAction(icon, name, this);
        pAction->setCheckable(true);
        pAction->setChecked(option);
        connect(pAction, &QAction::triggered, this,
                [this, &option](bool flag)
                {
                    option = flag;
                    plot();
                });
        return pAction;
    };

    // Create the actions
    QAction* pThicknessAction = createShowAction(QIcon(":/icons/draw-thickness.png"), tr("Show element thickness"), mOptions.showThickness);
    QAction* pSymmetryAction = createShowAction(QIcon(":/icons/draw-symmetry.png"), tr("Show symmetrical part of the model"),
                                                mOptions.showSymmetry);
    QAction* pWireframeAction = createShowAction(QIcon(":/icons/draw-wireframe.svg"), tr("Show wireframe"), mOptions.showWireframe);
    QAction* pAxesAction = createShowAction(QIcon(":/icons/draw-axes.svg"), tr("Show local axes"), mOptions.showLocalAxes);
    QAction* pViewEditorAction = new QAction(QIcon(":/icons/edit-view.png"), tr("Edit view options"), this);

    // Create the connections
    connect(pViewEditorAction, &QAction::triggered, this, &ModelView::showViewEditor);

    // Create the toolbar
    QToolBar* pToolBar = new QToolBar;
    pToolBar->addAction(pThicknessAction);
    pToolBar->addAction(pSymmetryAction);
    pToolBar->addAction(pWireframeAction);
    pToolBar->addAction(pAxesAction);
    pToolBar->addAction(pViewEditorAction);
    Utility::setShortcutHints(pToolBar);

    // Combine the widgets
    pLayout->addWidget(pToolBar);
    pLayout->addWidget(mRenderWidget);
    setLayout(pLayout);
}

//! Set the signals & slots
void ModelView::createConnections()
{
    connect(mStyle->handler, &InteractorHandler::selectItemsRequested, this, &ModelView::selectItemsRequested);
    connect(mStyle->handler, &InteractorHandler::editItemsRequested, this, &ModelView::editItemsRequested);
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
        auto transform = Utility::computeTransformation(pData->coords, pData->dihedralAngle, pData->sweepAngle, pData->zAngle);
        auto aeroTransform = Utility::computeTransformation(pData->coords, pData->dihedralAngle, 0.0, pData->zAngle);

        // Reflect the transformation about the XOY plane
        auto reflectTransform = Utility::reflectTransformation(transform);
        auto reflectAeroTransform = Utility::reflectTransformation(aeroTransform);

        // Draw the aero trapeziums
        for (auto type : skAeroTrapeziumTypes)
        {
            drawAeroTrapeziums(aeroTransform, iSurface, type);
            if (isSymmetry && mOptions.showSymmetry)
                drawAeroTrapeziums(reflectAeroTransform, iSurface, type);
        }

        // Draw the panels
        for (auto type : skPanelTypes)
        {
            if (mOptions.showThickness)
            {
                drawPanels3D(transform, iSurface, type);
                if (isSymmetry && mOptions.showSymmetry)
                    drawPanels3D(reflectTransform, iSurface, type);
            }
            else
            {
                drawPanels2D(transform, iSurface, type);
                if (isSymmetry && mOptions.showSymmetry)
                    drawPanels2D(reflectTransform, iSurface, type);
            }
        }

        // Draw the beams
        for (auto type : skBeamTypes)
        {
            if (mOptions.showThickness)
            {
                drawBeams3D(transform, iSurface, type);
                if (isSymmetry && mOptions.showSymmetry)
                    drawBeams3D(reflectTransform, iSurface, type);
            }
            else
            {
                drawBeams2D(transform, iSurface, type);
                if (isSymmetry && mOptions.showSymmetry)
                    drawBeams2D(reflectTransform, iSurface, type);
            }
        }

        // Draw the masses
        for (auto type : skMassTypes)
        {
            drawMasses(transform, iSurface, type);
            if (isSymmetry && mOptions.showSymmetry)
                drawMasses(reflectTransform, iSurface, type);
        }

        // Draw the local coordinate axes
        if (mOptions.showLocalAxes)
        {
            drawLocalAxes(transform);
            if (isSymmetry && mOptions.showSymmetry)
                drawLocalAxes(reflectTransform);
        }
    }

    // Process the special surface
    for (auto type : skSpringTypes)
    {
        drawSprings(false, type);
        if (mOptions.showSymmetry)
            drawSprings(true, type);
    }
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
    double radius = mOptions.beamScale * mMaximumDimension;

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

//! Render aerodynamic trapezium elements
void ModelView::drawAeroTrapeziums(Transformation const& transform, int iSurface, KCL::ElementType type)
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
    bool isVertical = Utility::isAeroVertical(type);
    bool isAileron = Utility::isAeroAileron(type);
    bool isMeshable = Utility::isAeroMeshable(type);
    for (int iElement = 0; iElement != numElements; ++iElement)
    {
        KCL::AbstractElement const* pElement = elements[iElement];
        if (pElement->subType() == KCL::ElementSubType::AE1)
            continue;

        // Slice element parameters
        KCL::VecN data = pElement->get();
        int iShift = isAileron ? 1 : 0;
        KCL::Vec2 coords0 = {data[iShift + 0], data[iShift + 1]};
        KCL::Vec2 coords1 = {data[iShift + 2], data[iShift + 3]};
        KCL::Vec2 coords2 = {data[iShift + 4], data[iShift + 5]};
        int numStrips = 1;
        int numPanels = 1;
        if (isMeshable)
        {
            numStrips = data[iShift + 6];
            numPanels = data[iShift + 7];
        }

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
    double w = mOptions.massScale * mMaximumDimension;

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

        // Build up the additional line which connects the mass to the elastic surface
        Vector3d endPosition;
        if (lengthRod > 0.0)
        {
            // Transform the coordiantes to the global coordinate system
            auto addTransform = Transformation::Identity();
            addTransform.rotate(AngleAxisd(qDegreesToRadians(angleRodZ), Vector3d::UnitY()));
            addTransform.translate(Vector3d(0, 0, lengthRod));
            endPosition = transform * addTransform * startPosition;
            startPosition = transform * startPosition;

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
        else
        {
            startPosition = transform * startPosition;
            endPosition = startPosition;
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
        unsigned long tag = interactor->AddObserver(vtkCommand::EndInteractionEvent, callback);
        mObserverTags.push_back(tag);
    }
}

//! Represent springs
void ModelView::drawSprings(bool isReflect, KCL::ElementType type)
{
    int const kNumTurns = 6;
    int const kResolution = 30;
    KCL::Vec3 kZeroVec = {0.0, 0.0, 0.0};

    // Slice the elements for rendering
    std::vector<KCL::AbstractElement const*> elements = mModel.specialSurface.elements(type);
    if (elements.empty() || !mOptions.maskElements[type])
        return;
    vtkColor3d color = mOptions.elementColors[type];

    // Retrieve the scene parameters
    double maxDimension = mMaximumDimension;

    // Process all the elements
    int numElements = elements.size();
    int numSurfaces = mModel.surfaces.size();
    for (int iElement = 0; iElement != numElements; ++iElement)
    {
        KCL::AbstractElement const* pBaseElement = elements[iElement];
        if (!mOptions.maskElements[pBaseElement->type()])
            continue;
        if (pBaseElement->type() != KCL::PR)
            continue;
        auto pElement = (KCL::SpringDamper const*) pBaseElement;

        // Process the first elastic surface
        int iFirstSurface = pElement->iFirstSurface - 1;
        if (iFirstSurface < 0 || iFirstSurface >= numSurfaces)
            continue;
        auto firstSurface = mModel.surfaces[iFirstSurface];
        auto pFirstData = (KCL::GeneralData*) firstSurface.element(KCL::OD);
        auto firstTransform = Utility::computeTransformation(pFirstData->coords, pFirstData->dihedralAngle, pFirstData->sweepAngle,
                                                             pFirstData->zAngle);
        auto addFirstTransform = Utility::computeTransformation(kZeroVec, 0.0, pElement->anglesFirstRod[0], pElement->anglesFirstRod[1]);
        if (pFirstData->iSymmetry != 0 && isReflect)
            continue;
        if (isReflect)
        {
            firstTransform = Utility::reflectTransformation(firstTransform);
            addFirstTransform = Utility::reflectTransformation(addFirstTransform);
        }
        Vector3d firstPosition = firstTransform * Vector3d(pElement->coordsFirstRod[0], 0.0, pElement->coordsFirstRod[1]);

        // Process the second elastic surface
        Vector3d secondPosition = {0.0, 0.0, 0.0};
        if (pElement->iSecondSurface > 0)
        {
            int iSecondSurface = pElement->iSecondSurface - 1;
            if (iSecondSurface < 0 || iSecondSurface >= numSurfaces)
                continue;
            auto secondSurface = mModel.surfaces[iSecondSurface];
            auto pSecondData = (KCL::GeneralData*) secondSurface.element(KCL::OD);
            auto secondTransform = Utility::computeTransformation(pSecondData->coords, pSecondData->dihedralAngle, pSecondData->sweepAngle,
                                                                  pSecondData->zAngle);
            if (pSecondData->iSymmetry != 0 && isReflect)
                continue;
            if (isReflect)
                secondTransform = Utility::reflectTransformation(secondTransform);
            secondPosition = secondTransform * Vector3d(pElement->coordsSecondRod[0], 0.0, pElement->coordsSecondRod[1]);
        }
        else
        {
            auto addFirstPosition = addFirstTransform * Vector3d({0.0, 0.0, pElement->lengthFirstRod});
            secondPosition = firstPosition + addFirstPosition;
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
        mSelector.registerActor(Core::Selection(type, iElement), actorPoints);

        // Add the actors to the scene
        mRenderer->AddActor(actorPoints);
        mRenderer->AddActor(actorHelix);
    }
}

//! Display local coordinate axes
void ModelView::drawLocalAxes(Transformation const& transform)
{
    // Constants
    double kXAxisColor[3] = {0.870, 0.254, 0.188};
    double kYAxisColor[3] = {0.952, 0.752, 0.090};
    double kZAxisColor[3] = {0.654, 0.823, 0.549};

    // Compute the axes length
    double length = mOptions.axesScale * mMaximumDimension;

    // Copy the transformation matrix
    vtkNew<vtkMatrix4x4> matrixTransform;
    int numRows = transform.matrix().rows();
    int numCols = transform.matrix().cols();
    for (int i = 0; i != numRows; ++i)
    {
        for (int j = 0; j != numCols; ++j)
            matrixTransform->SetElement(i, j, transform.matrix()(i, j));
    }

    // Create the transformation
    vtkNew<vtkTransform> axesTransform;
    axesTransform->SetMatrix(matrixTransform);

    // Create the actor
    vtkNew<vtkAxesActor> axesActor;
    axesActor->SetUserTransform(axesTransform);
    axesActor->SetTotalLength(length, length, length);
    axesActor->AxisLabelsOff();
    axesActor->UseBoundsOff();
    axesActor->GetXAxisShaftProperty()->SetColor(kXAxisColor);
    axesActor->GetYAxisShaftProperty()->SetColor(kYAxisColor);
    axesActor->GetZAxisShaftProperty()->SetColor(kZAxisColor);
    axesActor->GetXAxisTipProperty()->SetColor(kXAxisColor);
    axesActor->GetYAxisTipProperty()->SetColor(kYAxisColor);
    axesActor->GetZAxisTipProperty()->SetColor(kZAxisColor);

    // Add the actor to the scene
    mRenderer->AddActor(axesActor);
}

//! Set the isometric view
void ModelView::setIsometricView()
{
    vtkSmartPointer<vtkCamera> camera = mRenderer->GetActiveCamera();
    camera->SetPosition(-1, 1, 1);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(0, 1, 0);
    mRenderer->ResetCamera();
    camera->Zoom(1.5);
    mRenderWindow->Render();
}

ModelViewSelector::ModelViewSelector()
    : mIsVerbose(false)
{
}

//! Get number of selected model entities
int ModelViewSelector::numSelected() const
{
    return mSelection.count();
}

//! Retrieve selected model entities
QList<Core::Selection> ModelViewSelector::selected() const
{
    QList<Core::Selection> result;
    QList<vtkActor*> actors = mSelection.keys();
    int numActors = actors.size();
    for (int i = 0; i != numActors; ++i)
    {
        Core::Selection selection = find(actors[i]);
        if (selection.isValid())
            result.push_back(selection);
    }
    return result;
}

//! Set the verbosity mode
void ModelViewSelector::setVerbose(bool value)
{
    mIsVerbose = value;
}

//! Check if the verbosity is activated
bool ModelViewSelector::isVerbose() const
{
    return mIsVerbose;
}

//! Check if there are any elements selected
bool ModelViewSelector::isEmpty() const
{
    return numSelected() == 0;
}

//! Check if the actor has been selected
bool ModelViewSelector::isSelected(vtkActor* actor) const
{
    return mSelection.contains(actor);
}

//! Select all the actors on the scene
void ModelViewSelector::selectAll()
{
    QList<Core::Selection> const keys = mActors.keys();
    int numKeys = keys.size();
    for (int iKey = 0; iKey != numKeys; ++iKey)
    {
        QList<vtkActor*> values = mActors[keys[iKey]];
        int numValues = values.size();
        for (int iValue = 0; iValue != numValues; ++iValue)
            select(values[iValue], kMultipleSelection);
    }
}

//! Add the actor to the selection set
void ModelViewSelector::select(vtkActor* actor, Flags flags)
{
    // Check if the selection is enabled
    if (flags.testFlag(kNone) || !actor)
        return;

    // Deselect the actor on the second click
    if (isSelected(actor))
    {
        deselect(actor);
        return;
    }

    // Deselect all actors for the single selection mode
    if (flags.testFlag(kSingleSelection))
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
    if (mIsVerbose)
    {
        Core::Selection selection = find(actor);
        qInfo() << QObject::tr("Element %1 was selected").arg(Utility::getLabel(selection));
    }
}

//! Select all the actors associated with a model entity
void ModelViewSelector::select(Core::Selection key, Flags flags)
{
    // Check if there are any actors to select
    if (!mActors.contains(key))
        return;

    // Deselect all actors for the single selection mode
    if (flags.testFlag(kSingleSelection))
        deselectAll();

    // Select the actors associated with the selection
    QList<vtkActor*> values = mActors[key];
    int numValues = values.size();
    for (int i = 0; i != numValues; ++i)
        select(values[i], ModelViewSelector::kMultipleSelection);
}

//! Select all the actors associated with a set of model entities
void ModelViewSelector::select(QList<Core::Selection> const& keys)
{
    int numKeys = keys.size();
    for (int i = 0; i != numKeys; ++i)
        select(keys[i], ModelViewSelector::kMultipleSelection);
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
    if (mIsVerbose)
    {
        Core::Selection selection = find(actor);
        qInfo() << QObject::tr("Element %1 was deselected").arg(Utility::getLabel(selection));
    }

    // Remove the actor from the selection
    mSelection.remove(actor);
}

//! Deselect all the actors associated with a model entity
void ModelViewSelector::deselect(Core::Selection key)
{
    if (!mActors.contains(key))
        return;
    QList<vtkActor*> values = mActors[key];
    int numValues = values.size();
    for (int i = 0; i != numValues; ++i)
        deselect(values[i]);
}

//! Remove all the actors from the selection set
void ModelViewSelector::deselectAll()
{
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
void ModelViewSelector::registerActor(Core::Selection const& key, vtkActor* value)
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

//! Find a list of actors by selection
QList<vtkActor*> ModelViewSelector::find(Core::Selection selection)
{
    if (mActors.contains(selection))
        return mActors[selection];
    else
        return QList<vtkActor*>();
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

InteractorHandler::InteractorHandler(QObject* parent)
    : QObject(parent)
{
}

InteractorStyle::InteractorStyle()
    : selector(nullptr)
{
}

//! Process left button click
void InteractorStyle::OnLeftButtonDown()
{
    // Clear the created items
    clear();

    // Get the window interactor
    vtkRenderWindowInteractor* interactor = GetInteractor();

    // Get the location of the click (in window coordinates)
    int* position = interactor->GetEventPosition();

    // Construct the picker
    vtkNew<vtkCellPicker> picker;
    picker->SetTolerance(pickTolerance);

    // Pick from this location
    picker->Pick(position[0], position[1], 0.0, GetDefaultRenderer());

    // Get the selector state
    ModelViewSelector::Flags flags = getSelectorFlags();

    // Highlight the last actor
    vtkActorCollection* actors = picker->GetActors();
    actors->InitTraversal();
    int numActors = actors->GetNumberOfItems();
    if (numActors > 1)
    {
        createSelectionWidget(actors);
        return;
    }
    else if (numActors == 1)
    {
        selector->select(selector->find(actors->GetLastActor()), flags);
    }

    // Forward events
    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

//! Process right button click
void InteractorStyle::OnRightButtonDown()
{
    // Check if there are any items to process
    if (selector->isEmpty())
        return;

    // Create the menu
    QMenu* pMenu = new QMenu;

    // Create the actions
    QAction* pEditItemsAction = new QAction(QObject::tr("Edit corresponding tree items"), handler);
    QAction* pSelectItemsAction = new QAction(QObject::tr("Select corresponding tree items"), handler);

    // Set the icons
    pEditItemsAction->setIcon(QIcon(":/icons/edit-edit.svg"));
    pSelectItemsAction->setIcon(QIcon(":/icons/select-list.png"));

    // Set the actions signals & slots
    QObject::connect(pEditItemsAction, &QAction::triggered, handler, [this]() { emit handler->editItemsRequested(selector->selected()); });
    QObject::connect(pSelectItemsAction, &QAction::triggered, handler, [this]() { emit handler->selectItemsRequested(selector->selected()); });

    // Add the actions to the menu
    pMenu->addAction(pEditItemsAction);
    pMenu->addAction(pSelectItemsAction);

    // Save the menu pointer
    mMenus.push_back(pMenu);

    // Display the widget
    pMenu->popup(QCursor::pos());
}

//! Process key press events
void InteractorStyle::OnKeyPress()
{
    // Retrieve the window interactor
    vtkRenderWindowInteractor* interactor = GetInteractor();

    // Get the pressed key
    std::string key = interactor->GetKeySym();

    // Process the press
    if (key == "Escape" || key == "BackSpace" || key == "Delete")
    {
        clear();
        selector->deselectAll();
        interactor->Render();
    }
    else if (GetInteractor()->GetControlKey() && key == "a")
    {
        clear();
        selector->selectAll();
        interactor->Render();
    }

    // Forward events
    vtkInteractorStyleTrackballCamera::OnKeyPress();
}

//! Remove all the items created via interaction
void InteractorStyle::clear()
{
    // Remove silhouette actors
    removeHighlights();

    // Free the menus, if necessary
    if (!mMenus.empty())
    {
        int numMenus = mMenus.size();
        for (int i = 0; i != numMenus; ++i)
            mMenus[i]->deleteLater();
        mMenus.clear();
    }
}

ModelViewSelector::Flags InteractorStyle::getSelectorFlags()
{
    ModelViewSelector::Flags flags = ModelViewSelector::kSingleSelection;
    if (GetInteractor()->GetControlKey())
        flags = ModelViewSelector::kMultipleSelection;
    return flags;
}

//! Create the widget to select actors once ray intersect them
void InteractorStyle::createSelectionWidget(vtkActorCollection* actors)
{
    // Create the menu widget
    QMenu* pMenu = new QMenu;

    // Loop through all the actors
    int numActors = actors->GetNumberOfItems();
    for (int i = 0; i != numActors; ++i)
    {
        // Retrieve the pointers to model entities
        vtkActor* actor = actors->GetNextActor();
        Core::Selection selection = selector->find(actor);
        if (!selection.isValid())
            continue;

        // Create the action
        QString label = Utility::getLabel(selection);
        QIcon icon = Utility::getIcon(selection.type);
        QAction* action = new QAction(icon, label, handler);
        action->setData(QVariant::fromValue(selection));

        // Add the action to the widget
        pMenu->addAction(action);
    }

    // Get the selector state
    ModelViewSelector::Flags flags = getSelectorFlags();

    // Set the connections
    QObject::connect(pMenu, &QMenu::hovered, handler,
                     [this](QAction* action)
                     {
                         auto selection = action->data().value<Core::Selection>();
                         highlight(selection);
                         GetInteractor()->Render();
                     });
    QObject::connect(pMenu, &QMenu::triggered, handler,
                     [this, flags](QAction* action)
                     {
                         auto selection = action->data().value<Core::Selection>();
                         selector->select(selection, flags);
                         removeHighlights();
                         GetInteractor()->Render();
                     });

    // Save the menu pointer
    mMenus.push_back(pMenu);

    // Display the widget
    pMenu->popup(QCursor::pos());
}

//! Highlight the actor by adding a silhouette around it
void InteractorStyle::highlight(Core::Selection selection)
{
    // Constants
    vtkColor3d kColor = vtkColors->GetColor3d("Red");
    int const kLineWidth = 5;

    // Remove the silhouette actors from the scene
    removeHighlights();

    // Loop through all the actors asscoiated with the given selection
    QList<vtkActor*> actors = selector->find(selection);
    int numActors = actors.size();
    for (int i = 0; i != numActors; ++i)
    {
        // Slice the polygon data
        vtkDataSet* dataSet = actors[i]->GetMapper()->GetInput();
        vtkPolyData* polyData = vtkPolyData::SafeDownCast(dataSet);

        // Set the mapper data
        vtkNew<vtkPolyDataMapper> silhouetteMapper;
        if (polyData && polyData->GetNumberOfLines() > 0)
        {
            silhouetteMapper->SetInputData(polyData);
        }
        else
        {
            vtkNew<vtkGeometryFilter> filter;
            filter->SetInputData(dataSet);
            filter->Update();
            vtkNew<vtkPolyDataSilhouette> silhouette;
            silhouette->SetCamera(GetDefaultRenderer()->GetActiveCamera());
            silhouette->SetInputData(filter->GetOutput());
            silhouetteMapper->SetInputConnection(silhouette->GetOutputPort());
        }

        // Construct the silhouette actor
        vtkNew<vtkActor> silhouetteActor;
        silhouetteActor->SetMapper(silhouetteMapper);
        silhouetteActor->GetProperty()->SetColor(kColor.GetData());
        silhouetteActor->GetProperty()->SetLineWidth(kLineWidth);
        mHighlightActors.push_back(silhouetteActor);

        // Add the actor to the scene
        GetDefaultRenderer()->AddActor(silhouetteActor);
    }
}

//! Remove highlighted actors from the scene
void InteractorStyle::removeHighlights()
{
    int numActors = mHighlightActors.size();
    for (int i = 0; i != numActors; ++i)
        GetDefaultRenderer()->RemoveActor(mHighlightActors[i]);
    mHighlightActors.clear();
}

//! Represent a widget to change view properties
void ModelView::showViewEditor()
{
    // Create the widget
    QListWidget* pEditor = new QListWidget;

    // Mark the element types which are presented in the model
    QList<KCL::ElementType> const drawableTypes = Utility::drawableTypes();
    QMap<KCL::ElementType, bool> maskTypes;
    for (auto type : drawableTypes)
        maskTypes[type] = false;
    std::vector<KCL::ElementType> elementTypes;
    for (KCL::ElasticSurface const& surface : mModel.surfaces)
    {
        elementTypes = surface.types();
        for (KCL::ElementType type : elementTypes)
            maskTypes[type] = true;
    }
    elementTypes = mModel.specialSurface.types();
    for (KCL::ElementType type : elementTypes)
        maskTypes[type] = true;

    // Add the items
    for (KCL::ElementType type : drawableTypes)
    {
        // Skip the element types which are not presented in the model
        if (!maskTypes[type])
            continue;

        // Slice element data
        QString label = tr("Element: %1").arg(magic_enum::enum_name(type).data());
        QColor color = Utility::getColor(mOptions.elementColors[type]);
        Qt::CheckState state = mOptions.maskElements[type] ? Qt::Checked : Qt::Unchecked;

        // Create and initialize the item
        QListWidgetItem* pItem = new QListWidgetItem(label);
        pItem->setCheckState(state);
        pItem->setData(Qt::DecorationRole, color);
        pItem->setData(Qt::UserRole, type);

        // Add it to the edtior
        pEditor->addItem(pItem);
    }

    // Set the connections
    connect(pEditor, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem* pItem)
            {
                QColor color = pItem->data(Qt::DecorationRole).value<QColor>();
                color = QColorDialog::getColor(color, this, tr("Set element color"));
                pItem->setData(Qt::DecorationRole, color);
            });
    connect(pEditor, &QListWidget::itemChanged, this,
            [this](QListWidgetItem* pItem)
            {
                bool isEnabled = pItem->checkState() == Qt::Checked;
                QColor color = pItem->data(Qt::DecorationRole).value<QColor>();
                KCL::ElementType type = pItem->data(Qt::UserRole).value<KCL::ElementType>();
                mOptions.maskElements[type] = isEnabled;
                mOptions.elementColors[type] = Utility::getColor(color);
                plot();
            });

    // Create the dialog window
    QDialog* pDialog = new QDialog(this);
    pDialog->setWindowTitle(tr("View Editor"));

    // Add the edtior to it
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(pEditor);
    pDialog->setLayout(pLayout);

    // Show the dialog window
    pDialog->show();
    pDialog->raise();
    pDialog->activateWindow();

    // Position the dialog on the screen
    QPoint center = mapToGlobal(rect().center());
    pDialog->move(center.x() - pDialog->width() / 2, center.y() - pDialog->height() / 2);
}

//! Helper function to read texture from a file
vtkSmartPointer<vtkTexture> readTexture(QString const& pathFile)
{
    // Open the file for reading
    QByteArray data;
    QFile file(pathFile);
    if (file.open(QIODevice::ReadOnly))
    {
        data = file.readAll();
        file.close();
    }

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
