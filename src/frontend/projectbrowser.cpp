#include <QLineEdit>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

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
    update();
}

ProjectBrowser::~ProjectBrowser()
{
}

QSize ProjectBrowser::sizeHint() const
{
    return QSize(150, 1000);
}

//! Update the viewer content
void ProjectBrowser::update()
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
}

//! Create all the widgets and corresponding actions
void ProjectBrowser::createContent()
{
    uint const kMargin = 2;

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
    connect(mpView, &QTreeView::customContextMenuRequested, this, &ProjectBrowser::processContextMenuRequest);

    // Create the actions
    QAction* pExpandAction = new QAction(tr("E&xpand all"), this);
    QAction* pCollapseAction = new QAction(tr("&Collapse all"), this);

    // Set the shortcuts
    pExpandAction->setShortcut(Qt::CTRL | Qt::Key_E);
    pCollapseAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_E);

    // Set the icons
    pExpandAction->setIcon(QIcon(":/icons/arrows-expand.svg"));
    pCollapseAction->setIcon(QIcon(":/icons/arrows-collapse.svg"));

    // Connect the actions
    connect(pExpandAction, &QAction::triggered, mpView, &QTreeView::expandAll);
    connect(pCollapseAction, &QAction::triggered, mpView, &QTreeView::collapseAll);

    // Create the editor to set patterns for the filter
    mpFilterLineEdit = new QLineEdit(this);
    mpFilterLineEdit->setClearButtonEnabled(true);

    // Connect the filter editor
    connect(mpFilterLineEdit, &QLineEdit::textChanged, this, &ProjectBrowser::filterContent);

    // Create the tool bar
    QToolBar* pToolBar = new QToolBar();
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
    QList<QStandardItem*> items = selectedItems();
    if (items.isEmpty())
        return;

    // Create the context menu
    QMenu* pMenu = new QMenu(this);
    pMenu->setAttribute(Qt::WA_DeleteOnClose);

    // Fill up the menu with the common actions
    if (!pMenu->actions().isEmpty())
        pMenu->addSeparator();
    pMenu->addAction(tr("Expand"), this, [this]() { setSelectedItemsExpandedState(true); });
    pMenu->addAction(tr("Collapse"), this, [this]() { setSelectedItemsExpandedState(false); });

    // Show the menu
    QPoint position = mpView->mapToGlobal(point);
    pMenu->exec(position);
}

//! Retrieve the currently selected items
QList<QStandardItem*> ProjectBrowser::selectedItems()
{
    QModelIndexList indices = mpView->selectionModel()->selectedIndexes();
    QList<QStandardItem*> result;
    uint numIndices = indices.size();
    result.reserve(numIndices);
    for (int i = 0; i != numIndices; ++i)
    {
        QModelIndex const& proxyIndex = indices[i];
        QModelIndex sourceIndex = mpFilterModel->mapToSource(proxyIndex);
        QStandardItem* pItem = mpSourceModel->itemFromIndex(sourceIndex);
        result.push_back(pItem);
    }
    return result;
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
