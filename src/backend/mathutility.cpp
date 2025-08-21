#include <QList>

#include "mathutility.h"
#include "subproject.h"

using namespace Backend;

namespace Backend::Utility
{

//! Retrieve identifiers from the set of objects
template<typename Objects>
QList<QUuid> getIDs(Objects const& objects)
{
    QList<QUuid> result;
    uint numObjects = objects.size();
    result.reserve(numObjects);
    for (auto const& item : objects)
        result.push_back(item.id());
    return result;
}

//! Retrieve the index of the objects which has the specified identifier
template<typename Objects>
int getIndexByID(Objects const& objects, QUuid const& id)
{
    int numObjects = objects.size();
    for (int i = 0; i != numObjects; ++i)
    {
        if (objects[i].id() == id)
            return i;
    }
    return -1;
}

//! Retrieve the index of the objects which has the specified name
template<typename Objects>
int getIndexByName(Objects const& objects, QString const& name, Qt::CaseSensitivity sensitivity)
{
    int numObjects = objects.size();
    for (int i = 0; i != numObjects; ++i)
    {
        if (QString::compare(objects[i].name(), name, sensitivity) == 0)
            return i;
    }
    return -1;
}

template<typename T>
QList<T> combine(QList<T> const& first, QList<T> const& second)
{
    int numFirst = first.size();
    int numSecond = second.size();
    QList<T> result(numFirst + numSecond);
    for (int i = 0; i != numFirst; ++i)
        result[i] = first[i];
    for (int i = 0; i != numSecond; ++i)
        result[numFirst + i] = second[i];
    return result;
}

Eigen::VectorXi rowIndicesAbsMax(Eigen::MatrixXd const& data)
{
    int numRows = data.rows();
    int numCols = data.cols();
    Eigen::VectorXi result(numRows);
    for (int i = 0; i != numRows; ++i)
    {
        int iMax = -1;
        double absMax = 0.0;
        for (int j = 0; j != numCols; ++j)
        {
            double absValue = std::abs(data(i, j));
            if (absValue > absMax)
            {
                absMax = absValue;
                iMax = j;
            }
        }
        result[i] = iMax;
    }
    return result;
}

// Explicit template instantiation
template QList<QUuid> getIDs(QList<Core::Subproject> const&);
template int getIndexByID(QList<Core::Subproject> const&, QUuid const&);
template int getIndexByName(QList<Core::Subproject> const&, QString const&, Qt::CaseSensitivity);
template QList<double> combine(QList<double> const& first, QList<double> const& second);
template QList<QPair<double, double>> combine(QList<QPair<double, double>> const& first, QList<QPair<double, double>> const& second);
}
