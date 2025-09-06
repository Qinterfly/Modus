#ifndef ISERIALIZABLE_H
#define ISERIALIZABLE_H

#include <QMetaProperty>
#include <QMetaType>
#include <QXmlStreamWriter>

namespace Backend::Core
{

class ISerializable
{
public:
    virtual void serialize(QXmlStreamWriter& stream) const = 0;
    virtual void deserialize(QXmlStreamReader& stream) = 0;
    virtual QString elementName() const = 0;
};
}

#endif // ISERIALIZABLE_H
