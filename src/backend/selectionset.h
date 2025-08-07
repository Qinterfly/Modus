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

namespace Backend
{

struct Selection;

/*!
 * Class to select of model entities
 * 
 * By default none of entities are selected 
 */
class SelectionSet
{
public:
    SelectionSet();
    SelectionSet(KCL::Model const* pModel, QString const& name);
    ~SelectionSet() = default;

    KCL::Model const* model() const;
    QString const& name() const;
    bool isSelected(Selection const& selection) const;
    int numSelected() const;

    void selectAll();
    void selectNone();
    void inverse();
    void setSelected(Selection const& selection, bool flag);
    void setSelectedBySurface(int iSurface, bool flag);
    void setSelectedByType(KCL::ElementType type, bool flag);

private:
    void initialize();
    void validate();

private:
    KCL::Model const* mpModel;
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
