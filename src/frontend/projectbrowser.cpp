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

    // Insert the widgets into the main layout
    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->setContentsMargins(kMargin, kMargin, kMargin, kMargin);
    pLayout->addWidget(mpView);
    setLayout(pLayout);
}
