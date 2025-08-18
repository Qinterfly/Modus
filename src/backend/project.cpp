#include "project.h"
#include "mathutility.h"

using namespace Backend::Core;

Project::Project()
{
}

Project::~Project()
{
    clear();
}

QString const& Project::name() const
{
    return mName;
}

QString const& Project::pathFile() const
{
    return mPathFile;
}

QList<AbstractSubproject*>& Project::subprojects()
{
    return mSubprojects;
}

void Project::setName(QString const& name)
{
    mName = name;
}

void Project::addSubproject(AbstractSubproject* pSubproject)
{
    mSubprojects.push_back(pSubproject);
}

void Project::removeSubproject(QUuid const& id)
{
    int iRemove = Utility::getIndexByID(mSubprojects, id);
    if (iRemove >= 0)
    {
        AbstractSubproject* pSubproject = mSubprojects[iRemove];
        delete pSubproject;
        mSubprojects.remove(iRemove);
    }
}

void Project::setSubprojects(QList<AbstractSubproject*>&& subprojects)
{
    clear();
    mSubprojects = std::move(subprojects);
}

void Project::setPathFile(QString const& pathFile)
{
    mPathFile = pathFile;
}

void Project::clear()
{
    mName = QString();
    mPathFile = QString();
    qDeleteAll(mSubprojects.begin(), mSubprojects.end());
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
