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

    IView* currentView();
    IView* view(int iView);
    QList<IView*> views();
    int numViews() const;
    bool isEmpty() const;

    void processItems(QList<HierarchyItem*> const& items);
    IView* createView(KCL::Model const& model);
    void refresh();
    void clear();

private:
    void createContent();
    void setVTKFormat();
    QString newViewName();

private:
    QSettings& mSettings;
    QTabWidget* mpTabWidget;
};

}

#endif // VIEWMANAGER_H
