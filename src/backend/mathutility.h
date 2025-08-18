#ifndef MATHUTILITY_H
#define MATHUTILITY_H

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
}

#endif // MATHUTILITY_H
