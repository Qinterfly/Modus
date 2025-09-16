#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include "project.h"
#include "projectbrowser.h"
#include "projecthierarchymodel.h"

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
    return QSize(200, 1000);
}

//! Update the viewer content
void ProjectBrowser::update()
{
    mpView->setModel(nullptr);
    mpModel = new ProjectHierarchyModel(mProject, mpView);
    mpView->setModel(mpModel);
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

    // Create the tool bar
    QToolBar* pToolBar = new QToolBar();
    pToolBar->addAction(pExpandAction);
    pToolBar->addAction(pCollapseAction);

    // Insert the widgets into the main layout
    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->setContentsMargins(kMargin, kMargin, kMargin, kMargin);
    pLayout->addWidget(pToolBar);
    pLayout->addWidget(mpView);
    setLayout(pLayout);
}
