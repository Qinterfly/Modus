
#ifndef UIUTILITY_H
#define UIUTILITY_H

#include <QtGlobal>

#include <Eigen/Core>
#include <kcl/element.h>
#include <vtkNew.h>

QT_FORWARD_DECLARE_CLASS(QWidget);
QT_FORWARD_DECLARE_CLASS(QToolBar);
QT_FORWARD_DECLARE_CLASS(QColor);
QT_FORWARD_DECLARE_CLASS(QPalette);

class vtkActor;

namespace Frontend::Utility
{

// Text
QColor textColor(const QPalette& palette);
void setTextColor(QWidget* pWidget, const QColor& color);

// Ui
int showSaveDialog(QWidget* pWidget, QString const& title, QString const& message);
void fullScreenResize(QWidget* pWidget);
void setShortcutHints(QToolBar* pToolBar);
QString errorColorName(double value, double acceptThreshold, double criticalThreshold);

// File
void modifyFileSuffix(QString& pathFile, QString const& expectedSuffix);

// Rendering
QList<KCL::ElementType> beamTypes();
QList<KCL::ElementType> panelTypes();
QList<KCL::ElementType> aeroPanelsTypes();
QList<KCL::ElementType> massTypes();
QList<KCL::ElementType> springTypes();
vtkSmartPointer<vtkActor> createHelixActor(Eigen::Vector3d const& startPosition, Eigen::Vector3d const& endPosition, double radius, int numTurns,
                                           int resolution);
vtkSmartPointer<vtkActor> createPointsActor(QList<Eigen::Vector3d> const& positions, double radius);
vtkSmartPointer<vtkActor> createCylinderActor(Eigen::Vector3d const& startPosition, Eigen::Vector3d const& endPosition, double radius,
                                              int resolution);
}

#endif // UIUTILITY_H
