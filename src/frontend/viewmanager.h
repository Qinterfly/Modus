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

namespace Frontend
{

class HierarchyItem;

class ViewManager : public QWidget
{
    Q_OBJECT

public:
    ViewManager(QSettings& settings, QWidget* pParent = nullptr);
    ~ViewManager();

    QSize sizeHint() const override;

    IView* view(int iView);
    QList<IView*> views();
    IView* currentView();
    int numViews() const;
    bool isEmpty() const;
    IView* findView(KCL::Model const& model);

    IView* createView(KCL::Model const& model);
    void processItems(QList<HierarchyItem*> const& items);
    void refresh();
    void clear();

private:
    void createContent();
    void initialize();
    QString newViewName();

private:
    QSettings& mSettings;
    QTabWidget* mpTabWidget;
};

}

#endif // VIEWMANAGER_H
