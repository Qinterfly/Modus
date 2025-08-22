#ifndef SUBPROJECT_H
#define SUBPROJECT_H

#include <kcl/model.h>
#include <QString>

#include "identifier.h"
#include "optimsolver.h"
#include "selector.h"

namespace Backend::Core
{

struct Configuration
{
    QString name;
    OptimProblem problem;
    OptimOptions options;
};

class Subproject : public Identifier
{
public:
    Subproject();
    Subproject(QString const& name);
    ~Subproject() = default;

    QString const& name() const;
    Configuration const& configuration() const;
    KCL::Model const& model() const;

    Configuration& configuration();
    KCL::Model& model();

private:
    Configuration mConfiguration;
    KCL::Model mModel;
};

}

#endif // SUBPROJECT_H
