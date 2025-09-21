#include <DockManager.h>
#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFontDatabase>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolBar>

#include "config.h"
#include "logger.h"
#include "mainwindow.h"
#include "projectbrowser.h"
#include "uiconstants.h"
#include "uiutility.h"
#include "viewmanager.h"

using namespace ads;
using namespace Frontend;
using namespace Backend;

Logger* MainWindow::pLogger = nullptr;
QString MainWindow::language = "en";

MainWindow::MainWindow(QWidget* pParent)
    : QMainWindow(pParent)
    , mSettings(Constants::Settings::skFileName, QSettings::IniFormat)
{
    initializeWindow();
    createContent();
    createConnections();
    // restoreSettings();
    newProject();
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
    mpProjectBrowser->update();
}

//! Read the project located at the specified path
bool MainWindow::openProject(QString const& pathFile)
{
    if (mProject.read(pathFile))
    {
        qInfo() << tr("Project %1 was successfully opened").arg(pathFile);
        setModified(false);
        addToRecentProjects();
        mpProjectBrowser->update();
        return true;
    }
    return false;
}

//! Save the project at the last used path
void MainWindow::saveProject()
{
    QString const& lastPath = mProject.pathFile();
    if (lastPath.isEmpty())
    {
        saveAsProjectDialog();
        return;
    }
    if (mProject.write(lastPath))
    {
        qInfo() << tr("The project was saved using the previous location: %1").arg(lastPath);
        setModified(false);
    }
}

//! Save the project using the path specified
void MainWindow::saveAsProject(QString const& pathFile)
{
    if (mProject.write(pathFile))
    {
        qInfo() << tr("The project was saved as the following file %1").arg(pathFile);
        addToRecentProjects();
        setModified(false);
    }
}

//! Get the widget to browse project hierarchy
ProjectBrowser* MainWindow::projectBrowser()
{
    return mpProjectBrowser;
}

//! Get the manager to view project entities
ViewManager* MainWindow::viewManager()
{
    return mpViewManager;
}

//! Set a state and geometry of the main window
void MainWindow::initializeWindow()
{
    setWindowState(Qt::WindowMaximized);
    setWindowTitle(QString(APP_NAME) + "[*]");
    setTheme();
    qInstallMessageHandler(Frontend::logMessage);

    // Since OpenGL widgets tend to change the window geometry, we set the maximized state manually
    Utility::fullScreenResize(this);
}

//! Save project and settings before exit
void MainWindow::closeEvent(QCloseEvent* pEvent)
{
    bool isClose = saveProjectChangesDialog();
    if (isClose)
    {
        saveSettings();
        pEvent->accept();
    }
    else
    {
        pEvent->ignore();
    }
}

//! Create all the widgets and corresponding actions
void MainWindow::createContent()
{
    // Top widgets
    createFileActions();
    createWindowActions();
    createHelpActions();

    // Manager to place dockable widgets
    createDockManager();

    // Create project browser
    ads::CDockWidget* pWidget = createProjectBrowser();
    ads::CDockAreaWidget* pArea = mpDockManager->addDockWidget(ads::BottomDockWidgetArea, pWidget);

    // Create view manager
    pWidget = createViewManager();
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

//! Create a widget to navigate through project structure
ads::CDockWidget* MainWindow::createProjectBrowser()
{
    // Create the widget
    mpProjectBrowser = new ProjectBrowser(mProject, mSettings);

    // Construct the dock widget
    ads::CDockWidget* pDockWidget = new CDockWidget(mpDockManager, tr("Project Browser"));
    pDockWidget->setWidget(mpProjectBrowser);
    mpWindowMenu->addAction(pDockWidget->toggleViewAction());
    return pDockWidget;
}

//! Create a widget to display project entities
ads::CDockWidget* MainWindow::createViewManager()
{
    // Create the widget
    mpViewManager = new ViewManager(mSettings);

    // Construct the dock widget
    ads::CDockWidget* pDockWidget = new CDockWidget(mpDockManager, tr("View Manager"));
    pDockWidget->setWidget(mpViewManager);
    mpWindowMenu->addAction(pDockWidget->toggleViewAction());
    return pDockWidget;
}

//! Create a widget to log program events
ads::CDockWidget* MainWindow::createLogger()
{
    // Create the logger
    if (!pLogger)
        pLogger = new Logger;

    // Create a dock widget
    ads::CDockWidget* pDockWidget = new ads::CDockWidget(mpDockManager, tr("Log"));
    pDockWidget->setWidget(pLogger);
    mpWindowMenu->addAction(pDockWidget->toggleViewAction());
    return pDockWidget;
}

//! Connect the widgets between each other
void MainWindow::createConnections()
{
    connect(mpProjectBrowser, &ProjectBrowser::selectionChanged, mpViewManager, &ViewManager::processItems);
}

//! Create the actions to interact with files
void MainWindow::createFileActions()
{
    // Create the actions
    QAction* pNewAction = new QAction(tr("&New Project"), this);
    QAction* pOpenProjectAction = new QAction(tr("&Open Project..."), this);
    QAction* pSaveAction = new QAction(tr("&Save"), this);
    QAction* pSaveAsAction = new QAction(tr("&Save As..."), this);
    QAction* pExitAction = new QAction(tr("E&xit"), this);

    // Set the icons
    pNewAction->setIcon(QIcon(":/icons/document-new.svg"));
    pOpenProjectAction->setIcon(QIcon(":/icons/document-open.svg"));
    pSaveAction->setIcon(QIcon(":/icons/document-save.svg"));
    pSaveAsAction->setIcon(QIcon(":/icons/document-save-as.svg"));

    // Set the shortcuts
    pNewAction->setShortcut(QKeySequence::New);
    pOpenProjectAction->setShortcut(QKeySequence::Open);
    pSaveAction->setShortcut(QKeySequence::Save);
    pSaveAsAction->setShortcut(QKeySequence::SaveAs);
    pExitAction->setShortcut(QKeySequence::Quit);

    // Create the menu
    QMenu* pFileMenu = new QMenu(tr("&File"), this);
    mpRecentMenu = new QMenu(tr("Recent P&rojects"), this);
    pFileMenu->setFont(font());

    // Connect the actions
    connect(pNewAction, &QAction::triggered, this, &MainWindow::newProjectDialog);
    connect(pOpenProjectAction, &QAction::triggered, this, &MainWindow::openProjectDialog);
    connect(pSaveAction, &QAction::triggered, this, &MainWindow::saveProject);
    connect(pSaveAsAction, &QAction::triggered, this, &MainWindow::saveAsProjectDialog);
    connect(pExitAction, &QAction::triggered, qApp, &QApplication::quit);

    // Fill up the menu
    pFileMenu->addAction(pNewAction);
    pFileMenu->addAction(pOpenProjectAction);
    pFileMenu->addMenu(mpRecentMenu);
    pFileMenu->addSeparator();
    pFileMenu->addAction(pSaveAction);
    pFileMenu->addAction(pSaveAsAction);
    pFileMenu->addSeparator();
    pFileMenu->addAction(pExitAction);
    menuBar()->addMenu(pFileMenu);

    // Create the toolbar
    QToolBar* pFileToolBar = new QToolBar;
    pFileToolBar->setIconSize(Constants::Size::skToolBarIcon);
    pFileToolBar->addAction(pNewAction);
    pFileToolBar->addAction(pOpenProjectAction);
    pFileToolBar->addSeparator();
    pFileToolBar->addAction(pSaveAction);
    pFileToolBar->addAction(pSaveAsAction);
    Utility::setShortcutHints(pFileToolBar);
    addToolBar(pFileToolBar);
}

//! Create the actions to customize windows
void MainWindow::createWindowActions()
{
    mpWindowMenu = new QMenu(tr("&Window"), this);
    mpWindowMenu->setFont(font());
    menuBar()->addMenu(mpWindowMenu);
}

//! Create the actions to show the program info
void MainWindow::createHelpActions()
{
    // Create the actions
    QAction* pAboutAction = new QAction(tr("&About"), this);
    QAction* pAboutQtAction = new QAction(tr("&About Qt"), this);

    // Connect the actions
    connect(pAboutAction, &QAction::triggered, this, &MainWindow::about);
    connect(pAboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);

    // Fill up the menu
    QMenu* pHelpMenu = new QMenu(tr("&Help"), this);
    pHelpMenu->setFont(font());
    pHelpMenu->addAction(pAboutAction);
    pHelpMenu->addAction(pAboutQtAction);
    menuBar()->addMenu(pHelpMenu);
}

//! Open the project which was selected from the Recent Projects menu
void MainWindow::openRecentProject()
{
    if (!saveProjectChangesDialog())
        return;
    QAction* pAction = (QAction*) sender();
    if (pAction)
        openProject(pAction->text());
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
    QFontDatabase::addApplicationFont(":/fonts/RobotoMono.ttf");
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

//! Retrieve recent projects from the settings file
void MainWindow::retrieveRecentProjects()
{
    QList<QVariant> listSettingsProjects = mSettings.value(Constants::Settings::skRecent).toList();
    mPathRecentProjects.clear();
    mpRecentMenu->clear();
    QString pathProject;
    QList<QVariant> updatedPaths;
    int numRecentProjects = listSettingsProjects.size();
    for (int i = 0; i != numRecentProjects; ++i)
    {
        QVariant const& varPath = listSettingsProjects[i];
        pathProject = varPath.toString();
        if (QFileInfo::exists(pathProject))
        {
            updatedPaths.push_back(pathProject);
            QAction* pAction = mpRecentMenu->addAction(pathProject);
            connect(pAction, &QAction::triggered, this, &MainWindow::openRecentProject);
            mPathRecentProjects.push_back(pathProject);
        }
    }
    mSettings.setValue(Constants::Settings::skRecent, updatedPaths);
}

//! Add the current project to the recent ones
void MainWindow::addToRecentProjects()
{
    QString const& pathFile = mProject.pathFile();
    if (!pathFile.isEmpty())
    {
        if (!mPathRecentProjects.contains(pathFile))
            mPathRecentProjects.push_back(pathFile);
        while (mPathRecentProjects.count() > Constants::Size::skMaxRecentProjects)
            mPathRecentProjects.pop_front();
        mpRecentMenu->clear();
        QList<QVariant> listSettingsProjects;
        int numRecentProjects = mPathRecentProjects.size();
        for (int i = 0; i != numRecentProjects; ++i)
        {
            QString const& path = mPathRecentProjects[i];
            listSettingsProjects.push_back(path);
            QAction* pAction = mpRecentMenu->addAction(path);
            connect(pAction, &QAction::triggered, this, &MainWindow::openRecentProject);
        }
        mSettings.setValue(Constants::Settings::skRecent, listSettingsProjects);
    }
}

//! Save window settings to a file
void MainWindow::saveSettings()
{
    mSettings.beginGroup(Constants::Settings::skMainWindow);
    mSettings.setValue(Constants::Settings::skLanguage, MainWindow::language);
    mSettings.setValue(Constants::Settings::skGeometry, saveGeometry());
    mSettings.setValue(Constants::Settings::skState, saveState());
    mSettings.setValue(Constants::Settings::skDockingState, mpDockManager->saveState());
    mSettings.endGroup();
    if (mSettings.status() == QSettings::NoError)
        qInfo() << tr("Settings were written to the file %1").arg(Constants::Settings::skFileName);
}

//! Restore window settings from a file
void MainWindow::restoreSettings()
{
    if (mSettings.allKeys().empty())
        return;
    mSettings.beginGroup(Constants::Settings::skMainWindow);
    QString lang = mSettings.value(Constants::Settings::skLanguage).toString();
    if (lang == language)
    {
        bool isOk = restoreGeometry(mSettings.value(Constants::Settings::skGeometry).toByteArray())
                    && restoreState(mSettings.value(Constants::Settings::skState).toByteArray())
                    && mpDockManager->restoreState(mSettings.value(Constants::Settings::skDockingState).toByteArray());
        if (isOk)
            qInfo() << tr("Settings were restored from the file %1").arg(Constants::Settings::skFileName);
    }
    mSettings.endGroup();
    retrieveRecentProjects();
}

//! Create the new project using the dialog, if necessary
void MainWindow::newProjectDialog()
{
    if (!saveProjectChangesDialog())
        return;
    newProject();
}

//! Read the project using the file dialog
void MainWindow::openProjectDialog()
{
    static QString const kExpectedSuffix = Core::Project::fileSuffix();

    if (!saveProjectChangesDialog())
        return;

    // Create the file dialog
    QString pathFile = QFileDialog::getOpenFileName(this, tr("Open Project"), mProject.pathFile(),
                                                    tr("Project file format (*%1)").arg(kExpectedSuffix));
    if (pathFile.isEmpty())
        return;
    openProject(pathFile);
}

//! Save the project using the file dialog
void MainWindow::saveAsProjectDialog()
{
    static QString const kExpectedSuffix = Core::Project::fileSuffix();

    QString pathFile = QFileDialog::getSaveFileName(this, tr("Save Project"), mProject.pathFile(),
                                                    tr("Project file format (*%1)").arg(kExpectedSuffix));
    if (pathFile.isEmpty())
        return;

    // Modify the suffix, if necessary
    Utility::modifyFileSuffix(pathFile, kExpectedSuffix);

    // Save the project
    saveAsProject(pathFile);
}

//! Save project changes through dialog
bool MainWindow::saveProjectChangesDialog()
{
    QString const title = tr("Save project changes");
    QString const message = tr("The project containes unsaved changes.\n"
                               "Would you like to save it?");
    if (isWindowModified())
    {
        int iResult = Utility::showSaveDialog(this, title, message);
        if (iResult < 0)
            return false;
        if (iResult == 1)
            saveProject();
    }
    return true;
}

//! Show information about the program
void MainWindow::about()
{
    QString const build = QStringLiteral(__DATE__) + QStringLiteral(" ") + QStringLiteral(__TIME__);
    QString const author = tr("Pavel Lakiza");
    QString const message = tr("%1 is a program to perform aeroelastic analysis using polynomial models\n\n"
                               "Built on %3\n\n"
                               "Copyright (C) %2")
                                .arg(APP_NAME, author, build);
    QString const title = tr("About %1 v%2").arg(APP_NAME, VERSION_FULL);
    QMessageBox::about(this, title, message);
}

//! Helper function to log all the messages
void Frontend::logMessage(QtMsgType type, QMessageLogContext const& /*context*/, QString const& message)
{
    if (Frontend::MainWindow::pLogger)
        Frontend::MainWindow::pLogger->log(type, message);
}
