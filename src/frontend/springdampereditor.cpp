#include "springdampereditor.h"
#include "uiutility.h"

using namespace Frontend;

SpringDamperEditor::SpringDamperEditor(std::vector<KCL::ElasticSurface> const& surfaces, KCL::SpringDamper* pElement, QString const& name,
                                       QWidget* pParent)
    : Editor(kSpringDamper, name, Utility::getIcon(pElement->type()), pParent)
    , mSurfaces(surfaces)
    , mpElement(pElement)
{
}

QSize SpringDamperEditor::sizeHint() const
{
    return QSize(680, 350);
}

//! Update data of widgets from the element source
void SpringDamperEditor::refresh()
{
    // TODO
}

//! Create all the widgets
void SpringDamperEditor::createContent()
{
    // TODO
}

//! Specify the widget connections
void SpringDamperEditor::createConnections()
{
    // TODO
}

//! Set global coordinates by the local ones
void SpringDamperEditor::setGlobalByLocal()
{
    // TODO
}

//! Set local coordinates by the global ones
void SpringDamperEditor::setLocalByGlobal()
{
    // TODO
}

//! Slice data from widgets to set element data
void SpringDamperEditor::setElementData()
{
    // TODO
}
