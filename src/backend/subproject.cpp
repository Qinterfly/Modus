#include "subproject.h"
#include "fileutility.h"

using namespace Backend::Core;

Configuration::Configuration()
{
}

bool Configuration::operator==(Configuration const& another) const
{
    return Utility::areEqual(*this, another);
}

bool Configuration::operator!=(Configuration const& another) const
{
    return !(*this == another);
}

void Configuration::serialize(QXmlStreamWriter& stream) const
{
    Utility::serialize(stream, *this);
}

void Configuration::deserialize(QXmlStreamWriter& stream)
{
    // TODO
}

QString Configuration::elementName() const
{
    return "configuration";
}

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

bool Subproject::operator==(Subproject const& another) const
{
    return Utility::areEqual(*this, another);
}

bool Subproject::operator!=(Subproject const& another) const
{
    return !(*this == another);
}

void Subproject::serialize(QXmlStreamWriter& stream) const
{
    Utility::serialize(stream, *this);
}

void Subproject::deserialize(QXmlStreamWriter& stream)
{
    // TODO
}

QString Subproject::elementName() const
{
    return "subproject";
}
