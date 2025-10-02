#include <config.h>

#include "fileutility.h"
#include "testfrontend.h"
#include "viewmanager.h"

using namespace Tests;
using namespace Frontend;
using namespace Backend;

TestFrontend::TestFrontend()
{
    mpMainWindow = new MainWindow;
}

//! Open a project
void TestFrontend::testOpenProject()
{
    QString fileName = QString("tests.%1").arg(Core::Project::fileSuffix());
    QString pathFile = Utility::combineFilePath(EXAMPLES_DIR, fileName);
    QVERIFY(mpMainWindow->openProject(pathFile));
    mpMainWindow->show();
}

//! View a model using project browser hierarchy
void TestFrontend::testViewModel()
{
    KCL::Model& model = mpMainWindow->project().subprojects()[1].model();
    mpMainWindow->viewManager()->createView(model);
}

TestFrontend::~TestFrontend()
{
    QTest::qWait(30000);
    delete mpMainWindow;
}

QTEST_MAIN(TestFrontend)
