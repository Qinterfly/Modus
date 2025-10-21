#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include "lineedit.h"
#include "paneleditor.h"
#include "uiutility.h"

using namespace Frontend;
using namespace Eigen;

// Helper functions
bool isOrthotropic(KCL::ElementType type);
int countDepths(KCL::ElementType type);

PanelEditor::PanelEditor(KCL::ElasticSurface const& surface, KCL::AbstractElement* pElement, QString const& name, QWidget* pParent)
    : Editor(kPanel, name, Utility::getIcon(pElement->type()), pParent)
    , mTransform(Utility::computeTransformation(surface))
    , mpElement(pElement)
{
    createContent();
    createConnections();
    PanelEditor::refresh();
}

QSize PanelEditor::sizeHint() const
{
    return QSize(680, 350);
}

//! Create all the widgets
void PanelEditor::createContent()
{
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Create the thickness layout
    QHBoxLayout* pThicknessLayout = new QHBoxLayout;
    mpThicknessEdit = new Edit1d;
    mpThicknessEdit->setMinimum(0);
    pThicknessLayout->addWidget(new QLabel(tr("Thickness: ")));
    pThicknessLayout->addWidget(mpThicknessEdit);
    pThicknessLayout->addStretch(1);

    // Create the widgets to edit coordinates
    QHBoxLayout* pCoordsLayout = new QHBoxLayout;
    pCoordsLayout->addWidget(createLocalGroupBox());
    pCoordsLayout->addWidget(createGlobalGroupBox());

    // Set the main layout
    pMainLayout->addLayout(pThicknessLayout);
    pMainLayout->addLayout(pCoordsLayout);
    pMainLayout->addWidget(createDepthGroupBox());
    pMainLayout->addWidget(createMaterialGroupBox());
    pMainLayout->addStretch();
    setLayout(pMainLayout);
}

//! Update the widgets from the element source
void PanelEditor::refresh()
{
    // Slice element data
    KCL::VecN data = mpElement->get();
    int iData = 0;

    // Set thickness
    QSignalBlocker blockerThickness(mpThicknessEdit);
    mpThicknessEdit->setValue(data[iData]);
    ++iData;

    // Set coordinates
    int numPoints = mLocalEdits.size();
    for (int i = 0; i != numPoints; ++i)
    {
        // Retrieve the local position
        Vector2d position = {data[iData], data[iData + 1]};

        // Slice editors
        Edits2d& pointLocalEdits = mLocalEdits[i];
        Edits3d& pointGlobalEdits = mGlobalEdits[i];

        // Assign local coordinates
        int numLocals = pointLocalEdits.size();
        for (int j = 0; j != numLocals; ++j)
        {
            QSignalBlocker blocker(pointLocalEdits[j]);
            pointLocalEdits[j]->setValue(position[j]);
        }

        // Assign global coordinates
        Utility::setGlobalByLocalEdits(mTransform, pointLocalEdits, pointGlobalEdits);

        // Increase the counter
        iData += 2;
    }

    // Set depths
    int numDepths = countDepths(mpElement->type());
    for (int i = 0; i != numDepths; ++i)
    {
        QSignalBlocker blocker(mDepthEdits[i]);
        mDepthEdits[i]->setValue(data[iData]);
        ++iData;
    }

    // Set common material properties
    QSignalBlocker blockerYoungusModulus1(mpYoungsModulus1Edit);
    QSignalBlocker blockerDensity(mpDensityEdit);
    mpYoungsModulus1Edit->setValue(data[iData]);
    mpDensityEdit->setValue(data[iData + 1]);
    iData += 2;

    // Set orthotropic material properties
    bool isOrtho = isOrthotropic(mpElement->type());
    if (isOrtho)
    {
        QSignalBlocker blockerShearModulus(mpShearModulusEdit);
        QSignalBlocker blockerPoissonRatio(mpPoissonRatioEdit);
        QSignalBlocker blockerAngleE1Z(mpAngleE1ZEdit);
        QSignalBlocker blockerYoungusModulus2(mpYoungsModulus2Edit);
        mpShearModulusEdit->setValue(data[iData]);
        mpPoissonRatioEdit->setValue(data[iData + 1]);
        mpAngleE1ZEdit->setValue(data[iData + 2]);
        mpYoungsModulus2Edit->setValue(data[iData + 3]);
    }
}

//! Specify the widget connections
void PanelEditor::createConnections()
{
    // Local coordinates
    int numPoints = mLocalEdits.size();
    for (int i = 0; i != numPoints; ++i)
    {
        int numLocals = mLocalEdits[i].size();
        for (int j = 0; j != numLocals; ++j)
        {
            connect(mLocalEdits[i][j], &Edit1d::valueChanged, this, &PanelEditor::setGlobalByLocal);
            connect(mLocalEdits[i][j], &Edit1d::valueChanged, this, &PanelEditor::setElementData);
        }
    }

    // Global coordinates
    for (int i = 0; i != numPoints; ++i)
    {
        int numGlobals = mGlobalEdits[i].size();
        for (int j = 0; j != numGlobals; ++j)
            connect(mGlobalEdits[i][j], &Edit1d::valueChanged, this, &PanelEditor::setLocalByGlobal);
    }

    // Depths
    int numDepths = mDepthEdits.size();
    for (int i = 0; i != numDepths; ++i)
        connect(mDepthEdits[i], &Edit1d::valueChanged, this, &PanelEditor::setElementData);

    // Common material properties
    connect(mpYoungsModulus1Edit, &Edit1d::valueChanged, this, &PanelEditor::setElementData);
    connect(mpDensityEdit, &Edit1d::valueChanged, this, &PanelEditor::setElementData);

    // Orthotropic material properties
    bool isOrtho = isOrthotropic(mpElement->type());
    if (isOrtho)
    {
        connect(mpYoungsModulus2Edit, &Edit1d::valueChanged, this, &PanelEditor::setElementData);
        connect(mpShearModulusEdit, &Edit1d::valueChanged, this, &PanelEditor::setElementData);
        connect(mpPoissonRatioEdit, &Edit1d::valueChanged, this, &PanelEditor::setElementData);
        connect(mpAngleE1ZEdit, &Edit1d::valueChanged, this, &PanelEditor::setElementData);
    }
}

//! Set global coordinates by the local ones
void PanelEditor::setGlobalByLocal()
{
    int numPoints = mLocalEdits.size();
    for (int i = 0; i != numPoints; ++i)
        Utility::setGlobalByLocalEdits(mTransform, mLocalEdits[i], mGlobalEdits[i]);
}

//! Set local coordinates by the global ones
void PanelEditor::setLocalByGlobal()
{
    int numPoints = mLocalEdits.size();
    for (int i = 0; i != numPoints; ++i)
        Utility::setLocalByGlobalEdits(mTransform, mLocalEdits[i], mGlobalEdits[i]);
    setElementData();
}

//! Slice data from widgets to set element data
void PanelEditor::setElementData()
{
    // Slice the current data
    KCL::VecN data = mpElement->get();
    int iData = 0;

    // Set the thickness
    data[iData] = mpThicknessEdit->value();
    ++iData;

    // Set coordinates
    int numPoints = mLocalEdits.size();
    for (int i = 0; i != numPoints; ++i)
    {
        int numLocals = mLocalEdits[i].size();
        for (int j = 0; j != numLocals; ++j)
        {
            data[iData] = mLocalEdits[i][j]->value();
            ++iData;
        }
    }

    // Set depths
    int numDepths = countDepths(mpElement->type());
    for (int i = 0; i != numDepths; ++i)
    {
        data[iData] = mDepthEdits[i]->value();
        ++iData;
    }

    // Set common material properties
    data[iData] = mpYoungsModulus1Edit->value();
    data[iData + 1] = mpDensityEdit->value();
    iData += 2;

    // Set orthotropic material properties
    bool isOrtho = isOrthotropic(mpElement->type());
    if (isOrtho)
    {
        data[iData] = mpShearModulusEdit->value();
        data[iData + 1] = mpPoissonRatioEdit->value();
        data[iData + 2] = mpAngleE1ZEdit->value();
        data[iData + 3] = mpYoungsModulus2Edit->value();
    }

    // Set the updated data
    emit commandExecuted(new EditElements(mpElement, data, name()));
}

//! Create the group of widgets to edit local coordinates of the panel
QGroupBox* PanelEditor::createLocalGroupBox()
{
    QStringList const kColumnNames = {tr("X"), tr("Z")};

    QGridLayout* pLayout = new QGridLayout;

    // Set the header
    int numCoords = kColumnNames.size();
    for (int i = 0; i != numCoords; ++i)
        pLayout->addWidget(new QLabel(kColumnNames[i]), 0, 1 + i, Qt::AlignCenter);

    // Set the editors
    int numPoints = mLocalEdits.size();
    for (int i = 0; i != numPoints; ++i)
    {
        QString label = tr("P<sub>%1</sub>").arg(i);
        pLayout->addWidget(new QLabel(label), 1 + i, 0);
        for (int j = 0; j != numCoords; ++j)
        {
            mLocalEdits[i][j] = new Edit1d;
            pLayout->addWidget(mLocalEdits[i][j], 1 + i, 1 + j);
        }
    }

    // // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Local coordinates"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Create the group of widgets to edit global coordinates of the panel
QGroupBox* PanelEditor::createGlobalGroupBox()
{
    QStringList const kColumnNames = {tr("X"), tr("Y"), tr("Z")};

    QGridLayout* pLayout = new QGridLayout;

    // Set the header
    int numCoords = kColumnNames.size();
    for (int i = 0; i != numCoords; ++i)
        pLayout->addWidget(new QLabel(kColumnNames[i]), 0, 1 + i, Qt::AlignCenter);

    // Set the editors
    int numPoints = mGlobalEdits.size();
    for (int i = 0; i != numPoints; ++i)
    {
        QString label = tr("P<sub>%1</sub>").arg(i);
        pLayout->addWidget(new QLabel(label), 1 + i, 0);
        for (int j = 0; j != numCoords; ++j)
        {
            mGlobalEdits[i][j] = new Edit1d;
            pLayout->addWidget(mGlobalEdits[i][j], 1 + i, 1 + j);
        }
    }

    // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Global coordinates"));
    pGroupBox->setLayout(pLayout);
    return pGroupBox;
}

//! Create the group of widgets to edit depths of the panel
QGroupBox* PanelEditor::createDepthGroupBox()
{
    // Count the number of depths associated with the element
    int numDepths = countDepths(mpElement->type());

    // Set the editors
    QGridLayout* pLayout = new QGridLayout;
    mDepthEdits.resize(numDepths);
    for (int i = 0; i != numDepths; ++i)
    {
        QString label = tr("H<sub>%1</sub>").arg(i);
        mDepthEdits[i] = new Edit1d;
        pLayout->addWidget(new QLabel(label), 0, i, Qt::AlignCenter);
        pLayout->addWidget(mDepthEdits[i], 1, i, Qt::AlignCenter);
    }

    // Create the widget and set the layout
    QGroupBox* pGroupBox = new QGroupBox(tr("Depths"));
    QHBoxLayout* pMainLayout = new QHBoxLayout;
    pMainLayout->addLayout(pLayout);
    pMainLayout->addStretch();
    pGroupBox->setLayout(pMainLayout);
    return pGroupBox;
}

//! Create the group of widgets to edit material properties of the panel
QGroupBox* PanelEditor::createMaterialGroupBox()
{
    QChar const kRhoSymbol = QChar(0x03C1);
    QChar const kAngleSymbol = QChar(0x2220);

    // Initialize the pointers
    mpYoungsModulus1Edit = nullptr;
    mpYoungsModulus2Edit = nullptr;
    mpShearModulusEdit = nullptr;
    mpPoissonRatioEdit = nullptr;
    mpAngleE1ZEdit = nullptr;
    mpDensityEdit = nullptr;

    // Create the base editors
    mpYoungsModulus1Edit = new Edit1d;
    mpDensityEdit = new Edit1d;

    // Create the orthotropic editors
    bool isComposite = isOrthotropic(mpElement->type());
    if (isComposite)
    {
        mpYoungsModulus2Edit = new Edit1d;
        mpShearModulusEdit = new Edit1d;
        mpPoissonRatioEdit = new Edit1d;
        mpAngleE1ZEdit = new Edit1d;
        mpPoissonRatioEdit->setRange(0, 1);
        mpAngleE1ZEdit->setRange(-90, 90);
    }

    // Construct the layout
    QGridLayout* pLayout = new QGridLayout;
    if (isComposite)
    {
        pLayout->addWidget(new QLabel(tr("E<sub>1</sub>: ")), 0, 0);
        pLayout->addWidget(mpYoungsModulus1Edit, 0, 1);
        pLayout->addWidget(new QLabel(tr("E<sub>2</sub>: ")), 0, 2);
        pLayout->addWidget(mpYoungsModulus2Edit, 0, 3);
        pLayout->addWidget(new QLabel(tr("G: ")), 0, 4);
        pLayout->addWidget(mpShearModulusEdit, 0, 5);
        pLayout->addWidget(new QLabel(kAngleSymbol + tr("(OZ, E<sub>1</sub>)°: ")), 1, 0);
        pLayout->addWidget(mpAngleE1ZEdit, 1, 1);
        pLayout->addWidget(new QLabel(QString("%1:").arg(kRhoSymbol)), 1, 2);
        pLayout->addWidget(mpDensityEdit, 1, 3);
    }
    else
    {
        pLayout->addWidget(new QLabel(tr("E<sub>1</sub>: ")), 0, 0);
        pLayout->addWidget(mpYoungsModulus1Edit, 0, 1);
        pLayout->addWidget(new QLabel(QString("%1:").arg(kRhoSymbol)), 0, 2);
        pLayout->addWidget(mpDensityEdit, 0, 3);
    }

    // Create the widget
    QGroupBox* pGroupBox = new QGroupBox(tr("Material"));
    QHBoxLayout* pMainLayout = new QHBoxLayout;
    pMainLayout->addLayout(pLayout);
    pMainLayout->addStretch();
    pGroupBox->setLayout(pMainLayout);
    return pGroupBox;
}

//! Helper function to check if materis is orthotropic
bool isOrthotropic(KCL::ElementType type)
{
    return type == KCL::P4 || type == KCL::OP;
}

//! Helper function to estimate number of panel depths
int countDepths(KCL::ElementType type)
{
    if (type == KCL::PN || type == KCL::OP)
        return 3;
    else if (type == KCL::P4)
        return 4;
    return 0;
}
