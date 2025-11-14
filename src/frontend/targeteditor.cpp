#include <QLabel>
#include <QVBoxLayout>

#include "customtable.h"
#include "lineedit.h"
#include "modalsolver.h"
#include "targeteditor.h"

using namespace Frontend;

TargetEditor::TargetEditor(Eigen::VectorXi& indices, Eigen::VectorXd& weights, Backend::Core::ModalSolution& solution, QString const& name,
                           QWidget* pParent)
    : Editor(kTarget, name, QIcon(":/icons/target.svg"), pParent)
    , mIndices(indices)
    , mWeights(weights)
    , mSolution(solution)
{
    createContent();
    createConnections();
    TargetEditor::refresh();
}

QSize TargetEditor::sizeHint() const
{
    return QSize(680, 350);
}

//! Update the widgets from the source
void TargetEditor::refresh()
{
    // Constants
    int const kNumColumns = 3;

    // Slice dimensions
    int numRows = mIndices.size();

    // Set the number of modes
    QSignalBlocker blockerNumModes(mpNumModesEdit);
    mpNumModesEdit->setValue(numRows);

    // Clear the table content
    QSignalBlocker blockerTable(mpTable);
    mpTable->clear();

    // Set the table dimensions
    mpTable->setRowCount(numRows);
    mpTable->setColumnCount(kNumColumns);
    mpTable->setHorizontalHeaderLabels({tr("Index"), tr("Frequencies"), tr("Weights")});

    // Set the table data
    for (int i = 0; i != numRows; ++i)
    {
        // Slice data
        int iMode = mIndices[i];
        double frequency = mSolution.frequencies[iMode];
        double weight = mWeights[iMode];

        // Create the widgets
        Edit1i* pIndexEdit = new Edit1i;
        Edit1d* pFrequencyEdit = new Edit1d;
        Edit1d* pWeightEdit = new Edit1d;

        // Set the styles
        pIndexEdit->setStyleSheet(pIndexEdit->styleSheet().append("border: none;"));
        pFrequencyEdit->setStyleSheet(pFrequencyEdit->styleSheet().append("border: none;"));
        pWeightEdit->setStyleSheet(pWeightEdit->styleSheet().append("border: none;"));

        // Set the minimum values
        pIndexEdit->setMinimum(1);
        pFrequencyEdit->setMinimum(0.0);

        // Set the current values
        pIndexEdit->setValue(1 + iMode);
        pFrequencyEdit->setValue(frequency);
        pWeightEdit->setValue(weight);

        // Set the widgets
        mpTable->setCellWidget(i, 0, pIndexEdit);
        mpTable->setCellWidget(i, 1, pFrequencyEdit);
        mpTable->setCellWidget(i, 2, pWeightEdit);
    }
}

//! Create all the widgets
void TargetEditor::createContent()
{
    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    // Create the widget to edit number of modes
    mpNumModesEdit = new Edit1i;
    mpNumModesEdit->setMinimum(1);
    QHBoxLayout* pNumModesLayout = new QHBoxLayout;
    pNumModesLayout->addWidget(new QLabel(tr("Number of modes: ")));
    pNumModesLayout->addWidget(mpNumModesEdit);
    pNumModesLayout->setStretch(2, 0);
    pNumModesLayout->addStretch(1);
    pMainLayout->addLayout(pNumModesLayout);

    // Create the widget to edit target data
    mpTable = new CustomTable;
    mpTable->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    mpTable->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QHBoxLayout* pTableLayout = new QHBoxLayout;
    pTableLayout->addWidget(mpTable);
    pTableLayout->addStretch();
    pMainLayout->addLayout(pTableLayout);

    // Set the main layout
    pMainLayout->addStretch();
    setLayout(pMainLayout);
}

//! Specify the widget connections
void TargetEditor::createConnections()
{
    // TODO
}
