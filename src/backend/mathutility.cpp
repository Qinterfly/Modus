#include <future>
#include <kcl/solver.h>
#include <QList>

#include "mathutility.h"
#include "subproject.h"

using namespace Backend;
using namespace Eigen;

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

template<typename T>
T solve(std::function<T()> fun, double timeout)
{
    double const kTimeFactor = 1e6;
    if (timeout > 0)
    {
        std::future<T> future = std::async(fun);
        auto duration = std::chrono::microseconds((int) std::round(timeout * kTimeFactor));
        std::future_status status = future.wait_for(duration);
        if (status != std::future_status::ready)
            return T();
        return future.get();
    }
    else
    {
        return fun();
    }
}

VectorXi rowIndicesAbsMax(MatrixXd const& data)
{
    int numRows = data.rows();
    int numCols = data.cols();
    VectorXi result(numRows);
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

//! Compute the modal assurance criterion (MAC) between two modal vectors
double computeMAC(Eigen::VectorXd const& first, Eigen::VectorXd const& second)
{
    double numerator = std::pow(std::abs(first.dot(second)), 2.0);
    double denominator = std::abs(first.dot(first) * second.dot(second));
    return numerator / denominator;
}

//! Compute the MAC-matrix between two modeshapes
double computeMAC(MatrixXd const& first, MatrixXd const& second, Core::Matches const& matches)
{
    int numMatches = matches.size();
    int numDirections = first.cols();

    // Count the number of the common values
    int numValues = 0;
    for (int i = 0; i != numMatches; ++i)
    {
        int iFirstMatch = matches[i].first;
        int iSecondMatch = matches[i].second;
        for (int j = 0; j != numDirections; ++j)
        {
            if (!std::isnan(first(iFirstMatch, j)) && !std::isnan(second(iSecondMatch, j)))
                ++numValues;
        }
    }

    // Slice the common values
    VectorXd firstVector(numValues);
    VectorXd secondVector(numValues);
    numValues = 0;
    for (int i = 0; i != numMatches; ++i)
    {
        int iFirstMatch = matches[i].first;
        int iSecondMatch = matches[i].second;
        for (int j = 0; j != numDirections; ++j)
        {
            if (!std::isnan(first(iFirstMatch, j)) && !std::isnan(second(iSecondMatch, j)))
            {
                firstVector[numValues] = first(iFirstMatch, j);
                secondVector[numValues] = second(iSecondMatch, j);
                ++numValues;
            }
        }
    }
    return computeMAC(firstVector, secondVector);
}

//! Pair the modesets by indices of the modeshapes that maximize the MAC-criterion
Core::ModalPairs pairByMAC(MatrixXd const& MAC, double threshold)
{
    uint numRows = MAC.rows();
    uint numCols = MAC.cols();
    Core::ModalPairs result(numRows, {-1, std::nan("")});
    QList<bool> mask(numCols, true);
    for (uint iRow = 0; iRow != numRows; ++iRow)
    {
        // Find the maximum value
        double maxValue = threshold;
        int iMax = -1;
        for (uint jCol = 0; jCol != numCols; ++jCol)
        {
            double value = std::abs(MAC(iRow, jCol));
            if (value > maxValue && mask[jCol])
            {
                maxValue = value;
                iMax = jCol;
            }
        }
        // Set the result
        if (iMax >= 0)
        {
            mask[iMax] = false;
            result[iRow].first = iMax;
            result[iRow].second = maxValue;
        }
    }
    return result;
}

// Explicit template instantiation
template QList<QUuid> getIDs(QList<Core::Subproject> const&);
template int getIndexByID(QList<Core::Subproject> const&, QUuid const&);
template int getIndexByName(QList<Core::Subproject> const&, QString const&, Qt::CaseSensitivity);
template QList<double> combine(QList<double> const& first, QList<double> const& second);
template QList<QPair<double, double>> combine(QList<QPair<double, double>> const& first, QList<QPair<double, double>> const& second);
template KCL::EigenSolution solve(std::function<KCL::EigenSolution()>, double);
template KCL::FlutterSolution solve(std::function<KCL::FlutterSolution()>, double);
}
