#include <QVBoxLayout>

#include <vtkCameraOrientationWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <QVTKOpenGLNativeWidget.h>

#include "geometry.h"
#include "geometryview.h"
#include "modalsolver.h"
#include "uiconstants.h"

using namespace Backend::Core;
using namespace Frontend;
using namespace Constants::Colors;

DisplacementField::DisplacementField()
    : index(-1)
    , frequency(0.0)
    , damping(0.0)
{
}

DisplacementField::DisplacementField(ModalSolution const& solution, int iMode)
    : DisplacementField()
{
    if (solution.isEmpty())
        return;
    if (iMode < 0 || iMode >= solution.numModes())
        return;
    index = iMode;
    frequency = solution.frequencies(iMode);
    amplitude = solution.modeShapes[iMode];
    name = solution.names[iMode];
}

bool DisplacementField::isEmpty() const
{
    return realPart.size() == 0 && imagPart.size() == 0 && amplitude.size() == 0;
}

GeometryViewOptions::GeometryViewOptions()
{
    // Color scheme
    sceneColor = vtkColors->GetColor3d("aliceblue");
    sceneColor2 = vtkColors->GetColor3d("white");
    edgeColor = vtkColors->GetColor3d("gainsboro");

    // Flags
    showWireframe = false;
    showVertices = true;
    showLines = true;
    showQuadrangles = true;
}

GeometryView::GeometryView(Geometry const& geometry, DisplacementField const& displacement, GeometryViewOptions const& options)
    : mGeometry(geometry)
    , mDisplacement(displacement)
    , mOptions(options)
{
    createContent();
    initialize();
    createConnections();
}

GeometryView::~GeometryView()
{
}

//! Clear all the items from the scene
void GeometryView::clear()
{
    auto actors = mRenderer->GetActors();
    while (actors->GetLastActor())
        mRenderer->RemoveActor(actors->GetLastActor());
}

//! Draw the scene
void GeometryView::plot()
{
    // TODO
}

//! Update the scene
void GeometryView::refresh()
{
    // TODO
}

//! Get the view type
IView::Type GeometryView::type() const
{
    return IView::kGeometry;
}

//! Retrieve the geometry instance
Backend::Core::Geometry const& GeometryView::getGeometry()
{
    return mGeometry;
}

//! Get the view options
GeometryViewOptions& GeometryView::options()
{
    return mOptions;
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
}

//! Create all the widgets and corresponding actions
void GeometryView::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;

    // Create the VTK widget
    mRenderWidget = new QVTKOpenGLNativeWidget;

    // Combine the widgets
    pLayout->addWidget(mRenderWidget);
    setLayout(pLayout);
}

//! Set the signals & slots
void GeometryView::createConnections()
{
    // TODO
}
