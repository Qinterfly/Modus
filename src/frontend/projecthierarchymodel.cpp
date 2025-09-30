#include <QMimeData>

#include "fluttersolver.h"
#include "hierarchyitem.h"
#include "modalsolver.h"
#include "optimsolver.h"
#include "project.h"
#include "projecthierarchymodel.h"

using namespace Frontend;

ProjectHierarchyModel::ProjectHierarchyModel(Backend::Core::Project& project, QObject* pParent)
    : QStandardItemModel(pParent)
    , mProject(project)
{
    appendChildren();
    connect(this, &ProjectHierarchyModel::itemChanged, this, &ProjectHierarchyModel::processItemChange);
}

ProjectHierarchyModel::~ProjectHierarchyModel()
{

}

//! Create all the items associated with the project
void ProjectHierarchyModel::appendChildren()
{
    QStandardItem* pRootItem = invisibleRootItem();
    QUuid const& parentID = mProject.id();

    // Subprojects
    for (auto& subproject : mProject.subprojects())
        pRootItem->appendRow(new SubprojectHierarchyItem(subproject));
}

void ProjectHierarchyModel::processItemChange(QStandardItem* pItem)
{
    QString text = pItem->text();
    switch (pItem->type())
    {
    case HierarchyItem::kSubproject:
        static_cast<SubprojectHierarchyItem*>(pItem)->subproject().name() = text;
        break;
    case HierarchyItem::kSurface:
        static_cast<SurfaceHierarchyItem*>(pItem)->surface().name = text.toStdString();
        break;
    case HierarchyItem::kModalSolver:
        static_cast<ModalSolverHierarchyItem*>(pItem)->solver()->name = text;
        break;
    case HierarchyItem::kFlutterSolver:
        static_cast<FlutterSolverHierarchyItem*>(pItem)->solver()->name = text;
        break;
    case HierarchyItem::kOptimSolver:
        static_cast<OptimSolverHierarchyItem*>(pItem)->solver()->name = text;
        break;
    default:
        break;
    }
}
