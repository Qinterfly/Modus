#ifndef UIALIASDATA_H
#define UIALIASDATA_H

#include <Eigen/Geometry>

namespace Frontend
{

class IntLineEdit;
class DoubleLineEdit;
using LocalEdits = std::array<DoubleLineEdit*, 2>;
using GlobalEdits = std::array<DoubleLineEdit*, 3>;

using Transformation = Eigen::Transform<double, 3, Eigen::Affine>;
using Matrix42d = Eigen::Matrix<double, 4, 2>;
using Matrix43d = Eigen::Matrix<double, 4, 3>;

struct Point
{
    double x;
    double y;
};
}

#endif // UIALIASDATA_H
