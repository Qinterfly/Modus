#ifndef MODALOPTIONSEDITOR_H
#define MODALOPTIONSEDITOR_H

#include "custompropertyeditor.h"
#include "editormanager.h"
#include "modalsolver.h"

namespace Frontend
{

//! Class to edit options of modal solver
class ModalOptionsEditor : public Editor
{
    Q_OBJECT

public:
    enum Type
    {
        kNumModes,
        kTimeout
    };

    ModalOptionsEditor(Backend::Core::ModalOptions& options, QString const& name, QWidget* pParent = nullptr);
    ~ModalOptionsEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createProperties();
    void createConnections();

    void setIntValue(QtProperty* pProperty, int value);
    void setDoubleValue(QtProperty* pProperty, double value);

private:
    Backend::Core::ModalOptions& mOptions;
    CustomPropertyEditor* mpEditor;
};

}

#endif // MODALOPTIONSEDITOR_H
