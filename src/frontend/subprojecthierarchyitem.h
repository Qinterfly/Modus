#ifndef SUBPROJECTHIERARCHYITEM_H
#define SUBPROJECTHIERARCHYITEM_H

#include "abstracthierarchyitem.h"

namespace Backend::Core
{
class Subproject;
}

namespace Frontend
{

class SubprojectHierarchyItem : public AbstractHierarchyItem
{
public:
    SubprojectHierarchyItem(Backend::Core::Subproject& subproject, QUuid const& parentID);
    virtual ~SubprojectHierarchyItem();

    void appendChildren();

    int type() const override;
    QUuid id() const override;

    Backend::Core::Subproject& subproject();

private:
    Backend::Core::Subproject& mSubproject;
};

}

#endif // SUBPROJECTHIERARCHYITEM_H
