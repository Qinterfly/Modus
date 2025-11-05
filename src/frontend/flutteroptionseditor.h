#ifndef FLUTTEROPTIONSEDITOR_H
#define FLUTTEROPTIONSEDITOR_H

#include "custompropertyeditor.h"
#include "editormanager.h"
#include "fluttersolver.h"

namespace Frontend
{

//! Class to edit properties of flutter solver
class FlutterOptionsEditor : public Editor
{
    Q_OBJECT

public:
    enum Type
    {
        kNumModes,
        kTimeout
    };

    FlutterOptionsEditor(Backend::Core::FlutterOptions& options, QString const& name, QWidget* pParent = nullptr);
    ~FlutterOptionsEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createProperties();
    void createConnections();

    void setIntValue(QtProperty* pProperty, int value);
    void setDoubleValue(QtProperty* pProperty, double value);

private:
    Backend::Core::FlutterOptions& mOptions;
    CustomPropertyEditor* mpEditor;
};

}

#endif // FLUTTEROPTIONSEDITOR_H
