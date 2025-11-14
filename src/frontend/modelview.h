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
class vtkPolyDataSilhouette;

namespace Frontend
{

//! Class to select model entities on the scene
class ModelViewSelector
{
public:
    enum Flag
    {
        kNone = 0x0,
        kSingleSelection = 0x1,
        kMultipleSelection = 0x2
    };
    Q_DECLARE_FLAGS(Flags, Flag);

    ModelViewSelector();
    ~ModelViewSelector() = default;

    bool isVerbose() const;
    bool isEmpty() const;
    bool isSelected(vtkActor* actor) const;
    int numSelected() const;
    QList<Backend::Core::Selection> selected() const;
    void setVerbose(bool value);

    void selectAll();
    void select(vtkActor* actor, Flags flags);
    void select(Backend::Core::Selection key, Flags flags);
    void select(QList<Backend::Core::Selection> const& keys);
    void deselect(vtkActor* actor);
    void deselect(Backend::Core::Selection key);
    void deselectAll();
    void clear();

    void registerActor(Backend::Core::Selection const& key, vtkActor* value);
    Backend::Core::Selection find(vtkActor* actor) const;
    QList<vtkActor*> find(Backend::Core::Selection selection);

private:
    bool mIsVerbose;
    QMap<vtkActor*, vtkSmartPointer<vtkProperty>> mSelection;
    QMap<Backend::Core::Selection, QList<vtkActor*>> mActors;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(ModelViewSelector::Flags)

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

//! Class to handle signals & slots
class InteractorHandler : public QObject
{
    Q_OBJECT

public:
    InteractorHandler(QObject* parent = nullptr);
    ~InteractorHandler() = default;

signals:
    void selectItemsRequested(QList<Backend::Core::Selection> selections);
    void editItemsRequested(QList<Backend::Core::Selection> selections);
};

//! Class to process mouse and key events
class InteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static InteractorStyle* New();
    InteractorStyle();
    virtual void OnLeftButtonDown() override;
    virtual void OnRightButtonDown() override;
    virtual void OnKeyPress() override;
    void clear();

public:
    ModelViewSelector* selector;
    double pickTolerance;
    InteractorHandler* handler;

private:
    ModelViewSelector::Flags getSelectorFlags();
    void createSelectionWidget(vtkActorCollection* actors);
    void highlight(Backend::Core::Selection selection);
    void removeHighlights();

private:
    QList<QMenu*> mMenus;
    QList<vtkSmartPointer<vtkActor>> mHighlightActors;
};

//! Rendering options of KCL model
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
    double axesScale;

    // Flags
    bool showThickness;
    bool showSymmetry;
    bool showWireframe;
    bool showLocalAxes;

    // Tolerance
    double pickTolerance;
};

//! Class to plot KCL model
class ModelView : public IView
{
    Q_OBJECT

public:
    ModelView(KCL::Model const& model, ModelViewOptions const& options = ModelViewOptions());
    virtual ~ModelView();

    void clear() override;
    void plot() override;
    void refresh() override;
    IView::Type type() const override;
    KCL::Model const& model();
    ModelViewOptions& options();
    ModelViewSelector& selector();

    void setIsometricView();

signals:
    void selectItemsRequested(QList<Backend::Core::Selection> selections);
    void editItemsRequested(QList<Backend::Core::Selection> selections);

private:
    void initialize();
    void loadTextures();

    // Content
    void createContent();
    void createConnections();

    // Drawing
    void drawModel();
    void drawBeams2D(Transformation const& transform, int iSurface, KCL::ElementType type);
    void drawBeams3D(Transformation const& transform, int iSurface, KCL::ElementType type);
    void drawPanels2D(Transformation const& transform, int iSurface, KCL::ElementType type);
    void drawPanels3D(Transformation const& transform, int iSurface, KCL::ElementType type);
    void drawAeroTrapeziums(Transformation const& transform, int iSurface, KCL::ElementType type);
    void drawMasses(Transformation const& transform, int iSurface, KCL::ElementType type);
    void drawSprings(bool isReflect, KCL::ElementType type);
    void drawLocalAxes(Transformation const& transform);

    // Widgets
    void showViewEditor();

private:
    KCL::Model const& mModel;
    ModelViewOptions mOptions;
    ModelViewSelector mSelector;
    QVTKOpenGLNativeWidget* mRenderWidget;
    vtkSmartPointer<vtkRenderWindow> mRenderWindow;
    vtkSmartPointer<vtkRenderer> mRenderer;
    vtkSmartPointer<vtkCameraOrientationWidget> mOrientationWidget;
    QMap<QString, vtkSmartPointer<vtkTexture>> mTextures;
    vtkSmartPointer<InteractorStyle> mStyle;
    QList<unsigned long> mObserverTags;
    double mMaximumDimension;
};
}

#endif // MODELVIEW_H
