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
    // mpMainWindow->show();
}

//! View a model using project browser hierarchy
void TestFrontend::testViewModel()
{
    int iSubproject = 1;
    Core::Subproject& subproject = mpMainWindow->project().subprojects()[iSubproject];
    KCL::Model& model = subproject.model();
    mpMainWindow->viewManager()->createView(model);
}

//! Edit elements of different types through the manager
void TestFrontend::testEditorManager()
{
    int iSubproject = 1;
    int iSurface = 0;
    EditorManager* pManager = mpMainWindow->projectBrowser()->editorManager();
    Core::Subproject& subproject = mpMainWindow->project().subprojects()[iSubproject];
    mpModel = new KCL::Model(subproject.model());
    KCL::ElasticSurface& surface = mpModel->surfaces[iSurface];
    surface.insertElement(KCL::BK);
    surface.insertElement(KCL::PN);
    surface.insertElement(KCL::P4);
    surface.insertElement(KCL::SM);
    pManager->createEditor(*mpModel, Core::Selection(iSurface, KCL::OD));
    pManager->createEditor(*mpModel, Core::Selection(iSurface, KCL::BI));
    pManager->createEditor(*mpModel, Core::Selection(iSurface, KCL::BK));
    pManager->createEditor(*mpModel, Core::Selection(iSurface, KCL::PN));
    pManager->createEditor(*mpModel, Core::Selection(iSurface, KCL::P4));
    pManager->createEditor(*mpModel, Core::Selection(iSurface, KCL::OP));
    pManager->createEditor(*mpModel, Core::Selection(iSurface, KCL::SM));
    pManager->createEditor(*mpModel, Core::Selection(iSurface, KCL::M3));
    pManager->createEditor(*mpModel, Core::Selection(-1, KCL::CO));
    pManager->createEditor(*mpModel, Core::Selection(-1, KCL::WP));
    pManager->setCurrentEditor(pManager->numEditors() - 1);
    pManager->show();
}

TestFrontend::~TestFrontend()
{
    QTest::qWait(30000);
    mpMainWindow->deleteLater();
    delete mpModel;
}

QTEST_MAIN(TestFrontend)
