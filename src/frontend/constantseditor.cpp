#include <QLabel>
#include <QVBoxLayout>

#include <kcl/model.h>

#include "constantseditor.h"
#include "lineedit.h"
#include "uiutility.h"

using namespace Frontend;

ConstantsEditor::ConstantsEditor(KCL::Constants* pElement, QString const& name, QWidget* pParent)
    : Editor(Editor::kConstants, name, Utility::getIcon(pElement->type()), pParent)
    , mpElement(pElement)
{
    createContent();
    createConnections();
    ConstantsEditor::refresh();
}

QSize ConstantsEditor::sizeHint() const
{
    return QSize(600, 150);
}

//! Update data of widgets from the element source
void ConstantsEditor::refresh()
{
    // Block the widget signals
    QSignalBlocker blockerGravityAcceleration(mpGravityAccelerationEdit);
    QSignalBlocker blockerReferenceLength(mpReferenceLengthEdit);
    QSignalBlocker blockerAirDensity(mpAirDensityEdit);
    QSignalBlocker blockerSoundSpeed(mpSoundSpeedEdit);
    QSignalBlocker blockerMachNumber(mpMachNumberEdit);
    QSignalBlocker blockerStrouhalNumber(mpStrouhalNumberEdit);
    QSignalBlocker blockerReferenceChord(mpReferenceChordEdit);

    // Set the widget values
    mpGravityAccelerationEdit->setValue(mpElement->gravityAcceleration);
    mpReferenceLengthEdit->setValue(mpElement->referenceLength);
    mpAirDensityEdit->setValue(mpElement->airDensity);
    mpSoundSpeedEdit->setValue(mpElement->soundSpeed);
    mpMachNumberEdit->setValue(mpElement->machNumber);
    mpStrouhalNumberEdit->setValue(mpElement->strouhalNumber);
    mpReferenceChordEdit->setValue(mpElement->referenceChord);
}

//! Create all the widgets
void ConstantsEditor::createContent()
{
    // Create the widgets
    mpGravityAccelerationEdit = new Edit1d;
    mpReferenceLengthEdit = new Edit1d;
    mpAirDensityEdit = new Edit1d;
    mpSoundSpeedEdit = new Edit1d;
    mpMachNumberEdit = new Edit1d;
    mpStrouhalNumberEdit = new Edit1d;
    mpReferenceChordEdit = new Edit1d;

    // Set the widget ranges
    mpGravityAccelerationEdit->setMinimum(0.0);
    mpAirDensityEdit->setMinimum(0.0);
    mpSoundSpeedEdit->setMinimum(0.0);
    mpMachNumberEdit->setMinimum(0.0);
    mpStrouhalNumberEdit->setMinimum(0.0);

    // Add the widgets to the layout
    QGridLayout* pLayout = new QGridLayout;
    pLayout->addWidget(new QLabel(tr("Gravity (G):")), 0, 0);
    pLayout->addWidget(mpGravityAccelerationEdit, 0, 1);
    pLayout->addWidget(new QLabel("Reference length (B):"), 0, 2);
    pLayout->addWidget(mpReferenceLengthEdit, 0, 3);
    pLayout->addWidget(new QLabel("Air density (ROA):"), 1, 0);
    pLayout->addWidget(mpAirDensityEdit, 1, 1);
    pLayout->addWidget(new QLabel("Sound speed (VS):"), 1, 2);
    pLayout->addWidget(mpSoundSpeedEdit, 1, 3);
    pLayout->addWidget(new QLabel("Mach number (M):"), 2, 0);
    pLayout->addWidget(mpMachNumberEdit, 2, 1);
    pLayout->addWidget(new QLabel("Strouhal number (SH):"), 2, 2);
    pLayout->addWidget(mpStrouhalNumberEdit, 2, 3);
    pLayout->addWidget(new QLabel("Reference Chord (BA):"), 3, 0);
    pLayout->addWidget(mpReferenceChordEdit, 3, 1);

    // Set the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pLayout);
    pMainLayout->addStretch();
    setLayout(pMainLayout);
}

//! Specify signals and slots between widgets
void ConstantsEditor::createConnections()
{
    connect(mpGravityAccelerationEdit, &Edit1d::valueChanged, this, &ConstantsEditor::setElementData);
    connect(mpReferenceLengthEdit, &Edit1d::valueChanged, this, &ConstantsEditor::setElementData);
    connect(mpAirDensityEdit, &Edit1d::valueChanged, this, &ConstantsEditor::setElementData);
    connect(mpSoundSpeedEdit, &Edit1d::valueChanged, this, &ConstantsEditor::setElementData);
    connect(mpMachNumberEdit, &Edit1d::valueChanged, this, &ConstantsEditor::setElementData);
    connect(mpStrouhalNumberEdit, &Edit1d::valueChanged, this, &ConstantsEditor::setElementData);
    connect(mpReferenceChordEdit, &Edit1d::valueChanged, this, &ConstantsEditor::setElementData);
}

//! Slice data from widgets to set element data
void ConstantsEditor::setElementData()
{
    // Slice the current data
    KCL::VecN data = mpElement->get();

    // Set the data
    data[0] = mpGravityAccelerationEdit->value();
    data[1] = mpReferenceLengthEdit->value();
    data[2] = mpAirDensityEdit->value();
    data[3] = mpSoundSpeedEdit->value();
    data[4] = mpMachNumberEdit->value();
    data[5] = mpStrouhalNumberEdit->value();
    data[6] = mpReferenceChordEdit->value();

    // Set the updated data
    emit commandExecuted(new EditElements(mpElement, data, name()));
}
