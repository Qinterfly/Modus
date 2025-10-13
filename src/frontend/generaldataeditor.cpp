#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include <kcl/model.h>

#include "generaldataeditor.h"
#include "lineedit.h"
#include "uiutility.h"

using namespace Frontend;

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
    return QSize(640, 350);
}

//! Update data of widgets from the element source
void GeneralDataEditor::refresh()
{
    // Slice element data
    KCL::VecN data = mpElement->get();

    // TODO
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
    // TODO
}

//! Create the group of widgets to edit local coordinates
QGroupBox* GeneralDataEditor::createLocalGroupBox()
{
    QStringList const kLabels = {tr("X<sub>0</sub>"), tr("Z<sub>0</sub>")};

    // Create editors for coordinates
    QGridLayout* pLayout = new QGridLayout;
    int numCoords = mLocalEdits.size();
    for (int i = 0; i != numCoords; ++i)
    {
        mLocalEdits[i] = new DoubleLineEdit;
        pLayout->addWidget(new QLabel(kLabels[i]), 0, i, Qt::AlignCenter);
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
    QStringList const kLabels = {tr("X<sub>0</sub>"), tr("Y<sub>0</sub>"), tr("Z<sub>0</sub>")};

    // Create editors for coordinates
    QGridLayout* pLayout = new QGridLayout;
    int numCoords = mGlobalEdits.size();
    for (int i = 0; i != numCoords; ++i)
    {
        mGlobalEdits[i] = new DoubleLineEdit;
        pLayout->addWidget(new QLabel(kLabels[i]), 0, i, Qt::AlignCenter);
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
    mpDihedralEdit = new DoubleLineEdit;
    pLayout->addWidget(new QLabel(tr("Dihedral angle (V, °): ")));
    pLayout->addWidget(mpDihedralEdit);

    // Create editor for sweep angle
    mpSweepEdit = new DoubleLineEdit;
    pLayout->addWidget(new QLabel(tr("Sweep angle (HI, °): ")));
    pLayout->addWidget(mpSweepEdit);

    // Create editor for local Z-angle
    mpAttackEdit = new DoubleLineEdit;
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
    mpLiftSurfaceEdit = new IntLineEdit;
    pLayout->addWidget(new QLabel(tr("Lift surface index (ISN): ")), 1, 0);
    pLayout->addWidget(mpLiftSurfaceEdit, 1, 1);

    // Create the group edit
    mpGroupEdit = new IntLineEdit;
    pLayout->addWidget(new QLabel(tr("Group index (IAF): ")), 1, 2);
    pLayout->addWidget(mpGroupEdit, 1, 3);

    // Create the torsional edit
    mpTorsionalEdit = new DoubleLineEdit;
    pLayout->addWidget(new QLabel(tr("Torsional stiffness (TORS): ")), 2, 0);
    pLayout->addWidget(mpTorsionalEdit, 2, 1);

    // Create the bending edit
    mpTorsionalEdit = new DoubleLineEdit;
    pLayout->addWidget(new QLabel(tr("Bending stiffness (BEND): ")), 2, 2);
    pLayout->addWidget(mpTorsionalEdit, 2, 3);

    // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Parameters"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}
