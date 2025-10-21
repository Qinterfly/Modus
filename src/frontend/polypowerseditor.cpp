#include "polypowerseditor.h"
#include "uiutility.h"

using namespace Frontend;

PolyPowersEditor::PolyPowersEditor(KCL::PolyPowersX* pElementX, KCL::PolyPowersZ* pElementZ, QString const& name, QWidget* pParent)
    : Editor(kPolyPowers, name, Utility::getIcon(pElementX->type()), pParent)
    , mpElementX(pElementX)
    , mpElementZ(pElementZ)
{
    createContent();
    PolyPowersEditor::refresh();
}

QSize PolyPowersEditor::sizeHint() const
{
    return QSize(680, 350);
}

//! Update data of widgets from the element source
void PolyPowersEditor::refresh()
{
}

//! Create all the widgets
void PolyPowersEditor::createContent()
{
}

//! Change the element data dimension
void PolyPowersEditor::resizeElementData()
{
}

//! Update element data from the widgets
void PolyPowersEditor::setElementData()
{
}
