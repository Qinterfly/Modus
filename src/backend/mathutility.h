#ifndef MATHUTILITY_H
#define MATHUTILITY_H

#include <Eigen/Core>
#include <QString>
#include <QUuid>

#include "aliasdata.h"

namespace Backend::Utility
{

template<typename Objects>
QList<QUuid> getIDs(Objects const& objects);

template<typename Objects>
int getIndexByID(Objects const& objects, QUuid const& id);

template<typename Objects>
int getIndexByName(Objects const& objects, QString const& name, Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive);

template<typename T>
QList<T> combine(QList<T> const& first, QList<T> const& second);

template<typename T>
T solve(std::function<T()> fun, double timeout);

Eigen::VectorXi rowIndicesAbsMax(Eigen::MatrixXd const& data);
double computeMAC(Eigen::VectorXd const& first, Eigen::VectorXd const& second);
double computeMAC(Eigen::MatrixXd const& first, Eigen::MatrixXd const& second, Backend::Core::Matches const& matches);
Core::ModalPairs pairByMAC(Eigen::MatrixXd const& MAC, double threshold);
}

#endif // MATHUTILITY_H
