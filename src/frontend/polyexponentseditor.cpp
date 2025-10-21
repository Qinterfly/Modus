#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>

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
    int const kNumRows = 2;

    // Block the signals & slots connections
    QSignalBlocker blockerNumData(mpNumDataEdit);
    QSignalBlocker blockerDataTable(mpDataTable);

    // Update the editor of table dimension
    int numData = mpElementX->values.size();
    mpNumDataEdit->setValue(numData);

    // Resize the table
    mpDataTable->clear();
    mpDataTable->setRowCount(kNumRows);
    mpDataTable->setColumnCount(numData);
    mpDataTable->setVerticalHeaderLabels({"PK", "QK"});

    // Set the data
    KCL::VecN dataX = mpElementX->get();
    KCL::VecN dataZ = mpElementZ->get();
    for (int jColumn = 0; jColumn != numData; ++jColumn)
    {
        Edit1i* pEditX = new Edit1i;
        Edit1i* pEditZ = new Edit1i;
        QList<double> columnData = {dataX[jColumn], dataZ[jColumn]};
        QList<Edit1i*> columnEdits = {pEditX, pEditZ};
        for (int iRow = 0; iRow != kNumRows; ++iRow)
        {
            auto pEdit = columnEdits[iRow];
            pEdit->setMinimum(0);
            pEdit->setAlignment(Qt::AlignCenter);
            pEdit->setStyleSheet(pEdit->styleSheet().append("border: none;"));
            pEdit->setValue(columnData[iRow]);
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
    // Create the data table
    mpDataTable = new CustomTable;
    mpDataTable->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    mpDataTable->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Create layout to edit data length
    mpNumDataEdit = new Edit1i;
    mpNumDataEdit->setMinimum(0);
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->addWidget(new QLabel(tr("Number of exponents: ")));
    pLayout->addWidget(mpNumDataEdit);
    pLayout->addStretch(1);

    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pLayout);
    pMainLayout->addWidget(mpDataTable);
    pMainLayout->addStretch(1);
    setLayout(pMainLayout);

    // Set connections
    connect(mpNumDataEdit, &Edit1i::editingFinished, this, &PolyExponentsEditor::resizeElementData);
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
    emit commandExecuted(new EditElement(mpElementX, dataX, name()));
    emit commandExecuted(new EditElement(mpElementZ, dataZ, name()));

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
        dataX[i] = static_cast<Edit1i*>(mpDataTable->cellWidget(0, i))->value();
        dataZ[i] = static_cast<Edit1i*>(mpDataTable->cellWidget(1, i))->value();
    }

    // Set the updated data
    emit commandExecuted(new EditElement(mpElementX, dataX, name()));
    emit commandExecuted(new EditElement(mpElementZ, dataZ, name()));
}
