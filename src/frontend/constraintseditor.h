#ifndef CONSTRAINTSEDITOR_H
#define CONSTRAINTSEDITOR_H

#include "constraints.h"
#include "editormanager.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QCheckBox)

namespace Frontend
{

class CustomTable;

using variableBoolEdits = QMap<Backend::Core::VariableType, QCheckBox*>;
using variableDoubleEdits = QMap<Backend::Core::VariableType, Edit1d*>;

//! Class to edit optimization constraints
class ConstraintsEditor : public Editor
{
    Q_OBJECT

public:
    ConstraintsEditor(Backend::Core::Constraints& constraints, QString const& name, QWidget* pParent = nullptr);
    ~ConstraintsEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createConnections();
    void setData();
    bool validateCheckEdits();
    void updateBoundEdits();

private:
    Backend::Core::Constraints& mConstraints;
    CustomTable* mpTable;
    variableBoolEdits mEnabledEdits;
    variableBoolEdits mUnitedEdits;
    variableBoolEdits mMultipliedEdits;
    variableBoolEdits mNonzeroEdits;
    variableDoubleEdits mScaleEdits;
    variableDoubleEdits mMinBoundEdits;
    variableDoubleEdits mMaxBoundEdits;
};

}

#endif // CONSTRAINTSEDITOR_H
