#ifndef TARGETEDITOR_H
#define TARGETEDITOR_H

#include <Eigen/Core>

#include "editormanager.h"
#include "uialiasdata.h"

namespace Backend::Core
{
struct ModalSolution;
struct OptimTarget;
}

namespace Frontend
{

class CustomTable;

//! Class to set optimization targets
class TargetEditor : public Editor
{
    Q_OBJECT

public:
    TargetEditor(Backend::Core::OptimTarget& target, QString const& name, QWidget* pParent = nullptr);
    virtual ~TargetEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createConnections();
    void setNumModes();
    void setData();
    void executeCommand(Eigen::VectorXi const& indices, Eigen::VectorXd const& frequencies, Eigen::VectorXd const& weights);

private:
    Backend::Core::OptimTarget& mTarget;
    Edit1i* mpNumModesEdit;
    CustomTable* mpTable;
};

}

#endif // TARGETEDITOR_H
