#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <kcl/model.h>

#include "lineedit.h"
#include "springdampereditor.h"
#include "uiutility.h"

using namespace Frontend;

enum SpringType
{
    kLong = -2,
    kShort6 = 6,
    kShort36 = 36,
    kDistributed = 697
};

SpringDamperEditor::SpringDamperEditor(std::vector<KCL::ElasticSurface> const& surfaces, KCL::SpringDamper* pElement, QString const& name,
                                       QWidget* pParent)
    : Editor(kSpringDamper, name, Utility::getIcon(pElement->type()), pParent)
    , mSurfaces(surfaces)
    , mpElement(pElement)
{
    createContent();
    createConnections();
    SpringDamperEditor::refresh();
}

QSize SpringDamperEditor::sizeHint() const
{
    return QSize(750, 500);
}

//! Update data of widgets from the element source
void SpringDamperEditor::refresh()
{
    // TODO
}

//! Create all the widgets
void SpringDamperEditor::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;

    // Create the group box to pairs surfaces
    pLayout->addWidget(createPairGroupBox());

    // Create the surface widgets
    pLayout->addWidget(createSurfaceGroupBox(true));
    pLayout->addWidget(createSurfaceGroupBox(false));

    // Create the orientation widgets
    pLayout->addWidget(createOrientationGroupBox());

    // Create the matrix widgets
    pLayout->addWidget(createMatrixDataGroupBox());

    // Set the layout
    pLayout->addStretch();
    setLayout(pLayout);
}

//! Specify the widget connections
void SpringDamperEditor::createConnections()
{
    // TODO
}

//! Set global coordinates by the local ones
void SpringDamperEditor::setGlobalByLocal()
{
    // TODO
}

//! Set local coordinates by the global ones
void SpringDamperEditor::setLocalByGlobal()
{
    // TODO
}

//! Slice data from widgets to set element data
void SpringDamperEditor::setElementData()
{
    // TODO
}

//! Create a group box to pair elastic surfaces
QGroupBox* SpringDamperEditor::createPairGroupBox()
{
    // Create the widgets to select first and second surfaces
    int numSurfaces = mSurfaces.size();
    mpIFirstSurfaceEdit = new QComboBox;
    mpISecondSurfaceEdit = new QComboBox;
    for (int i = 0; i != numSurfaces; ++i)
    {
        QString name = mSurfaces[i].name.data();
        mpIFirstSurfaceEdit->addItem(name, i);
        mpISecondSurfaceEdit->addItem(name, i);
    }
    mpISecondSurfaceEdit->addItem(tr("Ground"), -1);

    // Add the widgets to the layout
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->addWidget(new QLabel(tr("First: ")));
    pLayout->addWidget(mpIFirstSurfaceEdit);
    pLayout->addWidget(new QLabel(tr("Second: ")));
    pLayout->addWidget(mpISecondSurfaceEdit);
    pLayout->addStretch();

    // Create the group box widget
    QGroupBox* pGroupBox = new QGroupBox(tr("Paired surfaces"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Create a group box to edit attachment points
QGroupBox* SpringDamperEditor::createSurfaceGroupBox(bool isFirst)
{
    // Constants
    QStringList const kLocalNames = {"X", "Z"};
    QStringList const kGlobalNames = {"X", "Y", "Z"};

    // Slice widgets
    Edits2d& localEdits = isFirst ? mFirstLocalEdits : mSecondLocalEdits;
    Edits3d& globalEdits = isFirst ? mFirstGlobalEdits : mSecondGlobalEdits;
    Edit1d** pLengthEdit = isFirst ? &mpFirstLengthEdit : &mpSecondLengthEdit;
    Edits2d& angleEdits = isFirst ? mFirstAngleEdits : mSecondAngleEdits;
    QString subscript = isFirst ? "<sub>I</sub>" : "<sub>J</sub>";
    QString name = isFirst ? tr("First surface") : tr("Second surface");

    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Create the length and angles widgets
    QHBoxLayout* pRodLayout = new QHBoxLayout;
    int numAngles = angleEdits.size();
    *pLengthEdit = new Edit1d;
    pRodLayout->addWidget(new QLabel(tr("Length: ")));
    pRodLayout->addWidget(*pLengthEdit);
    QStringList const angleNames = {tr("Sweep angle (HI%1): ").arg(subscript), tr("Attack angle (TE%1): ").arg(subscript)};
    for (int i = 0; i != numAngles; ++i)
    {
        angleEdits[i] = new Edit1d;
        pRodLayout->addWidget(new QLabel(angleNames[i]));
        pRodLayout->addWidget(angleEdits[i]);
    }
    pRodLayout->addStretch(1);
    pMainLayout->addLayout(pRodLayout);

    // Create the local widgets
    int numLocals = localEdits.size();
    QGridLayout* pLocalLayout = new QGridLayout;
    pLocalLayout->addWidget(new QLabel(tr("Local coordinates:")), 0, 0, 1, numLocals, Qt::AlignLeft);
    for (int i = 0; i != numLocals; ++i)
    {
        localEdits[i] = new Edit1d;
        pLocalLayout->addWidget(new QLabel(kLocalNames[i] + subscript), 1, i, Qt::AlignCenter);
        pLocalLayout->addWidget(localEdits[i], 2, i);
    }

    // Create the global widgets
    int numGlobals = globalEdits.size();
    QGridLayout* pGlobalLayout = new QGridLayout;
    pGlobalLayout->addWidget(new QLabel(tr("Global coordinates:")), 0, 0, 1, numGlobals, Qt::AlignLeft);
    for (int i = 0; i != numGlobals; ++i)
    {
        globalEdits[i] = new Edit1d;
        pGlobalLayout->addWidget(new QLabel(kGlobalNames[i] + subscript), 1, i, Qt::AlignCenter);
        pGlobalLayout->addWidget(globalEdits[i], 2, i);
    }

    // Combine the coords layouts
    QHBoxLayout* pCoordsLayout = new QHBoxLayout;
    pCoordsLayout->addLayout(pLocalLayout);
    pCoordsLayout->addSpacerItem(new QSpacerItem(100, 1, QSizePolicy::Maximum, QSizePolicy::Minimum));
    pCoordsLayout->addLayout(pGlobalLayout);
    pMainLayout->addLayout(pCoordsLayout);

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(name);
    pGroupBox->setLayout(pMainLayout);
    return pGroupBox;
}

//! Create a group box to edit spring-damper orientation
QGroupBox* SpringDamperEditor::createOrientationGroupBox()
{
    QStringList const kNames = {"Dihedral (VIP)", "Sweep (HIP)", "Attack (ALP)"};

    QHBoxLayout* pLayout = new QHBoxLayout;

    // Add the widgets
    int numEdits = mOrientationEdits.size();
    for (int i = 0; i != numEdits; ++i)
    {
        mOrientationEdits[i] = new Edit1d;
        pLayout->addWidget(new QLabel(kNames[i] + ": "));
        pLayout->addWidget(mOrientationEdits[i]);
    }
    pLayout->addStretch();

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(tr("Spring orientation angles"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Create a group box to edit stiffness and damping matrices
QGroupBox* SpringDamperEditor::createMatrixDataGroupBox()
{
    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Create the layout to edit type
    QHBoxLayout* pTypeLayout = new QHBoxLayout;
    mpTypeComboBox = new QComboBox;
    mpTypeComboBox->addItem(tr("Long"), SpringType::kLong);
    mpTypeComboBox->addItem(tr("Short 6"), SpringType::kShort6);
    mpTypeComboBox->addItem(tr("Short 36"), SpringType::kShort36);
    mpTypeComboBox->addItem(tr("Distributed"), SpringType::kDistributed);
    pTypeLayout->addWidget(new QLabel(tr("Type: ")));
    pTypeLayout->addWidget(mpTypeComboBox);
    pTypeLayout->addStretch();
    pMainLayout->addLayout(pTypeLayout);

    // Create the button to request editors
    QHBoxLayout* pDataLayout = new QHBoxLayout;
    QPushButton* pStiffnessButton = new QPushButton(tr("Stiffness matrix"));
    QPushButton* pDampingButton = new QPushButton(tr("Damping matrix"));
    pDataLayout->addStretch(10);
    pDataLayout->addWidget(pStiffnessButton);
    pDataLayout->addStretch(1);
    pDataLayout->addWidget(pDampingButton);
    pDataLayout->addStretch(10);
    pMainLayout->addLayout(pDataLayout);

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(tr("Spring data"));
    pGroupBox->setLayout(pMainLayout);
    return pGroupBox;
}
