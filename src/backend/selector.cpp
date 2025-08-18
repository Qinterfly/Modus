#include <kcl/model.h>
#include <QDebug>
#include <QObject>

#include "selector.h"

using namespace Backend::Core;

Selector::Selector()
    : mpModel(nullptr)
{
}

Selector::Selector(KCL::Model const* pModel)
    : mpModel(pModel)
{
}

//! Substitute the model
void Selector::setModel(KCL::Model const* pModel)
{
    mpModel = pModel;
    for (SelectionSet& set : mSets)
        set.setModel(mpModel);
}

//! Insert a new selection set
SelectionSet& Selector::add(QString const& name)
{
    if (contains(name))
        qWarning() << QObject::tr("The selection set named %1 has been created already. Choose a different name").arg(name);
    else
        mSets.emplaceBack(SelectionSet(mpModel, name));
    return mSets[find(name)];
}

//! Remove a set associated with a given name
bool Selector::remove(QString const& name)
{
    if (contains(name))
    {
        mSets.remove(find(name));
        return true;
    }
    return false;
}

//! Remove all the sets
void Selector::clear()
{
    mSets.clear();
}

//! Get all the selection sets
QList<SelectionSet>& Selector::get()
{
    return mSets;
}

//! Get the selection set located at the specified index
SelectionSet& Selector::get(int index)
{
    return mSets[index];
}

//! Find a selection set by a name
int Selector::find(QString const& name) const
{
    int iFound = -1;
    int numSets = mSets.size();
    for (int i = 0; i != numSets; ++i)
    {
        if (mSets[i].name() == name)
        {
            iFound = i;
            break;
        }
    }
    return iFound;
}

//! Check if the selection set of the specified name exists
bool Selector::contains(QString const& name) const
{
    return find(name) >= 0;
}

//! Acquire the number of sets
int Selector::numSets() const
{
    return mSets.size();
}

//! Check if there are any selection sets
bool Selector::isEmpty() const
{
    return numSets() == 0;
}
