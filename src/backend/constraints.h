#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H

#include <QMap>

namespace Backend::Core
{

using PairDouble = QPair<double, double>;

//! Types of variables to be used for updating
enum class VariableType
{
    // Beams
    kBendingStiffness,
    kTorsionalStiffness,
    // Panels
    kThickness,
    kYoungsModulus1,
    kYoungsModulus2,
    kShearModulus,
    kPoissonRatio,
    // Springs
    kSpringStiffness
};

//! Updating constraints applied to variables
class Constraints
{
public:
    Constraints();
    ~Constraints() = default;

    static QList<VariableType> types();

    bool isEnabled(VariableType type) const;
    bool isUnited(VariableType type) const;
    bool isMultiplied(VariableType type) const;
    double scale(VariableType type) const;
    PairDouble limits(VariableType type) const;

    void setAllEnabled(bool flag);
    void setAllUnited(bool flag);
    void setAllMultiplied(bool flag);

    void setEnabled(VariableType type, bool flag);
    void setUnited(VariableType type, bool flag);
    void setMultiplied(VariableType type, bool flag);
    void setScale(VariableType type, double value);
    void setLimits(VariableType type, PairDouble const& limits);

private:
    QMap<VariableType, bool> mEnabledState;
    QMap<VariableType, bool> mUnitedState;
    QMap<VariableType, bool> mMultipliedState;
    QMap<VariableType, double> mScales;
    QMap<VariableType, PairDouble> mLimits;
};

}

#endif // CONSTRAINTS_H
