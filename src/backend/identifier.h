
#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <QUuid>

namespace Backend::Core
{

//! Unique identifier of an entity
class Identifier
{
public:
    Identifier();
    ~Identifier() = default;

    QUuid const& id() const;

protected:
    QUuid mID;
};

}

#endif // IDENTIFIER_H
