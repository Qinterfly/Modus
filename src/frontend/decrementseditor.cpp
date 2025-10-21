#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>

#include <kcl/model.h>

#include "customtable.h"
#include "decrementseditor.h"
#include "lineedit.h"
#include "uialiasdata.h"
#include "uiutility.h"

using namespace Frontend;

DecrementsEditor::DecrementsEditor(KCL::Decrements* pElement, QString const& name, QWidget* pParent)
    : Editor(kGeneralData, name, Utility::getIcon(pElement->type()), pParent)
    , mpElement(pElement)
{
    createContent();
    DecrementsEditor::refresh();
}

QSize DecrementsEditor::sizeHint() const
{
    return QSize(680, 350);
}

//! Update data of widgets from the element source
void DecrementsEditor::refresh()
{
    // Block the signals & slots connections
    QSignalBlocker blockerNumData(mpNumDataEdit);
    QSignalBlocker blockerDataTable(mpDataTable);

    // Update the editor of table dimension
    int numData = mpElement->values.size();
    mpNumDataEdit->setValue(numData);

    // Resize the table
    mpDataTable->clear();
    mpDataTable->setRowCount(1);
    mpDataTable->setColumnCount(numData);

    // Set the data
    for (int i = 0; i != numData; ++i)
    {
        Edit1d* pEdit = new Edit1d;
        pEdit->setValue(mpElement->values[i]);
        pEdit->setAlignment(Qt::AlignCenter);
        pEdit->setStyleSheet("border: none");
        mpDataTable->setCellWidget(0, i, pEdit);
        connect(pEdit, &Edit1d::valueChanged, this, &DecrementsEditor::setElementData);
    }

    // Update the table geometry
    mpDataTable->resizeRowsToContents();
}

//! Create all the widgets
void DecrementsEditor::createContent()
{
    // Create the data table
    mpDataTable = new CustomTable;
    mpDataTable->verticalHeader()->setVisible(false);
    mpDataTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Create the number of data layout
    mpNumDataEdit = new Edit1i;
    mpNumDataEdit->setMinimum(0);
    QHBoxLayout* pNumDataLayout = new QHBoxLayout;
    pNumDataLayout->addWidget(new QLabel(tr("Number of decrements: ")));
    pNumDataLayout->addWidget(mpNumDataEdit);
    pNumDataLayout->addStretch(1);

    // Create the data layout
    QHBoxLayout* pDataLayout = new QHBoxLayout;
    pDataLayout->addWidget(new QLabel(tr("Decrements: ")));
    pDataLayout->addWidget(mpDataTable);

    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pNumDataLayout);
    pMainLayout->addLayout(pDataLayout);
    pMainLayout->addStretch(1);
    setLayout(pMainLayout);

    // Set connections
    connect(mpNumDataEdit, &Edit1i::editingFinished, this, &DecrementsEditor::resizeElementData);
}

//! Change the element data dimension
void DecrementsEditor::resizeElementData()
{
    // Slice element data
    KCL::VecN data = mpElement->values;

    // Resize data
    int numData = mpNumDataEdit->value();
    data.resize(numData);

    // Set the updated data
    emit commandExecuted(new EditElement(mpElement, data, name()));

    // Update the view
    refresh();
}

//! Update element data from the widgets
void DecrementsEditor::setElementData()
{
    // Get the data from widgets
    int numData = mpNumDataEdit->value();
    KCL::VecN data(numData);
    for (int i = 0; i != numData; ++i)
        data[i] = static_cast<Edit1d*>(mpDataTable->cellWidget(0, i))->value();

    // Set the updated data
    emit commandExecuted(new EditElement(mpElement, data, name()));
}
