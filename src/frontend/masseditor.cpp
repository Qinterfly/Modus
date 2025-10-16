#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include "lineedit.h"
#include "masseditor.h"
#include "uiutility.h"

using namespace Frontend;
using namespace Eigen;

bool is3D(KCL::ElementType type);

MassEditor::MassEditor(KCL::ElasticSurface const& surface, KCL::AbstractElement* pElement, QString const& name, QWidget* pParent)
    : Editor(kMass, name, Utility::getIcon(pElement->type()), pParent)
    , mTransform(Utility::computeTransformation(surface))
    , mpElement(pElement)
{
    createContent();
    createConnections();
    MassEditor::refresh();
}

QSize MassEditor::sizeHint() const
{
    return QSize(680, 350);
}

//! Create all the widgets
void MassEditor::createContent()
{
    // Constants
    QList<QString> const kLabels = {"I", "I<sub>0</sub>", "I<sub>y</sub>"};

    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Create the mass editor
    QHBoxLayout* pMassLayout = new QHBoxLayout;
    mpMassEdit = new Edit1d;
    pMassLayout->addWidget(new QLabel(tr("Mass (M): ")));
    pMassLayout->addWidget(mpMassEdit);
    pMassLayout->addStretch(1);
    pMainLayout->addLayout(pMassLayout);

    // Create the editors of inertia moments
    if (is3D(mpElement->type()))
    {
        QGridLayout* pInertiaLayout = new QGridLayout;
        int numEdits = mInertiaEdits.size();
        pInertiaLayout->addWidget(new QLabel(tr("Inertia moments: ")), 1, 0);
        for (int i = 0; i != numEdits; ++i)
        {
            mInertiaEdits[i] = new Edit1d;
            pInertiaLayout->addWidget(new QLabel(kLabels[i]), 0, 1 + i, Qt::AlignCenter);
            pInertiaLayout->addWidget(mInertiaEdits[i], 1, 1 + i);
        }
        pMainLayout->addLayout(pInertiaLayout);
    }
    else
    {
        QHBoxLayout* pInertiaLayout = new QHBoxLayout;
        mpInertiaEdit = new Edit1d;
        pInertiaLayout->addWidget(new QLabel(tr("Inertia moment (I): ")));
        pInertiaLayout->addWidget(mpInertiaEdit);
        pInertiaLayout->addStretch();
        pMainLayout->addLayout(pInertiaLayout);
    }

    // Create the widgets to edit coordinates
    QHBoxLayout* pCoordsLayout = new QHBoxLayout;
    pCoordsLayout->addWidget(createLocalGroupBox());
    pCoordsLayout->addWidget(createGlobalGroupBox());
    pMainLayout->addLayout(pCoordsLayout);

    // Create widgets to edit rod options
    QHBoxLayout* pRodLayout = new QHBoxLayout;
    mpLengthRodEdit = new Edit1d;
    mpAngleRodZEdit = new Edit1d;
    pRodLayout->addWidget(new QLabel(tr("Bracket length: ")));
    pRodLayout->addWidget(mpLengthRodEdit);
    pRodLayout->addWidget(new QLabel(tr("Angle between OZ and bracket: ")));
    pRodLayout->addWidget(mpAngleRodZEdit);
    pRodLayout->addStretch();
    pMainLayout->addLayout(pRodLayout);

    // Set the layout
    pMainLayout->addStretch();
    setLayout(pMainLayout);
}

//! Update the widgets from the element source
void MassEditor::refresh()
{
    // TODO
}

//! Specify the widget connections
void MassEditor::createConnections()
{
    // TODO
}

//! Set global coordinates by the local ones
void MassEditor::setGlobalByLocal()
{
    if (is3D(mpElement->type()))
        Utility::setGlobalByLocalEdits(mTransform, mLocalEdits3D, mGlobalEdits);
    else
        Utility::setGlobalByLocalEdits(mTransform, mLocalEdits2D, mGlobalEdits);
}

//! Set local coordinates by the global ones
void MassEditor::setLocalByGlobal()
{
    if (is3D(mpElement->type()))
        Utility::setLocalByGlobalEdits(mTransform, mLocalEdits3D, mGlobalEdits);
    else
        Utility::setLocalByGlobalEdits(mTransform, mLocalEdits2D, mGlobalEdits);
    setElementData();
}

//! Slice data from widgets to set element data
void MassEditor::setElementData()
{
    // TODO
}

//! Create the group of widgets to edit local coordinates
QGroupBox* MassEditor::createLocalGroupBox()
{
    QStringList const kLabels = {"X<sub>0</sub>", "Y<sub>0</sub>", "Z<sub>0</sub>"};

    // Process two- and three dimensional edits
    int numEdits = 0;
    Edit1d** edits = nullptr;
    QList<int> indicesLabels;
    if (is3D(mpElement->type()))
    {
        numEdits = mLocalEdits3D.size();
        edits = mLocalEdits3D.data();
        indicesLabels = {0, 1, 2};
    }
    else
    {
        numEdits = mLocalEdits2D.size();
        edits = mLocalEdits2D.data();
        indicesLabels = {0, 2};
    }

    // Create editors for coordinates
    QGridLayout* pLayout = new QGridLayout;
    for (int i = 0; i != numEdits; ++i)
    {
        edits[i] = new Edit1d;
        pLayout->addWidget(new QLabel(kLabels[indicesLabels[i]]), 0, i, Qt::AlignCenter);
        pLayout->addWidget(edits[i], 1, i);
    }

    // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Local coordinates"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Create the group of widgets to edit global coordinates
QGroupBox* MassEditor::createGlobalGroupBox()
{
    QStringList const kLabels = {"X<sub>0</sub>", "Y<sub>0</sub>", "Z<sub>0</sub>"};

    // Create editors for coordinates
    QGridLayout* pLayout = new QGridLayout;
    int numCoords = mGlobalEdits.size();
    for (int i = 0; i != numCoords; ++i)
    {
        mGlobalEdits[i] = new Edit1d;
        pLayout->addWidget(new QLabel(kLabels[i]), 0, i, Qt::AlignCenter);
        pLayout->addWidget(mGlobalEdits[i], 1, i);
    }

    // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Global coordinates"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}
//! Helper function to check whether element is three dimensional
bool is3D(KCL::ElementType type)
{
    return type == KCL::M3;
}
