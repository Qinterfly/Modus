#include <DockManager.h>
#include <QApplication>
#include <QCloseEvent>
#include <QFontDatabase>
#include <QMenuBar>
#include <QTreeView>

#include "config.h"
#include "mainwindow.h"
#include "uiconstants.h"
#include "uiutility.h"

using namespace ads;
using namespace Frontend;
using namespace Backend;

MainWindow::MainWindow(QWidget* pParent)
    : QMainWindow(pParent)
    , mSettings(Constants::Settings::skFileName, QSettings::IniFormat)
{
    initializeWindow();
    createContent();
}

MainWindow::~MainWindow()
{
}

//! Close the current project and create a new one
void MainWindow::newProject()
{
    mProject = Core::Project();
    qInfo() << tr("New project was created");
    setModified(false);
}

//! Read the project located at the specified path
bool MainWindow::openProject(QString const& pathFile)
{
    if (mProject.read(pathFile))
    {
        qInfo() << tr("Project %1 was successfully opened").arg(pathFile);
        setModified(false);
        return true;
    }
    return false;
}

//! Set a state and geometry of the main window
void MainWindow::initializeWindow()
{
    setWindowState(Qt::WindowMaximized);
    setWindowTitle(QString(APP_NAME) + "[*]");
    setTheme();

    // Since OpenGL widgets tend to change the window geometry, we set the maximized state manually
    Utility::fullScreenResize(this);
}

//! Save project and settings before exit
void MainWindow::closeEvent(QCloseEvent* pEvent)
{
    pEvent->accept();
    // TODO
}

//! Create all the widgets and corresponding actions
void MainWindow::createContent()
{
    createDockManager();

    // Create control manager
    ads::CDockWidget* pWidget = createControlManager();
    mpDockManager->addDockWidget(ads::TopDockWidgetArea, pWidget);

    // Create project browser
    pWidget = createProjectBrowser();
    ads::CDockAreaWidget* pArea = mpDockManager->addDockWidget(ads::BottomDockWidgetArea, pWidget);

    // Create plot manager
    pWidget = createPlotManager();
    pArea = mpDockManager->addDockWidget(ads::RightDockWidgetArea, pWidget, pArea);

    // Create logger
    pWidget = createLogger();
    mpDockManager->addDockWidget(ads::BottomDockWidgetArea, pWidget, pArea);
}

//! Create the dock manager and specify its configuration
void MainWindow::createDockManager()
{
    ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
    ads::CDockManager::setAutoHideConfigFlags({ads::CDockManager::DefaultAutoHideConfig});
    mpDockManager = new ads::CDockManager(this);
}

//! Create a widget to control all action
ads::CDockWidget* MainWindow::createControlManager()
{
    // Create the widget to edit panel data
    QWidget* pWidget = new QTabWidget;

    // Construct the dock widget
    ads::CDockWidget* pDockWidget = new CDockWidget(mpDockManager, tr("Control Manager"));
    pDockWidget->setWidget(pWidget);
    return pDockWidget;
}

//! Create a widget to navigate through project structure
ads::CDockWidget* MainWindow::createProjectBrowser()
{
    // Create the widget to edit panel data
    QWidget* pWidget = new QTreeView;

    // Construct the dock widget
    ads::CDockWidget* pDockWidget = new CDockWidget(mpDockManager, tr("Project Browser"));
    pDockWidget->setWidget(pWidget);
    return pDockWidget;
}

//! Create a widget to plot project entities
ads::CDockWidget* MainWindow::createPlotManager()
{
    // Create the widget to plot project entities
    QWidget* pWidget = new QTreeView;

    // Construct the dock widget
    ads::CDockWidget* pDockWidget = new CDockWidget(mpDockManager, tr("Plot Manager"));
    pDockWidget->setWidget(pWidget);
    return pDockWidget;
}

//! Create a widget to log program events
ads::CDockWidget* MainWindow::createLogger()
{
    // Create the widget to log events
    QWidget* pWidget = new QTreeView;

    // Create a dock widget
    ads::CDockWidget* pDockWidget = new ads::CDockWidget(mpDockManager, tr("Log"));
    pDockWidget->setWidget(pWidget);
    return pDockWidget;
}

//! Represent the project name
void MainWindow::setProjectTitle()
{
    QString const& pathFile = mProject.pathFile();
    if (pathFile.isEmpty())
        setWindowTitle(QString("%1[*]").arg(APP_NAME));
    else
        setWindowTitle(QString("%1: %2[*]").arg(APP_NAME, pathFile));
}

//! Whenever a project has been modified
void MainWindow::setModified(bool flag)
{
    setWindowModified(flag);
    setProjectTitle();
}

//! Set the application font, icon and stylesheet
void MainWindow::setTheme()
{
    // Font
    QFontDatabase::addApplicationFont(":/fonts/Roboto.ttf");
    uint fontSize = 12;
#ifdef Q_OS_WIN
    fontSize = 10;
#endif
    QFont font("Roboto", fontSize);
    setFont(font);
    qApp->setFont(font);
    menuBar()->setFont(font);

    // Icon
    qApp->setWindowIcon(QIcon(":/icons/application.svg"));

    // Style
    qApp->setStyle("Fusion");
    QFile file(":/styles/modern.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);
}
