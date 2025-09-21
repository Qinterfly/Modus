#ifndef PROJECTBROWSER_H
#define PROJECTBROWSER_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QTreeView);
QT_FORWARD_DECLARE_CLASS(QSettings);
QT_FORWARD_DECLARE_CLASS(QStandardItem)
QT_FORWARD_DECLARE_CLASS(QSortFilterProxyModel)
QT_FORWARD_DECLARE_CLASS(QItemSelection)

namespace Backend::Core
{
class Project;
}

namespace Frontend
{

class ProjectHierarchyModel;
class HierarchyItem;

class ProjectBrowser : public QWidget
{
    Q_OBJECT

public:
    ProjectBrowser(Backend::Core::Project& project, QSettings& settings, QWidget* pParent = nullptr);
    virtual ~ProjectBrowser();

    QSize sizeHint() const override;

    void update();

signals:
    void selectionChanged(QList<HierarchyItem*>);

private:
    // Content
    void createContent();
    void filterContent(QString const& pattern);
    void processContextMenuRequest(QPoint const& point);
    void processSelection(QItemSelection const& selected, QItemSelection const& deselected);

    // Subproject management
    QList<HierarchyItem*> selectedItems();
    void setSelectedItemsExpandedState(bool flag);

private:
    Backend::Core::Project& mProject;
    QSettings& mSettings;
    QTreeView* mpView;
    QLineEdit* mpFilterLineEdit;
    ProjectHierarchyModel* mpSourceModel;
    QSortFilterProxyModel* mpFilterModel;
};

}

#endif // PROJECTBROWSER_H
