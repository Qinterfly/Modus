#include <kcl/model.h>
#include <magicenum/magic_enum.hpp>

#include "hierarchyitem.h"
#include "subproject.h"
#include "uiconstants.h"

using namespace Backend;
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

SubprojectHierarchyItem::SubprojectHierarchyItem(Backend::Core::Subproject& subproject, QUuid const& parentID)
    : AbstractHierarchyItem(QIcon(":/icons/subproject.svg"), subproject.name(), parentID)
    , mSubproject(subproject)
{
    setEditable(false);
    appendChildren();
}

SubprojectHierarchyItem::~SubprojectHierarchyItem()
{
}

//! Represent the subproject content
void SubprojectHierarchyItem::appendChildren()
{
    QUuid const& parentID = mSubproject.id();

    // Model
    appendRow(new ModelHierarchyItem(mSubproject.model(), parentID));
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

ModelHierarchyItem::ModelHierarchyItem(KCL::Model& model, QUuid const& parentID)
    : AbstractHierarchyItem(QIcon(":/icons/model.svg"), QObject::tr("Model"), parentID)
    , mModel(model)
{
    setEditable(false);
    appendChildren();
}

ModelHierarchyItem::~ModelHierarchyItem()
{
}

void ModelHierarchyItem::appendChildren()
{
    QUuid parentID = data(Constants::Role::skParent).toUuid();

    // Elastic surfaces
    QIcon icon = QIcon(":/icons/surface.svg");
    int numSurfaces = mModel.surfaces.size();
    for (int i = 0; i != numSurfaces; ++i)
    {
        QString name = QObject::tr("Elastic surface: %1").arg(1 + i);
        appendRow(new SurfaceHierarchyItem(mModel.surfaces[i], icon, name, parentID));
    }

    // Special surface
    icon = QIcon(":/icons/surface-special.svg");
    appendRow(new SurfaceHierarchyItem(mModel.specialSurface, icon, QObject::tr("Special surface"), parentID));
}

int ModelHierarchyItem::type() const
{
    return AbstractHierarchyItem::Type::kModel;
}

QUuid ModelHierarchyItem::id() const
{
    return QUuid();
}

KCL::Model& ModelHierarchyItem::model()
{
    return mModel;
}

SurfaceHierarchyItem::SurfaceHierarchyItem(KCL::ElasticSurface& surface, QIcon const& icon, QString const& name, QUuid const& parentID)
    : AbstractHierarchyItem(icon, name, parentID)
    , mSurface(surface)
{
    setEditable(false);
    appendChildren();
}

SurfaceHierarchyItem::~SurfaceHierarchyItem()
{
}

void SurfaceHierarchyItem::appendChildren()
{
    QUuid parentID = data(Constants::Role::skParent).toUuid();

    std::vector<KCL::ElementType> types = mSurface.types();
    int numTypes = types.size();
    for (int iType = 0; iType != numTypes; ++iType)
    {
        KCL::ElementType type = types[iType];
        QString typeName = magic_enum::enum_name(type).data();
        int numElements = mSurface.numElements(type);
        for (int iElement = 0; iElement != numElements; ++iElement)
        {
            QString name = QObject::tr("%1: %2").arg(typeName).arg(1 + iElement);
            appendRow(new ElementHierarchyItem(mSurface.element(type, iElement), name, parentID));
        }
    }
}

int SurfaceHierarchyItem::type() const
{
    return AbstractHierarchyItem::kSurface;
}

QUuid SurfaceHierarchyItem::id() const
{
    return QUuid();
}

KCL::ElasticSurface& SurfaceHierarchyItem::surface()
{
    return mSurface;
}

ElementHierarchyItem::ElementHierarchyItem(KCL::AbstractElement* pElement, QString const& name, QUuid const& parentID)
    : AbstractHierarchyItem(name, parentID)
    , mpElement(pElement)
{
    setEditable(false);
}

ElementHierarchyItem::~ElementHierarchyItem()
{
}

int ElementHierarchyItem::type() const
{
    return AbstractHierarchyItem::kElement;
}

QUuid ElementHierarchyItem::id() const
{
    return QUuid();
}

KCL::AbstractElement* ElementHierarchyItem::element()
{
    return mpElement;
}
