#ifndef ANALYSISPARAMETERSEDITOR_H
#define ANALYSISPARAMETERSEDITOR_H

#include "editormanager.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QGroupBox)

namespace KCL
{
class AnalysisParameters;
}

namespace Frontend
{

//! Class to edit analysis parameters
class AnalysisParametersEditor : public Editor
{
public:
    AnalysisParametersEditor(KCL::AnalysisParameters* pElement, QString const& name, QWidget* pParent = nullptr);
    ~AnalysisParametersEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createConnections();
    void setElementData();
    QLayout* createSymmetryLayout();
    QGroupBox* createModalFlutterGroupBox();
    QGroupBox* createRootHodographGroupBox();
    QGroupBox* createAeroGroupBox();

private:
    KCL::AnalysisParameters* mpElement;
    QComboBox* mpSymmetryComboBox;
    // Modal and flutter
    Edit1i* mpNumModesEdit;
    Edit1i* mpIFlowEdit;
    Edit1d* mpInitFlowEdit;
    Edit1d* mpFlowStepEdit;
    Edit1i* mpNumFlowStepsEdit;
    Edit1d* mpMinRealFlutterFreqEdit;
    // Root hodograph
    Edits2d mLimitsRealFreqEdits;
    Edits2d mLimitsImagFreqEdits;
    Edits2i mGridStepEdits;
    // Aero
    Edits2i mIntegrationEdits;
    Edit1d* mpControlPointPositionEdit;
    QComboBox* mpHomogenousComboBox;
    Edit1i* mpBasicSurface;
};

}

#endif // ANALYSISPARAMETERSEDITOR_H
