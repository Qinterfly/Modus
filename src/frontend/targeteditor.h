#ifndef TARGETEDITOR_H
#define TARGETEDITOR_H

#include <Eigen/Core>

#include "editormanager.h"
#include "uialiasdata.h"

namespace Backend::Core
{
struct ModalSolution;
}

namespace Frontend
{

class CustomTable;

//! Class to set optimization targets
class TargetEditor : public Editor
{
    Q_OBJECT

public:
    TargetEditor(Eigen::VectorXi& indices, Eigen::VectorXd& weights, Backend::Core::ModalSolution& solution, QString const& name,
                 QWidget* pParent = nullptr);
    virtual ~TargetEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createConnections();

private:
    Eigen::VectorXi& mIndices;
    Eigen::VectorXd& mWeights;
    Backend::Core::ModalSolution& mSolution;
    Edit1i* mpNumModesEdit;
    CustomTable* mpTable;
};

}

#endif // TARGETEDITOR_H
