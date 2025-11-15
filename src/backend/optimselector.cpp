#include <kcl/model.h>
#include <QDebug>
#include <QObject>
#include <QXmlStreamWriter>

#include "fileutility.h"
#include "optimselector.h"

using namespace Backend::Core;

OptimSelector::OptimSelector()
{
}

OptimSelector::~OptimSelector()
{
}

//! Insert a new selection set
SelectionSet& OptimSelector::add(KCL::Model const& model, QString const& name)
{
    mSelectionSets.emplaceBack(SelectionSet(model, name));
    return mSelectionSets[find(name)];
}

//! Update the selection sets according to a new model
void OptimSelector::update(KCL::Model const& model)
{
    for (auto& set : mSelectionSets)
        set.update(model);
}

//! Remove a set associated with a given index
bool OptimSelector::remove(int index)
{
    if (index < numSets() && index >= 0)
        mSelectionSets.remove(index);
    return false;
}

//! Remove all the sets
void OptimSelector::clear()
{
    mSelectionSets.clear();
}

//! Get all the selection sets
QList<Backend::Core::SelectionSet> const& OptimSelector::get() const
{
    return mSelectionSets;
}

//! Get the selection set located at the specified index
SelectionSet const& OptimSelector::get(int index) const
{
    return mSelectionSets[index];
}

//! Get all the selection sets
QList<SelectionSet>& OptimSelector::get()
{
    return mSelectionSets;
}

//! Get the selection set located at the specified index
SelectionSet& OptimSelector::get(int index)
{
    return mSelectionSets[index];
}

//! Merge all the selection sets into one
QList<Selection> OptimSelector::allSelections() const
{
    QMap<Selection, bool> mapSelections;
    for (SelectionSet const& set : mSelectionSets)
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
int OptimSelector::find(QString const& name) const
{
    int iFound = -1;
    int numSets = mSelectionSets.size();
    for (int i = 0; i != numSets; ++i)
    {
        if (mSelectionSets[i].name() == name)
        {
            iFound = i;
            break;
        }
    }
    return iFound;
}

//! Check if the selection set of the specified name exists
bool OptimSelector::contains(QString const& name) const
{
    return find(name) >= 0;
}

//! Acquire the number of sets
int OptimSelector::numSets() const
{
    return mSelectionSets.size();
}

//! Check if there are any selection sets
bool OptimSelector::isEmpty() const
{
    return numSets() == 0;
}

bool OptimSelector::operator==(OptimSelector const& another) const
{
    return Utility::areEqual(*this, another);
}

bool OptimSelector::operator!=(OptimSelector const& another) const
{
    return !(*this == another);
}

void OptimSelector::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    Utility::serialize(stream, "selectionSets", "selectionSet", mSelectionSets);
    stream.writeEndElement();
}

void OptimSelector::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "selectionSets")
            Utility::deserialize(stream, "selectionSet", mSelectionSets);
        else
            stream.skipCurrentElement();
    }
}
