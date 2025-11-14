#include <QLabel>
#include <QVBoxLayout>

#include "customtable.h"
#include "lineedit.h"
#include "optimsolver.h"
#include "targeteditor.h"

using namespace Backend;
using namespace Frontend;

TargetEditor::TargetEditor(Backend::Core::OptimTarget& target, QString const& name, QWidget* pParent)
    : Editor(kOptimTarget, name, QIcon(":/icons/target.svg"), pParent)
    , mTarget(target)
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
    int numRows = mTarget.indices.size();

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
    bool isSolution = !mTarget.solution.isEmpty();
    for (int i = 0; i != numRows; ++i)
    {
        // Slice data
        int iMode = mTarget.indices[i];
        double frequency = isSolution ? mTarget.solution.frequencies[iMode] : mTarget.frequencies[i];
        double weight = mTarget.weights[i];

        // Create the widgets
        Edit1i* pIndexEdit = new Edit1i;
        Edit1d* pFrequencyEdit = new Edit1d;
        Edit1d* pWeightEdit = new Edit1d;

        // Set the styles
        pIndexEdit->hideBorders();
        pFrequencyEdit->hideBorders();
        pWeightEdit->hideBorders();

        // Set the text alignment
        pIndexEdit->setAlignment(Qt::AlignCenter);
        pFrequencyEdit->setAlignment(Qt::AlignCenter);
        pWeightEdit->setAlignment(Qt::AlignCenter);

        // Set the limits
        pIndexEdit->setMinimum(0);
        if (isSolution)
            pIndexEdit->setMaximum(mTarget.solution.numModes());
        pFrequencyEdit->setMinimum(0.0);

        // Set the current values
        pIndexEdit->setValue(1 + iMode);
        pFrequencyEdit->setValue(frequency);
        pWeightEdit->setValue(weight);

        // Set the connections
        pFrequencyEdit->setReadOnly(isSolution);
        connect(pIndexEdit, &Edit1i::valueChanged, this, &TargetEditor::setData);
        connect(pFrequencyEdit, &Edit1d::valueChanged, this, &TargetEditor::setData);
        connect(pWeightEdit, &Edit1d::valueChanged, this, &TargetEditor::setData);

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
    connect(mpNumModesEdit, &Edit1i::valueChanged, this, &TargetEditor::setNumModes);
}

//! Change number of modes
void TargetEditor::setNumModes()
{
    int oldCount = mTarget.indices.size();
    int newCount = mpNumModesEdit->value();

    // Allocate the target values
    Eigen::VectorXi indices(newCount);
    Eigen::VectorXd frequencies(newCount);
    Eigen::VectorXd weights(newCount);

    // Initialize the vectors by default values
    for (int i = 0; i != newCount; ++i)
    {
        indices[i] = 0;
        frequencies[i] = 0.0;
        weights[i] = 1.0;
    }

    // Copy the old data
    int count = std::min(oldCount, newCount);
    for (int i = 0; i != count; ++i)
    {
        indices[i] = mTarget.indices[i];
        frequencies[i] = mTarget.frequencies[i];
        weights[i] = mTarget.weights[i];
    }

    // Apply the changes
    executeCommand(indices, frequencies, weights);

    // Update the widget
    refresh();
}

//! Apply the changes
void TargetEditor::setData()
{
    // Allocate the objects
    int numModes = mpNumModesEdit->value();
    Eigen::VectorXi indices(numModes);
    Eigen::VectorXd frequencies(numModes);
    Eigen::VectorXd weights(numModes);

    // Set the new values
    for (int i = 0; i != numModes; ++i)
    {
        indices[i] = static_cast<Edit1i*>(mpTable->cellWidget(i, 0))->value() - 1;
        frequencies[i] = static_cast<Edit1d*>(mpTable->cellWidget(i, 1))->value();
        weights[i] = static_cast<Edit1d*>(mpTable->cellWidget(i, 2))->value();
    }

    // Apply the changes
    executeCommand(indices, frequencies, weights);
}

//! Apply the changes via command
void TargetEditor::executeCommand(Eigen::VectorXi const& indices, Eigen::VectorXd const& frequencies, Eigen::VectorXd const& weights)
{
    Core::OptimTarget target = mTarget;
    target.indices = indices;
    target.weights = weights;
    target.frequencies = frequencies;
    emit commandExecuted(new EditObject<Core::OptimTarget>(mTarget, name(), target));
}
