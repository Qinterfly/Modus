#ifndef SELECTOR_H
#define SELECTOR_H

#include <QList>

#include "selectionset.h"

namespace KCL
{
struct Model;
}

namespace Backend::Core
{

//! Class to handle selection sets of a model
class Selector
{
public:
    Selector();
    Selector(KCL::Model const* pModel);
    ~Selector() = default;

    void setModel(KCL::Model const* pModel);
    SelectionSet& add(QString const& name = QString());
    bool remove(QString const& name);
    void clear();
    QList<Backend::Core::SelectionSet>& get();
    SelectionSet& get(int index);
    int find(QString const& name) const;
    bool contains(QString const& name) const;
    int numSets() const;
    bool isEmpty() const;

private:
    KCL::Model const* mpModel;
    QList<Backend::Core::SelectionSet> mSets;
};

}

#endif // SELECTOR_H
