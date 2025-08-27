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

QString const& Project::name() const
{
    return mName;
}

QString const& Project::pathFile() const
{
    return mPathFile;
}

QList<Subproject>& Project::subprojects()
{
    return mSubprojects;
}

void Project::setName(QString const& name)
{
    mName = name;
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
    mName = QString();
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
    return "mxl";
}

//! Read a project from a text file
bool Project::read(const QString& pathFile)
{
    // TODO
    return true;
}

//! Write a project to a text file
bool Project::write(const QString& pathFile)
{
    auto pFile = Utility::openFile(pathFile, Project::fileSuffix(), QIODevice::WriteOnly);
    if (!pFile)
        return false;
    QXmlStreamWriter stream(pFile.data());

    // Write the header
    stream.setAutoFormatting(true);
    stream.writeStartDocument(skProjectIOVersion);
    stream.writeStartElement("project");

    // Write the data
    Utility::serialize(stream, *this);
    int numSubprojects = mSubprojects.size();
    for (int i = 0; i != numSubprojects; ++i)
        mSubprojects[i].serialize(stream);

    // Close the file
    stream.writeEndElement();
    stream.writeEndDocument();
    pFile->close();

    // Remember the filepath
    mPathFile = pathFile;

    return true;
}
