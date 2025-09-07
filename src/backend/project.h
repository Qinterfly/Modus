#ifndef PROJECT_H
#define PROJECT_H

#include "subproject.h"

#include <QList>
#include <QString>

namespace Backend::Core
{

class Project : public Identifier, public ISerializable
{
    Q_GADGET
    Q_PROPERTY(QUuid id MEMBER mID)
    Q_PROPERTY(QString name MEMBER mName)
    Q_PROPERTY(QString pathFile MEMBER mPathFile)
    Q_PROPERTY(QList<Subproject> subprojects MEMBER mSubprojects)

public:
    Project();
    ~Project() = default;

    bool operator==(Project const& another) const;
    bool operator!=(Project const& another) const;

    QString const& name() const;
    QString const& pathFile() const;
    QList<Subproject>& subprojects();

    void setName(QString const& name);
    void setPathFile(QString const& pathFile);
    void addSubproject(Subproject const& subproject);
    void removeSubproject(QUuid const& id);
    void setSubprojects(QList<Subproject> const& subprojects);
    void clear();

    int numSubprojects() const;
    bool isEmpty() const;
    static QString fileSuffix();

    bool read(QString const& pathFile);
    bool write(QString const& pathFile);

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

private:
    QString mName;
    QString mPathFile;
    QList<Subproject> mSubprojects;
};

}

#endif // PROJECT_H
