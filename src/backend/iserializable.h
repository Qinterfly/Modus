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
    virtual void serialize(QXmlStreamWriter& stream, QString const& elementName) const = 0;
    virtual void deserialize(QXmlStreamReader& stream) = 0;
};
}

#endif // ISERIALIZABLE_H
