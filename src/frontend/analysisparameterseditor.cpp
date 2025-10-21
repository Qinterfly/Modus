#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include <kcl/model.h>

#include "analysisparameterseditor.h"
#include "lineedit.h"
#include "uiutility.h"

using namespace Frontend;

// Helper enumerations
enum SymmetryKey
{
    kSymmetrical = 0,
    kASymmetrical = 1,
    kNotSymmetrical = -1
};

enum HomogenousKey
{
    kFlutter = 0,
    kAeroservoelasticity = 1,
    kAeroservoelasticityControl = 77
};

// Helper function
void setIndexByKey(QComboBox* pComboBox, int key);

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
    // Set symmetry data
    QSignalBlocker blockerSymmetry(mpSymmetryComboBox);
    setIndexByKey(mpSymmetryComboBox, mpElement->iSymmetry);

    // Set modal and flutter data
    QSignalBlocker blockerNumModes(mpNumModesEdit);
    QSignalBlocker blockerIFlow(mpIFlowEdit);
    QSignalBlocker blockerInitFlow(mpInitFlowEdit);
    QSignalBlocker blockerFlowStep(mpFlowStepEdit);
    QSignalBlocker blockerNumFlowSteps(mpNumFlowStepsEdit);
    QSignalBlocker blockerMinRealFlutterFreq(mpMinRealFlutterFreqEdit);
    mpNumModesEdit->setValue(mpElement->numLowModes);
    mpIFlowEdit->setValue(mpElement->iFlow);
    mpInitFlowEdit->setValue(mpElement->initFlow);
    mpFlowStepEdit->setValue(mpElement->flowStep);
    mpNumFlowStepsEdit->setValue(mpElement->numFlowSteps);
    mpMinRealFlutterFreqEdit->setValue(mpElement->minRealFlutterFreq);

    // Set hodograph data
    int numHodograph = mLimitsRealFreqEdits.size();
    for (int i = 0; i != numHodograph; ++i)
    {
        QSignalBlocker blockerLimitsRealFreq(mLimitsRealFreqEdits[i]);
        QSignalBlocker blockerLimitsImagFreq(mLimitsImagFreqEdits[i]);
        QSignalBlocker blockerGridStep(mGridStepEdits[i]);
        mLimitsRealFreqEdits[i]->setValue(mpElement->limitsRealFreq[i]);
        mLimitsImagFreqEdits[i]->setValue(mpElement->limitsImagFreq[i]);
        mGridStepEdits[i]->setValue(mpElement->gridSteps[i]);
    }

    // Set aero data
    int numIntegration = mIntegrationEdits.size();
    for (int i = 0; i != numIntegration; ++i)
    {
        QSignalBlocker blockerIntegration(mIntegrationEdits[i]);
        mIntegrationEdits[i]->setValue(mpElement->integrationParams[i]);
    }
    QSignalBlocker blockerControlPointPosition(mpControlPointPositionEdit);
    QSignalBlocker blockerHomogenous(mpHomogenousComboBox);
    QSignalBlocker blockerBasicSurface(mpBasicSurface);
    mpControlPointPositionEdit->setValue(mpElement->controlPointPosition);
    setIndexByKey(mpHomogenousComboBox, mpElement->iHomogenous);
    mpBasicSurface->setValue(mpElement->iBasicElasticSurface);
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
    connect(mpSymmetryComboBox, &QComboBox::currentIndexChanged, this, &AnalysisParametersEditor::setElementData);

    // Modal and flutter
    connect(mpNumModesEdit, &Edit1i::valueChanged, this, &AnalysisParametersEditor::setElementData);
    connect(mpIFlowEdit, &Edit1i::valueChanged, this, &AnalysisParametersEditor::setElementData);
    connect(mpInitFlowEdit, &Edit1d::valueChanged, this, &AnalysisParametersEditor::setElementData);
    connect(mpFlowStepEdit, &Edit1d::valueChanged, this, &AnalysisParametersEditor::setElementData);
    connect(mpNumFlowStepsEdit, &Edit1i::valueChanged, this, &AnalysisParametersEditor::setElementData);
    connect(mpMinRealFlutterFreqEdit, &Edit1d::valueChanged, this, &AnalysisParametersEditor::setElementData);

    // Root hodograph
    int numHodograph = mLimitsRealFreqEdits.size();
    for (int i = 0; i != numHodograph; ++i)
    {
        connect(mLimitsRealFreqEdits[i], &Edit1d::valueChanged, this, &AnalysisParametersEditor::setElementData);
        connect(mLimitsImagFreqEdits[i], &Edit1d::valueChanged, this, &AnalysisParametersEditor::setElementData);
        connect(mGridStepEdits[i], &Edit1i::valueChanged, this, &AnalysisParametersEditor::setElementData);
    }

    // Aero
    int numIntegration = mIntegrationEdits.size();
    for (int i = 0; i != numIntegration; ++i)
        connect(mIntegrationEdits[i], &Edit1i::valueChanged, this, &AnalysisParametersEditor::setElementData);
    connect(mpControlPointPositionEdit, &Edit1d::valueChanged, this, &AnalysisParametersEditor::setElementData);
    connect(mpHomogenousComboBox, &QComboBox::currentIndexChanged, this, &AnalysisParametersEditor::setElementData);
    connect(mpBasicSurface, &Edit1i::valueChanged, this, &AnalysisParametersEditor::setElementData);
}

//! Slice data from widgets to set element data
void AnalysisParametersEditor::setElementData()
{
    // Slice element data
    KCL::VecN data = mpElement->get();

    // Set symmetry feature
    data[14] = mpSymmetryComboBox->currentData().toInt();

    // Set modal and flutter parameters
    data[1] = mpNumModesEdit->value();
    data[2] = mpIFlowEdit->value();
    data[3] = mpInitFlowEdit->value();
    data[4] = mpFlowStepEdit->value();
    data[5] = mpNumFlowStepsEdit->value();
    data[13] = mpMinRealFlutterFreqEdit->value();

    // Set root hodograph data
    int numHodograph = mLimitsRealFreqEdits.size();
    for (int i = 0; i != numHodograph; ++i)
    {
        data[7 + i] = mLimitsRealFreqEdits[i]->value();
        data[9 + i] = mLimitsImagFreqEdits[i]->value();
        data[11 + i] = mGridStepEdits[i]->value();
    }

    // Set aero data
    int numIntegration = mIntegrationEdits.size();
    for (int i = 0; i != numIntegration; ++i)
        data[27 + i] = mIntegrationEdits[i]->value();
    data[6] = mpControlPointPositionEdit->value();
    data[15] = mpHomogenousComboBox->currentData().toInt();
    data[16] = mpBasicSurface->value();

    // Set the updated data
    emit commandExecuted(new EditElements(mpElement, data, name()));
}

//! Create the layout of widgets to define, if a structure is symmetrical
QLayout* AnalysisParametersEditor::createSymmetryLayout()
{
    QHBoxLayout* pLayout = new QHBoxLayout;
    mpSymmetryComboBox = new QComboBox;
    mpSymmetryComboBox->addItem(tr("Not symmetrical"), QVariant::fromValue(SymmetryKey::kNotSymmetrical));
    mpSymmetryComboBox->addItem(tr("Symmetrical"), QVariant::fromValue(SymmetryKey::kSymmetrical));
    mpSymmetryComboBox->addItem(tr("Asymmetrical"), QVariant::fromValue(SymmetryKey::kASymmetrical));
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

    // Initialize flutter widgets
    mpIFlowEdit->setMinimum(1);
    mpIFlowEdit->setMaximum(2);
    mpNumFlowStepsEdit->setMinimum(1);

    // Add flutter widgets to the layout
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
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Create the widgets
    QGridLayout* pGridLayout = new QGridLayout;
    int numIntegration = mIntegrationEdits.size();
    for (int i = 0; i != numIntegration; ++i)
        mIntegrationEdits[i] = new Edit1i;
    mpControlPointPositionEdit = new Edit1d;
    mpHomogenousComboBox = new QComboBox;
    mpBasicSurface = new Edit1i;

    // Initialize the widgets
    mpControlPointPositionEdit->setRange(0, 1);
    mpHomogenousComboBox->addItem(tr("Flutter"), QVariant::fromValue(HomogenousKey::kFlutter));
    mpHomogenousComboBox->addItem(tr("Aeroservoelasticity"), QVariant::fromValue(HomogenousKey::kAeroservoelasticity));
    mpHomogenousComboBox->addItem(tr("Aeroservoelasticity (control surfaces)"), QVariant::fromValue(HomogenousKey::kAeroservoelasticityControl));
    mpBasicSurface->setMinimum(0);

    // Create editors for integration parameters
    pGridLayout->addWidget(new QLabel(tr("Control point (KTS): ")), 0, 0);
    pGridLayout->addWidget(mpControlPointPositionEdit, 0, 1);
    pGridLayout->addWidget(new QLabel(tr("Integration X (NXM): ")), 0, 2);
    pGridLayout->addWidget(mIntegrationEdits[0], 0, 3);
    pGridLayout->addWidget(new QLabel(tr("Integration Y (NYM): ")), 0, 4);
    pGridLayout->addWidget(mIntegrationEdits[1], 0, 5);

    // Create editors to change modes
    QHBoxLayout* pModeLayout = new QHBoxLayout;
    pModeLayout->addWidget(new QLabel(tr("Mode (IF1): ")));
    pModeLayout->addWidget(mpHomogenousComboBox);
    pModeLayout->addWidget(new QLabel(tr("Basic surface (IF2): ")));
    pModeLayout->addWidget(mpBasicSurface);
    pModeLayout->addStretch();

    // Set the layout
    pMainLayout->addLayout(pGridLayout);
    pMainLayout->addLayout(pModeLayout);
    pMainLayout->addStretch();
    pWidget->setLayout(pMainLayout);
    return pWidget;
}

//! Helper function to set combobox current index by item key
void setIndexByKey(QComboBox* pComboBox, int key)
{
    int numItems = pComboBox->count();
    pComboBox->setCurrentIndex(-1);
    for (int i = 0; i != numItems; ++i)
    {
        if (pComboBox->itemData(i).toInt() == key)
        {
            pComboBox->setCurrentIndex(i);
            break;
        }
    }
}
