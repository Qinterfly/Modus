#include <kcl/model.h>
#include <QDebug>
#include <QObject>
#include <QXmlStreamWriter>

#include "fileutility.h"
#include "selector.h"

using namespace Backend::Core;

Selector::Selector()
{
}

//! Insert a new selection set
SelectionSet& Selector::add(KCL::Model const& model, QString const& name)
{
    if (contains(name))
        qWarning() << QObject::tr("The selection set named %1 has been created already. Choose a different name").arg(name);
    else
        mSets.emplaceBack(SelectionSet(model, name));
    return mSets[find(name)];
}

//! Update the selection sets according to a new model
void Selector::update(KCL::Model const& model)
{
    for (auto& set : mSets)
        set.update(model);
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
QList<Backend::Core::SelectionSet> const& Selector::get() const
{
    return mSets;
}

//! Get the selection set located at the specified index
SelectionSet const& Selector::get(int index) const
{
    return mSets[index];
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

//! Merge all the selection sets into one
QList<Selection> Selector::allSelections() const
{
    QMap<Selection, bool> mapSelections;
    for (SelectionSet const& set : mSets)
    {
        auto setSelections = set.selections();
        for (auto const [selection, flag] : setSelections.asKeyValueRange())
        {
            if (flag)
                mapSelections[selection] = true;
        }
    }
    return mapSelections.keys();
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

bool Selector::operator==(Selector const& another) const
{
    return Utility::areEqual(*this, another);
}

bool Selector::operator!=(Selector const& another) const
{
    return !(*this == another);
}

void Selector::serialize(QXmlStreamWriter& stream) const
{
    Utility::serialize(stream, *this);
}

void Selector::deserialize(QXmlStreamReader& stream)
{
    // TODO
}

QString Selector::elementName() const
{
    return "selector";
}

QXmlStreamWriter& operator<<(QXmlStreamWriter& stream, Selector const& selector)
{
    selector.serialize(stream);
    return stream;
}

QXmlStreamReader& operator>>(QXmlStreamReader& stream, Selector& selector)
{
    selector.deserialize(stream);
    return stream;
}
