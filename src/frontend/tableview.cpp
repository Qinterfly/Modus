#include <QVBoxLayout>

#include "customtable.h"
#include "fluttersolver.h"
#include "tableview.h"

using namespace Backend;
using namespace Frontend;

TableView::TableView(Core::FlutterSolution const& solution)
{
    createContent();
    setData(solution);
}

//! Clear all the table content
void TableView::clear()
{
    mpTable->clear();
}

//! Display the table content
void TableView::plot()
{
    clear();
    int numRows = mData.rows();
    int numCols = mData.cols();
    mpTable->setRowCount(numRows);
    mpTable->setColumnCount(numCols);
    mpTable->setHorizontalHeaderLabels(mHorizontalLabels);
    mpTable->setVerticalHeaderLabels(mVerticalLabels);
    for (int i = 0; i != numRows; ++i)
    {
        for (int j = 0; j != numCols; ++j)
        {
            QString text = QString::number(mData(i, j), 'g', 3);
            QTableWidgetItem* pItem = new QTableWidgetItem(text);
            pItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            pItem->setTextAlignment(Qt::AlignCenter);
            mpTable->setItem(i, j, pItem);
        }
    }
}

//! Update the table
void TableView::refresh()
{
    plot();
}

//! Get the view type
IView::Type TableView::type() const
{
    return IView::kTable;
}

//! Create all the widgets
void TableView::createContent()
{
    // Create the text widget
    mpTable = new CustomTable;
    mpTable->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    mpTable->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Set the layout
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->addWidget(mpTable);
    pLayout->setAlignment(Qt::AlignTop);
    pLayout->addStretch();
    setLayout(pLayout);
}

//! Set data using flutter solution
void TableView::setData(Backend::Core::FlutterSolution const& solution)
{
    // Slice dimensions
    int const numRows = solution.numCrit();
    int const numCols = 6;

    // Copy the data and set the header labels
    mData.resize(numRows, numCols);
    mHorizontalLabels = {"q", "Vtas", "F[hz]", "OMF[rad/s]", "Sh", "dDE/dV"};
    mVerticalLabels.resize(numRows);
    for (int i = 0; i != numRows; ++i)
    {
        mData(i, 0) = solution.critFlow[i];
        mData(i, 1) = solution.critSpeed[i];
        mData(i, 2) = solution.critFrequency[i];
        mData(i, 3) = solution.critCircFrequency[i];
        mData(i, 4) = solution.critStrouhal[i];
        mData(i, 5) = solution.critDamping[i];
        mVerticalLabels[i] = QString::number(1 + i);
    }
}
