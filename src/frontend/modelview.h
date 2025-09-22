#ifndef MODELVIEW_H
#define MODELVIEW_H

#include <Eigen/Geometry>
#include <kcl/alias.h>
#include <vtkPolyDataMapper.h>
#include <QWidget>

#include "iview.h"

namespace KCL
{
struct Model;
}

class QVTKOpenGLNativeWidget;
class vtkOrientationMarkerWidget;
class vtkNamedColors;
class vtkColor3d;

namespace Frontend
{

using Transformation = Eigen::Transform<double, 3, Eigen::Affine>;

enum Axis
{
    kX,
    kY,
    kZ,
};

class ModelView : public QWidget, public IView
{
public:
    ModelView(KCL::Model const& model);
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
    void drawBeam(Transformation const& transform, KCL::Vec2 const& startCoords, KCL::Vec2 const& endCoords, vtkColor3d color);

private:
    KCL::Model const& mModel;
    QVTKOpenGLNativeWidget* mRenderWidget;
    vtkSmartPointer<vtkRenderWindow> mRenderWindow;
    vtkSmartPointer<vtkRenderer> mRenderer;
    vtkSmartPointer<vtkOrientationMarkerWidget> mOrientationWidget;
    vtkSmartPointer<vtkNamedColors> mColors;
};
}

#endif // MODELVIEW_H
