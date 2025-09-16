
#ifndef ABSTRACTHIERARCHYITEM_H
#define ABSTRACTHIERARCHYITEM_H

#include <QStandardItem>
#include <QUuid>

namespace Frontend
{

class AbstractHierarchyItem : public QStandardItem
{
public:
    enum Type
    {
        kSubproject = QStandardItem::UserType,
    };

    AbstractHierarchyItem() = delete;
    AbstractHierarchyItem(QUuid const& parentID);
    AbstractHierarchyItem(QString const& text, QUuid const& parentID);
    AbstractHierarchyItem(QIcon const& icon, QString const& text, QUuid const& parentID);
    virtual ~AbstractHierarchyItem() = 0;

    virtual int type() const = 0;
    virtual QUuid id() const = 0;
};

}

#endif // ABSTRACTHIERARCHYITEM_H
