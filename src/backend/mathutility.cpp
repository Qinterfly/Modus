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

// Explicit template instantiation
template QList<QUuid> getIDs(QList<Core::Subproject> const&);
template int getIndexByID(QList<Core::Subproject> const&, QUuid const&);
template int getIndexByName(QList<Core::Subproject> const&, QString const&, Qt::CaseSensitivity);
}
