#ifndef KCLSUBPROJECT_H
#define KCLSUBPROJECT_H

#include <kcl/model.h>
#include <QString>

#include "abstractsubproject.h"
#include "optimsolver.h"
#include "selector.h"

namespace Backend::Core
{

struct KCLConfiguration
{
    QString name;
    OptimOptions options;
};

class KCLSubproject : public AbstractSubproject
{
public:
    KCLSubproject();
    KCLSubproject(QString const& name);
    ~KCLSubproject() = default;

    QString const& name() const override;
    KCLConfiguration const& configuration() const;
    KCL::Model const& model() const;

    KCLConfiguration& configuration();
    KCL::Model& model();

private:
    KCLConfiguration mConfiguration;
    KCL::Model mModel;
};

}

#endif // KCLSUBPROJECT_H
