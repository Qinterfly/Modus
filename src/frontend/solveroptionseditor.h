#ifndef SOLVEROPTIONSEDITOR_H
#define SOLVEROPTIONSEDITOR_H

#include "custompropertyeditor.h"
#include "editormanager.h"

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
    virtual ~ModalOptionsEditor() = default;

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
    virtual ~FlutterOptionsEditor() = default;

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

//! Class to edit options of optimization solver
class OptimOptionsEditor : public Editor
{
    Q_OBJECT

public:
    enum Type
    {
        kMaxNumIterations,
        kTimeoutIteration,
        kNumThreads,
        kDiffStepSize,
        kMinMAC,
        kPenaltyMAC,
        kMaxRelError,
        kNumModes
    };

    OptimOptionsEditor(Backend::Core::OptimOptions& options, QString const& name, QWidget* pParent = nullptr);
    virtual ~OptimOptionsEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createProperties();
    void createConnections();

    void setIntValue(QtProperty* pProperty, int value);
    void setDoubleValue(QtProperty* pProperty, double value);

private:
    Backend::Core::OptimOptions& mOptions;
    CustomPropertyEditor* mpEditor;
};
}

#endif // SOLVEROPTIONSEDITOR_H
