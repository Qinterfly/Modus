
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
QT_FORWARD_DECLARE_CLASS(QIcon);
QT_FORWARD_DECLARE_CLASS(QComboBox);
QT_FORWARD_DECLARE_CLASS(QSettings);
QT_FORWARD_DECLARE_CLASS(QDir);

class vtkActor;
class vtkColor3d;
class vtkLookupTable;
class vtkRenderer;

namespace Backend::Core
{
struct Selection;
}

namespace Frontend::Utility
{

// Text
QColor textColor(const QPalette& palette);
void setTextColor(QWidget* pWidget, const QColor& color);
QFont getMonospaceFont();

// Ui
QColor getColor(vtkColor3d color);
vtkColor3d getColor(QColor color);
int showSaveDialog(QWidget* pWidget, QString const& title, QString const& message);
void fullScreenResize(QWidget* pWidget);
QSize getScreenSize();
void setShortcutHints(QToolBar* pToolBar);
QString errorColorName(double value, double acceptThreshold, double criticalThreshold);
QString getLabel(Backend::Core::Selection selection);
QString getLabel(int iSurface);
QString getModeName(int iMode, double frequency);
void findItems(HierarchyItem* pRootItem, HierarchyItem::Type type, QList<HierarchyItem*>& result);
int getRepeatedIndex(int index, int size);

// File
void modifyFileSuffix(QString& pathFile, QString const& expectedSuffix);
QDir getLastDirectory(QSettings const& settings);
QString getLastPathFile(QSettings const& settings);
void setLastPathFile(QSettings& settings, QString const& pathFile);

// Hierarchy
template<typename Item>
QList<Item*> castHierarchyItems(QList<HierarchyItem*> const& items);
QList<HierarchyItem*> childItems(HierarchyItem* pItem);
bool isSameType(QList<HierarchyItem*> const& items);
bool isSameType(QList<HierarchyItem*> const& items, HierarchyItem::Type type);
HierarchyItem* findParentByType(HierarchyItem* pItem, HierarchyItem::Type type);

// KCL
QList<KCL::ElementType> drawableTypes();
QList<KCL::ElementType> beamTypes();
QList<KCL::ElementType> panelTypes();
QList<KCL::ElementType> aeroTrapeziumTypes();
QList<KCL::ElementType> massTypes();
QList<KCL::ElementType> springTypes();
QList<KCL::ElementType> polyTypes();
bool isAeroVertical(KCL::ElementType type);
bool isAeroAileron(KCL::ElementType type);
bool isAeroMeshable(KCL::ElementType type);
Transformation computeTransformation(KCL::ElasticSurface const& surface, bool isAero = false);
Transformation computeTransformation(KCL::Vec3 const& coords, double dihedralAngle, double sweepAngle, double zAngle);
Transformation reflectTransformation(Transformation const& transform);
void setupModel(KCL::Model& model);
KCL::Model readModel(QString const& pathFile);
bool writeModel(QString const& pathFile, KCL::Model const& model);
QString toString(KCL::Model const& model);
Backend::Core::Subproject createDefaultSubproject();
KCL::Model createDefaultModel();
KCL::ElasticSurface createDefaultSurface();
KCL::ElasticSurface createDefaultSpecialSurface();

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
vtkSmartPointer<vtkLookupTable> createBlueToRedColorMap();
double getMaximumDimension(vtkSmartPointer<vtkRenderer> renderer);

// Widgets
void setGlobalByLocalEdits(Transformation const& transform, Edit1d* pLocalEdit, Edit1d* pGlobalEdit);
void setGlobalByLocalEdits(Transformation const& transform, Edits2d const& localEdits, Edits3d& globalEdits,
                           Eigen::Vector2i const& indices = {0, 2});
void setGlobalByLocalEdits(Transformation const& transform, Edits3d const& localEdits, Edits3d& globalEdits);
void setLocalByGlobalEdits(Transformation const& transform, Edit1d* pLocalEdit, Edit1d* pGlobalEdit);
void setLocalByGlobalEdits(Transformation const& transform, Edits2d& localEdits, Edits3d const& globalEdits,
                           Eigen::Vector2i const& indices = {0, 2});
void setLocalByGlobalEdits(Transformation const& transform, Edits3d& localEdits, Edits3d const& globalEdits);
void setIndexByKey(QComboBox* pComboBox, int key);

// Icons
QIcon getIcon(KCL::AbstractElement const* pElement);
QIcon getIcon(KCL::ElementType type);
QIcon getIcon(Backend::Core::ISolver const* pSolver);
}

#endif // UIUTILITY_H
