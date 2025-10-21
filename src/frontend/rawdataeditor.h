#ifndef RAWDATAEDITOR_H
#define RAWDATAEDITOR_H

#include "editormanager.h"
#include "uialiasdata.h"

namespace Frontend
{

class CustomTable;

class RawDataEditor : public Editor
{
public:
    RawDataEditor(KCL::AbstractElement* pElement, QString const& name, QWidget* pParent = nullptr);
    ~RawDataEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createConnections();
    void resizeElementData();
    void setElementData();

private:
    KCL::AbstractElement* mpElement;
    Edit1i* mpNumDataEdit;
    CustomTable* mpDataTable;
};

}

#endif // RAWDATAEDITOR_H
