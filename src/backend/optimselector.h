#ifndef OPTIMSELECTOR_H
#define OPTIMSELECTOR_H

#include <QList>

#include "selectionset.h"

namespace KCL
{
struct Model;
}

namespace Backend::Core
{

//! Class to handle selection sets of a model
class OptimSelector : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(QList<Backend::Core::SelectionSet> selectionSets MEMBER mSelectionSets)

public:
    OptimSelector();
    ~OptimSelector();

    SelectionSet& add(KCL::Model const& model, QString const& name = QString());
    bool remove(int index);
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

    bool operator==(OptimSelector const& another) const;
    bool operator!=(OptimSelector const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

private:
    QList<Backend::Core::SelectionSet> mSelectionSets;
};
}

#endif // OPTIMSELECTOR_H
