#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include <kcl/model.h>

#include "analysisparameterseditor.h"
#include "lineedit.h"
#include "uiutility.h"

using namespace Frontend;

AnalysisParametersEditor::AnalysisParametersEditor(KCL::AnalysisParameters* pElement, QString const& name, QWidget* pParent)
    : Editor(kAnalysisParameters, name, Utility::getIcon(pElement->type()), pParent)
    , mpElement(pElement)
{
    createContent();
    createConnections();
    AnalysisParametersEditor::refresh();
}

QSize AnalysisParametersEditor::sizeHint() const
{
    return QSize(750, 400);
}

//! Update data of widgets from the element source
void AnalysisParametersEditor::refresh()
{
    // TODO
}

//! Create all the widgets
void AnalysisParametersEditor::createContent()
{
    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Create the symmetry widgets
    pMainLayout->addLayout(createSymmetryLayout());

    // Create the widgets to edit modal and flutter parameters
    pMainLayout->addWidget(createModalFlutterGroupBox());

    // Create the widgets to edit hodograph parameters
    pMainLayout->addWidget(createRootHodographGroupBox());

    // Create the widgets to edit aerodynamic parameters
    pMainLayout->addWidget(createAeroGroupBox());

    // Set the layout
    pMainLayout->addStretch();
    setLayout(pMainLayout);
}

//! Specify signals and slots between widgets
void AnalysisParametersEditor::createConnections()
{
    // TODO
}

//! Slice data from widgets to set element data
void AnalysisParametersEditor::setElementData()
{
    // TODO
}

//! Create the layout of widgets to define, if a structure is symmetrical
QLayout* AnalysisParametersEditor::createSymmetryLayout()
{
    QHBoxLayout* pLayout = new QHBoxLayout;
    mpSymmetryComboBox = new QComboBox;
    mpSymmetryComboBox->addItem(tr("Not symmetrical"));
    mpSymmetryComboBox->addItem(tr("Symmetrical"));
    mpSymmetryComboBox->addItem(tr("Asymmetrical"));
    pLayout->addWidget(new QLabel(tr("Symmetry feature: ")));
    pLayout->addWidget(mpSymmetryComboBox);
    pLayout->addStretch();
    return pLayout;
}

//! Create the widgets to edit modal and flutter parameters
QGroupBox* AnalysisParametersEditor::createModalFlutterGroupBox()
{
    // Create the group box widget and its layout
    QGroupBox* pWidget = new QGroupBox(tr("Modal and flutter parameters"));
    QGridLayout* pLayout = new QGridLayout;

    // Create modal widgets
    mpNumModesEdit = new Edit1i;
    mpNumModesEdit->setMinimum(0);
    pLayout->addWidget(new QLabel(tr("Number of modes (NT): ")), 0, 0);
    pLayout->addWidget(mpNumModesEdit, 0, 1);

    // Create flutter widgets
    mpIFlowEdit = new Edit1i;
    mpInitFlowEdit = new Edit1d;
    mpFlowStepEdit = new Edit1d;
    mpNumFlowStepsEdit = new Edit1i;
    mpMinRealFlutterFreqEdit = new Edit1d;
    mpIFlowEdit->setMinimum(1);
    mpIFlowEdit->setMaximum(2);
    mpNumFlowStepsEdit->setMinimum(1);
    pLayout->addWidget(new QLabel(tr("Flow indicator (KAP): ")), 0, 2);
    pLayout->addWidget(mpIFlowEdit, 0, 3);
    pLayout->addWidget(new QLabel(tr("Number of steps (NVV): ")), 1, 0);
    pLayout->addWidget(mpNumFlowStepsEdit, 1, 1);
    pLayout->addWidget(new QLabel(tr("Start flow (V0): ")), 1, 2);
    pLayout->addWidget(mpInitFlowEdit, 1, 3);
    pLayout->addWidget(new QLabel(tr("Flow step (DV): ")), 1, 4);
    pLayout->addWidget(mpFlowStepEdit, 1, 5);
    pLayout->addWidget(new QLabel(tr("Min frequency (DE0): ")), 0, 4);
    pLayout->addWidget(mpMinRealFlutterFreqEdit, 0, 5);

    // Set the layout
    pWidget->setLayout(pLayout);
    return pWidget;
}

//! Create the widgets to edit hodograph parameters
QGroupBox* AnalysisParametersEditor::createRootHodographGroupBox()
{
    // Create the group box widget and its layout
    QGroupBox* pWidget = new QGroupBox(tr("Root hodograph"));
    QGridLayout* pLayout = new QGridLayout;

    // Create the widgets
    int numLimits = mLimitsRealFreqEdits.size();
    for (int i = 0; i != numLimits; ++i)
    {
        mLimitsRealFreqEdits[i] = new Edit1d;
        mLimitsImagFreqEdits[i] = new Edit1d;
        mGridStepEdits[i] = new Edit1i;
        mGridStepEdits[i]->setMinimum(0);
    }

    // Add the widgets to the layout
    pLayout->addWidget(new QLabel(tr("Min frequency (DMI, OMI)")), 0, 1, Qt::AlignCenter);
    pLayout->addWidget(new QLabel(tr("Max frequency (DMA, OMA)")), 0, 2, Qt::AlignCenter);
    pLayout->addWidget(new QLabel(tr("Number of grid steps (DD, D0)")), 0, 3, Qt::AlignCenter);
    pLayout->addWidget(new QLabel(tr("Real")), 1, 0);
    pLayout->addWidget(new QLabel(tr("Imag")), 2, 0);
    pLayout->addWidget(mLimitsRealFreqEdits[0], 1, 1);
    pLayout->addWidget(mLimitsImagFreqEdits[0], 2, 1);
    pLayout->addWidget(mLimitsRealFreqEdits[1], 1, 2);
    pLayout->addWidget(mLimitsImagFreqEdits[1], 2, 2);
    pLayout->addWidget(mGridStepEdits[0], 1, 3);
    pLayout->addWidget(mGridStepEdits[1], 2, 3);

    // Set the layout
    pWidget->setLayout(pLayout);
    return pWidget;
}

//! Create the widgets to edit aerodynamic parameters
QGroupBox* AnalysisParametersEditor::createAeroGroupBox()
{
    // Create the group box widget and its layout
    QGroupBox* pWidget = new QGroupBox(tr("Aero"));
    QGridLayout* pLayout = new QGridLayout;

    // Create the widgets
    int numIntegration = mIntegrationEdits.size();
    for (int i = 0; i != numIntegration; ++i)
        mIntegrationEdits[i] = new Edit1i;
    mpControlPointPositionEdit = new Edit1d;
    mpControlPointPositionEdit->setRange(0, 1);

    // Create editors for integration parameters
    pLayout->addWidget(new QLabel(tr("Control point (KTS): ")), 0, 0);
    pLayout->addWidget(mpControlPointPositionEdit, 0, 1);
    pLayout->addWidget(new QLabel(tr("Integration X (NXM): ")), 0, 2);
    pLayout->addWidget(mIntegrationEdits[0], 0, 3);
    pLayout->addWidget(new QLabel(tr("Integration Y (NYM): ")), 0, 4);
    pLayout->addWidget(mIntegrationEdits[1], 0, 5);

    // Set the layout
    pWidget->setLayout(pLayout);
    return pWidget;
}
