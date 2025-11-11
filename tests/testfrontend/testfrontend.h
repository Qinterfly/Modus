#ifndef TESTFRONTEND_H
#define TESTFRONTEND_H

#include <QTest>

#include "mainwindow.h"

namespace Frontend
{
class MainWindow;
}

namespace Tests
{

class TestFrontend : public QObject
{
    Q_OBJECT

public:
    TestFrontend();
    virtual ~TestFrontend();

private slots:
    void testOpenProject();
    void testViewModel();
    void testViewGeometry();
    void testViewLog();
    void testViewFlutter();
    void testEditorManager();

private:
    Frontend::MainWindow* mpMainWindow;
    KCL::Model* mpModel;
};

}

#endif // TESTFRONTEND_H
