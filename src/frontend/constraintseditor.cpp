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

auto skTypes = magic_enum::enum_values<Core::VariableType>();
int const skNumTypes = skTypes.size();

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
    for (auto type : skTypes)
    {
        // Enabled
        QSignalBlocker blockerEnabled(mEnabledEdits[type]);
        mEnabledEdits[type]->setChecked(mConstraints.isEnabled(type));
        // United
        QSignalBlocker blockerUnited(mUnitedEdits[type]);
        mUnitedEdits[type]->setChecked(mConstraints.isUnited(type));
        // Multiplied
        QSignalBlocker blockerMultiplied(mMultipliedEdits[type]);
        mMultipliedEdits[type]->setChecked(mConstraints.isMultiplied(type));
        // Nonzero
        QSignalBlocker blockerNonzero(mNonzeroEdits[type]);
        mNonzeroEdits[type]->setChecked(mConstraints.isNonzero(type));
        // Scale
        QSignalBlocker blockerScale(mScaleEdits[type]);
        mScaleEdits[type]->setValue(mConstraints.scale(type));
        // Min bound
        QSignalBlocker blockerMinBound(mMinBoundEdits[type]);
        mMinBoundEdits[type]->setValue(mConstraints.bounds(type).first);
        // Max bound
        QSignalBlocker blockerMaxBound(mMaxBoundEdits[type]);
        mMaxBoundEdits[type]->setValue(mConstraints.bounds(type).second);
    }
}

//! Create all the widgets
void ConstraintsEditor::createContent()
{
    // Column labels
    QStringList kColumnNames = {tr("Variable"), tr("Enabled"), tr("United"),    tr("Multiplied"),
                                tr("Nonzero"),  tr("Scale"),   tr("Min bound"), tr("Max bound")};

    // Variable names
    QMap<VariableType, QString> kTypeNames;
    kTypeNames[VariableType::kBeamStiffness] = tr("Beam stiffness");
    kTypeNames[VariableType::kThickness] = tr("Thickness");
    kTypeNames[VariableType::kYoungsModulus1] = tr("Youngs modulus 1");
    kTypeNames[VariableType::kYoungsModulus2] = tr("Youngs modulus 2");
    kTypeNames[VariableType::kShearModulus] = tr("Shear modulus");
    kTypeNames[VariableType::kPoissonRatio] = tr("Poisson ratio");
    kTypeNames[VariableType::kSpringStiffness] = tr("Spring stiffness");

    // Retrieve the variables
    int numColumns = kColumnNames.size();

    // Create the table
    mpTable = new CustomTable;
    mpTable->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    mpTable->setRowCount(skNumTypes);
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
    for (int i = 0; i != skNumTypes; ++i)
    {
        VariableType type = skTypes[i];
        // Variable
        QTableWidgetItem* pItem = new QTableWidgetItem(kTypeNames[type]);
        pItem->setFlags(Qt::ItemIsEnabled);
        mpTable->setItem(i, 0, pItem);
        // Enabled
        createCheckEdit(i, 1, &mEnabledEdits[type]);
        // United
        createCheckEdit(i, 2, &mUnitedEdits[type]);
        // Multiplied
        createCheckEdit(i, 3, &mMultipliedEdits[type]);
        // Nonzero
        createCheckEdit(i, 4, &mNonzeroEdits[type]);
        // Scale
        createDoubleEdit(i, 5, &mScaleEdits[type]);
        // Min bound
        createDoubleEdit(i, 6, &mMinBoundEdits[type]);
        // Max bound
        createDoubleEdit(i, 7, &mMaxBoundEdits[type]);
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
    for (auto type : skTypes)
    {
        connect(mEnabledEdits[type], &QCheckBox::clicked, this, &ConstraintsEditor::setData);
        connect(mUnitedEdits[type], &QCheckBox::clicked, this, &ConstraintsEditor::setData);
        connect(mMultipliedEdits[type], &QCheckBox::clicked, this, &ConstraintsEditor::setData);
        connect(mNonzeroEdits[type], &QCheckBox::clicked, this, &ConstraintsEditor::setData);
        connect(mScaleEdits[type], &Edit1d::valueChanged, this, &ConstraintsEditor::setData);
        connect(mMinBoundEdits[type], &Edit1d::valueChanged, this, &ConstraintsEditor::setData);
        connect(mMaxBoundEdits[type], &Edit1d::valueChanged, this, &ConstraintsEditor::setData);
    }
}

//! Set the constraints
void ConstraintsEditor::setData()
{
    // Check if the flags are correct
    if (!validateFlagEdits())
    {
        refresh();
        return;
    }

    // Update value boundaries
    updateBoundEdits();

    // Set the new values of constraints
    Constraints newConstraints = mConstraints;
    for (auto type : skTypes)
    {
        newConstraints.setEnabled(type, mEnabledEdits[type]->isChecked());
        newConstraints.setUnited(type, mUnitedEdits[type]->isChecked());
        newConstraints.setMultiplied(type, mMultipliedEdits[type]->isChecked());
        newConstraints.setNonzero(type, mNonzeroEdits[type]->isChecked());
        newConstraints.setScale(type, mScaleEdits[type]->value());
        newConstraints.setBounds(type, {mMinBoundEdits[type]->value(), mMaxBoundEdits[type]->value()});
    }

    // Apply the changes
    emit commandExecuted(new EditObject<Constraints>(mConstraints, tr("Constraints"), newConstraints));
}

//! Update edits which set flags
bool ConstraintsEditor::validateFlagEdits()
{
    for (auto type : skTypes)
    {
        // Obtain the current flags
        bool isUnited = mUnitedEdits[type]->isChecked();
        bool isMultiplied = mMultipliedEdits[type]->isChecked();
        bool isNonzero = mNonzeroEdits[type]->isChecked();

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
    for (auto type : skTypes)
    {
        // Block the signals
        QSignalBlocker blockerMinBound(mMinBoundEdits[type]);
        QSignalBlocker blockerMaxBound(mMaxBoundEdits[type]);

        // Swap the bounds, if necessary
        double minBound = mMinBoundEdits[type]->value();
        double maxBound = mMaxBoundEdits[type]->value();
        if (minBound > maxBound)
            std::swap(minBound, maxBound);

        // Set the values
        mMinBoundEdits[type]->setValue(minBound);
        mMaxBoundEdits[type]->setValue(maxBound);
    }
}
