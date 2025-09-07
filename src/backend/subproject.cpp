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

void Configuration::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    stream.writeTextElement("name", name);
    problem.serialize(stream, "problem");
    options.serialize(stream, "options");
    stream.writeEndElement();
}

void Configuration::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "name")
            name = stream.readElementText();
        else if (stream.name() == "problem")
            problem.deserialize(stream);
        else if (stream.name() == "options")
            options.deserialize(stream);
        else
            stream.skipCurrentElement();
    }
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

QList<OptimSolution> const& Subproject::optimSolutions() const
{
    return mOptimSolutions;
}

Configuration& Subproject::configuration()
{
    return mConfiguration;
}

KCL::Model& Subproject::model()
{
    return mModel;
}

QList<OptimSolution>& Subproject::optimSolutions()
{
    return mOptimSolutions;
}

void Subproject::clear()
{
    mConfiguration = Configuration();
    mModel = KCL::Model();
    mOptimSolutions.clear();
}

bool Subproject::operator==(Subproject const& another) const
{
    return Utility::areEqual(*this, another);
}

bool Subproject::operator!=(Subproject const& another) const
{
    return !(*this == another);
}

void Subproject::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    mConfiguration.serialize(stream, "configuration");
    Utility::serialize(stream, "model", mModel);
    Utility::serialize(stream, "optimSolutions", "optimSolution", mOptimSolutions);
    stream.writeEndElement();
}

void Subproject::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "configuration")
            mConfiguration.deserialize(stream);
        else if (stream.name() == "model")
            Utility::deserialize(stream, mModel);
        else if (stream.name() == "optimSolutions")
            Utility::deserialize(stream, "optimSolution", mOptimSolutions);
        else
            stream.skipCurrentElement();
    }
}
