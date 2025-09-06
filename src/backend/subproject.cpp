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

void Configuration::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "name")
            name = stream.readElementText();
        else if (stream.name() == problem.elementName())
            problem.deserialize(stream);
        else if (stream.name() == options.elementName())
            options.deserialize(stream);
        else
            stream.skipCurrentElement();
    }
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

void Subproject::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == mConfiguration.elementName())
            mConfiguration.deserialize(stream);
        else if (stream.name() == "model")
            Utility::deserialize(stream, mModel);
        else
            stream.skipCurrentElement();
    }
}

QString Subproject::elementName() const
{
    return "subproject";
}

QXmlStreamWriter& operator<<(QXmlStreamWriter& stream, Configuration const& configuration)
{
    configuration.serialize(stream);
    return stream;
}

QXmlStreamReader& operator>>(QXmlStreamReader& stream, Configuration& configuration)
{
    configuration.deserialize(stream);
    return stream;
}

QXmlStreamWriter& operator<<(QXmlStreamWriter& stream, Subproject const& subproject)
{
    subproject.serialize(stream);
    return stream;
}

QXmlStreamReader& operator>>(QXmlStreamReader& stream, Subproject& subproject)
{
    subproject.deserialize(stream);
    return stream;
}
