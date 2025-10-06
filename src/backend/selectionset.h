#ifndef SELECTIONSET_H
#define SELECTIONSET_H

#include <kcl/element.h>
#include <QList>
#include <QMap>
#include <QMetaObject>
#include <QString>

#include "iserializable.h"

namespace KCL
{
struct Model;
}

namespace Backend::Core
{

//! Selection information associated with an element
struct Selection : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(int iSurface MEMBER iSurface)
    Q_PROPERTY(KCL::ElementType type MEMBER type)
    Q_PROPERTY(int iElement MEMBER iElement)

public:
    Selection();
    Selection(KCL::ElementType aType, int aIElement);
    Selection(int aISurface, KCL::ElementType aType, int aIElement);
    ~Selection();

    bool isValid() const;

    bool operator==(Selection const& another) const;
    bool operator!=(Selection const& another) const;
    bool operator<(Selection const& another) const;
    bool operator>(Selection const& another) const;
    bool operator<=(Selection const& another) const;
    bool operator>=(Selection const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    int iSurface;
    KCL::ElementType type;
    int iElement;
};

/*!
 * Class to select model entities
 * 
 * By default none of entities are selected 
 */
class SelectionSet : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(QString name MEMBER mName)
    Q_PROPERTY(QMap<Selection, bool> dataSet MEMBER mDataSet)

public:
    SelectionSet();
    SelectionSet(KCL::Model const& model, QString const& name);
    ~SelectionSet();

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

    bool operator==(SelectionSet const& another) const;
    bool operator!=(SelectionSet const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

private:
    void reset(KCL::Model const& model);

private:
    QString mName;
    QMap<Selection, bool> mDataSet;
};

inline size_t qHash(const Selection& key, size_t seed)
{
    return qHashMulti(seed, key.iSurface, (int) key.type, key.iElement);
}
}

#endif // SELECTIONSET_H
