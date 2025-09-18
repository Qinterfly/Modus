#include <kcl/model.h>
#include <magicenum/magic_enum.hpp>

#include "hierarchyitem.h"
#include "subproject.h"
#include "uiconstants.h"

using namespace Backend;
using namespace Frontend;

QIcon getIcon(KCL::AbstractElement const* pElement);

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

    //
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
        if (numElements > 1)
        {
            QStandardItem* pTypeItem = new QStandardItem(typeName);
            pTypeItem->setEditable(false);
            for (int iElement = 0; iElement != numElements; ++iElement)
            {
                QString name = QObject::tr("%1: %2").arg(typeName).arg(1 + iElement);
                ElementHierarchyItem* pElementItem = new ElementHierarchyItem(mSurface.element(type, iElement), name, parentID);
                pTypeItem->appendRow(pElementItem);
                if (pTypeItem->icon().isNull())
                    pTypeItem->setIcon(pElementItem->icon());
            }
            appendRow(pTypeItem);
        }
        else if (numElements == 1)
        {
            appendRow(new ElementHierarchyItem(mSurface.element(type), typeName, parentID));
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
    setIcon(getIcon(mpElement));
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

QIcon getIcon(KCL::AbstractElement const* pElement)
{
    switch (pElement->type())
    {
    case KCL::OD:
        return QIcon(":/icons/configuration.png");
    case KCL::SM:
        return QIcon(":/icons/mass.png");
    case KCL::BI:
        return QIcon(":/icons/beam-bending.png");
    case KCL::PN:
        return QIcon(":/icons/panel.png");
    case KCL::EL:
        return QIcon(":/icons/aileron.png");
    case KCL::DE:
        return QIcon(":/icons/aileron.png");
    case KCL::M3:
        return QIcon(":/icons/mass.png");
    case KCL::OP:
        return QIcon(":/icons/layer.png");
    case KCL::BK:
        return QIcon(":/icons/beam-torsion.png");
    case KCL::AE:
        return QIcon(":/icons/trapezium.png");
    case KCL::DQ:
        return QIcon(":/icons/function.png");
    case KCL::DA:
        return QIcon(":/icons/trapezium.png");
    case KCL::DB:
        return QIcon(":/icons/beam-bending.png");
    case KCL::PK:
        return QIcon(":/icons/function.png");
    case KCL::QK:
        return QIcon(":/icons/function.png");
    case KCL::WP:
        return QIcon(":/icons/setup.png");
    case KCL::PR:
        return QIcon(":/icons/spring.png");
    case KCL::TE:
        return QIcon(":/icons/damper.png");
    case KCL::CO:
        return QIcon(":/icons/constants.png");
    default:
        break;
    }
    return QIcon();
}
