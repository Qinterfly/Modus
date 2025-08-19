#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H

#include <QMap>

namespace Backend::Core
{

//! Types of variables to be used for updating
enum class VariableType
{
    // Beams
    kBendingStiffness,
    kTorsionalStiffness,
    // Panels
    kThickness,
    kYoungsModulus,
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
    QPair<double, double> limits(VariableType type) const;

    void enable(VariableType type, bool flag);
    void unite(VariableType type, bool flag);
    void multiply(VariableType type, bool flag);
    void setScale(VariableType type, double value);
    void setLimits(VariableType type, QPair<double, double> const& limits);

private:
    QMap<VariableType, bool> mEnabledState;
    QMap<VariableType, bool> mUnitedState;
    QMap<VariableType, bool> mMultipliedState;
    QMap<VariableType, double> mScales;
    QMap<VariableType, QPair<double, double>> mLimits;
};

}

#endif // CONSTRAINTS_H
