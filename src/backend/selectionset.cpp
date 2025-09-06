#include <kcl/model.h>
#include <QMap>

#include "constants.h"
#include "fileutility.h"
#include "selectionset.h"

using namespace Backend;
using namespace Backend::Core;

SelectionSet::SelectionSet()
{
}

SelectionSet::SelectionSet(KCL::Model const& model, QString const& name)
    : mName(name)
{
    reset(model);
}

QString const& SelectionSet::name() const
{
    return mName;
}

bool SelectionSet::isSelected(Selection const& selection) const
{
    if (mDataSet.contains(selection))
        return mDataSet[selection];
    else
        return false;
}

int SelectionSet::numSelected() const
{
    int result = 0;
    QList<Selection> const selections = mDataSet.keys();
    for (Selection const& selection : selections)
    {
        if (mDataSet[selection])
            ++result;
    }
    return result;
}

QMap<Selection, bool> const& SelectionSet::selections() const
{
    return mDataSet;
}

//! Select all elements
void SelectionSet::selectAll()
{
    QList<Selection> const selections = mDataSet.keys();
    for (Selection const& selection : selections)
        mDataSet[selection] = true;
}

//! Deselect all elements
void SelectionSet::selectNone()
{
    QList<Selection> const selections = mDataSet.keys();
    for (Selection const& selection : selections)
        mDataSet[selection] = false;
}

//! Inverse the selections
void SelectionSet::inverse()
{
    QList<Selection> const selections = mDataSet.keys();
    for (Selection const& selection : selections)
    {
        bool flag = mDataSet[selection];
        mDataSet[selection] = !flag;
    }
}

//! Set the selected state of the element
void SelectionSet::setSelected(Selection const& selection, bool flag)
{
    if (mDataSet.contains(selection))
        mDataSet[selection] = flag;
}

//! Set the selected state by surface index
void SelectionSet::setSelected(int iSurface, bool flag)
{
    QList<Selection> const selections = mDataSet.keys();
    for (Selection const& selection : selections)
    {
        if (selection.iSurface == iSurface)
            mDataSet[selection] = flag;
    }
}

//! Set the selected state by element type
void SelectionSet::setSelected(KCL::ElementType type, bool flag)
{
    QList<Selection> const selections = mDataSet.keys();
    for (Selection const& selection : selections)
    {
        if (selection.type == type)
            mDataSet[selection] = flag;
    }
}

//! Set the selected state by surface index and element type
void SelectionSet::setSelected(int iSurface, KCL::ElementType type, bool flag)
{
    QList<Selection> const selections = mDataSet.keys();
    for (Selection const& selection : selections)
    {
        if (selection.iSurface == iSurface && selection.type == type)
            mDataSet[selection] = flag;
    }
}

//! Clean up the selections
void SelectionSet::reset(KCL::Model const& model)
{
    mDataSet.clear();

    // Process elastic surfaces
    auto const& surfaces = model.surfaces;
    int numSurfaces = surfaces.size();
    for (int iSurface = 0; iSurface != numSurfaces; ++iSurface)
    {
        auto const& surface = surfaces[iSurface];
        auto types = surface.types();
        int numTypes = types.size();
        for (int iType = 0; iType != numTypes; ++iType)
        {
            auto type = types[iType];
            int numElements = surface.numElements(type);
            for (int iElement = 0; iElement != numElements; ++iElement)
            {
                Selection selection;
                selection.iSurface = iSurface;
                selection.type = type;
                selection.iElement = iElement;
                mDataSet[selection] = false;
            }
        }
    }

    // Process the special surface
    auto types = model.specialSurface.types();
    int numTypes = types.size();
    for (int iType = 0; iType != numTypes; ++iType)
    {
        auto type = types[iType];
        int numElements = model.specialSurface.numElements(type);
        for (int iElement = 0; iElement != numElements; ++iElement)
        {
            Selection selection;
            selection.iSurface = Constants::skISpecialSurface;
            selection.type = type;
            selection.iElement = iElement;
            mDataSet[selection] = false;
        }
    }
}

//! Update the selected items in case the model has been changed
void SelectionSet::update(KCL::Model const& model)
{
    QMap<Selection, bool> const oldDataSet = mDataSet;
    reset(model);
    for (auto const& [selection, flag] : oldDataSet.asKeyValueRange())
    {
        if (mDataSet.contains(selection))
            mDataSet[selection] = flag;
    }
}

bool SelectionSet::operator==(SelectionSet const& another) const
{
    return Utility::areEqual(*this, another);
}

bool SelectionSet::operator!=(SelectionSet const& another) const
{
    return !(*this == another);
}

QString SelectionSet::elementName() const
{
    return "selectionSet";
}

void SelectionSet::serialize(QXmlStreamWriter& stream) const
{
    Utility::serialize(stream, *this);
}

void SelectionSet::deserialize(QXmlStreamReader& stream)
{
    // TODO
}

Selection::Selection()
    : iSurface(-1)
    , type(KCL::ElementType::OD)
    , iElement(-1)
{
}

bool Selection::isValid() const
{
    return iSurface >= 0 && iElement >= 0;
}

bool Selection::operator==(Selection const& another) const
{
    return std::tie(iSurface, type, iElement) == std::tie(another.iSurface, another.type, another.iElement);
}

bool Selection::operator!=(Selection const& another) const
{
    return !(*this == another);
}

bool Selection::operator<(Selection const& another) const
{
    return std::tie(iSurface, type, iElement) < std::tie(another.iSurface, another.type, another.iElement);
}

bool Selection::operator>(Selection const& another) const
{
    return !(*this < another);
}

bool Selection::operator<=(Selection const& another) const
{
    return *this < another || *this == another;
}

bool Selection::operator>=(Selection const& another) const
{
    return *this > another || *this == another;
}

void Selection::serialize(QXmlStreamWriter& stream) const
{
    Utility::serialize(stream, *this);
}

void Selection::deserialize(QXmlStreamReader& stream)
{
    // TODO
}

QString Selection::elementName() const
{
    return "selection";
}

QXmlStreamWriter& operator<<(QXmlStreamWriter& stream, Selection const& selection)
{
    selection.serialize(stream);
    return stream;
}

QXmlStreamReader& operator>>(QXmlStreamReader& stream, Selection& selection)
{
    selection.deserialize(stream);
    return stream;
}

QXmlStreamWriter& operator<<(QXmlStreamWriter& stream, SelectionSet const& selectionSet)
{
    selectionSet.serialize(stream);
    return stream;
}

QXmlStreamReader& operator>>(QXmlStreamReader& stream, SelectionSet& selectionSet)
{
    selectionSet.deserialize(stream);
    return stream;
}
