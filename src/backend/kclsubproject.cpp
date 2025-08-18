#include "kclsubproject.h"

using namespace Backend::Core;

KCLSubproject::KCLSubproject()
    : AbstractSubproject(SubprojectType::kKCL)
{
    mConfiguration.options.selector.setModel(&mModel);
}

KCLSubproject::KCLSubproject(QString const& name)
    : KCLSubproject()
{
    mConfiguration.name = name;
}

QString const& KCLSubproject::name() const
{
    return mConfiguration.name;
}

KCLConfiguration const& KCLSubproject::configuration() const
{
    return mConfiguration;
}

KCL::Model const& KCLSubproject::model() const
{
    return mModel;
}

KCLConfiguration& KCLSubproject::configuration()
{
    return mConfiguration;
}

KCL::Model& KCLSubproject::model()
{
    return mModel;
}
