
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

#include "project.h"

namespace ads
{
class CDockWidget;
class CDockManager;
}

namespace Frontend
{

class Logger;
class ProjectBrowser;
class ViewManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* pParent = nullptr);
    virtual ~MainWindow();
    static Logger* pLogger;

    // File interaction
    void newProject();
    bool openProject(QString const& pathFile);
    void saveProject();
    void saveAsProject(QString const& pathFile);

    // Objects
    Backend::Core::Project& project();

    // Widgets
    ProjectBrowser* projectBrowser();
    ViewManager* viewManager();

public:
    static QString language;

private:
    void initializeWindow();
    void closeEvent(QCloseEvent* pEvent) override;

    // Content
    void createContent();
    void createFileActions();
    void createWindowActions();
    void createHelpActions();
    void createDockManager();
    ads::CDockWidget* createProjectBrowser();
    ads::CDockWidget* createViewManager();
    ads::CDockWidget* createLogger();
    void createConnections();

    // State
    void setProjectTitle();
    void setModified(bool flag);
    void setTheme();

    // Recent
    void retrieveRecentProjects();
    void addToRecentProjects();
    void openRecentProject();

    // Settings
    void saveSettings();
    void restoreSettings();

    // Dialogs
    void newProjectDialog();
    void openProjectDialog();
    void saveAsProjectDialog();
    bool saveProjectChangesDialog();
    void about();

private:
    QSettings mSettings;
    QList<QString> mPathRecentProjects;

    // UI
    ads::CDockManager* mpDockManager;
    QMenu* mpRecentMenu;
    QMenu* mpWindowMenu;
    ProjectBrowser* mpProjectBrowser;
    ViewManager* mpViewManager;

    // Project
    Backend::Core::Project mProject;
};

void logMessage(QtMsgType type, QMessageLogContext const& /*context*/, QString const& message);
}

#endif // MAINWINDOW_H
