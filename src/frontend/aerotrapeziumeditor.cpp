#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include <kcl/model.h>

#include "aerotrapeziumeditor.h"
#include "lineedit.h"
#include "uiutility.h"

using namespace Frontend;

static int const skNumPoints = 4;

QString getPointName(int index);

AeroTrapeziumEditor::AeroTrapeziumEditor(KCL::ElasticSurface const& surface, KCL::AbstractElement* pElement, QString const& name,
                                         QWidget* pParent)
    : Editor(Editor::kAeroTrapezium, name, Utility::getIcon(pElement->type()), pParent)
    , mTransform(Utility::computeTransformation(surface))
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
    // TODO
}

//! Create all the widgets
void AeroTrapeziumEditor::createContent()
{
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Create the widgets to edit coordinates
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->addWidget(createLocalGroupBox());
    pLayout->addWidget(createGlobalGroupBox());
    pMainLayout->addLayout(pLayout);

    // TODO

    // Set the main layout
    pMainLayout->addStretch();
    setLayout(pMainLayout);
}

//! Specify signals and slots between widgets
void AeroTrapeziumEditor::createConnections()
{
    // TODO
}

//! Slice data from widgets to set element data
void AeroTrapeziumEditor::setElementData()
{
    // TODO
}

//! Create the group of widgets to edit local coordinates of the aerodynamic trapezium
QGroupBox* AeroTrapeziumEditor::createLocalGroupBox()
{
    // Construct names for coordinate directions
    QStringList directionNames;
    if (mpElement->type() == KCL::AE)
        directionNames = {"X", "Z"};
    else
        directionNames = {"X", "Y"};

    // Create editors
    QGridLayout* pLayout = new QGridLayout;
    int numLocals = mLocal0Edits.size();
    for (int i = 0; i != numLocals; ++i)
    {
        mLocal0Edits[i] = new Edit1d;
        mLocal1Edits[i] = new Edit1d;
    }
    mpLocal2Edit = new Edit1d;
    mpLocal3Edit = new Edit1d;

    // Add the widgets to the layout
    for (int i = 0; i != skNumPoints; ++i)
        pLayout->addWidget(new QLabel(getPointName(i)), 1 + i, 0);
    for (int i = 0; i != numLocals; ++i)
    {
        pLayout->addWidget(new QLabel(directionNames[i]), 0, 1 + i, Qt::AlignCenter);
        pLayout->addWidget(mLocal0Edits[i], 1, 1 + i);
        pLayout->addWidget(mLocal1Edits[i], 2, 1 + i);
    }
    pLayout->addWidget(mpLocal2Edit, 3, 1);
    pLayout->addWidget(mpLocal3Edit, 4, 1);

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
    mpGlobal2Edit = new Edit1d;
    mpGlobal3Edit = new Edit1d;

    // Add the widgets to the layout
    for (int i = 0; i != skNumPoints; ++i)
        pLayout->addWidget(new QLabel(getPointName(i)), 1 + i, 0);
    for (int i = 0; i != numGlobals; ++i)
    {
        pLayout->addWidget(new QLabel(kColumnNames[i]), 0, 1 + i, Qt::AlignCenter);
        pLayout->addWidget(mGlobal0Edits[i], 1, 1 + i, Qt::AlignCenter);
        pLayout->addWidget(mGlobal1Edits[i], 2, 1 + i, Qt::AlignCenter);
    }
    pLayout->addWidget(mpGlobal2Edit, 3, 1, Qt::AlignCenter);
    pLayout->addWidget(mpGlobal3Edit, 4, 1, Qt::AlignCenter);

    // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Global coordinates"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Helper function to create point names using their indices
QString getPointName(int index)
{
    return QString("P<sub>%1</sub>").arg(index);
}
