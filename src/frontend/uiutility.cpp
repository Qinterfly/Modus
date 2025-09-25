#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QScreen>
#include <QToolBar>
#include <QWidget>

#include <Eigen/Geometry>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

#include "uiutility.h"

using namespace Eigen;

namespace Frontend::Utility
{

//! Retrieve the active text color from the palette
QColor textColor(const QPalette& palette)
{
    return palette.color(QPalette::Active, QPalette::Text);
}

//! Change the text color of the widget
void setTextColor(QWidget* pWidget, const QColor& color)
{
    auto palette = pWidget->palette();
    if (textColor(palette) != color)
    {
        palette.setColor(QPalette::Active, QPalette::Text, color);
        pWidget->setPalette(palette);
    }
}

//! Show save dialog when closing a widget and process its output
int showSaveDialog(QWidget* pWidget, QString const& title, QString const& message)
{
    QMessageBox* pMessageBox = new QMessageBox(QMessageBox::Question, title, message,
                                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    pMessageBox->setFont(pWidget->font());
    auto result = pMessageBox->exec();
    switch (result)
    {
    case QMessageBox::Save:
        return 1;
    case QMessageBox::Discard:
        return 0;
    default:
        return -1;
    }
}

//! Resize widget to maximize its width and height
void fullScreenResize(QWidget* pWidget)
{
    QRect screenGeometry = QApplication::QGuiApplication::primaryScreen()->availableGeometry();
    pWidget->resize(screenGeometry.width(), screenGeometry.height());
}

//! Add shortcurt hints to all items contained in a tool bar
void setShortcutHints(QToolBar* pToolBar)
{
    QList<QAction*> actions = pToolBar->actions();
    int numActions = actions.size();
    for (int i = 0; i != numActions; ++i)
    {
        QAction* pAction = actions[i];
        QKeySequence shortcut = pAction->shortcut();
        if (shortcut.isEmpty())
            continue;
        pAction->setToolTip(QString("%1 (%2)").arg(pAction->toolTip(), shortcut.toString()));
    }
}

//! Get color name associated with error value
QString errorColorName(double value, double acceptThreshold, double criticalThreshold)
{
    QString result = "yellow";
    if (qAbs(value) < acceptThreshold)
        result = "green";
    else if (qAbs(value) > criticalThreshold)
        result = "red";
    return result;
}

//! Substitute a file suffix to the expected one, if necessary
void modifyFileSuffix(QString& pathFile, QString const& expectedSuffix)
{
    QFileInfo info(pathFile);
    QString currentSuffix = info.suffix();
    if (currentSuffix.isEmpty())
        pathFile.append(QString(".%1").arg(expectedSuffix));
    else if (currentSuffix != expectedSuffix)
        pathFile.replace(currentSuffix, expectedSuffix);
}

//! Retrieve the KCL types which are associated with beam elements
QList<KCL::ElementType> beamTypes()
{
    return {KCL::BI, KCL::BK, KCL::DB, KCL::ST, KCL::BP};
}

//! Retrieve the KCL types which are associated with panel elements
QList<KCL::ElementType> panelTypes()
{
    return {KCL::PN, KCL::OP, KCL::P4};
}

//! Retrieve the KCL types which are associated with aerodynamic panel elements
QList<KCL::ElementType> aeroPanelsTypes()
{
    return {KCL::AE, KCL::DA};
}

//! Retrieve the KCL types which are associated with mass elements
QList<KCL::ElementType> massTypes()
{
    return {KCL::M3, KCL::SM};
}

//! Retrieve the KCL types which are associated with spring elements
QList<KCL::ElementType> springTypes()
{
    return {KCL::PR};
}

//! Construct a helix of the given radius between two points
vtkSmartPointer<vtkActor> createHelix(Eigen::Vector3d const& startPosition, Eigen::Vector3d const& endPosition, double radius, int numTurns,
                                      int resolution)
{
    int kNumCellPoints = 2;

    // Compute the direction vector
    Vector3d direction = endPosition - startPosition;
    double length = direction.norm();
    direction.normalize();

    // Compute the rotation axis as well as angle
    auto rotationAxis = Vector3d::UnitZ().cross(direction);
    rotationAxis.normalize();
    double rotationAngle = qAcos(Vector3d::UnitZ().dot(direction));

    // Build up the transformation matrix
    auto transform = Eigen::Transform<double, 3, Eigen::Affine>::Identity();
    transform.rotate(AngleAxisd(rotationAngle, rotationAxis));

    // Construct the list of points
    vtkNew<vtkPoints> points;
    int numPoints = resolution * numTurns;
    double h = 2.0 * M_PI * numTurns / (numPoints - 1);
    double t = 0.0;
    for (int k = 0; k != numPoints; ++k)
    {
        Vector3d positon = startPosition + transform * Vector3d(radius * cos(t), radius * sin(t), length * k / numPoints);
        points->InsertNextPoint(positon[0], positon[1], positon[2]);
        t += h;
    }

    // Set the connectivity list
    vtkNew<vtkCellArray> indices;
    for (int k = 0; k != numPoints - 1; ++k)
    {
        indices->InsertNextCell(kNumCellPoints);
        indices->InsertCellPoint(k);
        indices->InsertCellPoint(k + 1);
    }

    // Create the polygons
    vtkNew<vtkPolyData> data;
    data->SetPoints(points);
    data->SetLines(indices);

    // Map the geometrical data
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(data);

    // Create the actor
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);

    return actor;
}
}
