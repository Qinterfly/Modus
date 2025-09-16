#include "abstracthierarchyitem.h"
#include "uiconstants.h"

using namespace Frontend;

AbstractHierarchyItem::AbstractHierarchyItem(QUuid const& parentID)
{
    setData(parentID, Constants::Role::skParent);
}

AbstractHierarchyItem::AbstractHierarchyItem(QString const& text, QUuid const& parentID)
    : AbstractHierarchyItem(parentID)
{
    setText(text);
}

AbstractHierarchyItem::AbstractHierarchyItem(QIcon const& icon, QString const& text, QUuid const& parentID)
    : AbstractHierarchyItem(text, parentID)
{
    setIcon(icon);
}

AbstractHierarchyItem::~AbstractHierarchyItem()
{

}
