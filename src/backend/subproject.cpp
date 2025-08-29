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

//! Output configuration to a XML stream
void Configuration::serialize(QXmlStreamWriter& stream) const
{
    stream.writeStartElement("configuration");
    Utility::serialize(stream, *this);
    stream.writeEndElement();
}

//! Read configuration from a XML stream
void Configuration::deserialize(QXmlStreamWriter& stream)
{
    // TODO
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

//! Output subproject to a XML stream
void Subproject::serialize(QXmlStreamWriter& stream) const
{
    stream.writeStartElement("subproject");
    Utility::serialize(stream, *this);
    stream.writeEndElement();
}

//! Read subproject from a XML stream
void Subproject::deserialize(QXmlStreamWriter& stream)
{
    // TODO
}
