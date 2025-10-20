#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include <kcl/model.h>

#include "aerotrapeziumeditor.h"
#include "lineedit.h"
#include "uiutility.h"

using namespace Frontend;
using namespace Eigen;

static int const skNumPoints = 4;

QString getPointName(int index);
Vector2i getGlobalIndices(KCL::ElementType type);
bool isFactors(KCL::ElementType type);

AeroTrapeziumEditor::AeroTrapeziumEditor(KCL::ElasticSurface const& surface, KCL::AbstractElement* pElement, QString const& name,
                                         QWidget* pParent)
    : Editor(Editor::kAeroTrapezium, name, Utility::getIcon(pElement->type()), pParent)
    , mTransform(Utility::computeTransformation(surface, true))
    , mpElement(pElement)
{
    createContent();
    createConnections();
    AeroTrapeziumEditor::refresh();
}

QSize AeroTrapeziumEditor::sizeHint() const
{
    return QSize(680, 350);
}

//! Update data of widgets from the element source
void AeroTrapeziumEditor::refresh()
{
    // Slice element data
    KCL::VecN data = mpElement->get();

    // Get flags
    bool isAileron = Utility::isAeroAileron(mpElement->type());
    bool isMeshable = Utility::isAeroMeshable(mpElement->type());

    // Set aileron data
    int iShift = 0;
    if (isAileron)
    {
        QSignalBlocker blockerAileronIndex(mpAileronIndexEdit);
        mpAileronIndexEdit->setValue(data[0]);
        iShift = 1;
    }

    // Set local coordinates
    int numLocals = mLocal0Edits.size();
    for (int i = 0; i != numLocals; ++i)
    {
        QSignalBlocker blockerLocal0(mLocal0Edits[i]);
        QSignalBlocker blockerLocal1(mLocal1Edits[i]);
        QSignalBlocker blockerLocal2(mLocal1Edits[i]);
        mLocal0Edits[i]->setValue(data[iShift + i]);
        mLocal1Edits[i]->setValue(data[iShift + 2 + i]);
        mLocal2Edits[i]->setValue(data[iShift + 4 + i]);
    }

    // Set global coordinates
    setGlobalByLocal();

    // Set mesh data
    if (isMeshable)
    {
        QSignalBlocker blockerNumStrips(mpNumStripsEdit);
        QSignalBlocker blockerNumPanels(mpNumPanelsEdit);
        mpNumStripsEdit->setValue(data[iShift + 6]);
        mpNumPanelsEdit->setValue(data[iShift + 7]);
    }

    // Set factors
    if (isFactors(mpElement->type()))
    {
        QSignalBlocker blockerStiffnessFactor(mpStiffnessFactorEdit);
        QSignalBlocker blockerDampingFactor(mpDampingFactorEdit);
        mpStiffnessFactorEdit->setValue(data[iShift + 6]);
        mpDampingFactorEdit->setValue(data[iShift + 7]);
    }
}

//! Create all the widgets
void AeroTrapeziumEditor::createContent()
{
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Get flags
    bool isAileron = Utility::isAeroAileron(mpElement->type());
    bool isMeshable = Utility::isAeroMeshable(mpElement->type());

    // Create the aileron index editor
    if (isAileron)
        pMainLayout->addLayout(createAileronLayout());

    // Create the widgets to edit coordinates
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->addWidget(createLocalGroupBox());
    pLayout->addWidget(createGlobalGroupBox());
    pMainLayout->addLayout(pLayout);

    // Create the widget to edit mesh
    if (isMeshable)
        pMainLayout->addWidget(createMeshGroupBox());

    // Create the widget to edit factors
    if (isFactors(mpElement->type()))
        pMainLayout->addWidget(createFactorsGroupBox());

    // Set the main layout
    pMainLayout->addStretch();
    setLayout(pMainLayout);
}

//! Specify signals and slots between widgets
void AeroTrapeziumEditor::createConnections()
{
    // Get flags
    bool isAileron = Utility::isAeroAileron(mpElement->type());
    bool isMeshable = Utility::isAeroMeshable(mpElement->type());

    // Aileron
    if (isAileron)
        connect(mpAileronIndexEdit, &Edit1i::valueChanged, this, &AeroTrapeziumEditor::setElementData);

    // Local coordinates
    int numLocals = mLocal0Edits.size();
    for (int i = 0; i != numLocals; ++i)
    {
        connect(mLocal0Edits[i], &Edit1d::valueChanged, this, &AeroTrapeziumEditor::setGlobalByLocal);
        connect(mLocal1Edits[i], &Edit1d::valueChanged, this, &AeroTrapeziumEditor::setGlobalByLocal);
        connect(mLocal2Edits[i], &Edit1d::valueChanged, this, &AeroTrapeziumEditor::setGlobalByLocal);
        connect(mLocal0Edits[i], &Edit1d::valueChanged, this, &AeroTrapeziumEditor::setElementData);
        connect(mLocal1Edits[i], &Edit1d::valueChanged, this, &AeroTrapeziumEditor::setElementData);
        connect(mLocal2Edits[i], &Edit1d::valueChanged, this, &AeroTrapeziumEditor::setElementData);
    }

    // Global coordinates
    int numGlobals = mGlobal0Edits.size();
    for (int i = 0; i != numGlobals; ++i)
    {
        connect(mGlobal0Edits[i], &Edit1d::valueChanged, this, &AeroTrapeziumEditor::setLocalByGlobal);
        connect(mGlobal1Edits[i], &Edit1d::valueChanged, this, &AeroTrapeziumEditor::setLocalByGlobal);
    }
    connect(mGlobal2Edits[0], &Edit1d::valueChanged, this, &AeroTrapeziumEditor::setLocalByGlobal);
    connect(mGlobal2Edits[1], &Edit1d::valueChanged, this, &AeroTrapeziumEditor::setLocalByGlobal);

    // Mesh parameters
    if (isMeshable)
    {
        connect(mpNumStripsEdit, &Edit1i::valueChanged, this, &AeroTrapeziumEditor::setElementData);
        connect(mpNumPanelsEdit, &Edit1i::valueChanged, this, &AeroTrapeziumEditor::setElementData);
    }

    // Factors
    if (isFactors(mpElement->type()))
    {
        connect(mpStiffnessFactorEdit, &Edit1d::valueChanged, this, &AeroTrapeziumEditor::setElementData);
        connect(mpDampingFactorEdit, &Edit1d::valueChanged, this, &AeroTrapeziumEditor::setElementData);
    }
}

//! Set global coordinates by the local ones
void AeroTrapeziumEditor::setGlobalByLocal()
{
    auto indices = getGlobalIndices(mpElement->type());
    Utility::setGlobalByLocalEdits(mTransform, mLocal0Edits, mGlobal0Edits, indices);
    Utility::setGlobalByLocalEdits(mTransform, mLocal1Edits, mGlobal1Edits, indices);
    Utility::setGlobalByLocalEdits(mTransform, mLocal2Edits[0], mGlobal2Edits[0]);
    Utility::setGlobalByLocalEdits(mTransform, mLocal2Edits[1], mGlobal2Edits[1]);
}

//! Set local coordinates by the global ones
void AeroTrapeziumEditor::setLocalByGlobal()
{
    auto indices = getGlobalIndices(mpElement->type());
    Utility::setLocalByGlobalEdits(mTransform, mLocal0Edits, mGlobal0Edits, indices);
    Utility::setLocalByGlobalEdits(mTransform, mLocal1Edits, mGlobal1Edits, indices);
    Utility::setLocalByGlobalEdits(mTransform, mLocal2Edits[0], mGlobal2Edits[0]);
    Utility::setLocalByGlobalEdits(mTransform, mLocal2Edits[1], mGlobal2Edits[1]);
    setElementData();
}

//! Slice data from widgets to set element data
void AeroTrapeziumEditor::setElementData()
{
    // Slice element data
    KCL::VecN data = mpElement->get();

    // Get flags
    bool isAileron = Utility::isAeroAileron(mpElement->type());
    bool isMeshable = Utility::isAeroMeshable(mpElement->type());

    // Set aileron data
    int iShift = 0;
    if (isAileron)
    {
        data[0] = mpAileronIndexEdit->value();
        iShift = 1;
    }

    // Set local coordinates
    int numLocals = mLocal0Edits.size();
    for (int i = 0; i != numLocals; ++i)
    {
        data[iShift + i] = mLocal0Edits[i]->value();
        data[iShift + 2 + i] = mLocal1Edits[i]->value();
        data[iShift + 4 + i] = mLocal2Edits[i]->value();
    }

    // Set mesh parameters
    if (isMeshable)
    {
        data[iShift + 6] = mpNumStripsEdit->value();
        data[iShift + 7] = mpNumPanelsEdit->value();
    }

    // Set factors
    if (isFactors(mpElement->type()))
    {
        data[iShift + 6] = mpStiffnessFactorEdit->value();
        data[iShift + 7] = mpDampingFactorEdit->value();
    }

    // Set the updated data
    emit commandExecuted(new EditElement(mpElement, data, name()));
}

//! Create the layout to edit aileron index
QLayout* AeroTrapeziumEditor::createAileronLayout()
{
    QHBoxLayout* pLayout = new QHBoxLayout;
    mpAileronIndexEdit = new Edit1i;
    mpAileronIndexEdit->setMinimum(0);
    pLayout->addWidget(new QLabel(tr("Aileron index: ")));
    pLayout->addWidget(mpAileronIndexEdit);
    pLayout->addStretch();
    return pLayout;
}

//! Create the group of widgets to edit local coordinates of the aerodynamic trapezium
QGroupBox* AeroTrapeziumEditor::createLocalGroupBox()
{
    QStringList const kColumnNames = {"X", "Y", "Z"};

    // Create editors
    QGridLayout* pLayout = new QGridLayout;
    int numLocals = mLocal0Edits.size();
    for (int i = 0; i != numLocals; ++i)
    {
        mLocal0Edits[i] = new Edit1d;
        mLocal1Edits[i] = new Edit1d;
        mLocal2Edits[i] = new Edit1d;
    }

    // Add the widgets to the layout
    auto indices = getGlobalIndices(mpElement->type());
    for (int i = 0; i != skNumPoints; ++i)
        pLayout->addWidget(new QLabel(getPointName(i)), 1 + i, 0);
    for (int i = 0; i != numLocals; ++i)
    {
        pLayout->addWidget(new QLabel(kColumnNames[indices[i]]), 0, 1 + i, Qt::AlignCenter);
        pLayout->addWidget(mLocal0Edits[i], 1, 1 + i);
        pLayout->addWidget(mLocal1Edits[i], 2, 1 + i);
    }
    pLayout->addWidget(mLocal2Edits[0], 3, 1);
    pLayout->addWidget(mLocal2Edits[1], 4, 1);

    // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Local coordinates"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Create the group of widgets to edit global coordinates of the aerodynamic trapezium
QGroupBox* AeroTrapeziumEditor::createGlobalGroupBox()
{
    QStringList const kColumnNames = {"X", "Y", "Z"};

    QGridLayout* pLayout = new QGridLayout;

    // Create editors for coodinates
    int numGlobals = mGlobal0Edits.size();
    for (int i = 0; i != numGlobals; ++i)
    {
        mGlobal0Edits[i] = new Edit1d;
        mGlobal1Edits[i] = new Edit1d;
    }
    mGlobal2Edits[0] = new Edit1d;
    mGlobal2Edits[1] = new Edit1d;

    // Add the widgets to the layout
    for (int i = 0; i != skNumPoints; ++i)
        pLayout->addWidget(new QLabel(getPointName(i)), 1 + i, 0);
    for (int i = 0; i != numGlobals; ++i)
    {
        pLayout->addWidget(new QLabel(kColumnNames[i]), 0, 1 + i, Qt::AlignCenter);
        pLayout->addWidget(mGlobal0Edits[i], 1, 1 + i, Qt::AlignCenter);
        pLayout->addWidget(mGlobal1Edits[i], 2, 1 + i, Qt::AlignCenter);
    }
    pLayout->addWidget(mGlobal2Edits[0], 3, 1, Qt::AlignCenter);
    pLayout->addWidget(mGlobal2Edits[1], 4, 1, Qt::AlignCenter);

    // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Global coordinates"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Create the group of widgets to edit grid parameters
QGroupBox* AeroTrapeziumEditor::createMeshGroupBox()
{
    QHBoxLayout* pLayout = new QHBoxLayout;

    // Create the edits
    mpNumPanelsEdit = new Edit1i;
    mpNumStripsEdit = new Edit1i;
    mpNumPanelsEdit->setMinimum(1);
    mpNumStripsEdit->setMinimum(1);

    // Add the widgets to the layout
    pLayout->addWidget(new QLabel(tr("Number of strips: ")));
    pLayout->addWidget(mpNumStripsEdit);
    pLayout->addWidget(new QLabel(tr("Number of panels: ")));
    pLayout->addWidget(mpNumPanelsEdit);
    pLayout->addStretch();

    // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Mesh parameters"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Create the group of widgets to edit factors
QGroupBox* AeroTrapeziumEditor::createFactorsGroupBox()
{
    QHBoxLayout* pLayout = new QHBoxLayout;

    // Create the edits
    mpStiffnessFactorEdit = new Edit1d;
    mpDampingFactorEdit = new Edit1d;

    // Add the widgets to the layout
    pLayout->addWidget(new QLabel(tr("Stiffness: ")));
    pLayout->addWidget(mpStiffnessFactorEdit);
    pLayout->addWidget(new QLabel(tr("Damping: ")));
    pLayout->addWidget(mpDampingFactorEdit);
    pLayout->addStretch();

    // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Factors"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Helper function to create point names using their indices
QString getPointName(int index)
{
    return QString("P<sub>%1</sub>").arg(index);
}

//! Helper function to map local indices to global ones
Vector2i getGlobalIndices(KCL::ElementType type)
{
    if (type == KCL::DA)
        return {0, 1};
    else
        return {0, 2};
}

//! Helper function to check if trapezium can have factors
bool isFactors(KCL::ElementType type)
{
    return type == KCL::GS;
}
