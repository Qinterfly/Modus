#ifndef UIALIASDATA_H
#define UIALIASDATA_H

#include <Eigen/Geometry>

namespace Frontend
{

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
