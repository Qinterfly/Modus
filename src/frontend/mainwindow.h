
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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* pParent = nullptr);
    virtual ~MainWindow();

    // File interaction
    void newProject();
    bool openProject(QString const& pathFile);

private:
    void initializeWindow();
    void closeEvent(QCloseEvent* pEvent) override;

    // Content
    void createContent();
    void createDockManager();
    ads::CDockWidget* createControlManager();
    ads::CDockWidget* createProjectBrowser();
    ads::CDockWidget* createPlotManager();
    ads::CDockWidget* createLogger();

    // State
    void setProjectTitle();
    void setModified(bool flag);
    void setTheme();

private:
    QSettings mSettings;

    // UI
    ads::CDockManager* mpDockManager;

    // Project
    Backend::Core::Project mProject;
};
}

#endif // MAINWINDOW_H
