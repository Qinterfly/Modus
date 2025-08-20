#ifndef SELECTIONSET_H
#define SELECTIONSET_H

#include <kcl/element.h>
#include <QList>
#include <QMap>
#include <QString>

namespace KCL
{
struct Model;
}

namespace Backend::Core
{

struct Selection;

/*!
 * Class to select model entities
 * 
 * By default none of entities are selected 
 */
class SelectionSet
{
public:
    SelectionSet();
    SelectionSet(KCL::Model const& model, QString const& name);
    ~SelectionSet() = default;

    QString const& name() const;
    bool isSelected(Selection const& selection) const;
    int numSelected() const;
    QMap<Selection, bool> const& selections() const;

    void selectAll();
    void selectNone();
    void inverse();
    void setSelected(Selection const& selection, bool flag);
    void setSelected(int iSurface, bool flag);
    void setSelected(KCL::ElementType type, bool flag);
    void setSelected(int iSurface, KCL::ElementType type, bool flag);
    void update(KCL::Model const& model);

private:
    void reset(KCL::Model const& model);

private:
    QString mName;
    QMap<Selection, bool> mDataSet;
};

//! Selection information associated with an element
struct Selection
{
    Selection();
    ~Selection() = default;

    bool isValid() const;

    bool operator==(Selection const& another) const;
    bool operator<(Selection const& another) const;
    bool operator>(Selection const& another) const;
    bool operator<=(Selection const& another) const;
    bool operator>=(Selection const& another) const;

    int iSurface;
    KCL::ElementType type;
    int iElement;
};
}

#endif // SELECTIONSET_H
