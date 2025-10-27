#ifndef PROJECTBROWSER_H
#define PROJECTBROWSER_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QTreeView);
QT_FORWARD_DECLARE_CLASS(QSettings);
QT_FORWARD_DECLARE_CLASS(QStandardItem)
QT_FORWARD_DECLARE_CLASS(QSortFilterProxyModel)
QT_FORWARD_DECLARE_CLASS(QItemSelection)

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

class ProjectHierarchyModel;
class HierarchyItem;
class ElementHierarchyItem;
class EditorManager;

class ProjectBrowser : public QWidget
{
    Q_OBJECT

public:
    ProjectBrowser(Backend::Core::Project& project, QSettings& settings, QWidget* pParent = nullptr);
    virtual ~ProjectBrowser();

    QSize sizeHint() const override;

    Backend::Core::Project& project();
    EditorManager* editorManager();

    void refresh();
    void selectItems(KCL::Model const& model, QList<Backend::Core::Selection> const& selections);
    void editItems(KCL::Model const& model, QList<Backend::Core::Selection> const& selections);

signals:
    void selectionChanged(QList<HierarchyItem*>);
    void editingFinished();

private:
    // Content
    void createContent();
    void filterContent(QString const& pattern);
    void processContextMenuRequest(QPoint const& point);
    void processSelection(QItemSelection const& selected, QItemSelection const& deselected);
    void processDoubleClick(QModelIndex const& index);
    void createElementEditor(HierarchyItem* pBaseItem);
    void createElementEditors(QList<HierarchyItem*>& items);

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
    EditorManager* mpEditorManager;
};

}

#endif // PROJECTBROWSER_H
