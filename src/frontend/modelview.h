#ifndef MODELVIEW_H
#define MODELVIEW_H

#include <vtkColor.h>
#include <vtkPolyDataMapper.h>
#include <QWidget>

#include <kcl/element.h>

#include "iview.h"
#include "uialiasdata.h"

namespace KCL
{
struct Model;
}

class QVTKOpenGLNativeWidget;
class vtkOrientationMarkerWidget;
class vtkTexture;

namespace Frontend
{

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
};

class ModelView : public QWidget, public IView
{
public:
    ModelView(KCL::Model const& model, ModelViewOptions const& options = ModelViewOptions());
    virtual ~ModelView();

    void clear() override;
    void refresh() override;
    IView::Type type() const override;

    void setIsometricView();
    void setPlaneView(Axis axis, bool isReverse = false);

private:
    void initialize();
    void loadTextures();
    void createContent();
    void drawAxes();
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
    vtkSmartPointer<vtkOrientationMarkerWidget> mOrientationWidget;
    QMap<QString, vtkSmartPointer<vtkTexture>> mTextures;
};
}

#endif // MODELVIEW_H
