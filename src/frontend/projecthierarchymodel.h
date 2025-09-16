#ifndef PROJECTHIERARCHYMODEL_H
#define PROJECTHIERARCHYMODEL_H

#include <QStandardItemModel>

namespace Backend::Core
{
class Project;
}

namespace Frontend
{

class ProjectHierarchyModel : public QStandardItemModel
{
    Q_OBJECT

public:
    ProjectHierarchyModel(Backend::Core::Project& project, QObject* pParent = nullptr);
    ~ProjectHierarchyModel();

private:
    Backend::Core::Project& mProject;
};

}

#endif // PROJECTHIERARCHYMODEL_H
