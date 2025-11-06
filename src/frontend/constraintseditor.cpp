#include <QCheckBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>

#include <magicenum/magic_enum.hpp>

#include "constraintseditor.h"
#include "customtable.h"
#include "lineedit.h"

using namespace Backend;
using namespace Frontend;
using namespace Core;

auto skVariables = magic_enum::enum_values<Core::VariableType>();
int const skNumVariables = skVariables.size();

ConstraintsEditor::ConstraintsEditor(Constraints& constraints, QString const& name, QWidget* pParent)
    : Editor(kConstraints, name, QIcon(":/icons/constraints.png"), pParent)
    , mConstraints(constraints)
{
    createContent();
    createConnections();
    ConstraintsEditor::refresh();
}

QSize ConstraintsEditor::sizeHint() const
{
    return QSize(900, 350);
}

//! Update data of widgets from the element source
void ConstraintsEditor::refresh()
{
    for (auto variable : skVariables)
    {
        // Enabled
        QSignalBlocker blockerEnabled(mEnabledEdits[variable]);
        mEnabledEdits[variable]->setChecked(mConstraints.isEnabled(variable));
        // United
        QSignalBlocker blockerUnited(mUnitedEdits[variable]);
        mUnitedEdits[variable]->setChecked(mConstraints.isUnited(variable));
        // Multiplied
        QSignalBlocker blockerMultiplied(mMultipliedEdits[variable]);
        mMultipliedEdits[variable]->setChecked(mConstraints.isMultiplied(variable));
        // Nonzero
        QSignalBlocker blockerNonzero(mNonzeroEdits[variable]);
        mNonzeroEdits[variable]->setChecked(mConstraints.isNonzero(variable));
        // Scale
        QSignalBlocker blockerScale(mScaleEdits[variable]);
        mScaleEdits[variable]->setValue(mConstraints.scale(variable));
        // Min bound
        QSignalBlocker blockerMinBound(mMinBoundEdits[variable]);
        mMinBoundEdits[variable]->setValue(mConstraints.bounds(variable).first);
        // Max bound
        QSignalBlocker blockerMaxBound(mMaxBoundEdits[variable]);
        mMaxBoundEdits[variable]->setValue(mConstraints.bounds(variable).second);
    }
}

//! Create all the widgets
void ConstraintsEditor::createContent()
{
    // Column labels
    QStringList kColumnNames = {tr("Variable"), tr("Enabled"), tr("United"),    tr("Multiplied"),
                                tr("Nonzero"),  tr("Scale"),   tr("Min bound"), tr("Max bound")};

    // Variable names
    QMap<VariableType, QString> kVariableNames;
    kVariableNames[VariableType::kBeamStiffness] = tr("Beam stiffness");
    kVariableNames[VariableType::kThickness] = tr("Thickness");
    kVariableNames[VariableType::kYoungsModulus1] = tr("Youngs modulus 1");
    kVariableNames[VariableType::kYoungsModulus2] = tr("Youngs modulus 2");
    kVariableNames[VariableType::kShearModulus] = tr("Shear modulus");
    kVariableNames[VariableType::kPoissonRatio] = tr("Poisson ratio");
    kVariableNames[VariableType::kSpringStiffness] = tr("Spring stiffness");

    // Retrieve the variables
    int numColumns = kColumnNames.size();

    // Create the table
    mpTable = new CustomTable;
    mpTable->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    mpTable->setRowCount(skNumVariables);
    mpTable->setColumnCount(numColumns);
    mpTable->setHorizontalHeaderLabels(kColumnNames);
    mpTable->verticalHeader()->setVisible(false);
    mpTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    // Create auxiliary functions
    auto createCheckEdit = [this](int iRow, int iColumn, QCheckBox** ppEdit)
    {
        QWidget* pWidget = new QWidget;
        QHBoxLayout* pLayout = new QHBoxLayout;
        pLayout->setContentsMargins(0, 0, 0, 0);
        *ppEdit = new QCheckBox;
        auto pEdit = *ppEdit;
        pLayout->addWidget(pEdit);
        pLayout->setAlignment(Qt::AlignCenter);
        pWidget->setLayout(pLayout);
        mpTable->setCellWidget(iRow, iColumn, pWidget);
    };
    auto createDoubleEdit = [this](int iRow, int iColumn, Edit1d** ppEdit)
    {
        *ppEdit = new Edit1d;
        auto pEdit = *ppEdit;
        pEdit->setAlignment(Qt::AlignCenter);
        pEdit->setStyleSheet(pEdit->styleSheet().append("border: none;"));
        mpTable->setCellWidget(iRow, iColumn, pEdit);
    };

    // Add items to the table
    for (int i = 0; i != skNumVariables; ++i)
    {
        VariableType variable = skVariables[i];
        // Variable
        QTableWidgetItem* pItem = new QTableWidgetItem(kVariableNames[variable]);
        pItem->setFlags(Qt::ItemIsEnabled);
        mpTable->setItem(i, 0, pItem);
        // Enabled
        createCheckEdit(i, 1, &mEnabledEdits[variable]);
        // United
        createCheckEdit(i, 2, &mUnitedEdits[variable]);
        // Multiplied
        createCheckEdit(i, 3, &mMultipliedEdits[variable]);
        // Nonzero
        createCheckEdit(i, 4, &mNonzeroEdits[variable]);
        // Scale
        createDoubleEdit(i, 5, &mScaleEdits[variable]);
        // Min bound
        createDoubleEdit(i, 6, &mMinBoundEdits[variable]);
        // Max bound
        createDoubleEdit(i, 7, &mMaxBoundEdits[variable]);
    }

    // Set the main layout
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(mpTable);
    pLayout->addStretch();
    setLayout(pLayout);
}

//! Specify signals and slots between widgets
void ConstraintsEditor::createConnections()
{
    for (auto variable : skVariables)
    {
        connect(mEnabledEdits[variable], &QCheckBox::clicked, this, &ConstraintsEditor::setData);
        connect(mUnitedEdits[variable], &QCheckBox::clicked, this, &ConstraintsEditor::setData);
        connect(mMultipliedEdits[variable], &QCheckBox::clicked, this, &ConstraintsEditor::setData);
        connect(mNonzeroEdits[variable], &QCheckBox::clicked, this, &ConstraintsEditor::setData);
        connect(mScaleEdits[variable], &Edit1d::valueChanged, this, &ConstraintsEditor::setData);
        connect(mMinBoundEdits[variable], &Edit1d::valueChanged, this, &ConstraintsEditor::setData);
        connect(mMaxBoundEdits[variable], &Edit1d::valueChanged, this, &ConstraintsEditor::setData);
    }
}

//! Set the constraints
void ConstraintsEditor::setData()
{
    // Check if the flags are correct
    if (!validateCheckEdits())
    {
        refresh();
        return;
    }

    // Update value boundaries
    updateBoundEdits();

    // Set the new values of constraints
    Constraints newConstraints = mConstraints;
    for (auto variable : skVariables)
    {
        newConstraints.setEnabled(variable, mEnabledEdits[variable]->isChecked());
        newConstraints.setUnited(variable, mUnitedEdits[variable]->isChecked());
        newConstraints.setMultiplied(variable, mMultipliedEdits[variable]->isChecked());
        newConstraints.setNonzero(variable, mNonzeroEdits[variable]->isChecked());
        newConstraints.setScale(variable, mScaleEdits[variable]->value());
        newConstraints.setBounds(variable, {mMinBoundEdits[variable]->value(), mMaxBoundEdits[variable]->value()});
    }

    // Apply the changes
    emit commandExecuted(new EditObject<Constraints>(mConstraints, tr("Constraints"), newConstraints));
}

//! Update edits which set flags
bool ConstraintsEditor::validateCheckEdits()
{
    for (auto variable : skVariables)
    {
        // Obtain the current flags
        bool isUnited = mUnitedEdits[variable]->isChecked();
        bool isMultiplied = mMultipliedEdits[variable]->isChecked();
        bool isNonzero = mNonzeroEdits[variable]->isChecked();

        // Validate flags
        if (isUnited && isMultiplied)
        {
            QMessageBox::warning(this, tr("Constraints Warning"), tr("Unification and multiplication flags cannot be both enabled at once"));
            return false;
        }
        if (isNonzero && (isUnited || isMultiplied))
        {
            QMessageBox::warning(this, tr("Constraints Warning"),
                                 tr("Nonzero flag cannot be used with unification or multiplication flags at once"));
            return false;
        }
    }
    return true;
}

//! Update edits which set boundaries
void ConstraintsEditor::updateBoundEdits()
{
    for (auto variable : skVariables)
    {
        // Block the signals
        QSignalBlocker blockerMinBound(mMinBoundEdits[variable]);
        QSignalBlocker blockerMaxBound(mMaxBoundEdits[variable]);

        // Swap the bounds, if necessary
        double minBound = mMinBoundEdits[variable]->value();
        double maxBound = mMaxBoundEdits[variable]->value();
        if (minBound > maxBound)
            std::swap(minBound, maxBound);

        // Set the values
        mMinBoundEdits[variable]->setValue(minBound);
        mMaxBoundEdits[variable]->setValue(maxBound);
    }
}
