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
class Selector : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(QList<Backend::Core::SelectionSet> selectionSets MEMBER mSelectionSets)

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

    bool operator==(Selector const& another) const;
    bool operator!=(Selector const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

private:
    QList<Backend::Core::SelectionSet> mSelectionSets;
};

QXmlStreamWriter& operator<<(QXmlStreamWriter& stream, Selector const& selector);
QXmlStreamReader& operator>>(QXmlStreamReader& stream, Selector& selector);
}

#endif // SELECTOR_H
