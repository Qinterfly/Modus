#include <QFileDialog>
#include <QLineEdit>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include <kcl/model.h>

#include "editormanager.h"
#include "hierarchyitem.h"
#include "project.h"
#include "projectbrowser.h"
#include "projecthierarchymodel.h"
#include "uiutility.h"

using namespace Frontend;
using namespace Backend;

ProjectBrowser::ProjectBrowser(Core::Project& project, QSettings& settings, QWidget* pParent)
    : QWidget(pParent)
    , mProject(project)
    , mSettings(settings)
{
    createContent();
    refresh();
}

ProjectBrowser::~ProjectBrowser()
{
}

QSize ProjectBrowser::sizeHint() const
{
    return QSize(150, 1000);
}

Backend::Core::Project& ProjectBrowser::project()
{
    return mProject;
}

EditorManager* ProjectBrowser::editorManager()
{
    return mpEditorManager;
}

//! Retrieve the currently selected items
QList<HierarchyItem*> ProjectBrowser::selectedItems()
{
    QModelIndexList indices = mpView->selectionModel()->selectedIndexes();
    QList<HierarchyItem*> result;
    uint numIndices = indices.size();
    result.reserve(numIndices);
    for (int i = 0; i != numIndices; ++i)
    {
        QModelIndex const& proxyIndex = indices[i];
        QModelIndex sourceIndex = mpFilterModel->mapToSource(proxyIndex);
        HierarchyItem* pItem = (HierarchyItem*) mpSourceModel->itemFromIndex(sourceIndex);
        result.push_back(pItem);
    }
    return result;
}

//! Update the viewer content
void ProjectBrowser::refresh()
{
    // Create the model to represent the project hierarchy
    QItemSelectionModel* pOldSelectionModel = mpView->selectionModel();
    mpView->setModel(nullptr);
    mpSourceModel = new ProjectHierarchyModel(mProject, mpView);
    mpFilterModel = new QSortFilterProxyModel(mpView);
    mpFilterModel->setRecursiveFilteringEnabled(true);
    mpFilterModel->setSourceModel(mpSourceModel);
    mpView->setModel(mpFilterModel);
    delete pOldSelectionModel;

    // Restore the model state
    setModelState();

    // Specify the connections between the model and view widget
    connect(mpView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ProjectBrowser::processSelection);
}

//! Select model items
void ProjectBrowser::selectItems(KCL::Model const& model, QList<Backend::Core::Selection> const& selections)
{
    if (mpSourceModel && !selections.isEmpty())
    {
        // Select the items according to the set
        mpView->collapseAll();
        {
            QItemSelectionModel* pSelectionModel = mpView->selectionModel();
            QSignalBlocker blocker(pSelectionModel);
            pSelectionModel->clearSelection();
            mpSourceModel->selectItems(model, selections);
        }

        // Process the selection
        QList<HierarchyItem*> items = selectedItems();
        int numItems = items.size();
        if (!items.isEmpty())
        {
            // Expand all the branches
            for (int i = 0; i != numItems; ++i)
            {
                QStandardItem* pItem = items[i]->parent();
                while (pItem)
                {
                    if (pItem == mpSourceModel->invisibleRootItem())
                        break;
                    mpView->expand(mpFilterModel->mapFromSource(pItem->index()));
                    pItem = pItem->parent();
                }
            }

            // Scroll to the last selected item
            QModelIndex itemIndex = mpFilterModel->mapFromSource(items.last()->index());
            mpView->scrollTo(itemIndex);
        }
    }
}

//! Select hierarchy items and edit them
void ProjectBrowser::editItems(KCL::Model const& model, QList<Backend::Core::Selection> const& selections)
{
    // Select hierarchy items
    selectItems(model, selections);

    // Retrieve the selected items
    QList<HierarchyItem*> items = selectedItems();
    if (items.isEmpty())
        return;

    // Create editors for all selected items
    mpEditorManager->clear();
    createItemEditors(items);

    // Show the editors
    if (!mpEditorManager->isEmpty())
        mpEditorManager->show();
}

//! Create all the widgets and corresponding actions
void ProjectBrowser::createContent()
{
    uint const kMargin = 2;

    // Create the editor manager
    mpEditorManager = new EditorManager(this);
    connect(mpEditorManager, &EditorManager::modelEdited, this, &ProjectBrowser::modelEdited);
    connect(mpEditorManager, &EditorManager::modalOptionsEdited, this, &ProjectBrowser::edited);
    connect(mpEditorManager, &EditorManager::flutterOptionsEdited, this, &ProjectBrowser::edited);
    connect(mpEditorManager, &EditorManager::optimOptionsEdited, this, &ProjectBrowser::edited);
    connect(mpEditorManager, &EditorManager::constraintsEdited, this, &ProjectBrowser::edited);
    connect(mpEditorManager, &EditorManager::optimTargetEdited, this, &ProjectBrowser::edited);

    // Create the view widget
    mpView = new QTreeView;
    mpView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mpView->setSelectionBehavior(QAbstractItemView::SelectItems);
    mpView->setHeaderHidden(true);
    mpView->setAcceptDrops(false);
    mpView->setDragEnabled(false);
    mpView->setSortingEnabled(false);
    mpView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Connect the view widget
    connect(mpView, &QTreeView::expanded, this, &ProjectBrowser::processExpansion);
    connect(mpView, &QTreeView::collapsed, this, &ProjectBrowser::processExpansion);
    connect(mpView, &QTreeView::customContextMenuRequested, this, &ProjectBrowser::processContextMenuRequest);
    connect(mpView, &QTreeView::doubleClicked, this, &ProjectBrowser::processDoubleClick);

    // Create the actions
    QAction* pExpandAction = new QAction(QIcon(":/icons/arrows-expand.svg"), tr("E&xpand all"), this);
    QAction* pCollapseAction = new QAction(QIcon(":/icons/arrows-collapse.svg"), tr("&Collapse all"), this);

    // Set the shortcuts
    pExpandAction->setShortcut(Qt::CTRL | Qt::Key_E);
    pCollapseAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_E);

    // Connect the actions
    connect(pExpandAction, &QAction::triggered, mpView, &QTreeView::expandAll);
    connect(pCollapseAction, &QAction::triggered, mpView, &QTreeView::collapseAll);

    // Create the editor to set patterns for the filter
    mpFilterLineEdit = new QLineEdit(this);
    mpFilterLineEdit->setClearButtonEnabled(true);

    // Connect the filter editor
    connect(mpFilterLineEdit, &QLineEdit::textChanged, this, &ProjectBrowser::filterContent);

    // Create the tool bar
    QToolBar* pToolBar = new QToolBar;
    pToolBar->addAction(pExpandAction);
    pToolBar->addAction(pCollapseAction);
    pToolBar->addSeparator();
    pToolBar->addWidget(mpFilterLineEdit);
    Utility::setShortcutHints(pToolBar);

    // Insert the widgets into the main layout
    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->setContentsMargins(kMargin, kMargin, kMargin, kMargin);
    pLayout->addWidget(pToolBar);
    pLayout->addWidget(mpView);
    setLayout(pLayout);
}

//! Filter the browser content using the specified pattern
void ProjectBrowser::filterContent(QString const& pattern)
{
    // Construct the regular expression based on the pattern
    QRegularExpression expression(pattern, QRegularExpression::CaseInsensitiveOption);

    // Set the expression
    if (expression.isValid())
    {
        mpFilterLineEdit->setToolTip(QString());
        mpFilterModel->setFilterRegularExpression(expression);
        Utility::setTextColor(mpFilterLineEdit, Utility::textColor(style()->standardPalette()));
    }
    else
    {
        mpFilterLineEdit->setToolTip(expression.errorString());
        mpFilterModel->setFilterRegularExpression(QRegularExpression());
        Utility::setTextColor(mpFilterLineEdit, Qt::red);
    }
}

//! Show a context menu when selected item clicked
void ProjectBrowser::processContextMenuRequest(QPoint const& point)
{
    // Retrieve the selected items
    QList<HierarchyItem*> items = selectedItems();
    if (items.isEmpty())
        return;

    // Create the context menu
    QMenu* pMenu = new QMenu(this);
    pMenu->setAttribute(Qt::WA_DeleteOnClose);

    // Create editors for all selected items
    mpEditorManager->clear();
    createItemEditors(items);

    // Create the edit action
    if (!mpEditorManager->isEmpty())
    {
        QAction* pEditAction = new QAction(QIcon(":/icons/edit-edit.svg"), tr("&Edit"));
        connect(pEditAction, &QAction::triggered, mpEditorManager, &EditorManager::show);
        pMenu->addAction(pEditAction);
    }

    // Create the item associated actions
    createModelActions(pMenu, items);

    // Fill up the menu with the common actions
    if (!pMenu->actions().isEmpty())
        pMenu->addSeparator();
    pMenu->addAction(QIcon(":/icons/arrows-expand.svg"), tr("Expand"), this, [this]() { setSelectedItemsExpandedState(true); });
    pMenu->addAction(QIcon(":/icons/arrows-collapse.svg"), tr("Collapse"), this, [this]() { setSelectedItemsExpandedState(false); });

    // Show the menu
    QPoint position = mpView->mapToGlobal(point);
    pMenu->exec(position);
}

//! Process selection of hierarchy items
void ProjectBrowser::processSelection(QItemSelection const& selected, QItemSelection const& deselected)
{
    mSelectedState.clear();

    // Save the selected state
    QModelIndexList const indices = mpView->selectionModel()->selectedIndexes();
    for (auto const& index : indices)
    {
        QModelIndex proxyIndex = mpFilterModel->mapToSource(index);
        HierarchyItem* pItem = (HierarchyItem*) mpSourceModel->itemFromIndex(proxyIndex);
        mSelectedState[pItem->id()] = true;
    }

    // Send the selected items
    emit selectionChanged(selectedItems());
}

//! Show an editor on double click
void ProjectBrowser::processDoubleClick(QModelIndex const& index)
{
    // Retrieve the item
    QModelIndex sourceIndex = mpFilterModel->mapToSource(index);
    HierarchyItem* pItem = (HierarchyItem*) mpSourceModel->itemFromIndex(sourceIndex);

    // Create the editor
    mpEditorManager->clear();
    createItemEditor(pItem);
    if (!mpEditorManager->isEmpty())
        mpEditorManager->show();
}

//! Process a collapsed or expanded item
void ProjectBrowser::processExpansion(QModelIndex const& index)
{
    QModelIndex proxyIndex = mpFilterModel->mapToSource(index);
    HierarchyItem* pItem = (HierarchyItem*) mpSourceModel->itemFromIndex(proxyIndex);
    mExpandedState[pItem->id()] = mpView->isExpanded(index);
}

//! Construct an element editor for a hierarchy item
void ProjectBrowser::createElementEditor(HierarchyItem* pBaseItem)
{
    ElementHierarchyItem* pItem = (ElementHierarchyItem*) pBaseItem;
    Core::Selection selection = Core::Selection(pItem->iSurface(), pItem->element()->type(), pItem->iElement());
    KCL::Model* pModel = pItem->kclModel();
    if (pModel)
        mpEditorManager->createEditor(*pModel, selection);
}

//! Create an editor for hierarchy item
void ProjectBrowser::createItemEditor(HierarchyItem* pBaseItem)
{
    auto type = pBaseItem->type();
    switch (type)
    {
    case HierarchyItem::kGroupElements:
    {
        QList<HierarchyItem*> childItems = Utility::childItems(pBaseItem);
        int numChildren = childItems.size();
        for (int iChild = 0; iChild != numChildren; ++iChild)
            createElementEditor(childItems[iChild]);
        break;
    }
    case HierarchyItem::kElement:
        createElementEditor(pBaseItem);
        break;
    case HierarchyItem::kModel:
        mpEditorManager->createEditor(static_cast<ModelHierarchyItem*>(pBaseItem)->kclModel());
        break;
    case HierarchyItem::kModalOptions:
        mpEditorManager->createEditor(static_cast<ModalOptionsHierarchyItem*>(pBaseItem)->options());
        break;
    case HierarchyItem::kFlutterOptions:
        mpEditorManager->createEditor(static_cast<FlutterOptionsHierarchyItem*>(pBaseItem)->options());
        break;
    case HierarchyItem::kOptimOptions:
        mpEditorManager->createEditor(static_cast<OptimOptionsHierarchyItem*>(pBaseItem)->options());
        break;
    case HierarchyItem::kOptimConstraints:
        mpEditorManager->createEditor(static_cast<OptimConstraintsHierarchyItem*>(pBaseItem)->constraints());
        break;
    case HierarchyItem::kOptimTarget:
        mpEditorManager->createEditor(static_cast<OptimTargetHierarchyItem*>(pBaseItem)->target());
        break;
    default:
        break;
    }
}

//! Create multiple editors for hierarchy items
void ProjectBrowser::createItemEditors(QList<HierarchyItem*> const& items)
{
    int numItems = items.size();
    for (int i = 0; i != numItems; ++i)
    {
        HierarchyItem* pBaseItem = items[i];
        createItemEditor(pBaseItem);
    }
}

//! Create model associated actions
void ProjectBrowser::createModelActions(QMenu* pMenu, QList<HierarchyItem*> const& items)
{
    // Obtain the model hierarchy item
    if (items.size() != 1)
        return;
    HierarchyItem* pBaseItem = items.first();
    if (pBaseItem->type() != HierarchyItem::kModel)
        return;
    ModelHierarchyItem* pItem = (ModelHierarchyItem*) pBaseItem;

    // Get the data
    KCL::Model* pModel = &pItem->kclModel();
    QString subprojectName = pItem->subproject()->name();

    // Create the actions
    QAction* pOpenAction = new QAction(tr("&Open..."), this);
    QAction* pSaveAsAction = new QAction(tr("&Save as..."), this);

    // Set the icons
    pOpenAction->setIcon(QIcon(":/icons/document-model.svg"));
    pSaveAsAction->setIcon(QIcon(":/icons/document-save-as.svg"));

    // Set the connections
    connect(pOpenAction, &QAction::triggered, this,
            [this, pModel]()
            {
                QString defaultDir = Utility::getLastDirectory(mSettings).absolutePath();
                QString pathFile = QFileDialog::getOpenFileName(this, tr("Open Model"), defaultDir, tr("Model file format (*.dat *.txt)"));
                if (pathFile.isEmpty())
                    return;
                *pModel = Utility::readModel(pathFile);
                Utility::setLastPathFile(mSettings, pathFile);
                refresh();
                emit modelSubstituted(*pModel);
            });
    connect(pSaveAsAction, &QAction::triggered, this,
            [this, pModel, subprojectName]()
            {
                int const kMaxLength = 4;
                QString name = subprojectName;
                name = name.replace(" ", "-").toUpper();
                if (name.length() > kMaxLength)
                    name = name.first(kMaxLength);
                QString defaultFileName = QString("DAT%1.dat").arg(name);
                QString defaultPathFile = Utility::getLastDirectory(mSettings).absoluteFilePath(defaultFileName);
                QString pathFile = QFileDialog::getSaveFileName(this, tr("Save Model"), defaultPathFile, tr("Model file format (*.dat *.txt)"));
                if (pathFile.isEmpty())
                    return;
                Utility::writeModel(pathFile, *pModel);
                Utility::setLastPathFile(mSettings, pathFile);
            });

    // Add the actions to the menu
    if (!pMenu->actions().isEmpty())
        pMenu->addSeparator();
    pMenu->addAction(pOpenAction);
    pMenu->addAction(pSaveAsAction);
}

//! Set selected and expanded states of the tree model
void ProjectBrowser::setModelState()
{
    uint numRows = mpFilterModel->rowCount();
    for (uint iRow = 0; iRow != numRows; ++iRow)
        setItemModelState(mpFilterModel->index(iRow, 0));
}

//! Specify selected and expanded states of a tree model item
void ProjectBrowser::setItemModelState(QModelIndex const& index)
{
    // Check if the index is valid
    if (!index.isValid())
        return;

    // Set the item state
    QModelIndex proxyIndex = mpFilterModel->mapToSource(index);
    HierarchyItem* pItem = (HierarchyItem*) mpSourceModel->itemFromIndex(proxyIndex);
    QString id = pItem->id();
    if (mSelectedState.contains(id) && mSelectedState[id])
        mpView->selectionModel()->select(index, QItemSelectionModel::Select);
    if (mExpandedState.contains(id))
        mpView->setExpanded(index, mExpandedState[id]);

    // Process the children
    uint numRows = mpFilterModel->rowCount(index);
    for (uint iRow = 0; iRow != numRows; ++iRow)
        setItemModelState(mpFilterModel->index(iRow, 0, index));
}

//! Expand or collapse selected items recursively
void ProjectBrowser::setSelectedItemsExpandedState(bool flag)
{
    QModelIndexList indices = mpView->selectionModel()->selectedIndexes();
    if (indices.empty())
        indices << mpView->rootIndex();
    int numIndices = indices.size();
    for (int i = 0; i != numIndices; ++i)
    {
        QModelIndex const& index = indices[i];
        if (flag)
            mpView->expandRecursively(index);
        else
            mpView->collapse(index);
    }
}
