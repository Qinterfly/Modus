#ifndef MATHUTILITY_H
#define MATHUTILITY_H

#include <Eigen/Core>
#include <QString>
#include <QUuid>

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

Eigen::VectorXi rowIndicesAbsMax(Eigen::MatrixXd const& data);
}

#endif // MATHUTILITY_H
