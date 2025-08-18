#include <kcl/model.h>
#include <QMap>

#include "selectionset.h"

using namespace Backend::Core;

SelectionSet::SelectionSet()
    : mpModel(nullptr)
{
}

SelectionSet::SelectionSet(KCL::Model const* pModel, QString const& name)
    : mpModel(pModel)
    , mName(name)
{
    reset();
}

KCL::Model const* SelectionSet::model() const
{
    return mpModel;
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

void SelectionSet::setModel(KCL::Model const* pModel)
{
    mpModel = pModel;
    validate();
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
void SelectionSet::reset()
{
    mDataSet.clear();
    if (mpModel == nullptr)
        return;
    auto const& surfaces = mpModel->surfaces;
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
}

//! Validate the selected items in case the model has been changed
void SelectionSet::validate()
{
    QMap<Selection, bool> const oldDataSet = mDataSet;
    reset();
    for (auto const& [selection, flag] : oldDataSet.asKeyValueRange())
    {
        if (mDataSet.contains(selection))
            mDataSet[selection] = flag;
    }
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
