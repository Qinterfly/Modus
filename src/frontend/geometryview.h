#ifndef GEOMETRYVIEW_H
#define GEOMETRYVIEW_H

#include <Eigen/Core>

#include <vtkColor.h>
#include <vtkPolyDataMapper.h>

#include "iview.h"

class QVTKOpenGLNativeWidget;
class vtkCameraOrientationWidget;
class vtkPoints;
class vtkCellArray;
class vtkLookupTable;
class vtkDoubleArray;

namespace Backend::Core
{
struct Geometry;
struct ModalSolution;
}

namespace Frontend
{

//! Class to represent vertex displacements for modal and flutter solutions
struct VertexField
{
    VertexField();
    VertexField(int iMode, double modeFrequency, Eigen::MatrixXd const& modeShape);
    VertexField(Backend::Core::ModalSolution const& solution, int iMode);
    ~VertexField() = default;

    bool isEmpty() const;
    void normalize();

    int index;
    double frequency;
    double damping;
    Eigen::MatrixXd values;
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
    vtkColor3d undeformedColor;

    // Opacity
    double edgeOpacity;
    double undeformedOpacity;

    // Flags
    bool showWireframe;
    bool showUndeformed;
    bool showVertices;
    bool showLines;
    bool showTriangles;
    bool showQuadrangles;

    // Dimensions
    Eigen::Vector3d sceneScale;
};

//! Class to render geometry as well as modeshapes
class GeometryView : public IView
{
    Q_OBJECT

public:
    GeometryView(Backend::Core::Geometry const& geometry, VertexField const& field = VertexField(),
                 GeometryViewOptions const& options = GeometryViewOptions());
    virtual ~GeometryView();

    void clear() override;
    void plot() override;
    void refresh() override;
    IView::Type type() const override;

    Backend::Core::Geometry const& getGeometry() const;
    QList<VertexField> const& fields() const;
    GeometryViewOptions& options();

    int numFields() const;
    void insertField(VertexField const& field);
    void removeField(int index);
    void clearFields();

    void setIsometricView();

private:
    void initialize();

    // Content
    void createContent();

    // Drawing
    vtkSmartPointer<vtkPoints> createPoints();
    vtkSmartPointer<vtkCellArray> createPolygons(Eigen::MatrixXi const& indices);
    void deformPoints(vtkSmartPointer<vtkPoints> points, VertexField const& field);
    vtkSmartPointer<vtkDoubleArray> getMagnitudes(VertexField const& field);
    void drawUndeformed();
    void drawDeformed();
    void drawElements(vtkSmartPointer<vtkPoints> points, Eigen::MatrixXi const& indices, vtkColor3d color, double opacity = 1.0);
    void drawElements(vtkSmartPointer<vtkPoints> points, Eigen::MatrixXi const& indices, vtkSmartPointer<vtkDoubleArray> scalars,
                      vtkSmartPointer<vtkLookupTable> lut);

private:
    Backend::Core::Geometry const& mGeometry;
    QList<VertexField> mFields;
    GeometryViewOptions mOptions;
    QVTKOpenGLNativeWidget* mRenderWidget;
    vtkSmartPointer<vtkRenderWindow> mRenderWindow;
    vtkSmartPointer<vtkRenderer> mRenderer;
    vtkSmartPointer<vtkCameraOrientationWidget> mOrientationWidget;
    vtkSmartPointer<vtkPoints> mUndeformedPoints;
};

}

#endif // GEOMETRYVIEW_H
