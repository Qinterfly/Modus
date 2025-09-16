#include "subprojecthierarchyitem.h"
#include "subproject.h"

using namespace Frontend;
using namespace Backend;

SubprojectHierarchyItem::SubprojectHierarchyItem(Backend::Core::Subproject& subproject, QUuid const& parentID)
    : AbstractHierarchyItem(QIcon(":/icons/subproject.svg"), subproject.name(), parentID)
    , mSubproject(subproject)
{
    appendChildren();
}

SubprojectHierarchyItem::~SubprojectHierarchyItem()
{

}

//! Represent the subproject content
void SubprojectHierarchyItem::appendChildren()
{
    QUuid const& parentID = mSubproject.id();

    // TODO
}

int SubprojectHierarchyItem::type() const
{
    return AbstractHierarchyItem::Type::kSubproject;
}

QUuid SubprojectHierarchyItem::id() const
{
    return mSubproject.id();
}

Core::Subproject& SubprojectHierarchyItem::subproject()
{
    return mSubproject;
}
