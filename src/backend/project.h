#ifndef PROJECT_H
#define PROJECT_H

#include "kclsubproject.h"

#include <QList>
#include <QString>

namespace Backend::Core
{

class Project : public Identifier
{
public:
    Project();
    ~Project();

    QString const& name() const;
    QString const& pathFile() const;
    QList<AbstractSubproject*>& subprojects();

    void setName(QString const& name);
    void setPathFile(QString const& pathFile);
    void addSubproject(AbstractSubproject* pSubproject);
    void removeSubproject(QUuid const& id);
    void setSubprojects(QList<AbstractSubproject*>&& subprojects);
    void clear();

    int numSubprojects() const;
    bool isEmpty() const;
    static QString fileSuffix();

private:
    QString mName;
    QString mPathFile;
    QList<AbstractSubproject*> mSubprojects;
};

}

#endif // PROJECT_H
