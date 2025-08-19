#include "project.h"
#include "mathutility.h"

using namespace Backend::Core;

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

void Project::setPathFile(QString const& pathFile)
{
    mPathFile = pathFile;
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
