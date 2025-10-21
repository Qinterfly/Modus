#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>

#include <kcl/model.h>
#include <magicenum/magic_enum.hpp>

#include "customtable.h"
#include "lineedit.h"
#include "rawdataeditor.h"
#include "uialiasdata.h"
#include "uiutility.h"

using namespace Frontend;

bool isResizable(KCL::ElementType type);

RawDataEditor::RawDataEditor(KCL::AbstractElement* pElement, QString const& name, QWidget* pParent)
    : Editor(kRawData, name, Utility::getIcon(pElement->type()), pParent)
    , mpElement(pElement)
{
    createContent();
    RawDataEditor::refresh();
}

QSize RawDataEditor::sizeHint() const
{
    return QSize(680, 350);
}

//! Update data of widgets from the element source
void RawDataEditor::refresh()
{
    // Block the signals & slots connections
    QSignalBlocker blockerNumData(mpNumDataEdit);
    QSignalBlocker blockerDataTable(mpDataTable);

    // Update the editor of table dimension
    KCL::VecN data = mpElement->get();
    int numData = data.size();
    mpNumDataEdit->setValue(numData);

    // Resize the table
    QString typeName = magic_enum::enum_name(mpElement->type()).data();
    mpDataTable->clear();
    mpDataTable->setRowCount(1);
    mpDataTable->setColumnCount(numData);
    mpDataTable->setVerticalHeaderLabels({typeName});

    // Set the data
    bool isPoly = Utility::polyTypes().contains(mpElement->type());
    for (int i = 0; i != numData; ++i)
    {
        QLineEdit* pBaseEdit;
        if (isPoly)
        {
            Edit1i* pEdit = new Edit1i;
            pEdit->setValue(data[i]);
            connect(pEdit, &Edit1i::valueChanged, this, &RawDataEditor::setElementData);
            pBaseEdit = pEdit;
        }
        else
        {
            Edit1d* pEdit = new Edit1d;
            pEdit->setValue(data[i]);
            connect(pEdit, &Edit1d::valueChanged, this, &RawDataEditor::setElementData);
            pBaseEdit = pEdit;
        }
        pBaseEdit->setAlignment(Qt::AlignCenter);
        pBaseEdit->setStyleSheet(pBaseEdit->styleSheet().append("border: none;"));
        mpDataTable->setCellWidget(0, i, pBaseEdit);
    }

    // Set the names
    KCL::VecNS names = mpElement->names();
    int numNames = names.size();
    if (numNames > 0)
    {
        QStringList labels(numNames);
        for (int i = 0; i != numNames; ++i)
            labels[i] = names[i].data();
        mpDataTable->setHorizontalHeaderLabels(labels);
    }

    // Update the table geometry
    mpDataTable->resizeRowsToContents();
    mpDataTable->updateGeometry();
}

//! Create all the widgets
void RawDataEditor::createContent()
{
    // Create the data table
    mpDataTable = new CustomTable;
    mpDataTable->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    mpDataTable->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    mpDataTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Create layout to edit data length
    mpNumDataEdit = new Edit1i;
    mpNumDataEdit->setMinimum(0);
    mpNumDataEdit->setReadOnly(!isResizable(mpElement->type()));
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->addWidget(new QLabel(tr("Number of values: ")));
    pLayout->addWidget(mpNumDataEdit);
    pLayout->addStretch(1);

    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pLayout);
    pMainLayout->addWidget(mpDataTable);
    pMainLayout->addStretch(1);
    setLayout(pMainLayout);

    // Set connections
    connect(mpNumDataEdit, &Edit1i::editingFinished, this, &RawDataEditor::resizeElementData);
}

//! Change the element data dimension
void RawDataEditor::resizeElementData()
{
    // Slice element data
    KCL::VecN data = mpElement->get();

    // Resize data
    int numData = mpNumDataEdit->value();
    data.resize(numData);

    // Set the updated data
    emit commandExecuted(new EditElements(mpElement, data, name()));

    // Update the view
    refresh();
}

//! Update element data from the widgets
void RawDataEditor::setElementData()
{
    // Get the data from the table
    int numData = mpNumDataEdit->value();
    KCL::VecN data(numData);
    for (int i = 0; i != numData; ++i)
        data[i] = static_cast<Edit1d*>(mpDataTable->cellWidget(0, i))->value();

    // Set the updated data
    emit commandExecuted(new EditElements(mpElement, data, name()));
}

//! Check if element data can be resized
bool isResizable(KCL::ElementType type)
{
    return type == KCL::PK || type == KCL::QK || type == KCL::DQ || type == KCL::TE;
}
