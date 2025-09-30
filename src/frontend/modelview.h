#ifndef MODELVIEW_H
#define MODELVIEW_H

#include <QFlags>
#include <QWidget>

#include <vtkCallbackCommand.h>
#include <vtkColor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPolyDataMapper.h>

#include <kcl/element.h>

#include "iview.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QListWidget)

namespace KCL
{
struct Model;
}

namespace Backend::Core
{
struct Selection;
}

class QVTKOpenGLNativeWidget;
class vtkCameraOrientationWidget;
class vtkTexture;
class vtkProperty;
class vtkPlaneSource;
class vtkCamera;
class vtkActorCollection;

namespace Frontend
{

//! Class to select model entities on the scene
class ModelViewSelector
{
public:
    enum Flag
    {
        kNone,
        kSingleSelection,
        kMultipleSelection,
        kVerbose
    };
    Q_DECLARE_FLAGS(State, Flag);

    ModelViewSelector(State aState = kSingleSelection);
    ~ModelViewSelector() = default;

    bool isEmpty() const;
    bool isSelected(vtkActor* actor) const;
    int numSelected() const;
    void select(vtkActor* actor);
    void deselect(vtkActor* actor);
    void select(Backend::Core::Selection key);
    void deselect(Backend::Core::Selection key);
    void deselectAll();
    void clear();
    void registerActor(Backend::Core::Selection const& key, vtkActor* value);
    Backend::Core::Selection find(vtkActor* actor) const;

public:
    State state;

private:
    QMap<vtkActor*, vtkSmartPointer<vtkProperty>> mSelection;
    QMap<Backend::Core::Selection, QList<vtkActor*>> mActors;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(ModelViewSelector::State)

//! Rendering options
struct ModelViewOptions
{
    ModelViewOptions();
    ~ModelViewOptions() = default;

    // Color scheme
    vtkColor3d sceneColor;
    vtkColor3d sceneColor2;
    vtkColor3d edgeColor;
    QMap<KCL::ElementType, vtkColor3d> elementColors;

    // Elements
    QMap<KCL::ElementType, bool> maskElements;

    // Dimensions
    double edgeOpacity;
    float beamLineWidth;
    float springLineWidth;
    double massScale;
    double springScale;
    double pointScale;
    double beamScale;

    // Flags
    bool showThickness;
    bool showWireframe;
    bool showSymmetry;

    // Tolerance
    double pickTolerance;
};

//! Model plotter
class ModelView : public QWidget, public IView
{
public:
    ModelView(KCL::Model const& model, ModelViewOptions const& options = ModelViewOptions());
    virtual ~ModelView();

    void clear() override;
    void refresh() override;
    IView::Type type() const override;
    ModelViewSelector& selector();

    void setIsometricView();
    void setPlaneView(Axis axis, bool isReverse = false);

private:
    void initialize();
    void loadTextures();
    void createContent();
    void drawModel();
    void drawBeams2D(Transformation const& transform, int iSurface, KCL::ElementType type);
    void drawBeams3D(Transformation const& transform, int iSurface, KCL::ElementType type);
    void drawPanels2D(Transformation const& transform, int iSurface, KCL::ElementType type);
    void drawPanels3D(Transformation const& transform, int iSurface, KCL::ElementType type);
    void drawAeroPanels(Transformation const& transform, int iSurface, KCL::ElementType type);
    void drawMasses(Transformation const& transform, int iSurface, KCL::ElementType type);
    void drawSprings(bool isReflect, KCL::ElementType type);
    double getMaximumDimension();

private:
    KCL::Model const& mModel;
    ModelViewOptions mOptions;
    QVTKOpenGLNativeWidget* mRenderWidget;
    vtkSmartPointer<vtkRenderWindow> mRenderWindow;
    vtkSmartPointer<vtkRenderer> mRenderer;
    vtkSmartPointer<vtkCameraOrientationWidget> mOrientationWidget;
    QMap<QString, vtkSmartPointer<vtkTexture>> mTextures;
    ModelViewSelector mSelector;
};

//! Class to rotate planes, so that they point to the camera
class PlaneFollowerCallback : public vtkCallbackCommand
{
public:
    static PlaneFollowerCallback* New();
    void Execute(vtkObject* caller, unsigned long evId, void*) override;

    double scale;
    QList<vtkPlaneSource*> sources;
    vtkCamera* camera;
};

//! Class to process mouse and key events
class InteractorStyle : public QObject, public vtkInteractorStyleTrackballCamera
{
    Q_OBJECT

public:
    static InteractorStyle* New();
    InteractorStyle();
    virtual void OnLeftButtonDown() override;
    virtual void OnKeyPress() override;
    virtual void OnKeyUp() override;

    QVTKOpenGLNativeWidget* renderWidget;
    ModelViewSelector* selector;
    double pickTolerance;

private:
    void updateSelectorState();
    void createSelectionWidget(vtkActorCollection* actors);

private:
    QListWidget* mSelectionWidget;
};

}

#endif // MODELVIEW_H
