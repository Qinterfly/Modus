#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include "beameditor.h"
#include "lineedit.h"
#include "uiutility.h"

using namespace Frontend;
using namespace Eigen;

// Helper functions
int countValues(KCL::ElementType type);
QString getStiffnessPrefix(KCL::ElementType type);
QString getInertiaPrefix(KCL::ElementType type);

BeamEditor::BeamEditor(KCL::ElasticSurface const& surface, KCL::AbstractElement* pElement, QString const& name, QWidget* pParent)
    : Editor(kBeam, name, Utility::getIcon(pElement->type()), pParent)
    , mTransform(Utility::computeTransformation(surface))
    , mpElement(pElement)
{
    createContent();
    createConnections();
    BeamEditor::refresh();
}

QSize BeamEditor::sizeHint() const
{
    return QSize(640, 350);
}

//! Create all the widgets
void BeamEditor::createContent()
{
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Create the widgets to edit coordinates
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->addWidget(createLocalGroupBox());
    pLayout->addWidget(createGlobalGroupBox());
    pMainLayout->addLayout(pLayout);

    // Create the widgets to edit stiffness
    QGroupBox* pGroupBox = createStifnessGroupBox();
    if (pGroupBox)
        pMainLayout->addWidget(pGroupBox);

    // Create the widgets to edit inertia
    pGroupBox = createInertiaGroupBox();
    if (pGroupBox)
        pMainLayout->addWidget(pGroupBox);

    // Set the main layout
    pMainLayout->addStretch();
    setLayout(pMainLayout);
}

//! Update the widgets from the element source
void BeamEditor::refresh()
{
    // Slice element data
    KCL::VecN data = mpElement->get();

    // Set local coordinates
    Vector2d startPosition = {data[0], data[1]};
    Vector2d endPosition = {data[2], data[3]};
    int numCoords = startPosition.size();
    for (int i = 0; i != numCoords; ++i)
    {
        QSignalBlocker blockerStart(mStartLocalEdits[i]);
        QSignalBlocker blockerEnd(mEndLocalEdits[i]);
        mStartLocalEdits[i]->setValue(startPosition[i]);
        mEndLocalEdits[i]->setValue(endPosition[i]);
    }

    // Set global coordinates
    setGlobalByLocal();

    // Set stiffness and inertia values
    int numValues = mStiffnessEdits.size();
    for (int i = 0; i != numValues; ++i)
    {
        QSignalBlocker blockerStiffness(mStiffnessEdits[i]);
        QSignalBlocker blockerInertia(mInertiaEdits[i]);
        mStiffnessEdits[i]->setValue(data[4 + i]);
        mInertiaEdits[i]->setValue(data[4 + numValues + i]);
    }
}

//! Specify the widget connections
void BeamEditor::createConnections()
{
    // Local coordinates
    int numLocals = mStartLocalEdits.size();
    for (int i = 0; i != numLocals; ++i)
    {
        connect(mStartLocalEdits[i], &DoubleLineEdit::valueChanged, this, &BeamEditor::setGlobalByLocal);
        connect(mEndLocalEdits[i], &DoubleLineEdit::valueChanged, this, &BeamEditor::setGlobalByLocal);
        connect(mStartLocalEdits[i], &DoubleLineEdit::valueChanged, this, &BeamEditor::setElementData);
        connect(mEndLocalEdits[i], &DoubleLineEdit::valueChanged, this, &BeamEditor::setElementData);
    }
    // Global coordinates
    int numGlobals = mStartGlobalEdits.size();
    for (int i = 0; i != numGlobals; ++i)
    {
        connect(mStartGlobalEdits[i], &DoubleLineEdit::valueChanged, this, &BeamEditor::setLocalByGlobal);
        connect(mEndGlobalEdits[i], &DoubleLineEdit::valueChanged, this, &BeamEditor::setLocalByGlobal);
    }
    // Stiffness and inertia values
    int numValues = mStiffnessEdits.size();
    for (int i = 0; i != numValues; ++i)
    {
        connect(mStiffnessEdits[i], &DoubleLineEdit::valueChanged, this, &BeamEditor::setElementData);
        connect(mInertiaEdits[i], &DoubleLineEdit::valueChanged, this, &BeamEditor::setElementData);
    }
}

//! Set global coordinates by the local ones
void BeamEditor::setGlobalByLocal()
{
    // Compute the positions
    auto startPosition = mTransform * Vector3d({mStartLocalEdits[0]->value(), 0.0, mStartLocalEdits[1]->value()});
    auto endPosition = mTransform * Vector3d({mEndLocalEdits[0]->value(), 0.0, mEndLocalEdits[1]->value()});

    // Set the positions
    int numCoords = startPosition.size();
    for (int i = 0; i != numCoords; ++i)
    {
        QSignalBlocker blockerStart(mStartGlobalEdits[i]);
        QSignalBlocker blockerEnd(mEndGlobalEdits[i]);
        mStartGlobalEdits[i]->setValue(startPosition[i]);
        mEndGlobalEdits[i]->setValue(endPosition[i]);
    }
}

//! Set local coordinates by the global ones
void BeamEditor::setLocalByGlobal()
{
    // Constants
    QList<int> const kMapIndices = {0, 2};

    // Compute the positions
    auto invTransform = mTransform.inverse();
    auto startPosition = invTransform * Vector3d({mStartGlobalEdits[0]->value(), mStartGlobalEdits[1]->value(), mStartGlobalEdits[2]->value()});
    auto endPosition = invTransform * Vector3d({mEndGlobalEdits[0]->value(), mEndGlobalEdits[1]->value(), mEndGlobalEdits[2]->value()});

    // Set the positions
    int numLocals = mStartLocalEdits.size();
    for (int i = 0; i != numLocals; ++i)
    {
        QSignalBlocker blockerStart(mStartLocalEdits[i]);
        QSignalBlocker blockerEnd(mEndLocalEdits[i]);
        int iSlice = kMapIndices[i];
        mStartLocalEdits[i]->setValue(startPosition[iSlice]);
        mEndLocalEdits[i]->setValue(endPosition[iSlice]);
    }

    // Update element data
    setElementData();
}

//! Create the group of widgets to edit local coordinates of the beam
QGroupBox* BeamEditor::createLocalGroupBox()
{
    QStringList const kLabels = {tr("X"), tr("Z")};
    QGroupBox* pGroupBox = new QGroupBox(tr("Local coordinates"));
    QGridLayout* pLayout = new QGridLayout;
    pLayout->addWidget(new QLabel(tr("Start: ")), 1, 0);
    pLayout->addWidget(new QLabel(tr("End: ")), 2, 0);
    int numCoords = mStartLocalEdits.size();
    for (int i = 0; i != numCoords; ++i)
    {
        mStartLocalEdits[i] = new DoubleLineEdit;
        mEndLocalEdits[i] = new DoubleLineEdit;
        pLayout->addWidget(new QLabel(kLabels[i]), 0, 1 + i, Qt::AlignCenter);
        pLayout->addWidget(mStartLocalEdits[i], 1, 1 + i);
        pLayout->addWidget(mEndLocalEdits[i], 2, 1 + i);
    }
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Slice data from widgets to set element data
void BeamEditor::setElementData()
{
    // Slice the current data
    KCL::VecN data = mpElement->get();

    // Set the coordinates
    int numCoords = mStartLocalEdits.size();
    for (int i = 0; i != numCoords; ++i)
    {
        data[i] = mStartLocalEdits[i]->value();
        data[numCoords + i] = mEndLocalEdits[i]->value();
    }

    // Set the stiffness and inertia values
    int numValues = mStiffnessEdits.size();
    for (int i = 0; i != numValues; ++i)
    {
        data[4 + i] = mStiffnessEdits[i]->value();
        data[4 + numValues + i] = mInertiaEdits[i]->value();
    }

    // Set the updated data
    emit commandExecuted(new EditElement(mpElement, data, name()));
}

//! Create the group of widgets to edit global coordinates of the beam
QGroupBox* BeamEditor::createGlobalGroupBox()
{
    QStringList const kLabels = {tr("X"), tr("Y"), tr("Z")};
    QGroupBox* pGroupBox = new QGroupBox(tr("Global coordinates"));
    QGridLayout* pLayout = new QGridLayout;
    pLayout->addWidget(new QLabel(tr("Start: ")), 1, 0);
    pLayout->addWidget(new QLabel(tr("End: ")), 2, 0);
    int numCoords = mStartGlobalEdits.size();
    for (int i = 0; i != numCoords; ++i)
    {
        mStartGlobalEdits[i] = new DoubleLineEdit;
        mEndGlobalEdits[i] = new DoubleLineEdit;
        pLayout->addWidget(new QLabel(kLabels[i]), 0, 1 + i, Qt::AlignCenter);
        pLayout->addWidget(mStartGlobalEdits[i], 1, 1 + i);
        pLayout->addWidget(mEndGlobalEdits[i], 2, 1 + i);
    }
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Create the group of widgets to edit stiffness properties
QGroupBox* BeamEditor::createStifnessGroupBox()
{
    KCL::ElementType type = mpElement->type();
    int numValues = countValues(type);
    if (numValues == 0)
        return nullptr;
    QGroupBox* pGroupBox = new QGroupBox(tr("Stiffness"));
    QGridLayout* pLayout = new QGridLayout;
    mStiffnessEdits.resize(numValues);
    QString prefix = getStiffnessPrefix(type);
    for (int i = 0; i != numValues; ++i)
    {
        mStiffnessEdits[i] = new DoubleLineEdit;
        QString label = QString("%1<sub>%2</sub>").arg(prefix).arg(1 + i);
        pLayout->addWidget(new QLabel(label), 0, i, Qt::AlignCenter);
        pLayout->addWidget(mStiffnessEdits[i], 1, i);
    }
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Create the group of widgets to edit inertia properties
QGroupBox* BeamEditor::createInertiaGroupBox()
{
    KCL::ElementType type = mpElement->type();
    int numValues = countValues(type);
    if (numValues == 0)
        return nullptr;
    QGroupBox* pGroupBox = new QGroupBox(tr("Inertia"));
    QGridLayout* pLayout = new QGridLayout;
    mInertiaEdits.resize(numValues);
    QString prefix = getInertiaPrefix(type);
    for (int i = 0; i != numValues; ++i)
    {
        mInertiaEdits[i] = new DoubleLineEdit;
        QString label = QString("%1<sub>%2</sub>").arg(prefix).arg(1 + i);
        pLayout->addWidget(new QLabel(label), 0, i, Qt::AlignCenter);
        pLayout->addWidget(mInertiaEdits[i], 1, i);
    }
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Helper function to estimate number of element entities
int countValues(KCL::ElementType type)
{
    if (type == KCL::BI || type == KCL::BK || type == KCL::DB)
        return 4;
    else if (type == KCL::ST)
        return 1;
    return 0;
}

//! Helper function to get element stiffnes prefix to display
QString getStiffnessPrefix(KCL::ElementType type)
{
    if (type == KCL::BI || type == KCL::DB)
        return "EJ";
    else if (type == KCL::BK)
        return "GJ";
    else if (type == KCL::ST)
        return "TU";
    return QString();
}

//! Helper function to get element stiffnes prefix to display
QString getInertiaPrefix(KCL::ElementType type)
{
    if (type == KCL::BI || type == KCL::DB || type == KCL::ST)
        return "M";
    else if (type == KCL::BK)
        return "J";
    return QString();
}
