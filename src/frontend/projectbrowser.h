#ifndef PROJECTBROWSER_H
#define PROJECTBROWSER_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QTreeView);
QT_FORWARD_DECLARE_CLASS(QSettings);

namespace Backend::Core
{
class Project;
}

namespace Frontend
{

class ProjectHierarchyModel;

class ProjectBrowser : public QWidget
{
public:
    ProjectBrowser(Backend::Core::Project& project, QSettings& settings, QWidget* pParent = nullptr);
    virtual ~ProjectBrowser();

    QSize sizeHint() const override;

    void update();

private:
    void createContent();

private:
    Backend::Core::Project& mProject;
    QSettings& mSettings;
    QTreeView* mpView;
    ProjectHierarchyModel* mpModel;
};

}

#endif // PROJECTBROWSER_H
