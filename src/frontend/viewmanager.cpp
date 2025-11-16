#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

#include <kcl/model.h>

#include "customtabwidget.h"
#include "flutterview.h"
#include "geometry.h"
#include "geometryview.h"
#include "hierarchyitem.h"
#include "logview.h"
#include "modelview.h"
#include "selectionset.h"
#include "subproject.h"
#include "tableview.h"
#include "uiutility.h"
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

//! Get number of views of the specified type
int ViewManager::numViews(IView::Type type)
{
    int result = 0;
    int count = numViews();
    for (int i = 0; i != count; ++i)
    {
        if (view(i)->type() == type)
            ++result;
    }
    return result;
}

//! Check if there are any views
bool ViewManager::isEmpty() const
{
    return numViews() == 0;
}

//! Find the view associated with the current model
IView* ViewManager::findModelView(KCL::Model const& model)
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

//! Find the view associated with the current geometry
IView* ViewManager::findGeometryView(Backend::Core::Geometry const& geometry)
{
    int count = numViews();
    for (int i = 0; i != count; ++i)
    {
        IView* pView = view(i);
        if (pView->type() == IView::kGeometry && &static_cast<GeometryView*>(pView)->getGeometry() == &geometry)
            return pView;
    }
    return nullptr;
}

//! Find the view associated with the current log
IView* ViewManager::findLogView(QString const& log)
{
    int count = numViews();
    for (int i = 0; i != count; ++i)
    {
        IView* pView = view(i);
        if (pView->type() == IView::kLog && &static_cast<LogView*>(pView)->log() == &log)
            return pView;
    }
    return nullptr;
}

//! Find the view associated with the current flutter solution
IView* ViewManager::findFlutterView(Backend::Core::FlutterSolution const& solution)
{
    int count = numViews();
    for (int i = 0; i != count; ++i)
    {
        IView* pView = view(i);
        if (pView->type() == IView::kFlutter && &static_cast<FlutterView*>(pView)->solution() == &solution)
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
IView* ViewManager::createModelView(KCL::Model const& model, QString const& name)
{
    // Set the view as the current one if it has been already created
    IView* pBaseView = findModelView(model);
    if (pBaseView)
    {
        mpTabWidget->setCurrentWidget(pBaseView);
        mpTabWidget->setTabText(mpTabWidget->currentIndex(), name);
        return pBaseView;
    }

    // Create the model view otherwise
    ModelView* pView = new ModelView(model);
    pView->plot();
    pView->setIsometricView();

    // Set the connections
    connect(pView, &ModelView::selectItemsRequested, this,
            [pView, this](QList<Core::Selection> selections) { emit selectItemsRequested(pView->model(), selections); });
    connect(pView, &ModelView::editItemsRequested, this,
            [pView, this](QList<Core::Selection> selections) { emit editItemsRequested(pView->model(), selections); });

    // Add it to the tab
    QString label = name.isEmpty() ? getDefaultViewName(IView::kModel) : name;
    mpTabWidget->addTab(pView, QIcon(":/icons/model.svg"), label);
    mpTabWidget->setCurrentWidget(pView);

    return pView;
}

//! Create the view associated with a geometry
IView* ViewManager::createGeometryView(Backend::Core::Geometry const& geometry, VertexField const& field, QString const& name)
{
    // Set the view as the current one if it has been already created
    IView* pBaseView = findGeometryView(geometry);
    if (pBaseView)
    {
        GeometryView* pView = (GeometryView*) pBaseView;
        pView->insertField(field);
        pView->plot();
        mpTabWidget->setCurrentWidget(pView);
        mpTabWidget->setTabText(mpTabWidget->currentIndex(), name);
        return pView;
    }

    // Create the geometry view otherwise
    GeometryView* pView = new GeometryView(geometry, field);
    pView->plot();
    pView->setIsometricView();

    // Add it to the tab
    QString label = name.isEmpty() ? getDefaultViewName(IView::kGeometry) : name;
    mpTabWidget->addTab(pView, QIcon(":/icons/mode.png"), label);
    mpTabWidget->setCurrentWidget(pView);

    return pView;
}

//! Create the view associated with a log
IView* ViewManager::createLogView(QString const& log, QString const& name)
{
    // Set the view as the current one if it has been already created
    IView* pBaseView = findLogView(log);
    if (pBaseView)
    {
        LogView* pView = (LogView*) pBaseView;
        pView->plot();
        mpTabWidget->setCurrentWidget(pView);
        mpTabWidget->setTabText(mpTabWidget->currentIndex(), name);
        return pView;
    }

    // Create the log view otherwise
    LogView* pView = new LogView(log);
    pView->plot();

    // Add it to the tab
    QString label = name.isEmpty() ? getDefaultViewName(IView::kLog) : name;
    mpTabWidget->addTab(pView, QIcon(":/icons/log.png"), label);
    mpTabWidget->setCurrentWidget(pView);

    return pView;
}

//! Create the view associated with a flutter solution
IView* ViewManager::createFlutterView(Backend::Core::FlutterSolution const& solution, QString const& name)
{
    // Set the view as the current one if it has been already created
    IView* pBaseView = findFlutterView(solution);
    if (pBaseView)
    {
        FlutterView* pView = (FlutterView*) pBaseView;
        pView->plot();
        mpTabWidget->setCurrentWidget(pView);
        mpTabWidget->setTabText(mpTabWidget->currentIndex(), name);
        return pView;
    }

    // Create the flutter view otherwise
    FlutterView* pView = new FlutterView(solution);
    pView->plot();

    // Add it to the tab
    QString label = name.isEmpty() ? getDefaultViewName(IView::kFlutter) : name;
    mpTabWidget->addTab(pView, QIcon(":/icons/roots.svg"), label);
    mpTabWidget->setCurrentWidget(pView);

    return pView;
}

//! Create the table view associated with a flutter solution
IView* ViewManager::createTableView(Core::FlutterSolution const& solution, QString const& name)
{
    // Create the flutter view otherwise
    TableView* pView = new TableView(solution);
    pView->plot();

    // Add it to the tab
    QString label = name.isEmpty() ? getDefaultViewName(IView::kTable) : name;
    mpTabWidget->addTab(pView, QIcon(":/icons/table.png"), label);
    mpTabWidget->setCurrentWidget(pView);

    return pView;
}

//! Create the table view associated with a vector
IView* ViewManager::createTableView(Core::ModalSolution const& solution, QString const& name)
{
    // Create the flutter view otherwise
    TableView* pView = new TableView(solution);
    pView->plot();

    // Add it to the tab
    QString label = name.isEmpty() ? getDefaultViewName(IView::kTable) : name;
    mpTabWidget->addTab(pView, QIcon(":/icons/table.png"), label);
    mpTabWidget->setCurrentWidget(pView);

    return pView;
}

//! Remove view which is used as one of the tabs
void ViewManager::removeView(IView* pView)
{
    if (!pView)
        return;
    int count = numViews();
    int iFound = -1;
    for (int i = 0; i != count; ++i)
    {
        if (mpTabWidget->widget(i) == pView)
        {
            iFound = i;
            break;
        }
    }
    if (iFound >= 0)
        mpTabWidget->removePage(iFound);
}

//! Create views associated with project hierarchy items
void ViewManager::processItems(QList<HierarchyItem*> const& items)
{
    // Constants
    QSet<HierarchyItem::Type> kModelTypes = {HierarchyItem::kSubproject,    HierarchyItem::kModel,   HierarchyItem::kSurface,
                                             HierarchyItem::kGroupElements, HierarchyItem::kElement, HierarchyItem::kOptimSelectionSet};
    QSet<HierarchyItem::Type> kGeometryTypes = {HierarchyItem::kModalSolution, HierarchyItem::kModalFrequencies, HierarchyItem::kModalPole};
    QSet<HierarchyItem::Type> kFlutterTypes = {HierarchyItem::kFlutterSolution, HierarchyItem::kFlutterRoots, HierarchyItem::kFlutterCritData};

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
    QSet<IView*> modifiedViews;
    for (HierarchyItem::Type type : types)
    {
        QList<HierarchyItem*> typeItems = mapItems[type];
        if (kModelTypes.contains(type))
            processModelItems(typeItems, modifiedViews);
        else if (kGeometryTypes.contains(type))
            processGeometryItems(typeItems, modifiedViews);
        else if (kFlutterTypes.contains(type))
            processFlutterItems(typeItems, modifiedViews);
        else if (type == HierarchyItem::kLog)
            processLogItems(typeItems, modifiedViews);
    }

    // Refresh the modified views
    for (auto iter = modifiedViews.begin(); iter != modifiedViews.end(); ++iter)
        (*iter)->refresh();
}

//! Set the selection set by the model view
void ViewManager::setSelectionByView(KCL::Model& model, Core::SelectionSet& selectionSet)
{
    // Find the view
    IView* pBaseView = findModelView(model);
    if (!pBaseView)
        return;
    ModelView* pView = (ModelView*) pBaseView;

    // Copy the selection
    QList<Core::Selection> selections = pView->selector().selected();
    selectionSet.selectNone();
    selectionSet.setSelected(selections, true);
}

//! Process hierarchy items associated with the ModelView
void ViewManager::processModelItems(QList<HierarchyItem*> const& items, QSet<IView*>& modifiedViews)
{
    for (HierarchyItem* pBaseItem : items)
    {
        auto type = pBaseItem->type();
        ModelView* pView = nullptr;
        switch (type)
        {
        case HierarchyItem::kSubproject:
        {
            SubprojectHierarchyItem* pItem = (SubprojectHierarchyItem*) pBaseItem;
            Core::Subproject& subproject = pItem->subproject();
            QString label = getViewName(pItem);
            pView = (ModelView*) createModelView(subproject.model(), label);
            pView->selector().deselectAll();
            modifiedViews.insert(pView);
            break;
        }
        case HierarchyItem::kModel:
        {
            ModelHierarchyItem* pItem = (ModelHierarchyItem*) pBaseItem;
            QString label = getViewName(pItem);
            pView = (ModelView*) createModelView(pItem->kclModel(), label);
            pView->selector().deselectAll();
            modifiedViews.insert(pView);
            break;
        }
        case HierarchyItem::kSurface:
            processModelItems(Utility::childItems(pBaseItem), modifiedViews);
            break;
        case HierarchyItem::kGroupElements:
            processModelItems(Utility::childItems(pBaseItem), modifiedViews);
            break;
        case HierarchyItem::kElement:
        {
            ElementHierarchyItem* pItem = (ElementHierarchyItem*) pBaseItem;

            // Set the selection data
            int iSurface = pItem->iSurface();
            if (iSurface < -1)
                continue;
            KCL::ElementType type = pItem->element()->type();
            int iElement = pItem->iElement();
            Core::Selection selection(iSurface, type, iElement);

            // Slice the model
            KCL::Model* pModel = pItem->kclModel();
            if (!pModel)
                continue;

            // Create the view, if necessary
            QString label = getViewName(pItem);
            pView = (ModelView*) createModelView(*pModel, label);
            if (!modifiedViews.contains(pView))
                pView->selector().deselectAll();

            // Add the selection set to the view
            pView->selector().select(selection, ModelViewSelector::kMultipleSelection);
            break;
        }
        case HierarchyItem::kOptimSelectionSet:
        {
            OptimSelectionSetHierarchyItem* pItem = (OptimSelectionSetHierarchyItem*) pBaseItem;

            // Slice the model
            KCL::Model* pModel = pItem->kclModel();
            if (!pModel)
                continue;

            // Create the view, if necessary
            QString label = getViewName(pItem);
            pView = (ModelView*) createModelView(*pModel, label);
            if (!modifiedViews.contains(pView))
                pView->selector().deselectAll();

            // Add the selection set to the view
            pView->selector().select(pItem->selectionSet().selected());
            break;
        }
        default:
            break;
        }

        // Add the view to the modified list
        if (pView)
            modifiedViews.insert(pView);
    }
}

//! Process hierarchy items associated with the GeometryView
void ViewManager::processGeometryItems(QList<HierarchyItem*> const& items, QSet<IView*>& modifiedViews)
{
    for (HierarchyItem* pBaseItem : items)
    {
        auto type = pBaseItem->type();
        IView* pView = nullptr;
        switch (type)
        {
        case HierarchyItem::kModalSolution:
        {
            processGeometryItems(Utility::childItems(pBaseItem), modifiedViews);
            break;
        }
        case HierarchyItem::kModalFrequencies:
        {
            ModalFrequenciesHierarchyItem* pItem = (ModalFrequenciesHierarchyItem*) pBaseItem;
            QString label = getViewName(pItem);
            pView = createTableView(pItem->solution(), label);
            break;
        }
        case HierarchyItem::kModalPole:
        {
            ModalPoleHierarchyItem* pItem = (ModalPoleHierarchyItem*) pBaseItem;
            QString label = getViewName(pItem);
            VertexField field(pItem->iMode(), pItem->frequency(), pItem->modeShape());
            IView* pBaseView = findGeometryView(pItem->geometry());
            if (pBaseView)
            {
                if (!modifiedViews.contains(pBaseView))
                    static_cast<GeometryView*>(pBaseView)->clearFields();
            }
            pView = (GeometryView*) createGeometryView(pItem->geometry(), field, label);
            modifiedViews.insert(pView);
            break;
        }
        default:
            break;
        }

        // Add the view to the modified list
        if (pView)
            modifiedViews.insert(pView);
    }
}

//! Process hierarchy items associated with the LogView
void ViewManager::processLogItems(QList<HierarchyItem*> const& items, QSet<IView*>& modifiedViews)
{
    for (HierarchyItem* pBaseItem : items)
    {
        LogHierarchyItem* pItem = (LogHierarchyItem*) pBaseItem;
        QString label = getViewName(pItem);
        LogView* pView = (LogView*) createLogView(pItem->log(), label);
        modifiedViews.insert(pView);
    }
}

//! Process hierarchy items associated with the FlutterView
void ViewManager::processFlutterItems(QList<HierarchyItem*> const& items, QSet<IView*>& modifiedViews)
{
    for (HierarchyItem* pBaseItem : items)
    {
        auto type = pBaseItem->type();
        IView* pView = nullptr;
        switch (type)
        {
        case HierarchyItem::kFlutterSolution:
        {
            FlutterSolutionHierarchyItem* pItem = (FlutterSolutionHierarchyItem*) pBaseItem;
            QString label = getViewName(pItem);
            pView = createFlutterView(pItem->solution(), label);
            break;
        }
        case HierarchyItem::kFlutterRoots:
        {
            FlutterRootsHierarchyItem* pItem = (FlutterRootsHierarchyItem*) pBaseItem;
            QString label = getViewName(pItem);
            pView = createFlutterView(pItem->solution(), label);
            break;
        }
        case HierarchyItem::kFlutterCritData:
        {
            FlutterCritDataHierarchyItem* pItem = (FlutterCritDataHierarchyItem*) pBaseItem;
            QString label = getViewName(pItem);
            pView = createTableView(pItem->solution(), label);
            break;
        }
        default:
            break;
        }

        // Add the view to the modified list
        if (pView)
            modifiedViews.insert(pView);
    }
}

//! Render all the views
void ViewManager::refresh()
{
    int count = numViews();
    for (int i = 0; i != count; ++i)
        view(i)->refresh();
}

//! Replot all the views
void ViewManager::plot()
{
    int count = numViews();
    for (int i = 0; i != count; ++i)
        view(i)->plot();
}

//! Replot the model associated view
void ViewManager::replot(KCL::Model const& model)
{
    IView* pBaseView = findModelView(model);
    if (!pBaseView)
        return;
    auto pView = (ModelView*) pBaseView;
    pView->clear();
    pView->plot();
    pView->setIsometricView();
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
    mpTabWidget = new CustomTabWidget;

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
    createModelView(KCL::Model());
    mpTabWidget->removeTab(0);
}

//! Construct a default view name
QString ViewManager::getDefaultViewName(IView::Type type)
{
    QString prefix = tr("View");
    switch (type)
    {
    case IView::kModel:
        prefix = tr("Model");
        break;
    case IView::kGeometry:
        prefix = tr("Geometry");
        break;
    case IView::kLog:
        prefix = tr("Log");
        break;
    case IView::kFlutter:
        prefix = tr("Flutter");
        break;
    case IView::kTable:
        prefix = tr("Table");
        break;
    default:
        break;
    }
    return QString("%1 %2").arg(prefix).arg(numViews(type) + 1);
}

//! Consturt a name for a new view
QString ViewManager::getViewName(HierarchyItem* pItem)
{
    // Constants
    int const kMaxNumTokens = 1;

    // Get the full path
    QString path = pItem->path();
    int numPath = path.size();

    // Slice the N parts of the name
    int numTokens = 0;
    int iLast = 0;
    for (iLast = 0; iLast != numPath; ++iLast)
    {
        if (path.at(iLast) == HierarchyItem::separator())
        {
            ++numTokens;
            if (numTokens > kMaxNumTokens)
                break;
        }
    }
    return path.first(iLast);
}
