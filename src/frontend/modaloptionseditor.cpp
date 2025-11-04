#include <QApplication>
#include <QVBoxLayout>

#include "modaloptionseditor.h"

using namespace Frontend;

ModalOptionsEditor::ModalOptionsEditor(Backend::Core::ModalOptions& options, QString const& name, QWidget* pParent)
    : Editor(kModalOptions, name, QIcon(":/icons/options.png"), pParent)
    , mOptions(options)
{
    createContent();
    createProperties();
    createConnections();
}

QSize ModalOptionsEditor::sizeHint() const
{
    return QSize(600, 400);
}

//! Update the editor state
void ModalOptionsEditor::refresh()
{
    mpEditor->clear();
    createProperties();
}

//! Create all the widgets
void ModalOptionsEditor::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;
    mpEditor = new CustomPropertyEditor;
    pLayout->addWidget(mpEditor);
    setLayout(pLayout);
}

//! Create interactions between widgets
void ModalOptionsEditor::createConnections()
{
    connect(mpEditor, &CustomPropertyEditor::intValueChanged, this, &ModalOptionsEditor::setIntValue);
    connect(mpEditor, &CustomPropertyEditor::doubleValueChanged, this, &ModalOptionsEditor::setDoubleValue);
}

//! Create the properties to view and edit
void ModalOptionsEditor::createProperties()
{
    mpEditor->createIntProperty(kNumModes, tr("Number of modes"), mOptions.numModes, 1);
    mpEditor->createDoubleProperty(kTimeout, tr("Timeout"), mOptions.timeout, 0.0);
}

//! Process changing of an integer value
void ModalOptionsEditor::setIntValue(QtProperty* pProperty, int value)
{
    if (mpEditor->id(pProperty) == kNumModes)
        emit commandExecuted(new EditProperty<Backend::Core::ModalOptions>(mOptions, "numModes", value));
}

//! Process changing of a double value
void ModalOptionsEditor::setDoubleValue(QtProperty* pProperty, double value)
{
    if (mpEditor->id(pProperty) == kTimeout)
        emit commandExecuted(new EditProperty<Backend::Core::ModalOptions>(mOptions, "timeout", value));
}
