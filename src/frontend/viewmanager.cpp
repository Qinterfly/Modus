#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

#include <kcl/model.h>

#include "hierarchyitem.h"
#include "modelview.h"
#include "selectionset.h"
#include "viewmanager.h"

using namespace Backend;
using namespace Frontend;

ViewManager::ViewManager(QSettings& settings, QWidget* pParent)
    : QWidget(pParent)
    , mSettings(settings)
{
    createContent();
    initialize();
}

ViewManager::~ViewManager()
{
    clear();
}

QSize ViewManager::sizeHint() const
{
    return QSize(800, 600);
}

//! Get the view located at the specified position
IView* ViewManager::view(int iView)
{
    if (iView >= 0 && iView < numViews())
        return (IView*) mpTabWidget->widget(iView);
    return nullptr;
}

//! Retrieve all the views
QList<IView*> ViewManager::views()
{
    int count = numViews();
    QList<IView*> result(count);
    for (int i = 0; i != count; ++i)
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

//! Find the view associated with the current model
IView* ViewManager::findView(KCL::Model const& model)
{
    int count = numViews();
    for (int i = 0; i != count; ++i)
    {
        IView* pView = view(i);
        if (pView->type() == IView::kModel && &static_cast<ModelView*>(pView)->model() == &model)
            return pView;
    }
    return nullptr;
}

//! Retrieve the current view
IView* ViewManager::currentView()
{
    if (numViews() > 0)
        return (IView*) mpTabWidget->currentWidget();
    return nullptr;
}

//! Create the view associated with a KCL model
IView* ViewManager::createView(KCL::Model const& model)
{
    // Set the view as the current one if it has been already created
    IView* pView = findView(model);
    if (pView)
    {
        mpTabWidget->setCurrentWidget(pView);
        return pView;
    }

    // Create the model view otherwise
    ModelView* pModelView = new ModelView(model);
    pModelView->refresh();
    pModelView->setIsometricView();

    // Set the connections
    connect(pModelView, &ModelView::selectItemsRequested, this,
            [pModelView, this](QList<Core::Selection> selections) { emit selectItemsRequested(pModelView->model(), selections); });

    // Add it to the tab
    mpTabWidget->addTab(pModelView, newViewName());
    mpTabWidget->setCurrentWidget(pModelView);

    return pModelView;
}

//! Select entities on the view
void ViewManager::selectOnView(KCL::Model const& model, Backend::Core::Selection const& selection)
{
    // Slice the view widget
    ModelView* pView = (ModelView*) createView(model);

    // TODO
}

//! Create views associated with project hierarchy items
void ViewManager::processItems(QList<HierarchyItem*> const& items)
{
    // Check if there are any items to view
    if (items.isEmpty())
        return;

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
        for (HierarchyItem* pBaseItem : typeItems)
        {
            switch (type)
            {
            case HierarchyItem::kModel:
            {
                createView(static_cast<ModelHierarchyItem*>(pBaseItem)->kclModel());
                break;
            }
            case HierarchyItem::kElement:
            {
                ElementHierarchyItem* pItem = (ElementHierarchyItem*) pBaseItem;

                // Set the selection data
                int iSurface = pItem->iSurface();
                if (std::isnan(iSurface))
                    continue;
                KCL::ElementType type = pItem->element()->type();
                int iElement = pItem->iElement();
                Core::Selection selection(iSurface, type, iElement);

                // Slice the model
                KCL::Model* pModel = pItem->kclModel();
                if (!pModel)
                    continue;

                // Add to the selection set
                selectOnView(*pModel, selection);
                break;
            }
            default:
                break;
            }
        }
    }
}

//! Update all the views
void ViewManager::refresh()
{
    int count = numViews();
    for (int i = 0; i != count; ++i)
        view(i)->refresh();
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
    mpTabWidget->setContentsMargins(0, 0, 0, 0);
    mpTabWidget->setTabsClosable(true);
    connect(mpTabWidget->tabBar(), &QTabBar::tabCloseRequested, this, [this](int index) { mpTabWidget->removeTab(index); });

    // Insert the widgets into the main layout
    QHBoxLayout* pLayout = new QHBoxLayout();
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->addWidget(mpTabWidget);
    setLayout(pLayout);
}

//! Set the manager initial state
void ViewManager::initialize()
{
    // Specify the format for the VTK library
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
    vtkObject::GlobalWarningDisplayOff();

    // Heat up the renderer
    createView(KCL::Model());
    mpTabWidget->removeTab(0);
}

//! Generate a name for a new view
QString ViewManager::newViewName()
{
    return QString("View %1").arg(numViews() + 1);
}
