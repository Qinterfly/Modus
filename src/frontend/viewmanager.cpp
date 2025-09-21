#include <QTabWidget>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

#include <kcl/model.h>

#include "hierarchyitem.h"
#include "modelview.h"
#include "viewmanager.h"

using namespace Frontend;

ViewManager::ViewManager(QSettings& settings, QWidget* pParent)
    : QWidget(pParent)
    , mSettings(settings)
{
    createContent();
    setVTKFormat();
    createView(KCL::Model());
}

ViewManager::~ViewManager()
{
    clear();
}

QSize ViewManager::sizeHint() const
{
    return QSize(800, 600);
}

//! Retrieve the current view
IView const* ViewManager::currentView() const
{
    if (numViews() > 0)
        return (IView*) mpTabWidget->currentWidget();
    return nullptr;
}

//! Get the view located at the specified position
IView const* ViewManager::view(int iView) const
{
    if (iView >= 0 && iView < numViews())
        return (IView*) mpTabWidget->widget(iView);
    return nullptr;
}

//! Retrieve all the views
QList<IView const*> ViewManager::views() const
{
    int numItems = numViews();
    QList<IView const*> result(numItems);
    for (int i = 0; i != numItems; ++i)
        result[i] = view(i);
    return result;
}

//! Get total number of views
int ViewManager::numViews() const
{
    return mpTabWidget->count();
}

//! Check if there are any views
bool ViewManager::isEmpty() const
{
    return numViews() == 0;
}

//! Create views associated with project hierarchy items
void ViewManager::processItems(QList<HierarchyItem*> const& items)
{
    // Check if there are any items to view
    if (items.isEmpty())
        return;

    // Destroy previously created views
    clear();

    // Map the items
    QMap<HierarchyItem::Type, QList<HierarchyItem*>> mapItems;
    for (HierarchyItem* pItem : items)
    {
        auto type = (HierarchyItem::Type) pItem->type();
        mapItems[type].push_back(pItem);
    }

    // Loop through all the types
    QList<HierarchyItem::Type> const types = mapItems.keys();
    for (HierarchyItem::Type type : types)
    {
        QList<HierarchyItem*> typeItems = mapItems[type];
        for (HierarchyItem* pItem : typeItems)
        {
            switch (type)
            {
            case HierarchyItem::kModel:
                createView(static_cast<ModelHierarchyItem*>(pItem)->model());
                break;
            default:
                break;
            }
        }
    }
}

//! Create the view associated with a KCL model
IView* ViewManager::createView(KCL::Model const& model)
{
    ModelView* pView = new ModelView(model);
    mpTabWidget->addTab(pView, newViewName());
    return pView;
}

//! Destroy all views
void ViewManager::clear()
{
    mpTabWidget->clear();
}

//! Create all the widgets and corresponding actions
void ViewManager::createContent()
{
    // Create the central widget
    mpTabWidget = new QTabWidget;

    // Insert the widgets into the main layout
    QHBoxLayout* pLayout = new QHBoxLayout();
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->addWidget(mpTabWidget);
    setLayout(pLayout);
}

//! Specify the format for the VTK library
void ViewManager::setVTKFormat()
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
    vtkObject::GlobalWarningDisplayOff();
}

//! Generate a name for a new view
QString ViewManager::newViewName()
{
    return QString("View %1").arg(numViews() + 1);
}
