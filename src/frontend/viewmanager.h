#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <QWidget>

#include <Eigen/Core>

#include "iview.h"

QT_FORWARD_DECLARE_CLASS(QSettings)

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
struct FlutterSolution;
class SelectionSet;
}

namespace Frontend
{

class CustomTabWidget;
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

    IView* findModelView(KCL::Model const& model);
    IView* findGeometryView(Backend::Core::Geometry const& geometry);
    IView* findLogView(QString const& log);
    IView* findFlutterView(Backend::Core::FlutterSolution const& solution);

    IView* createModelView(KCL::Model const& model, QString const& name = QString());
    IView* createGeometryView(Backend::Core::Geometry const& geometry, VertexField const& field, QString const& name = QString());
    IView* createLogView(QString const& log, QString const& name = QString());
    IView* createFlutterView(Backend::Core::FlutterSolution const& solution, QString const& name = QString());
    IView* createTableView(Backend::Core::FlutterSolution const& solution, QString const& name = QString());
    IView* createTableView(Backend::Core::ModalSolution const& solution, QString const& name = QString());

    void removeView(IView* pView);

    void processItems(QList<HierarchyItem*> const& items);
    void setSelectionByView(KCL::Model& model, Backend::Core::SelectionSet& selectionSet);
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
    void processLogItems(QList<HierarchyItem*> const& items, QSet<IView*>& modifiedViews);
    void processFlutterItems(QList<HierarchyItem*> const& items, QSet<IView*>& modifiedViews);
    IView* createView(ModelHierarchyItem* pItem);
    QString getDefaultViewName(IView::Type type);
    QString getViewName(HierarchyItem* pItem);

private:
    QSettings& mSettings;
    CustomTabWidget* mpTabWidget;
};

}

#endif // VIEWMANAGER_H
