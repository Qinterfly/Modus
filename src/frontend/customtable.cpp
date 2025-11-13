
#include <QApplication>
#include <QClipboard>
#include <QTextStream>
#include <QMenu>

#include "customtable.h"

using namespace Frontend;

CustomTable::CustomTable(QWidget* pParent)
    : QTableWidget(pParent)
{
    createActions();
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTableWidget::customContextMenuRequested, this, &CustomTable::processContextMenuRequested);
}

CustomTable::~CustomTable()
{

}

//! Align the data using the specified flag
void CustomTable::setDataAlignment(Qt::Alignment alignment)
{
    for (uint i = 0; i != rowCount(); ++i)
    {
        for (uint j = 0; j != columnCount(); ++j)
        {
            QTableWidgetItem* pItem = item(i, j);
            if (pItem)
                pItem->setTextAlignment(alignment);
        }
    }
}

//! Create actions to handle table content
void CustomTable::createActions()
{
    addAction(QIcon(":/icons/edit-copy.svg"), tr("&Copy data"), QKeySequence::Copy, this, &CustomTable::copySelection);
    addAction(QIcon(":/icons/edit-paste.svg"), tr("&Paste data"), QKeySequence::Paste, this, &CustomTable::pasteSelection);
    addAction(QIcon(":/icons/edit-delete.svg"), tr("&Clear data"), QKeySequence::Delete, this, &CustomTable::clearSelection);
}

//! Copy selected data to the clipboard
void CustomTable::copySelection()
{
    // Retrieve the selection
    QModelIndexList indices = selectionModel()->selectedIndexes();
    if (indices.isEmpty())
        return;

    // Sort the indices
    std::sort(indices.begin(), indices.end());

    // Loop through all the indices
    QString selectionText;
    uint numIndices = indices.size();
    QAbstractItemModel* pModel = model();
    for (uint i = 0; i != numIndices; ++i)
    {
        QModelIndex const& current = indices[i];
        QString text = pModel->data(current).toString();
        selectionText.append(text);
        if (i + 1 < numIndices)
        {
            QModelIndex const& next = indices[i + 1];
            if (current.row() != next.row())
                selectionText.append('\n');
            else
                selectionText.append('\t');
        }
    }

    // Assign the data to the clipboard
    QApplication::clipboard()->setText(selectionText);
}

//! Substitute selected data with the ones stored in the clipboard
void CustomTable::pasteSelection()
{
    // Retrieve the selection
    QModelIndexList indices = selectionModel()->selectedIndexes();
    if (indices.isEmpty())
        return;

    // Sort the indices and take the first one as the start
    std::sort(indices.begin(), indices.end());
    QModelIndex startIndex = indices.first();
    uint iStartRow = startIndex.row();
    uint iStartColumn = startIndex.column();

    // Retrieve the clipboard data
    QString selectionText = QApplication::clipboard()->text();
    QStringList rows = selectionText.split('\n', Qt::SkipEmptyParts);
    int numRows = rows.size();
    int maxNumColumns = 0;
    QList<QStringList> data(numRows);
    for (uint i = 0; i != numRows; ++i)
    {
        data[i] = rows[i].split('\t', Qt::SkipEmptyParts);
        maxNumColumns = std::max(maxNumColumns, (int)data[i].size());
    }

    // Enlarge the table, if necessary
    int iEndRow = iStartRow + numRows;
    int iEndColumn = iStartColumn + maxNumColumns;
    if (iEndRow > rowCount())
        setRowCount(iEndRow);
    if (iEndColumn > columnCount())
        setColumnCount(iEndColumn);

    // Loop through all the copied data
    for (uint i = 0; i != numRows; ++i)
    {
        QStringList const& columns = data[i];
        int numColumns = columns.size();
        for (uint j = 0; j != numColumns; ++j)
            setItem(iStartRow + i, iStartColumn + j, new QTableWidgetItem(columns[j]));
    }
}

//! Clear selected data
void CustomTable::clearSelection()
{
    QModelIndexList indices = selectionModel()->selectedIndexes();
    int numIndices = indices.size();
    for (int i = 0; i != numIndices; ++i)
    {
        QModelIndex const& index = indices[i];
        setItem(index.row(), index.column(), new QTableWidgetItem);
    }
}

//! Show the context menum at the specified location
void CustomTable::processContextMenuRequested(QPoint const& /*position*/)
{
    QMenu* pMenu = new QMenu(this);
    pMenu->addActions(actions());
    pMenu->exec(QCursor::pos());
}
