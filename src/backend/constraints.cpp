#include <magicenum/magic_enum.hpp>
#include <QDebug>
#include <QObject>

#include "constraints.h"

using namespace Backend::Core;

Constraints::Constraints()
{
    double inf = std::numeric_limits<double>::infinity();
    QList<VariableType> const keys = types();
    for (auto key : keys)
    {
        mEnabledState[key] = true;
        mUnitedState[key] = false;
        mMultipliedState[key] = false;
        mScales[key] = 1.0;
        mLimits[key] = {-inf, inf};
    }
}

bool Constraints::isEnabled(VariableType type) const
{
    return mEnabledState[type];
}

bool Constraints::isUnited(VariableType type) const
{
    return mUnitedState[type];
}

bool Constraints::isMultiplied(VariableType type) const
{
    return mMultipliedState[type];
}

double Constraints::scale(VariableType type) const
{
    return mScales[type];
}

PairDouble Constraints::limits(VariableType type) const
{
    return mLimits[type];
}

//! Retrieve all variable types
QList<VariableType> Constraints::types()
{
    constexpr auto values = magic_enum::enum_values<VariableType>();
    int numValues = values.size();
    QList<VariableType> result(numValues);
    for (int i = 0; i != numValues; ++i)
        result[i] = values[i];
    return result;
}

//! Enable the variable for updating
void Constraints::setEnabled(VariableType type, bool flag)
{
    mEnabledState[type] = flag;
}

//! Set united state of variables
void Constraints::setUnited(VariableType type, bool flag)
{
    if (mMultipliedState[type])
    {
        QString typeName = magic_enum::enum_name(type).data();
        qWarning() << QObject::tr("Multiplication is already enabled for type: %1. Unification request is ignored").arg(typeName);
        return;
    }
    mUnitedState[type] = flag;
}

//! Set multiplied state of variables
void Constraints::setMultiplied(VariableType type, bool flag)
{
    if (mUnitedState[type])
    {
        QString typeName = magic_enum::enum_name(type).data();
        qWarning() << QObject::tr("Unification is already enabled for type: %1. Multiplication request is ignored").arg(typeName);
        return;
    }
    mMultipliedState[type] = flag;
}

//! Set the variable scaling factor
void Constraints::setScale(VariableType type, double value)
{
    mScales[type] = value;
}

//! Assign the variable limits
void Constraints::setLimits(VariableType type, PairDouble const& limits)
{
    mLimits[type] = limits;
}
