#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <QWidget>

#include "iview.h"

QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(QTabWidget)

namespace KCL
{
struct Model;
}

namespace Backend::Core
{
struct Selection;
class Subproject;
struct Geometry;
struct ModalSolution;
}

namespace Frontend
{

class HierarchyItem;
class ElementHierarchyItem;
class ModelHierarchyItem;
struct VertexField;

class ViewManager : public QWidget
{
    Q_OBJECT

public:
    ViewManager(QSettings& settings, QWidget* pParent = nullptr);
    virtual ~ViewManager();

    QSize sizeHint() const override;

    IView* view(int iView);
    QList<IView*> views();
    IView* currentView();
    int numViews() const;
    int numViews(IView::Type type);
    bool isEmpty() const;
    IView* findView(KCL::Model const& model);
    IView* findView(Backend::Core::Geometry const& geometry);

    IView* createView(KCL::Model const& model, QString const& name = QString());
    IView* createView(Backend::Core::Geometry const& geometry, VertexField const& field, QString const& name = QString());
    void processItems(QList<HierarchyItem*> const& items);
    void refresh();
    void plot();
    void replot(KCL::Model const& model);
    void clear();

signals:
    void selectItemsRequested(KCL::Model const& model, QList<Backend::Core::Selection> selections);
    void editItemsRequested(KCL::Model const& model, QList<Backend::Core::Selection> selections);

private:
    void createContent();
    void initialize();
    void processModelItems(QList<HierarchyItem*> const& items, QSet<IView*>& modifiedViews);
    void processGeometryItems(QList<HierarchyItem*> const& items, QSet<IView*>& modifiedViews);
    IView* createView(ModelHierarchyItem* pItem);
    QString getDefaultViewName(IView::Type type);
    QString getViewName(Backend::Core::Subproject* pSubproject, IView::Type type);

private:
    QSettings& mSettings;
    QTabWidget* mpTabWidget;
};

}

#endif // VIEWMANAGER_H
