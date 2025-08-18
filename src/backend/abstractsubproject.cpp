#include "abstractsubproject.h"

using namespace Backend::Core;

AbstractSubproject::AbstractSubproject(SubprojectType type)
    : mkType(type)
{
}

AbstractSubproject::~AbstractSubproject()
{
}

SubprojectType AbstractSubproject::type() const
{
    return mkType;
}
