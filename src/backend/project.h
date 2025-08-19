#ifndef PROJECT_H
#define PROJECT_H

#include "subproject.h"

#include <QList>
#include <QString>

namespace Backend::Core
{

class Project : public Identifier
{
public:
    Project();
    ~Project() = default;

    QString const& name() const;
    QString const& pathFile() const;
    QList<Subproject>& subprojects();

    void setName(QString const& name);
    void setPathFile(QString const& pathFile);
    void addSubproject(Subproject const& subproject);
    void removeSubproject(QUuid const& id);
    void setSubprojects(QList<Subproject> const& subprojects);

    int numSubprojects() const;
    bool isEmpty() const;
    static QString fileSuffix();

private:
    QString mName;
    QString mPathFile;
    QList<Subproject> mSubprojects;
};

}

#endif // PROJECT_H
