#ifndef OPTIMOPTIONSEDITOR_H
#define OPTIMOPTIONSEDITOR_H

#include "custompropertyeditor.h"
#include "editormanager.h"
#include "optimsolver.h"

namespace Frontend
{

//! Class to edit options of modal solver
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
    ~OptimOptionsEditor() = default;

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

#endif // OPTIMOPTIONSEDITOR_H
