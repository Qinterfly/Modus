#include <QMimeData>

#include <kcl/model.h>

#include "fluttersolver.h"
#include "hierarchyitem.h"
#include "modalsolver.h"
#include "optimsolver.h"
#include "project.h"
#include "projecthierarchymodel.h"
#include "selectionset.h"
#include "uiutility.h"

using namespace Backend;
using namespace Frontend;

ProjectHierarchyModel::ProjectHierarchyModel(Backend::Core::Project& project, QObject* pParent)
    : QStandardItemModel(pParent)
    , mProject(project)
{
    appendChildren();
    connect(this, &ProjectHierarchyModel::itemChanged, this, &ProjectHierarchyModel::processItemChange);
}

//! Select model elements
void ProjectHierarchyModel::selectItems(KCL::Model const& model, QList<Backend::Core::Selection> const& selections)
{
    QStandardItem* pRootItem = invisibleRootItem();
    int numItems = pRootItem->rowCount();
    for (int i = 0; i != numItems; ++i)
    {
        HierarchyItem* pBaseItem = (HierarchyItem*) pRootItem->child(i);
        if (pBaseItem->type() == HierarchyItem::kSubproject)
        {
            SubprojectHierarchyItem* pSubprojectItem = (SubprojectHierarchyItem*) pBaseItem;
            pSubprojectItem->selectItems(model, selections);
        }
    }
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
