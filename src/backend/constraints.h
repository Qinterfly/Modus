#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H

#include <QMap>
#include <QMetaType>

#include "aliasdata.h"
#include "iserializable.h"

namespace Backend::Core
{

//! Types of variables to be used for updating
enum class VariableType
{
    // Beams
    kBeamStiffness,
    // Panels
    kThickness,
    kYoungsModulus1,
    kYoungsModulus2,
    kShearModulus,
    kPoissonRatio,
    // Springs
    kSpringStiffness
};

using VariableFlags = QMap<VariableType, bool>;
using VariableValues = QMap<VariableType, double>;
using VariableLimits = QMap<VariableType, PairDouble>;

//! Updating constraints applied to variables
class Constraints : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(VariableFlags enabledState MEMBER mEnabledState)
    Q_PROPERTY(VariableFlags unitedState MEMBER mUnitedState)
    Q_PROPERTY(VariableFlags multipliedState MEMBER mMultipliedState)
    Q_PROPERTY(VariableFlags nonzeroState MEMBER mNonzeroState)
    Q_PROPERTY(VariableValues scales MEMBER mScales)
    Q_PROPERTY(VariableLimits limits MEMBER mLimits)

public:
    Constraints();
    ~Constraints() = default;

    bool operator==(Constraints const& another) const;
    bool operator!=(Constraints const& another) const;

    void serialize(QXmlStreamWriter& stream) const override;
    void deserialize(QXmlStreamWriter& stream) override;

    static QList<VariableType> types();

    bool isEnabled(VariableType type) const;
    bool isUnited(VariableType type) const;
    bool isMultiplied(VariableType type) const;
    bool isNonzero(VariableType type) const;
    double scale(VariableType type) const;
    PairDouble limits(VariableType type) const;

    void setAllEnabled(bool flag);
    void setAllUnited(bool flag);
    void setAllMultiplied(bool flag);
    void setAllNonzero(bool flag);
    void setAllScale(double value);

    void setEnabled(VariableType type, bool flag);
    void setUnited(VariableType type, bool flag);
    void setMultiplied(VariableType type, bool flag);
    void setNonzero(VariableType type, bool flag);
    void setScale(VariableType type, double value);
    void setLimits(VariableType type, PairDouble const& limits);

private:
    void setDefaultEnabled();
    void setDefaultUnited();
    void setDefaultMultiplied();
    void setDefaultNonzero();
    void setDefaultScales();
    void setDefaultLimits();

private:
    VariableFlags mEnabledState;
    VariableFlags mUnitedState;
    VariableFlags mMultipliedState;
    VariableFlags mNonzeroState;
    VariableValues mScales;
    VariableLimits mLimits;
};

}

#endif // CONSTRAINTS_H
