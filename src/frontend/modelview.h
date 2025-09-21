#ifndef MODELVIEW_H
#define MODELVIEW_H

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

namespace Frontend
{

class ModelView : public QWidget, public IView
{
public:
    ModelView(KCL::Model const& model);
    virtual ~ModelView();

    void clear() override;
    void refresh() override;
    IView::Type type() const override;

    void initialize();
    void createContent();

private:
    KCL::Model const& mModel;
    QVTKOpenGLNativeWidget* mRenderWidget;
    vtkSmartPointer<vtkRenderWindow> mRenderWindow;
    vtkSmartPointer<vtkRenderer> mRenderer;
    vtkSmartPointer<vtkOrientationMarkerWidget> mOrientationMarker;
    vtkSmartPointer<vtkNamedColors> mColors;
};
}

#endif // MODELVIEW_H
