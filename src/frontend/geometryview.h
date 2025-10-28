#ifndef GEOMETRYVIEW_H
#define GEOMETRYVIEW_H

#include <Eigen/Core>

#include <vtkColor.h>
#include <vtkPolyDataMapper.h>

#include "iview.h"

class QVTKOpenGLNativeWidget;
class vtkCameraOrientationWidget;

namespace Backend::Core
{
struct Geometry;
struct ModalSolution;
}

namespace Frontend
{

//! Class to represent vertex displacements for modal and flutter solutions
struct DisplacementField
{
    DisplacementField();
    DisplacementField(Backend::Core::ModalSolution const& solution, int iMode);
    ~DisplacementField() = default;

    bool isEmpty() const;

    int index;
    double frequency;
    double damping;
    Eigen::MatrixXd realPart;
    Eigen::MatrixXd imagPart;
    Eigen::MatrixXd amplitude;
    QString name;
};

//! Rendering options
struct GeometryViewOptions
{
    GeometryViewOptions();
    ~GeometryViewOptions() = default;

    // Color scheme
    vtkColor3d sceneColor;
    vtkColor3d sceneColor2;
    vtkColor3d edgeColor;

    // Flags
    bool showWireframe;
    bool showVertices;
    bool showLines;
    bool showQuadrangles;
};

//! Class to render geometry as well as modeshapes
class GeometryView : public IView
{
    Q_OBJECT

public:
    GeometryView(Backend::Core::Geometry const& geometry, DisplacementField const& displacement = DisplacementField(),
                 GeometryViewOptions const& options = GeometryViewOptions());
    virtual ~GeometryView();

    void clear() override;
    void plot() override;
    void refresh() override;
    IView::Type type() const override;

    Backend::Core::Geometry const& getGeometry();
    GeometryViewOptions& options();

private:
    void initialize();

    // Content
    void createContent();
    void createConnections();

private:
    Backend::Core::Geometry const& mGeometry;
    DisplacementField mDisplacement;
    GeometryViewOptions mOptions;
    QVTKOpenGLNativeWidget* mRenderWidget;
    vtkSmartPointer<vtkRenderWindow> mRenderWindow;
    vtkSmartPointer<vtkRenderer> mRenderer;
    vtkSmartPointer<vtkCameraOrientationWidget> mOrientationWidget;
};

}

#endif // GEOMETRYVIEW_H
