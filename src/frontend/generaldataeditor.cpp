#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include <kcl/model.h>

#include "generaldataeditor.h"
#include "lineedit.h"
#include "uiutility.h"

using namespace Frontend;
using namespace Eigen;

GeneralDataEditor::GeneralDataEditor(KCL::ElasticSurface const& surface, KCL::GeneralData* pElement, QString const& name, QWidget* pParent)
    : Editor(kGeneralData, name, Utility::getIcon(pElement->type()), pParent)
    , mTransform(Utility::computeTransformation(surface))
    , mpElement(pElement)
{
    createContent();
    createConnections();
    GeneralDataEditor::refresh();
}

QSize GeneralDataEditor::sizeHint() const
{
    return QSize(680, 350);
}

//! Update data of widgets from the element source
void GeneralDataEditor::refresh()
{
    // Set local coordinates
    int numLocals = mLocalEdits.size();
    for (int i = 0; i != numLocals; ++i)
    {
        QSignalBlocker blocker(mLocalEdits[i]);
        mLocalEdits[i]->setValue(mpElement->coords[i]);
    }

    // Set global coordinates
    setGlobalByLocal();

    // Set angles
    QSignalBlocker blockerDihedral(mpDihedralEdit);
    QSignalBlocker blockerSweep(mpSweepEdit);
    QSignalBlocker blockerAttack(mpAttackEdit);
    mpDihedralEdit->setValue(mpElement->dihedralAngle);
    mpSweepEdit->setValue(mpElement->sweepAngle);
    mpAttackEdit->setValue(mpElement->zAngle);

    // Set parameters
    QSignalBlocker blockerSymmetry(mpSymmetryCheckBox);
    QSignalBlocker blockerLiftSurfaces(mpLiftSurfacesEdit);
    QSignalBlocker blockerGroup(mpGroupEdit);
    QSignalBlocker blockerTorsional(mpTorsionalEdit);
    QSignalBlocker blockerBending(mpBendingEdit);
    mpSymmetryCheckBox->setCheckState(mpElement->iSymmetry == 0 ? Qt::Checked : Qt::Unchecked);
    mpLiftSurfacesEdit->setValue(mpElement->iLiftSurfaces);
    mpGroupEdit->setValue(mpElement->iGroup);
    mpTorsionalEdit->setValue(mpElement->torsionalFactor);
    mpBendingEdit->setValue(mpElement->bendingFactor);
}

//! Create all the widgets
void GeneralDataEditor::createContent()
{
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Create the widgets to edit parameters
    pMainLayout->addWidget(createParametersGroupBox());

    // Create the widgets to edit coordinates
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->addWidget(createLocalGroupBox());
    pLayout->addWidget(createGlobalGroupBox());
    pMainLayout->addLayout(pLayout);

    // Create the widget to edit angles
    pMainLayout->addWidget(createAnglesGroupBox());

    // Set the main layout
    pMainLayout->addStretch();
    setLayout(pMainLayout);
}

//! Specify signals and slots between widgets
void GeneralDataEditor::createConnections()
{
    // Local coordinates
    int numLocals = mLocalEdits.size();
    for (int i = 0; i != numLocals; ++i)
    {
        connect(mLocalEdits[i], &Edit1d::valueChanged, this, &GeneralDataEditor::setGlobalByLocal);
        connect(mLocalEdits[i], &Edit1d::valueChanged, this, &GeneralDataEditor::setElementData);
    }

    // Global coordinates
    int numGlobals = mGlobalEdits.size();
    for (int i = 0; i != numGlobals; ++i)
        connect(mGlobalEdits[i], &Edit1d::valueChanged, this, &GeneralDataEditor::setLocalByGlobal);

    // Angles
    connect(mpDihedralEdit, &Edit1d::valueChanged, this, &GeneralDataEditor::setElementData);
    connect(mpSweepEdit, &Edit1d::valueChanged, this, &GeneralDataEditor::setElementData);
    connect(mpAttackEdit, &Edit1d::valueChanged, this, &GeneralDataEditor::setElementData);

    // Parameters
    connect(mpSymmetryCheckBox, &QCheckBox::toggled, this, &GeneralDataEditor::setElementData);
    connect(mpLiftSurfacesEdit, &Edit1i::valueChanged, this, &GeneralDataEditor::setElementData);
    connect(mpGroupEdit, &Edit1i::valueChanged, this, &GeneralDataEditor::setElementData);
    connect(mpTorsionalEdit, &Edit1d::valueChanged, this, &GeneralDataEditor::setElementData);
    connect(mpBendingEdit, &Edit1d::valueChanged, this, &GeneralDataEditor::setElementData);
}

//! Set global coordinates by the local ones
void GeneralDataEditor::setGlobalByLocal()
{
    Utility::setGlobalByLocalEdits(mTransform, mLocalEdits, mGlobalEdits);
}

//! Set local coordinates by the global ones
void GeneralDataEditor::setLocalByGlobal()
{
    Utility::setLocalByGlobalEdits(mTransform, mLocalEdits, mGlobalEdits);
    setElementData();
}

//! Update element data from the widgets
void GeneralDataEditor::setElementData()
{
    // Slice the current data
    KCL::VecN data = mpElement->get();

    // Set the data for updating
    int numLocals = mLocalEdits.size();
    for (int i = 0; i != numLocals; ++i)
        data[1 + i] = mLocalEdits[i]->value();
    data[4] = mpDihedralEdit->value();
    data[5] = mpSweepEdit->value();
    data[6] = mpLiftSurfacesEdit->value();
    data[7] = mpSymmetryCheckBox->isChecked() ? 0 : 1;
    data[8] = mpAttackEdit->value();
    data[9] = mpGroupEdit->value();
    data[10] = mpTorsionalEdit->value();
    data[11] = mpBendingEdit->value();

    // Set the updated data
    emit commandExecuted(new EditElements(mpElement, data, name()));
}

//! Create the group of widgets to edit local coordinates
QGroupBox* GeneralDataEditor::createLocalGroupBox()
{
    QStringList const kColumnNames = {"X<sub>0</sub>", "Z<sub>0</sub>"};

    // Create editors for coordinates
    QGridLayout* pLayout = new QGridLayout;
    int numCoords = mLocalEdits.size();
    for (int i = 0; i != numCoords; ++i)
    {
        mLocalEdits[i] = new Edit1d;
        pLayout->addWidget(new QLabel(kColumnNames[i]), 0, i, Qt::AlignCenter);
        pLayout->addWidget(mLocalEdits[i], 1, i);
    }

    // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Local coordinates"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Create the group of widgets to edit global coordinates
QGroupBox* GeneralDataEditor::createGlobalGroupBox()
{
    QStringList const kColumnNames = {"X<sub>0</sub>", "Y<sub>0</sub>", "Z<sub>0</sub>"};

    // Create editors for coordinates
    QGridLayout* pLayout = new QGridLayout;
    int numCoords = mGlobalEdits.size();
    for (int i = 0; i != numCoords; ++i)
    {
        mGlobalEdits[i] = new Edit1d;
        pLayout->addWidget(new QLabel(kColumnNames[i]), 0, i, Qt::AlignCenter);
        pLayout->addWidget(mGlobalEdits[i], 1, i);
    }

    // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Global coordinates"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Create the group of widgets to edit local rotations
QGroupBox* GeneralDataEditor::createAnglesGroupBox()
{
    QHBoxLayout* pLayout = new QHBoxLayout;

    // Create editor for dihedral angle
    mpDihedralEdit = new Edit1d;
    pLayout->addWidget(new QLabel(tr("Dihedral angle (V, °): ")));
    pLayout->addWidget(mpDihedralEdit);

    // Create editor for sweep angle
    mpSweepEdit = new Edit1d;
    pLayout->addWidget(new QLabel(tr("Sweep angle (HI, °): ")));
    pLayout->addWidget(mpSweepEdit);

    // Create editor for local Z-angle
    mpAttackEdit = new Edit1d;
    pLayout->addWidget(new QLabel(tr("Attack angle (Alf, °): ")));
    pLayout->addWidget(mpAttackEdit);

    // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Local rotations"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Create the group of widgets to edit parameters
QGroupBox* GeneralDataEditor::createParametersGroupBox()
{
    QGridLayout* pLayout = new QGridLayout;

    // Create the symmetry edit
    mpSymmetryCheckBox = new QCheckBox(tr("Symmetry (IOD)"));
    pLayout->addWidget(mpSymmetryCheckBox, 0, 0);

    // Create the lift surface edit
    mpLiftSurfacesEdit = new Edit1i;
    mpLiftSurfacesEdit->setMinimum(0);
    pLayout->addWidget(new QLabel(tr("Lift surfaces index (ISN): ")), 1, 0);
    pLayout->addWidget(mpLiftSurfacesEdit, 1, 1);

    // Create the group edit
    mpGroupEdit = new Edit1i;
    mpGroupEdit->setMinimum(0);
    pLayout->addWidget(new QLabel(tr("Group index (IAF): ")), 1, 2);
    pLayout->addWidget(mpGroupEdit, 1, 3);

    // Create the torsional edit
    mpTorsionalEdit = new Edit1d;
    pLayout->addWidget(new QLabel(tr("Torsional stiffness (TORS): ")), 2, 0);
    pLayout->addWidget(mpTorsionalEdit, 2, 1);

    // Create the bending edit
    mpBendingEdit = new Edit1d;
    pLayout->addWidget(new QLabel(tr("Bending stiffness (BEND): ")), 2, 2);
    pLayout->addWidget(mpBendingEdit, 2, 3);

    // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Parameters"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}
