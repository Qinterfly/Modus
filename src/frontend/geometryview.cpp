#include <QToolBar>
#include <QVBoxLayout>

#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCameraOrientationWidget.h>
#include <vtkCellArray.h>
#include <vtkColorSeries.h>
#include <vtkColorTransferFunction.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkLegendBoxActor.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkSphereSource.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <QVTKOpenGLNativeWidget.h>

#include "geometry.h"
#include "geometryview.h"
#include "modalsolver.h"
#include "uiconstants.h"
#include "uiutility.h"

using namespace Backend::Core;
using namespace Frontend;
using namespace Constants::Colors;

// Helper functions
QString getModeName(int index, double frequency);
int getRepeatedIndex(int index, int size);

// Alias
using UpdateFun = std::function<void(double)>;

//! Callback command to be called after each timer event
class vtkTimerCallback : public vtkCallbackCommand
{
public:
    vtkTimerCallback()
        : timerMaxCount(60)
        , updateFun()
        , mTimerCount(0)
    {
    }

    static vtkTimerCallback* New();
    virtual void Execute(vtkObject* caller, unsigned long eventId, void* vtkNotUsed(callData))
    {
        double phase = 2.0 * M_PI * mTimerCount / timerMaxCount;
        updateFun(phase);
        if (vtkCommand::TimerEvent == eventId)
            ++mTimerCount;
        if (mTimerCount > timerMaxCount)
            mTimerCount = 0;
    }

public:
    int timerMaxCount;
    UpdateFun updateFun;

private:
    int mTimerCount;
};

vtkStandardNewMacro(vtkTimerCallback);

VertexField::VertexField()
    : index(-1)
    , frequency(0.0)
    , damping(0.0)
{
}

VertexField::VertexField(int iMode, double modeFrequency, Eigen::MatrixXd const& modeShape)
    : index(iMode)
    , frequency(modeFrequency)
    , values(modeShape)
    , name(getModeName(iMode, modeFrequency))
{
}

VertexField::VertexField(ModalSolution const& solution, int iMode)
    : VertexField()
{
    if (solution.isEmpty())
        return;
    if (iMode < 0 || iMode >= solution.numModes())
        return;
    index = iMode;
    frequency = solution.frequencies(iMode);
    values = solution.modeShapes[iMode];
    if (!solution.names.empty())
        name = solution.names[iMode];
    else
        name = getModeName(iMode, frequency);
}

bool VertexField::isEmpty() const
{
    return values.size() == 0;
}

void VertexField::normalize()
{
    double norm = 0.0;
    int numRows = values.rows();
    int numCols = values.cols();
    for (int i = 0; i != numRows; ++i)
    {
        for (int j = 0; j != numCols; ++j)
            norm = std::max(norm, std::abs(values(i, j)));
    }
    if (norm > std::numeric_limits<double>::epsilon())
        values /= norm;
}

GeometryViewOptions::GeometryViewOptions()
{
    // Color scheme
    sceneColor = vtkColors->GetColor3d("aliceblue");
    sceneColor2 = vtkColors->GetColor3d("white");
    edgeColor = vtkColors->GetColor3d("gainsboro");
    undeformedColor = vtkColors->GetColor3d("black");
    deformedColors = {vtkColors->GetColor3d("red"),  vtkColors->GetColor3d("green"),   vtkColors->GetColor3d("blue"),
                      vtkColors->GetColor3d("cyan"), vtkColors->GetColor3d("magenta"), vtkColors->GetColor3d("orange")};

    // Opacity
    edgeOpacity = 0.5;
    undeformedOpacity = 0.5;

    // Flags
    animate = true;
    showWireframe = false;
    showUndeformed = true;
    showVertices = true;
    showLines = true;
    showTriangles = true;
    showQuadrangles = true;

    // Animation
    animationDuration = 20;
    numAnimationFrames = 60;

    // Scales
    sceneScale = {1.0, 1.0, -1.0};
    deformedScales = {0.1};
    deformedInitPhases = {0};
}

GeometryView::GeometryView(Geometry const& geometry, VertexField const& field, GeometryViewOptions const& options)
    : mGeometry(geometry)
    , mOptions(options)
{
    insertField(field);
    createContent();
    initialize();
}

GeometryView::~GeometryView()
{
}

//! Clear all the items from the scene
void GeometryView::clear()
{
    auto interactor = mRenderWindow->GetInteractor();

    // Stop the timer
    if (mTimerId >= 0)
    {
        interactor->DestroyTimer(mTimerId);
        mTimerId = -1;
    }

    // Remove the observers
    int numObservers = mObserverTags.size();
    for (int i = 0; i != numObservers; ++i)
        interactor->RemoveObserver(mObserverTags[i]);
    mObserverTags.clear();

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
void GeometryView::plot()
{
    // Clean up the scene
    clear();

    // Render the undeformed state
    mUndeformedPoints = createPoints();
    if (mOptions.showUndeformed)
        drawUndeformed();

    // Render the deformed state
    drawDeformed();

    // Display legend related to the rendered fields
    drawLegend();

    // Start the timer
    if (mOptions.animate)
    {
        auto interactor = mRenderWindow->GetInteractor();
        mTimerId = interactor->CreateRepeatingTimer(mOptions.animationDuration);
    }

    // Render the scene
    mRenderWindow->Render();
}

//! Update the scene
void GeometryView::refresh()
{
    mRenderWindow->Render();
}

//! Get the view type
IView::Type GeometryView::type() const
{
    return IView::kGeometry;
}

//! Retrieve the geometry instance
Backend::Core::Geometry const& GeometryView::getGeometry() const
{
    return mGeometry;
}

//! Get all vertex fields
QList<VertexField> const& GeometryView::fields() const
{
    return mFields;
}

//! Get the view options
GeometryViewOptions& GeometryView::options()
{
    return mOptions;
}

//! Get number of vertex fields
int GeometryView::numFields() const
{
    return mFields.size();
}

//! Add vertex field to render
void GeometryView::insertField(VertexField const& field)
{
    VertexField tField = field;
    tField.normalize();
    mFields.push_back(tField);
}

//! Remove vertex field by index
void GeometryView::removeField(int index)
{
    if (index >= 0 && index < mFields.size())
        mFields.remove(index);
}

//! Erase all vertex fields
void GeometryView::clearFields()
{
    mFields.clear();
}

//! Set the isometric view
void GeometryView::setIsometricView()
{
    vtkSmartPointer<vtkCamera> camera = mRenderer->GetActiveCamera();
    camera->SetPosition(-1, 1, 1);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(0, 1, 0);
    mRenderer->ResetCamera();
    mRenderWindow->Render();
}

//! Set the initial state of widgets
void GeometryView::initialize()
{
    int const kNumAnimationFrames = 15;

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

    // Initialize the timer
    mTimerId = -1;
}

//! Create all the widgets and corresponding actions
void GeometryView::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;

    // Create the VTK widget
    mRenderWidget = new QVTKOpenGLNativeWidget;

    // Create auxiliary function
    auto createShowAction = [this](QIcon const& icon, QString const& name, bool& option)
    {
        QAction* pAction = new QAction(icon, name);
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

    // Create the show actions
    QAction* pLinesAction = createShowAction(QIcon(":/icons/draw-line.svg"), tr("Show lines"), mOptions.showLines);
    QAction* pTrianlesAction = createShowAction(QIcon(":/icons/draw-triangle.svg"), tr("Show triangles"), mOptions.showTriangles);
    QAction* pQuadrangleAction = createShowAction(QIcon(":/icons/draw-quadrangle.png"), tr("Show quadrangles"), mOptions.showQuadrangles);
    QAction* pWireframeAction = createShowAction(QIcon(":/icons/draw-wireframe.svg"), tr("Show wireframe"), mOptions.showWireframe);
    QAction* pUndeformedAction = createShowAction(QIcon(":/icons/draw-undeformed.png"), tr("Show undeformed"), mOptions.showUndeformed);

    // Create the animation actions
    QAction* pStartAction = new QAction(QIcon(":/icons/process-start.svg"), tr("Start animation"));
    QAction* pStopAction = new QAction(QIcon(":/icons/process-stop.svg"), tr("Stop animation"));
    auto animateFun = [this, pStartAction, pStopAction]()
    {
        mOptions.animate = !mOptions.animate;
        pStartAction->setVisible(!mOptions.animate);
        pStopAction->setVisible(mOptions.animate);
        plot();
    };

    // Set initial state of actions
    pStartAction->setVisible(!mOptions.animate);
    pStopAction->setVisible(mOptions.animate);

    // Set shortcuts
    pStartAction->setShortcut(Qt::Key_Space);
    pStopAction->setShortcut(Qt::Key_Space);

    // Set the connections
    connect(pStartAction, &QAction::triggered, this, animateFun);
    connect(pStopAction, &QAction::triggered, this, animateFun);

    // Create the toolbar
    QToolBar* pToolBar = new QToolBar;
    pToolBar->addAction(pStartAction);
    pToolBar->addAction(pStopAction);
    pToolBar->addSeparator();
    pToolBar->addAction(pLinesAction);
    pToolBar->addAction(pTrianlesAction);
    pToolBar->addAction(pQuadrangleAction);
    pToolBar->addAction(pWireframeAction);
    pToolBar->addAction(pUndeformedAction);
    Utility::setShortcutHints(pToolBar);

    // Combine the widgets
    pLayout->addWidget(pToolBar);
    pLayout->addWidget(mRenderWidget);
    setLayout(pLayout);
}

//! Create points which are associated with the geometry
vtkSmartPointer<vtkPoints> GeometryView::createPoints()
{
    vtkNew<vtkPoints> points;
    int numVertices = mGeometry.numVertices();
    for (int i = 0; i != numVertices; ++i)
    {
        Eigen::Vector3d position = mGeometry.vertices[i].position;
        position = position.cwiseProduct(mOptions.sceneScale);
        points->InsertPoint(i, position[0], position[1], position[2]);
    }
    return points;
}

//! Create polygons using given indices
vtkSmartPointer<vtkCellArray> GeometryView::createPolygons(Eigen::MatrixXi const& indices)
{
    vtkNew<vtkCellArray> polygons;
    int numElements = indices.rows();
    int numElementIndices = indices.cols();
    for (int i = 0; i != numElements; ++i)
    {
        vtkNew<vtkPolygon> polygon;
        for (int j = 0; j != numElementIndices; ++j)
        {
            int iVertex = indices(i, j);
            polygon->GetPointIds()->InsertNextId(iVertex);
        }
        polygons->InsertNextCell(polygon);
    }
    return polygons;
}

//! Apply the field transformation to the points
void GeometryView::deformPoints(vtkSmartPointer<vtkPoints> points, VertexField const& field, double amplitude, double phase)
{
    // Constants
    int const kNumDirections = 3;

    // Check if the field can be applied
    int numPoints = points->GetNumberOfPoints();
    bool isField = field.values.rows() == numPoints && field.values.cols() == kNumDirections;
    if (!isField)
        return;

    // Apply the transformation
    for (int i = 0; i != numPoints; ++i)
    {
        double position[kNumDirections];
        mUndeformedPoints->GetPoint(i, position);
        for (int j = 0; j != kNumDirections; ++j)
        {
            double value = field.values(i, j);
            if (!std::isnan(value))
                position[j] += amplitude * mOptions.sceneScale[j] * value * cos(phase);
        }
        points->SetPoint(i, position);
    }
    points->Modified();
}

//! Get magnitudes at each point
vtkSmartPointer<vtkDoubleArray> GeometryView::getMagnitudes(VertexField const& field)
{
    vtkNew<vtkDoubleArray> magnitudes;
    int numPoints = mUndeformedPoints->GetNumberOfPoints();
    int numFieldValues = field.values.cols();
    magnitudes->SetNumberOfTuples(numPoints);
    bool isField = field.values.rows() == numPoints;
    for (int i = 0; i != numPoints; ++i)
    {
        double magnitude = 0.0;
        for (int j = 0; j != numFieldValues; ++j)
        {
            double value = field.values(i, j);
            if (isField && !std::isnan(value))
                magnitude = std::max(magnitude, std::abs(value));
        }
        magnitudes->SetValue(i, magnitude);
    }
    return magnitudes;
}

//! Represent the geometry
void GeometryView::drawUndeformed()
{
    if (mOptions.showLines)
        drawElements(mUndeformedPoints, mGeometry.lines, mOptions.undeformedColor, mOptions.undeformedOpacity, false);
    if (mOptions.showTriangles)
        drawElements(mUndeformedPoints, mGeometry.triangles, mOptions.undeformedColor, mOptions.undeformedOpacity, false);
    if (mOptions.showQuadrangles)
        drawElements(mUndeformedPoints, mGeometry.quadrangles, mOptions.undeformedColor, mOptions.undeformedOpacity, false);
}

//! Represent vertex fields
void GeometryView::drawDeformed()
{
    // Retrieve window interactor
    auto interactor = mRenderWindow->GetInteractor();

    // Slice dimensions
    int numColors = mOptions.deformedColors.size();
    int numScales = mOptions.deformedScales.size();
    int numPhases = mOptions.deformedInitPhases.size();
    double maxDimension = Utility::getMaximumDimension(mRenderer);

    // Create the colormap
    vtkSmartPointer<vtkLookupTable> lut = Utility::createBlueToRedColorMap();

    // Loop through all the fields
    int count = numFields();
    bool isCompare = count > 1;
    for (int iField = 0; iField != count; ++iField)
    {
        // Construct the points and evaluate scalars
        vtkSmartPointer<vtkPoints> points = createPoints();
        VertexField const& field = mFields[iField];
        vtkSmartPointer<vtkDoubleArray> magnitudes = getMagnitudes(field);

        // Get the color
        int iColor = getRepeatedIndex(iField, numColors);
        vtkColor3d color = mOptions.deformedColors[iColor];

        // Get the amplitude
        int iScale = getRepeatedIndex(iField, numScales);
        double amplitude = mOptions.deformedScales[iScale] * maxDimension;

        // Get the initial phase
        int iPhase = getRepeatedIndex(iField, numPhases);
        double initPhase = mOptions.deformedInitPhases[iPhase];

        // Construct the function for drawing elements
        std::function<void(Eigen::MatrixXi)> drawFun;
        if (isCompare)
            drawFun = [this, points, color](Eigen::MatrixXi indices) { drawElements(points, indices, color); };
        else
            drawFun = [this, points, magnitudes, lut](Eigen::MatrixXi indices) { drawElements(points, indices, magnitudes, lut); };

        // Apply the field to the points
        deformPoints(points, field, amplitude, initPhase);

        // Draw all the elements
        if (mOptions.showLines)
            drawFun(mGeometry.lines);
        if (mOptions.showTriangles)
            drawFun(mGeometry.triangles);
        if (mOptions.showQuadrangles)
            drawFun(mGeometry.quadrangles);

        // Set the callback function
        vtkNew<vtkTimerCallback> callback;
        callback->timerMaxCount = mOptions.numAnimationFrames;
        callback->updateFun = [this, points, field, amplitude, initPhase](double phase)
        {
            deformPoints(points, field, amplitude, initPhase + phase);
            mRenderWindow->Render();
        };

        // Add the callback function to the interactor
        unsigned long tag = interactor->AddObserver(vtkCommand::TimerEvent, callback);
        mObserverTags.push_back(tag);
    }

    // Add the scalar bar
    if (!isCompare)
    {
        int maxWidth = ceil((double) Utility::getScreenSize().width() / 15);
        vtkNew<vtkScalarBarActor> scalarBar;
        scalarBar->SetLabelFormat("%5.3f");
        scalarBar->GetLabelTextProperty()->SetShadow(false);
        scalarBar->GetLabelTextProperty()->SetBold(false);
        scalarBar->GetLabelTextProperty()->SetColor(vtkColors->GetColor3d("black").GetData());
        scalarBar->SetLookupTable(lut);
        scalarBar->SetNumberOfLabels(4);
        scalarBar->SetMaximumWidthInPixels(maxWidth);
        scalarBar->SetPosition(0.9, 0.05);
        scalarBar->SetPosition2(0.95, 0.6);
        mRenderer->AddViewProp(scalarBar);
    }
}

//! Render elements using one color
void GeometryView::drawElements(vtkSmartPointer<vtkPoints> points, Eigen::MatrixXi const& indices, vtkColor3d color, double opacity,
                                bool isEdgeVisible)
{
    // Check if there are any elements to render
    if (indices.rows() == 0)
        return;

    // Create polygons
    vtkSmartPointer<vtkCellArray> polygons = createPolygons(indices);

    // Group polygons
    vtkNew<vtkPolyData> polyData;
    polyData->SetPoints(points);
    polyData->SetPolys(polygons);

    // Build the mapper
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(polyData);

    // Create the actor
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(color.GetData());
    actor->GetProperty()->SetOpacity(opacity);
    if (isEdgeVisible)
    {
        actor->GetProperty()->SetEdgeColor(mOptions.edgeColor.GetData());
        actor->GetProperty()->SetEdgeOpacity(mOptions.edgeOpacity);
        actor->GetProperty()->EdgeVisibilityOn();
    }
    if (mOptions.showWireframe)
        actor->GetProperty()->SetRepresentationToWireframe();

    // Add the actor to the scene
    mRenderer->AddActor(actor);
}

//! Render color interpolated elements
void GeometryView::drawElements(vtkSmartPointer<vtkPoints> points, Eigen::MatrixXi const& indices, vtkSmartPointer<vtkDoubleArray> scalars,
                                vtkSmartPointer<vtkLookupTable> lut)
{
    // Check if there are any elements to render
    if (indices.rows() == 0)
        return;

    // Create polygons
    vtkSmartPointer<vtkCellArray> polygons = createPolygons(indices);

    // Group polygons
    vtkNew<vtkPolyData> polyData;
    polyData->SetPoints(points);
    polyData->SetPolys(polygons);
    polyData->GetPointData()->SetScalars(scalars);

    // Build the mapper
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(polyData);
    mapper->SetScalarRange(scalars->GetRange()[0], scalars->GetRange()[1]);
    mapper->SetLookupTable(lut);

    // Create the actor and add to the scene
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetEdgeColor(mOptions.edgeColor.GetData());
    actor->GetProperty()->SetEdgeOpacity(mOptions.edgeOpacity);
    actor->GetProperty()->EdgeVisibilityOn();
    if (mOptions.showWireframe)
        actor->GetProperty()->SetRepresentationToWireframe();

    // Add the actor to the scene
    mRenderer->AddActor(actor);
}

//! Display field names which are plotted
void GeometryView::drawLegend()
{
    // Constants
    Eigen::Vector2d const kTopRightCorner = {-0.6, 0.95};
    double const kWidth = 0.35;
    double const kStep = 0.2;

    // Check if there are any fields
    if (mFields.empty())
        return;
    int count = numFields();

    // Create the symbol actor
    vtkNew<vtkSphereSource> symbol;
    symbol->SetCenter(0.0, 0.5, 0.0);
    symbol->Update();

    // Create the legend actor
    vtkNew<vtkLegendBoxActor> legend;
    legend->SetNumberOfEntries(count);
    legend->UseBackgroundOff();
    legend->BorderOff();

    // Loop through all the fields
    for (int iField = 0; iField != count; ++iField)
    {
        VertexField const& field = mFields[iField];

        // Get the color
        int iColor = getRepeatedIndex(iField, mOptions.deformedColors.size());
        vtkColor3d color = mOptions.deformedColors[iColor];
        if (count == 1)
            color = vtkColors->GetColor3d("black");

        // Add the entry
        legend->SetEntry(iField, symbol->GetOutput(), field.name.toStdString().c_str(), color.GetData());
    }

    // Set the bottom left corner
    Eigen::Vector2d bottomLeftCorner = {kTopRightCorner[0] - kWidth, kTopRightCorner[1] - count * kStep};
    bottomLeftCorner[1] = std::max(-1.0, bottomLeftCorner[1]);
    legend->GetPositionCoordinate()->SetCoordinateSystemToView();
    legend->GetPositionCoordinate()->SetValue(bottomLeftCorner[0], bottomLeftCorner[1]);

    // Set the top right corner
    legend->GetPosition2Coordinate()->SetCoordinateSystemToView();
    legend->GetPosition2Coordinate()->SetValue(kTopRightCorner[0], kTopRightCorner[1]);

    // Add the legend to the scene
    mRenderer->AddActor(legend);
}

//! Helper function to construct mode name
QString getModeName(int index, double frequency)
{
    return QObject::tr("Mode %1 (%2 Hz)").arg(1 + index).arg(QString::number(frequency, 'f', 2));
}

//! Helper function to get circular index
int getRepeatedIndex(int index, int size)
{
    if (index >= size)
        return index % size;
    return index;
}
