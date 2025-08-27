#ifndef ISERIALIZABLE_H
#define ISERIALIZABLE_H

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QXmlStreamWriter)

namespace Backend::Core
{

class ISerializable
{
public:
    virtual void serialize(QXmlStreamWriter& stream) const = 0;
    virtual void deserialize(QXmlStreamWriter& stream) = 0;
};

}

#endif // ISERIALIZABLE_H
