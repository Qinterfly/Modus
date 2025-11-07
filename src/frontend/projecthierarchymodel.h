#ifndef PROJECTHIERARCHYMODEL_H
#define PROJECTHIERARCHYMODEL_H

#include <QStandardItemModel>

namespace KCL
{
struct Model;
}

namespace Backend::Core
{
class Project;
struct Selection;
}

namespace Frontend
{

class ProjectHierarchyModel : public QStandardItemModel
{
    Q_OBJECT

public:
    ProjectHierarchyModel(Backend::Core::Project& project, QObject* pParent = nullptr);
    virtual ~ProjectHierarchyModel() = default;

    void selectItems(KCL::Model const& model, QList<Backend::Core::Selection> const& selections);

private:
    void appendChildren();
    void processItemChange(QStandardItem* pItem);

private:
    Backend::Core::Project& mProject;
};

}

#endif // PROJECTHIERARCHYMODEL_H
