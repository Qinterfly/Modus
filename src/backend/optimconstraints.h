#ifndef OPTIMCONSTRAINTS_H
#define OPTIMCONSTRAINTS_H

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
using VariableBounds = QMap<VariableType, PairDouble>;

//! Updating constraints applied to variables
class OptimConstraints : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(VariableFlags enabledState MEMBER mEnabledState)
    Q_PROPERTY(VariableFlags unitedState MEMBER mUnitedState)
    Q_PROPERTY(VariableFlags multipliedState MEMBER mMultipliedState)
    Q_PROPERTY(VariableFlags nonzeroState MEMBER mNonzeroState)
    Q_PROPERTY(VariableValues scales MEMBER mScales)
    Q_PROPERTY(VariableBounds bounds MEMBER mBounds)

public:
    OptimConstraints();
    ~OptimConstraints();

    bool operator==(OptimConstraints const& another) const;
    bool operator!=(OptimConstraints const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    static QList<VariableType> types();

    bool isEnabled(VariableType type) const;
    bool isUnited(VariableType type) const;
    bool isMultiplied(VariableType type) const;
    bool isNonzero(VariableType type) const;
    double scale(VariableType type) const;
    PairDouble bounds(VariableType type) const;

    void setAllEnabled(bool flag);
    void setAllUnited(bool flag);
    void setAllMultiplied(bool flag);
    void setAllNonzero(bool flag);
    void setAllScale(double value);
    void setAllInfiniteBounds();

    void setEnabled(VariableType type, bool flag);
    void setUnited(VariableType type, bool flag);
    void setMultiplied(VariableType type, bool flag);
    void setNonzero(VariableType type, bool flag);
    void setScale(VariableType type, double value);
    void setBounds(VariableType type, PairDouble const& bounds);
    void setInfiniteBounds(VariableType type);

private:
    void setDefaultEnabled();
    void setDefaultUnited();
    void setDefaultMultiplied();
    void setDefaultNonzero();
    void setDefaultScales();
    void setDefaultBounds();

private:
    VariableFlags mEnabledState;
    VariableFlags mUnitedState;
    VariableFlags mMultipliedState;
    VariableFlags mNonzeroState;
    VariableValues mScales;
    VariableBounds mBounds;
};
}

#endif // OPTIMCONSTRAINTS_H
