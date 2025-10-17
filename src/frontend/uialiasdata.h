#ifndef UIALIASDATA_H
#define UIALIASDATA_H

#include <Eigen/Geometry>
#include <QList>

namespace Frontend
{

class IntLineEdit;
class DoubleLineEdit;
using Edit1i = IntLineEdit;
using Edit1d = DoubleLineEdit;
using Edits2i = std::array<Edit1i*, 2>;
using Edits2d = std::array<Edit1d*, 2>;
using Edits3d = std::array<Edit1d*, 3>;
using EditsXd = QList<Edit1d*>;

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
