#include <QMimeData>

#include "project.h"
#include "projecthierarchymodel.h"
#include "subprojecthierarchyitem.h"

using namespace Frontend;

ProjectHierarchyModel::ProjectHierarchyModel(Backend::Core::Project& project, QObject* pParent)
    : QStandardItemModel(pParent)
    , mProject(project)
{
    QStandardItem* pRootItem = invisibleRootItem();
    QUuid const& parentID = mProject.id();

    // Subprojects
    for (auto& subproject : mProject.subprojects())
        pRootItem->appendRow(new SubprojectHierarchyItem(subproject, parentID));
}

ProjectHierarchyModel::~ProjectHierarchyModel()
{

}

