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
    ~Selector() = default;

    SelectionSet& add(KCL::Model const& model, QString const& name);
    bool remove(QString const& name);
    void clear();
    void update(KCL::Model const& model);

    QList<Backend::Core::SelectionSet> const& get() const;
    SelectionSet const& get(int index) const;
    QList<Backend::Core::SelectionSet>& get();
    SelectionSet& get(int index);
    QList<Selection> allSelections() const;

    int find(QString const& name) const;
    bool contains(QString const& name) const;
    int numSets() const;
    bool isEmpty() const;

private:
    QList<Backend::Core::SelectionSet> mSets;
};

}

#endif // SELECTOR_H
