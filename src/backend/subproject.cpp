#include "subproject.h"

using namespace Backend::Core;

Subproject::Subproject()
{
}

Subproject::Subproject(QString const& name)
{
    mConfiguration.name = name;
}

QString const& Subproject::name() const
{
    return mConfiguration.name;
}

Configuration const& Subproject::configuration() const
{
    return mConfiguration;
}

KCL::Model const& Subproject::model() const
{
    return mModel;
}

Configuration& Subproject::configuration()
{
    return mConfiguration;
}

KCL::Model& Subproject::model()
{
    return mModel;
}
