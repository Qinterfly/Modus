#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>

#include <magicenum/magic_enum.hpp>

#include "customtable.h"
#include "lineedit.h"
#include "polyexponentseditor.h"
#include "uiutility.h"

using namespace Frontend;

PolyExponentsEditor::PolyExponentsEditor(KCL::PolyExponentsX* pElementX, KCL::PolyExponentsZ* pElementZ, QString const& name, QWidget* pParent)
    : Editor(kPolyExponents, name, Utility::getIcon(pElementX->type()), pParent)
    , mpElementX(pElementX)
    , mpElementZ(pElementZ)
{
    createContent();
    PolyExponentsEditor::refresh();
}

QSize PolyExponentsEditor::sizeHint() const
{
    return QSize(680, 350);
}

//! Update data of widgets from the element source
void PolyExponentsEditor::refresh()
{
    // Constants
    int const kNumColumns = 2;

    // Block the signals & slots connections
    QSignalBlocker blockerNumData(mpNumDataEdit);
    QSignalBlocker blockerDataTable(mpDataTable);

    // Update the type combobox
    updateTypeComboBox();

    // Update the editor of table dimension
    KCL::VecN dataX = mpElementX->get();
    KCL::VecN dataZ = mpElementZ->get();
    int numData = mpElementX->values.size();
    mpNumDataEdit->setValue(numData);

    // Resize the table
    mpDataTable->clear();
    mpDataTable->setRowCount(numData);
    mpDataTable->setColumnCount(kNumColumns);
    mpDataTable->setHorizontalHeaderLabels({"PK", "QK"});

    // Set the data
    for (int iRow = 0; iRow != numData; ++iRow)
    {
        Edit1i* pEditX = new Edit1i;
        Edit1i* pEditZ = new Edit1i;
        QList<double> rowData = {dataX[iRow], dataZ[iRow]};
        QList<Edit1i*> rowEdits = {pEditX, pEditZ};
        for (int jColumn = 0; jColumn != kNumColumns; ++jColumn)
        {
            auto pEdit = rowEdits[jColumn];
            pEdit->setMinimum(0);
            pEdit->setAlignment(Qt::AlignCenter);
            pEdit->hideBorders();
            pEdit->setValue(rowData[jColumn]);
            connect(pEdit, &Edit1i::valueChanged, this, &PolyExponentsEditor::setElementData);
            mpDataTable->setCellWidget(iRow, jColumn, pEdit);
        }
    }

    // Update the table geometry
    mpDataTable->resizeRowsToContents();
    mpDataTable->updateGeometry();
}

//! Create all the widgets
void PolyExponentsEditor::createContent()
{
    // Constants
    QMap<KCL::PolyType, QString> kMapTypes;
    kMapTypes[KCL::BendingBeamX] = tr("Bending beam X");
    kMapTypes[KCL::BendingBeamZ] = tr("Bending beam Z");
    kMapTypes[KCL::TorsionBeamX] = tr("Torsion beam X");
    kMapTypes[KCL::TorsionBeamZ] = tr("Torsion beam Z");
    kMapTypes[KCL::BendingTorsionBeamX] = tr("Bending-torsion beam X");
    kMapTypes[KCL::BendingTorsionBeamZ] = tr("Bending-torsion beam Z");
    kMapTypes[KCL::Plate] = tr("Plate");

    // Create the data table layout
    QHBoxLayout* pDataLayout = new QHBoxLayout;
    mpDataTable = new CustomTable;
    mpDataTable->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    mpDataTable->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    pDataLayout->addWidget(new QLabel(tr("Exponents: ")));
    pDataLayout->addWidget(mpDataTable);
    pDataLayout->addStretch();

    // Create the layout to edit polynomial type
    mpTypeComboBox = new QComboBox;
    auto types = magic_enum::enum_values<KCL::PolyType>();
    for (auto type : types)
        mpTypeComboBox->addItem(kMapTypes[type], type);
    mpTypeComboBox->setCurrentIndex(-1);
    QHBoxLayout* pTypeLayout = new QHBoxLayout;
    pTypeLayout->addWidget(new QLabel(tr("Type: ")));
    pTypeLayout->addWidget(mpTypeComboBox);
    pTypeLayout->addStretch();

    // Create layout to edit data length
    mpNumDataEdit = new Edit1i;
    mpNumDataEdit->setMinimum(0);
    QHBoxLayout* pNumDataLayout = new QHBoxLayout;
    pNumDataLayout->addWidget(new QLabel(tr("Number of exponents: ")));
    pNumDataLayout->addWidget(mpNumDataEdit);
    pNumDataLayout->addStretch(1);

    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pTypeLayout);
    pMainLayout->addLayout(pNumDataLayout);
    pMainLayout->addLayout(pDataLayout);
    pMainLayout->addStretch();
    setLayout(pMainLayout);

    // Set connections
    connect(mpNumDataEdit, &Edit1i::editingFinished, this, &PolyExponentsEditor::resizeElementData);
    connect(mpTypeComboBox, &QComboBox::currentIndexChanged, this, &PolyExponentsEditor::setElementDataByType);
}

//! Change the element data dimension
void PolyExponentsEditor::resizeElementData()
{
    // Slice element data
    KCL::VecN dataX = mpElementX->get();
    KCL::VecN dataZ = mpElementZ->get();

    // Resize data
    int numData = mpNumDataEdit->value();
    dataX.resize(numData);
    dataZ.resize(numData);

    // Set the updated data
    emit commandExecuted(new EditElements({mpElementX, mpElementZ}, {dataX, dataZ}, name()));

    // Update the view
    refresh();
}

//! Update element data from the widgets
void PolyExponentsEditor::setElementData()
{
    // Get the data from the table
    int numData = mpNumDataEdit->value();
    KCL::VecN dataX(numData);
    KCL::VecN dataZ(numData);
    for (int i = 0; i != numData; ++i)
    {
        dataX[i] = static_cast<Edit1i*>(mpDataTable->cellWidget(i, 0))->value();
        dataZ[i] = static_cast<Edit1i*>(mpDataTable->cellWidget(i, 1))->value();
    }

    // Set the updated data
    emit commandExecuted(new EditElements({mpElementX, mpElementZ}, {dataX, dataZ}, name()));

    // Update the combobox of predefined data
    updateTypeComboBox();
}

//! Set element data associated with a type
void PolyExponentsEditor::setElementDataByType()
{
    // Check if any items selected
    if (mpTypeComboBox->currentIndex() < 0)
        return;

    // Generate new data
    auto type = mpTypeComboBox->currentData().value<KCL::PolyType>();
    auto itemData = KCL::getPolyData(type);

    // Set the updated data
    emit commandExecuted(new EditElements({mpElementX, mpElementZ}, {itemData.first, itemData.second}, name()));

    // Update the view
    refresh();
}

//! Update the combobox to select predefined types
void PolyExponentsEditor::updateTypeComboBox()
{
    // Block all the signals and reset the selection
    QSignalBlocker blocker(mpTypeComboBox);
    mpTypeComboBox->setCurrentIndex(-1);

    // Slice element data
    KCL::VecN dataX = mpElementX->get();
    KCL::VecN dataZ = mpElementZ->get();

    // Check if the data equals to one of the predefined sets
    int numItems = mpTypeComboBox->count();
    for (int i = 0; i != numItems; ++i)
    {
        auto type = mpTypeComboBox->itemData(i).value<KCL::PolyType>();
        auto itemData = KCL::getPolyData(type);
        if (itemData.first == dataX && itemData.second == dataZ)
        {
            mpTypeComboBox->setCurrentIndex(i);
            break;
        }
    }
}
