#ifndef SELECTOR_H
#define SELECTOR_H

#include <QList>

#include "selectionset.h"

namespace KCL
{
struct Model;
}

namespace Backend
{

//! Class to handle selection sets of a model
class Selector
{
public:
    Selector();
    Selector(KCL::Model const* pModel);
    ~Selector() = default;

    SelectionSet& add(QString const& name);
    bool remove(QString const& name);
    QList<Backend::SelectionSet>& get();
    SelectionSet& get(int index);
    int find(QString const& name) const;
    bool contains(QString const& name) const;

private:
    KCL::Model const* mpModel;
    QList<Backend::SelectionSet> mSets;
};

}

#endif // SELECTOR_H
