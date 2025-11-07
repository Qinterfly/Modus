#ifndef POLYEXPONENTSEDITOR_H
#define POLYEXPONENTSEDITOR_H

#include "editormanager.h"
#include "uialiasdata.h"

namespace KCL
{
struct PolyExponentsX;
struct PolyExponentsZ;
}

namespace Frontend
{

class CustomTable;

//! Class to edit polynomial exponents
class PolyExponentsEditor : public Editor
{
    Q_OBJECT

public:
    PolyExponentsEditor(KCL::PolyExponentsX* pElementX, KCL::PolyExponentsZ* pElementZ, QString const& name, QWidget* pParent = nullptr);
    virtual ~PolyExponentsEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createConnections();
    void resizeElementData();
    void setElementData();
    void setElementDataByType();
    void updateTypeComboBox();

private:
    KCL::PolyExponentsX* mpElementX;
    KCL::PolyExponentsZ* mpElementZ;
    QComboBox* mpTypeComboBox;
    Edit1i* mpNumDataEdit;
    CustomTable* mpDataTable;
};

}

#endif // POLYEXPONENTSEDITOR_H
