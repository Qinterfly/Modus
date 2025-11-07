#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <kcl/model.h>

#include "customtable.h"
#include "lineedit.h"
#include "springdampereditor.h"
#include "uiutility.h"

using namespace Frontend;

static int const skGroundIndex = 0;

// Helper enumerations
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
    // Pairing widgets
    QSignalBlocker blockerIFirstSurface(mpIFirstSurfaceComboBox);
    QSignalBlocker blockerISecondSurface(mpISecondSurfaceComboBox);
    Utility::setIndexByKey(mpIFirstSurfaceComboBox, mpElement->iFirstSurface);
    Utility::setIndexByKey(mpISecondSurfaceComboBox, mpElement->iSecondSurface);
    bool isGround = mpElement->iSecondSurface == skGroundIndex;

    // Local coordinates
    int numLocals = mFirstLocalEdits.size();
    for (int i = 0; i != numLocals; ++i)
    {
        QSignalBlocker blockerFirstLocal(mFirstLocalEdits[i]);
        QSignalBlocker blockerSecondLocal(mSecondLocalEdits[i]);
        mFirstLocalEdits[i]->setValue(mpElement->coordsFirstRod[i]);
        mSecondLocalEdits[i]->setValue(mpElement->coordsSecondRod[i]);
        mSecondLocalEdits[i]->setReadOnly(isGround);
    }

    // Global coordinates
    setGlobalByLocal();
    int numGlobals = mSecondGlobalEdits.size();
    for (int i = 0; i != numGlobals; ++i)
        mSecondGlobalEdits[i]->setReadOnly(isGround);

    // Length
    QSignalBlocker blockerFirstLength(mpFirstLengthEdit);
    QSignalBlocker blockerSecondLength(mpSecondLengthEdit);
    mpFirstLengthEdit->setValue(mpElement->lengthFirstRod);
    mpSecondLengthEdit->setValue(mpElement->lengthSecondRod);

    // Angles
    int numAngles = mFirstAngleEdits.size();
    for (int i = 0; i != numAngles; ++i)
    {
        QSignalBlocker blockerFirstAngle(mFirstAngleEdits[i]);
        QSignalBlocker blockerSecondAngle(mSecondAngleEdits[i]);
        mFirstAngleEdits[i]->setValue(mpElement->anglesFirstRod[i]);
        mSecondAngleEdits[i]->setValue(mpElement->anglesSecondRod[i]);
    }

    // Orientation
    int numOrientation = mOrientationEdits.size();
    for (int i = 0; i != numOrientation; ++i)
    {
        QSignalBlocker blockerOrientation(mOrientationEdits[i]);
        mOrientationEdits[i]->setValue(mpElement->anglesCSys[i]);
    }

    // Type
    QSignalBlocker blockerType(mpTypeComboBox);
    Utility::setIndexByKey(mpTypeComboBox, mpElement->iSwitch);
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
    pLayout->addWidget(createMatrixGroupBox());

    // Set the layout
    pLayout->addStretch();
    setLayout(pLayout);
}

//! Specify the widget connections
void SpringDamperEditor::createConnections()
{
    // Pairing
    connect(mpIFirstSurfaceComboBox, &QComboBox::currentIndexChanged, this, &SpringDamperEditor::setSurfaceIndices);
    connect(mpISecondSurfaceComboBox, &QComboBox::currentIndexChanged, this, &SpringDamperEditor::setSurfaceIndices);

    // Surface local coordinates
    int numLocals = mFirstLocalEdits.size();
    for (int i = 0; i != numLocals; ++i)
    {
        connect(mFirstLocalEdits[i], &Edit1d::valueChanged, this, &SpringDamperEditor::setGlobalByLocal);
        connect(mFirstLocalEdits[i], &Edit1d::valueChanged, this, &SpringDamperEditor::setElementData);
        connect(mSecondLocalEdits[i], &Edit1d::valueChanged, this, &SpringDamperEditor::setGlobalByLocal);
        connect(mSecondLocalEdits[i], &Edit1d::valueChanged, this, &SpringDamperEditor::setElementData);
    }

    // Surface global coordinates
    int numGlobals = mFirstGlobalEdits.size();
    for (int i = 0; i != numGlobals; ++i)
    {
        connect(mFirstGlobalEdits[i], &Edit1d::valueChanged, this, &SpringDamperEditor::setLocalByGlobal);
        connect(mSecondGlobalEdits[i], &Edit1d::valueChanged, this, &SpringDamperEditor::setLocalByGlobal);
    }

    // Surface length
    connect(mpFirstLengthEdit, &Edit1d::valueChanged, this, &SpringDamperEditor::setElementData);
    connect(mpSecondLengthEdit, &Edit1d::valueChanged, this, &SpringDamperEditor::setElementData);

    // Surface angles
    int numAngles = mFirstAngleEdits.size();
    for (int i = 0; i != numAngles; ++i)
    {
        connect(mFirstAngleEdits[i], &Edit1d::valueChanged, this, &SpringDamperEditor::setElementData);
        connect(mSecondAngleEdits[i], &Edit1d::valueChanged, this, &SpringDamperEditor::setElementData);
    }

    // Orientation
    int numOrientation = mOrientationEdits.size();
    for (int i = 0; i != numOrientation; ++i)
        connect(mOrientationEdits[i], &Edit1d::valueChanged, this, &SpringDamperEditor::setElementData);

    // Type
    connect(mpTypeComboBox, &QComboBox::currentIndexChanged, this, &SpringDamperEditor::setElementData);

    // Matrices
    connect(mpStiffnessButton, &QPushButton::clicked, this, [this]() { showMatrixEditor(true); });
    connect(mpDampingButton, &QPushButton::clicked, this, [this]() { showMatrixEditor(false); });
}

//! Set global coordinates by the local ones
void SpringDamperEditor::setGlobalByLocal()
{
    // First surface
    int iFirstSurface = mpIFirstSurfaceComboBox->currentData().toInt();
    auto firstTransform = Utility::computeTransformation(mSurfaces[iFirstSurface - 1]);
    Utility::setGlobalByLocalEdits(firstTransform, mFirstLocalEdits, mFirstGlobalEdits);

    // Second surface
    int iSecondSurface = mpISecondSurfaceComboBox->currentData().toInt();
    if (iSecondSurface > 0)
    {
        auto secondTransform = Utility::computeTransformation(mSurfaces[iSecondSurface - 1]);
        Utility::setGlobalByLocalEdits(secondTransform, mSecondLocalEdits, mSecondGlobalEdits);
    }
}

//! Set local coordinates by the global ones
void SpringDamperEditor::setLocalByGlobal()
{
    // First surface
    int iFirstSurface = mpIFirstSurfaceComboBox->currentData().toInt();
    auto firstTransform = Utility::computeTransformation(mSurfaces[iFirstSurface - 1]);
    Utility::setLocalByGlobalEdits(firstTransform, mFirstLocalEdits, mFirstGlobalEdits);

    // Second surface
    int iSecondSurface = mpISecondSurfaceComboBox->currentData().toInt();
    if (iSecondSurface > 0)
    {
        auto secondTransform = Utility::computeTransformation(mSurfaces[iSecondSurface - 1]);
        Utility::setLocalByGlobalEdits(secondTransform, mSecondLocalEdits, mSecondGlobalEdits);
    }

    // Update the element data
    setElementData();
}

//! Slice data from widgets to set element data
void SpringDamperEditor::setElementData()
{
    // Slice element data
    KCL::VecN data = mpElement->get();

    // Pairing
    data[0] = mpIFirstSurfaceComboBox->currentData().toInt();
    data[6] = mpISecondSurfaceComboBox->currentData().toInt();

    // Local coordinates
    int numLocals = mFirstLocalEdits.size();
    for (int i = 0; i != numLocals; ++i)
    {
        data[1 + i] = mFirstLocalEdits[i]->value();
        data[7 + i] = mSecondLocalEdits[i]->value();
    }

    // Length
    data[3] = mpFirstLengthEdit->value();
    data[9] = mpSecondLengthEdit->value();

    // Angles
    int numAngles = mFirstAngleEdits.size();
    for (int i = 0; i != numAngles; ++i)
    {
        data[4 + i] = mFirstAngleEdits[i]->value();
        data[10 + i] = mSecondAngleEdits[i]->value();
    }

    // Orientation
    int numOrientation = mOrientationEdits.size();
    for (int i = 0; i != numOrientation; ++i)
        data[12 + i] = mOrientationEdits[i]->value();

    // Type
    data[15] = mpTypeComboBox->currentData().toInt();

    // Set the updated data
    emit commandExecuted(new EditElements(mpElement, data, name(), this));
}

//! Process selection of an elastic surface for pairing
void SpringDamperEditor::setSurfaceIndices()
{
    setElementData();
    refresh();
}

//! Modify the matrix data
void SpringDamperEditor::setMatrixData(bool isStiffness, int iRow, int iColumn, double value)
{
    // Slice element data
    KCL::VecN data = mpElement->get();

    // Get the start index for writing
    int matSize = mpElement->stiffness.size();
    int matLength = matSize * matSize;
    int iShift = KCL::SpringDamper::skNumBaseParams;
    if (!isStiffness)
        iShift += matLength;

    // Set the data
    data[iShift + iRow * matSize + iColumn] = value;

    // Set the updated data
    emit commandExecuted(new EditElements(mpElement, data, name(), this));
}

//! Create a group box to pair elastic surfaces
QGroupBox* SpringDamperEditor::createPairGroupBox()
{
    // Create the widgets to select first and second surfaces
    int numSurfaces = mSurfaces.size();
    mpIFirstSurfaceComboBox = new QComboBox;
    mpISecondSurfaceComboBox = new QComboBox;
    for (int i = 0; i != numSurfaces; ++i)
    {
        QString name = mSurfaces[i].name.data();
        mpIFirstSurfaceComboBox->addItem(name, 1 + i);
        mpISecondSurfaceComboBox->addItem(name, 1 + i);
    }
    mpISecondSurfaceComboBox->addItem(tr("Ground"), skGroundIndex);

    // Add the widgets to the layout
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->addWidget(new QLabel(tr("First: ")));
    pLayout->addWidget(mpIFirstSurfaceComboBox);
    pLayout->addWidget(new QLabel(tr("Second: ")));
    pLayout->addWidget(mpISecondSurfaceComboBox);
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
QGroupBox* SpringDamperEditor::createMatrixGroupBox()
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
    mpStiffnessButton = new QPushButton(tr("Stiffness matrix"));
    mpDampingButton = new QPushButton(tr("Damping matrix"));
    pDataLayout->addStretch(10);
    pDataLayout->addWidget(mpStiffnessButton);
    pDataLayout->addStretch(1);
    pDataLayout->addWidget(mpDampingButton);
    pDataLayout->addStretch(10);
    pMainLayout->addLayout(pDataLayout);

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(tr("Spring data"));
    pGroupBox->setLayout(pMainLayout);
    return pGroupBox;
}

//! Show widget to edit matrices
void SpringDamperEditor::showMatrixEditor(bool isStiffness)
{
    // Create the dialog widget
    QDialog* pDialog = new QDialog;
    QString title = isStiffness ? tr("Stiffness Matrix Editor") : tr("Damping Matrix Editor");
    pDialog->setWindowTitle(title);
    pDialog->setAttribute(Qt::WA_DeleteOnClose, true);

    // Create the table
    CustomTable* pTable = new CustomTable;
    pTable->setSizeAdjustPolicy(QTableWidget::AdjustToContents);

    // Add the cell editors
    int numMat = mpElement->stiffness.size();
    pTable->setRowCount(numMat);
    pTable->setColumnCount(numMat);
    for (int i = 0; i != numMat; ++i)
    {
        for (int j = 0; j != numMat; ++j)
        {
            Edit1d* pEdit = new Edit1d;
            pEdit->setReadOnly(true);
            pEdit->setStyleSheet(pEdit->styleSheet().append("border: none;"));
            pEdit->setAlignment(Qt::AlignCenter);
            pTable->setCellWidget(i, j, pEdit);
        }
    }

    // Get the indices of elements which can be edited
    QList<QPair<int, int>> indices;
    switch (mpElement->iSwitch)
    {
    case kLong:
        indices.push_back({2, 2});
        indices.push_back({5, 5});
        break;
    case kShort36:
        for (int i = 0; i != numMat; ++i)
        {
            for (int j = 0; j != numMat; ++j)
                indices.push_back({i, j});
        }
        break;
    default:
        for (int i = 0; i != numMat; ++i)
            indices.push_back({i, i});
        break;
    }

    // Set the matrix data
    KCL::Mat6x6 const& matrix = isStiffness ? mpElement->stiffness : mpElement->damping;
    int numIndices = indices.size();
    for (int i = 0; i != numIndices; ++i)
    {
        auto [iRow, iColumn] = indices[i];
        auto pEdit = (Edit1d*) pTable->cellWidget(iRow, iColumn);
        pEdit->setReadOnly(false);
        pEdit->setValue(matrix[iRow][iColumn]);
    }

    // Set the connections
    for (int i = 0; i != numMat; ++i)
    {
        for (int j = 0; j != numMat; ++j)
        {
            auto pEdit = (Edit1d*) pTable->cellWidget(i, j);
            connect(pEdit, &Edit1d::valueChanged, this, [this, isStiffness, i, j, pEdit]() { setMatrixData(isStiffness, i, j, pEdit->value()); });
        }
    }

    // Set the layout
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(pTable);
    pDialog->setLayout(pLayout);

    // Show the dialog
    pDialog->exec();
}
