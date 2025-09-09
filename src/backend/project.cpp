#include <QXmlStreamWriter>

#include "fileutility.h"
#include "mathutility.h"
#include "project.h"

using namespace Backend;
using namespace Backend::Core;

static const QString skProjectIOVersion = "1.0";

Project::Project()
{
}

bool Project::operator==(Project const& another) const
{
    return Utility::areEqual(*this, another);
}

bool Project::operator!=(Project const& another) const
{
    return !(*this == another);
}

QString const& Project::pathFile() const
{
    return mPathFile;
}

QList<Subproject>& Project::subprojects()
{
    return mSubprojects;
}

void Project::setPathFile(QString const& pathFile)
{
    mPathFile = pathFile;
}

void Project::addSubproject(Subproject const& subproject)
{
    mSubprojects.push_back(subproject);
}

void Project::removeSubproject(QUuid const& id)
{
    int iRemove = Utility::getIndexByID(mSubprojects, id);
    if (iRemove >= 0)
        mSubprojects.remove(iRemove);
}

void Project::setSubprojects(QList<Subproject> const& subprojects)
{
    mSubprojects = subprojects;
}

void Project::clear()
{
    mPathFile = QString();
    mSubprojects.clear();
}

int Project::numSubprojects() const
{
    return mSubprojects.size();
}

bool Project::isEmpty() const
{
    return mSubprojects.empty();
}

QString Project::fileSuffix()
{
    return "xmod";
}

//! Read a project from a XML-formatted file
bool Project::read(const QString& pathFile)
{
    // Open the file for reading
    auto pFile = Utility::openFile(pathFile, Project::fileSuffix(), QIODevice::ReadOnly);
    if (!pFile)
        return false;
    QXmlStreamReader stream(pFile.data());

    // Check the document version
    if (stream.readNext())
    {
        if (stream.documentVersion() != skProjectIOVersion)
        {
            qWarning() << QObject::tr("The unsupported document version detected: %1").arg(stream.documentVersion());
            return false;
        }
    }

    // Check the root item
    stream.readNextStartElement();
    if (stream.name() != "project")
    {
        qWarning() << QObject::tr("The unsupported project detected: %1").arg(stream.name());
        return false;
    }

    // Clean up the project data
    clear();

    // Retrieve the project data
    deserialize(stream);

    // Remember the filepath
    mPathFile = pathFile;

    return true;
}

//! Write a project to a XML-formatted file
bool Project::write(const QString& pathFile)
{
    auto pFile = Utility::openFile(pathFile, Project::fileSuffix(), QIODevice::WriteOnly);
    if (!pFile)
        return false;
    QXmlStreamWriter stream(pFile.data());

    // Write the header
    stream.setAutoFormatting(true);
    stream.writeStartDocument(skProjectIOVersion);

    // Write the data
    serialize(stream, "project");

    // Close the file
    stream.writeEndDocument();
    pFile->close();

    // Remember the filepath
    mPathFile = pathFile;

    return true;
}

//! Output project to a XML stream
void Project::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    stream.writeTextElement("id", mID.toString());
    stream.writeTextElement("pathFile", mPathFile);
    Utility::serialize(stream, "subprojects", "subproject", mSubprojects);
    stream.writeEndElement();
}

//! Output project to a XML stream
void Project::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "id")
        {
            mID = QUuid::fromString(stream.readElementText());
        }
        else if (stream.name() == "pathFile")
        {
            mPathFile = stream.readElementText();
        }
        else if (stream.name() == "subprojects")
        {
            Utility::deserialize(stream, "subproject", mSubprojects);
        }
        else
        {
            stream.skipCurrentElement();
        }
    }
}
