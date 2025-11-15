#include <magicenum/magic_enum.hpp>
#include <QDebug>
#include <QObject>
#include <QXmlStreamWriter>

#include "optimconstraints.h"
#include "fileutility.h"

using namespace Backend::Core;

OptimConstraints::OptimConstraints()
{
    double inf = std::numeric_limits<double>::infinity();
    // Insert the keys
    auto const keys = types();
    for (auto type : keys)
    {
        mEnabledState[type] = true;
        mUnitedState[type] = false;
        mMultipliedState[type] = false;
        mNonzeroState[type] = false;
        mScales[type] = 1.0;
        mBounds[type] = {-inf, inf};
    }
    // Set the default values of fields
    setDefaultEnabled();
    setDefaultUnited();
    setDefaultMultiplied();
    setDefaultNonzero();
    setDefaultScales();
    setDefaultBounds();
}

OptimConstraints::~OptimConstraints()
{
}

bool OptimConstraints::operator==(OptimConstraints const& another) const
{
    return Utility::areEqual(*this, another);
}

bool OptimConstraints::operator!=(OptimConstraints const& another) const
{
    return !(*this == another);
}

void OptimConstraints::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    Utility::serialize(stream, "enabledState", mEnabledState);
    Utility::serialize(stream, "unitedState", mUnitedState);
    Utility::serialize(stream, "multipliedState", mMultipliedState);
    Utility::serialize(stream, "nonzeroState", mNonzeroState);
    Utility::serialize(stream, "scales", mScales);
    Utility::serialize(stream, "bounds", mBounds);
    stream.writeEndElement();
}

void OptimConstraints::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "enabledState")
            Utility::deserialize(stream, mEnabledState);
        else if (stream.name() == "unitedState")
            Utility::deserialize(stream, mUnitedState);
        else if (stream.name() == "multipliedState")
            Utility::deserialize(stream, mMultipliedState);
        else if (stream.name() == "nonzeroState")
            Utility::deserialize(stream, mNonzeroState);
        else if (stream.name() == "scales")
            Utility::deserialize(stream, mScales);
        else if (stream.name() == "bounds")
            Utility::deserialize(stream, mBounds);
        else
            stream.skipCurrentElement();
    }
}

bool OptimConstraints::isEnabled(VariableType type) const
{
    return mEnabledState[type];
}

bool OptimConstraints::isUnited(VariableType type) const
{
    return mUnitedState[type];
}

bool OptimConstraints::isMultiplied(VariableType type) const
{
    return mMultipliedState[type];
}

bool OptimConstraints::isNonzero(VariableType type) const
{
    return mNonzeroState[type];
}

double OptimConstraints::scale(VariableType type) const
{
    return mScales[type];
}

PairDouble OptimConstraints::bounds(VariableType type) const
{
    return mBounds[type];
}

//! Retrieve all variable types
QList<VariableType> OptimConstraints::types()
{
    constexpr auto values = magic_enum::enum_values<VariableType>();
    int numValues = values.size();
    QList<VariableType> result(numValues);
    for (int i = 0; i != numValues; ++i)
        result[i] = values[i];
    return result;
}

//! Enable all the variables for updating
void OptimConstraints::setAllEnabled(bool flag)
{
    QList<VariableType> const keys = types();
    for (VariableType key : keys)
        setEnabled(key, flag);
}

//! Set united state of all the variables
void OptimConstraints::setAllUnited(bool flag)
{
    QList<VariableType> const keys = types();
    for (VariableType key : keys)
        setUnited(key, flag);
}

//! Set multiplied state of all the variables
void OptimConstraints::setAllMultiplied(bool flag)
{
    QList<VariableType> const keys = types();
    for (VariableType key : keys)
        setMultiplied(key, flag);
}

//! Set nonzero state of all the variables
void OptimConstraints::setAllNonzero(bool flag)
{
    QList<VariableType> const keys = types();
    for (VariableType key : keys)
        setNonzero(key, flag);
}

//! Set scale of all the variables
void OptimConstraints::setAllScale(double value)
{
    QList<VariableType> const keys = types();
    for (VariableType key : keys)
        setScale(key, value);
}

//! Set all boundaries to infinite values
void OptimConstraints::setAllInfiniteBounds()
{
    QList<VariableType> const keys = types();
    for (VariableType key : keys)
        setInfiniteBounds(key);
}

//! Enable the variable for updating
void OptimConstraints::setEnabled(VariableType type, bool flag)
{
    mEnabledState[type] = flag;
}

//! Set united state of variables
void OptimConstraints::setUnited(VariableType type, bool flag)
{
    if (flag && mMultipliedState[type])
    {
        QString typeName = magic_enum::enum_name(type).data();
        qWarning() << QObject::tr("Multiplication is already enabled for type: %1. Unification request is ignored").arg(typeName);
        return;
    }
    mUnitedState[type] = flag;
}

//! Set multiplied state of variables
void OptimConstraints::setMultiplied(VariableType type, bool flag)
{
    if (flag && mUnitedState[type])
    {
        QString typeName = magic_enum::enum_name(type).data();
        qWarning() << QObject::tr("Unification is already enabled for type: %1. Multiplication request is ignored").arg(typeName);
        return;
    }
    mMultipliedState[type] = flag;
}

//! Set nonzero state of variables
void OptimConstraints::setNonzero(VariableType type, bool flag)
{
    if (flag && (mUnitedState[type] || mMultipliedState[type]))
    {
        QString typeName = magic_enum::enum_name(type).data();
        qWarning() << QObject::tr("Unification or multiplication is already enabled for type: %1. Nonzero request is ignored").arg(typeName);
        return;
    }
    mNonzeroState[type] = flag;
}

//! Set the variable scaling factor
void OptimConstraints::setScale(VariableType type, double value)
{
    mScales[type] = value;
}

//! Assign the variable boundaries
void OptimConstraints::setBounds(VariableType type, PairDouble const& bounds)
{
    mBounds[type] = bounds;
}

//! Assign the variable infinite boundaries
void OptimConstraints::setInfiniteBounds(VariableType type)
{
    double inf = std::numeric_limits<double>::infinity();
    mBounds[type] = {-inf, inf};
}

//! Enable default variables
void OptimConstraints::setDefaultEnabled()
{
    // Beams
    mEnabledState[VariableType::kBeamStiffness] = true;
    // Panels
    mEnabledState[VariableType::kYoungsModulus1] = true;
    mEnabledState[VariableType::kYoungsModulus2] = true;
    mEnabledState[VariableType::kShearModulus] = true;
    mEnabledState[VariableType::kPoissonRatio] = false;
    // Springs
    mEnabledState[VariableType::kSpringStiffness] = true;
}

//! Set default united state
void OptimConstraints::setDefaultUnited()
{
    setAllUnited(false);
}

//! Set default multiplied state
void OptimConstraints::setDefaultMultiplied()
{
    setAllMultiplied(true);
    mMultipliedState[VariableType::kSpringStiffness] = false;
}

//! Set default nonzero state
void OptimConstraints::setDefaultNonzero()
{
    setAllNonzero(false);
    mNonzeroState[VariableType::kSpringStiffness] = true;
}

//! Set default scales
void OptimConstraints::setDefaultScales()
{
    // Beams
    mScales[VariableType::kBeamStiffness] = 1e-4;
    // Panels
    mScales[VariableType::kThickness] = 1e2;
    mScales[VariableType::kYoungsModulus1] = 1e-8;
    mScales[VariableType::kYoungsModulus1] = mScales[VariableType::kYoungsModulus2];
    mScales[VariableType::kShearModulus] = mScales[VariableType::kYoungsModulus1];
    mScales[VariableType::kPoissonRatio] = 1;
    // Springs
    mScales[VariableType::kSpringStiffness] = 0;
}

//! Set default boundaries
void OptimConstraints::setDefaultBounds()
{
    // Beams
    mBounds[VariableType::kBeamStiffness] = {0, 1e9};
    // Panels
    mBounds[VariableType::kThickness] = {1e-3, 0.2};
    mBounds[VariableType::kYoungsModulus1] = {1e2, 1e13};
    mBounds[VariableType::kYoungsModulus2] = mBounds[VariableType::kYoungsModulus1];
    mBounds[VariableType::kShearModulus] = mBounds[VariableType::kYoungsModulus1];
    mBounds[VariableType::kPoissonRatio] = {0, 1};
    // Springs
    mBounds[VariableType::kSpringStiffness] = {1e-9, 1e9};
}
