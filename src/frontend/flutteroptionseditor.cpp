#include <QApplication>
#include <QVBoxLayout>

#include "flutteroptionseditor.h"

using namespace Frontend;

FlutterOptionsEditor::FlutterOptionsEditor(Backend::Core::FlutterOptions& options, QString const& name, QWidget* pParent)
    : Editor(kFlutterOptions, name, QIcon(":/icons/options.png"), pParent)
    , mOptions(options)
{
    createContent();
    createProperties();
    createConnections();
}

QSize FlutterOptionsEditor::sizeHint() const
{
    return QSize(600, 400);
}

//! Update the editor state
void FlutterOptionsEditor::refresh()
{
    mpEditor->clear();
    createProperties();
}

//! Create all the widgets
void FlutterOptionsEditor::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;
    mpEditor = new CustomPropertyEditor;
    pLayout->addWidget(mpEditor);
    setLayout(pLayout);
}

//! Create interactions between widgets
void FlutterOptionsEditor::createConnections()
{
    connect(mpEditor, &CustomPropertyEditor::intValueChanged, this, &FlutterOptionsEditor::setIntValue);
    connect(mpEditor, &CustomPropertyEditor::doubleValueChanged, this, &FlutterOptionsEditor::setDoubleValue);
}

//! Create the properties to view and edit
void FlutterOptionsEditor::createProperties()
{
    mpEditor->createIntProperty(kNumModes, tr("Number of modes"), mOptions.numModes, 1);
    mpEditor->createDoubleProperty(kTimeout, tr("Timeout"), mOptions.timeout, 0.0);
}

//! Process changing of an integer value
void FlutterOptionsEditor::setIntValue(QtProperty* pProperty, int value)
{
    if (mpEditor->id(pProperty) == kNumModes)
        emit commandExecuted(new EditProperty<Backend::Core::FlutterOptions>(mOptions, "numModes", value));
}

//! Process changing of a double value
void FlutterOptionsEditor::setDoubleValue(QtProperty* pProperty, double value)
{
    if (mpEditor->id(pProperty) == kTimeout)
        emit commandExecuted(new EditProperty<Backend::Core::FlutterOptions>(mOptions, "timeout", value));
}
