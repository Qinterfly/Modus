#include <config.h>

#include <kcl/model.h>

#include "editormanager.h"
#include "fileutility.h"
#include "projectbrowser.h"
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

//! Edit elements of different types through the manager
void TestFrontend::testEditorManager()
{
    int iSubproject = 0;
    int iSurface = 0;
    EditorManager* pManager = mpMainWindow->projectBrowser()->editorManager();
    Core::Subproject& subproject = mpMainWindow->project().subprojects()[iSubproject];
    KCL::Model& model = subproject.model();
    pManager->createEditor(model, Core::Selection(iSurface, KCL::BI, 0));
    pManager->createEditor(model, Core::Selection(iSurface, KCL::BK, 0));
}

TestFrontend::~TestFrontend()
{
    QTest::qWait(30000);
    mpMainWindow->deleteLater();
}

QTEST_MAIN(TestFrontend)
