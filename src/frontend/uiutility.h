
#ifndef UIUTILITY_H
#define UIUTILITY_H

#include <QtGlobal>

#include <kcl/element.h>
#include <vtkNew.h>

#include "hierarchyitem.h"
#include "isolver.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QWidget);
QT_FORWARD_DECLARE_CLASS(QToolBar);
QT_FORWARD_DECLARE_CLASS(QColor);
QT_FORWARD_DECLARE_CLASS(QPalette);
QT_FORWARD_DECLARE_CLASS(QIcon)

class vtkActor;

namespace Backend::Core
{
struct Selection;
}

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
QString getLabel(Backend::Core::Selection selection);
QList<HierarchyItem*> findItems(HierarchyItem* pRootItem, HierarchyItem::Type type);

// File
void modifyFileSuffix(QString& pathFile, QString const& expectedSuffix);

// Hierarchy
template<typename Item>
QList<Item*> castHierarchyItems(QList<HierarchyItem*> const& items);
QList<HierarchyItem*> childItems(HierarchyItem* pItem);

// KCL
QList<KCL::ElementType> beamTypes();
QList<KCL::ElementType> panelTypes();
QList<KCL::ElementType> aeroPanelsTypes();
QList<KCL::ElementType> massTypes();
QList<KCL::ElementType> springTypes();

// Rendering
QList<int> jarvisMarch(QList<Point> const& points);
void setLastDepth(Matrix42d const& coords, Eigen::Vector4d& depths);
vtkSmartPointer<vtkActor> createHelixActor(Eigen::Vector3d const& startPosition, Eigen::Vector3d const& endPosition, double radius, int numTurns,
                                           int resolution);
vtkSmartPointer<vtkActor> createPointsActor(QList<Eigen::Vector3d> const& positions, double radius);
vtkSmartPointer<vtkActor> createCylinderActor(Eigen::Vector3d const& startPosition, Eigen::Vector3d const& endPosition, double radius,
                                              int resolution);
vtkSmartPointer<vtkActor> createShellActor(Transformation const& transform, Matrix42d const& coords, Eigen::Vector4d const& depths,
                                           double thickness);

// Icons
QIcon getIcon(KCL::AbstractElement const* pElement);
QIcon getIcon(KCL::ElementType type);
QIcon getIcon(Backend::Core::ISolver const* pSolver);
}

#endif // UIUTILITY_H
