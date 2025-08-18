#ifndef ABSTRACTSUBPROJECT_H
#define ABSTRACTSUBPROJECT_H

#include <QString>

#include "identifier.h"

namespace Backend::Core
{

enum class SubprojectType
{
    kKCL,
    kExperimental
};

class AbstractSubproject : public Identifier
{
public:
    AbstractSubproject(SubprojectType type);
    virtual ~AbstractSubproject() = 0;

    virtual QString const& name() const = 0;
    SubprojectType type() const;

private:
    SubprojectType const mkType;
};

}

#endif // ABSTRACTSUBPROJECT_H
