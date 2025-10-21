#ifndef DECREMENTSEDITOR_H
#define DECREMENTSEDITOR_H

#include "editormanager.h"
#include "uialiasdata.h"

namespace KCL
{
class Decrements;
}

namespace Frontend
{

class CustomTable;

class DecrementsEditor : public Editor
{
public:
    DecrementsEditor(KCL::Decrements* pElement, QString const& name, QWidget* pParent = nullptr);
    ~DecrementsEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createConnections();
    void resizeElementData();
    void setElementData();

private:
    KCL::Decrements* mpElement;
    Edit1i* mpNumDataEdit;
    CustomTable* mpDataTable;
};

}

#endif // DECREMENTSEDITOR_H
