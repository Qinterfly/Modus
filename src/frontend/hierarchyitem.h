
#ifndef HIERARCHYITEM_H
#define HIERARCHYITEM_H

#include <QStandardItem>
#include <QUuid>

namespace KCL
{
class ElasticSurface;
class Model;
class AbstractElement;
}

namespace Backend::Core
{
class Subproject;
}

namespace Frontend
{

//! Abstract item of project hierarchy
class AbstractHierarchyItem : public QStandardItem
{
public:
    enum Type
    {
        kSubproject = QStandardItem::UserType,
        kModel,
        kSurface,
        kElement
    };

    AbstractHierarchyItem() = delete;
    AbstractHierarchyItem(QUuid const& parentID);
    AbstractHierarchyItem(QString const& text, QUuid const& parentID);
    AbstractHierarchyItem(QIcon const& icon, QString const& text, QUuid const& parentID);
    virtual ~AbstractHierarchyItem() = 0;

    virtual int type() const = 0;
    virtual QUuid id() const = 0;
};

//! Subproject item of project hierarchy
class SubprojectHierarchyItem : public AbstractHierarchyItem
{
public:
    SubprojectHierarchyItem(Backend::Core::Subproject& subproject, QUuid const& parentID);
    virtual ~SubprojectHierarchyItem();

    int type() const override;
    QUuid id() const override;

    Backend::Core::Subproject& subproject();

private:
    void appendChildren();

    Backend::Core::Subproject& mSubproject;
};

//! Model item of subproject hierarchy
class ModelHierarchyItem : public AbstractHierarchyItem
{
public:
    ModelHierarchyItem(KCL::Model& model, QUuid const& parentID);
    virtual ~ModelHierarchyItem();

    int type() const override;
    QUuid id() const override;

    KCL::Model& model();

private:
    void appendChildren();

    KCL::Model& mModel;
};

//! Elastic surface item of subproject hierarchy
class SurfaceHierarchyItem : public AbstractHierarchyItem
{
public:
    SurfaceHierarchyItem(KCL::ElasticSurface& surface, QIcon const& icon, QString const& name, QUuid const& parentID);
    virtual ~SurfaceHierarchyItem();

    int type() const override;
    QUuid id() const override;

    KCL::ElasticSurface& surface();

private:
    void appendChildren();

    KCL::ElasticSurface& mSurface;
};

//! Element item of subproject hierarchy
class ElementHierarchyItem : public AbstractHierarchyItem
{
public:
    ElementHierarchyItem(KCL::AbstractElement* pElement, QString const& name, QUuid const& parentID);
    virtual ~ElementHierarchyItem();

    int type() const override;
    QUuid id() const override;

    KCL::AbstractElement* element();

private:
    KCL::AbstractElement* mpElement;
};
}

#endif // HIERARCHYITEM_H
