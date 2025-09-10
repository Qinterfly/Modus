#ifndef ISOLVER_H
#define ISOLVER_H

#include "identifier.h"
#include "iserializable.h"

namespace Backend::Core
{

class ISolver : public Identifier, public ISerializable
{
public:
    enum Type
    {
        kModal,
        kOptim
    };
    virtual Type type() const = 0;
    virtual ISolver* clone() const = 0;
    virtual void clear() = 0;
    virtual void solve() = 0;
    virtual bool operator==(ISolver const* pBaseSolver) const = 0;
    virtual bool operator!=(ISolver const* pBaseSolver) const = 0;
    virtual ~ISolver() = default;
};
}

#endif // ISOLVER_H
