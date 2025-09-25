#ifndef MODELVIEW_H
#define MODELVIEW_H

#include <Eigen/Geometry>
#include <vtkColor.h>
#include <vtkPolyDataMapper.h>
#include <QWidget>

#include <kcl/element.h>

#include "iview.h"

namespace KCL
{
struct Model;
}

class QVTKOpenGLNativeWidget;
class vtkOrientationMarkerWidget;
class vtkTexture;

namespace Frontend
{

using Transformation = Eigen::Transform<double, 3, Eigen::Affine>;

enum Axis
{
    kX,
    kY,
    kZ,
};

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
    double massSize;
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
    void drawBeams(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color);
    void drawPanels(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color);
    void drawAeroPanels(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color);
    void drawMasses(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements);
    void drawSprings(std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color);
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
