#ifndef MODELVIEW_H
#define MODELVIEW_H

#include <Eigen/Geometry>
#include <vtkNamedColors.h>
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
    vtkSmartPointer<vtkNamedColors> const availableColors;
    vtkColor3d sceneColor;
    vtkColor3d symmetryColor;
    QMap<KCL::ElementType, vtkColor3d> elementColors;

    // Elements
    QMap<KCL::ElementType, bool> maskElements;
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
    void createContent();
    void drawAxes();
    void drawModel();
    void drawBeams(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color);
    void drawPanels(Transformation const& transform, std::vector<KCL::AbstractElement const*> const& elements, vtkColor3d color);

private:
    KCL::Model const& mModel;
    ModelViewOptions mOptions;
    QVTKOpenGLNativeWidget* mRenderWidget;
    vtkSmartPointer<vtkRenderWindow> mRenderWindow;
    vtkSmartPointer<vtkRenderer> mRenderer;
    vtkSmartPointer<vtkOrientationMarkerWidget> mOrientationWidget;
};
}

#endif // MODELVIEW_H
