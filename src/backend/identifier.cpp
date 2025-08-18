#include "identifier.h"

using namespace Backend::Core;

Identifier::Identifier()
    : mID(QUuid::createUuid())
{
}

QUuid const& Identifier::id() const
{
    return mID;
}
