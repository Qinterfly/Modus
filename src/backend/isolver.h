#ifndef ISOLVER_H
#define ISOLVER_H

#include "iserializable.h"

namespace Backend::Core
{

class ISolver : public ISerializable
{
public:
    enum Type
    {
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
