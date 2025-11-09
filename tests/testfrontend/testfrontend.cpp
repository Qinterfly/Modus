#include <config.h>

#include <kcl/model.h>

#include "editormanager.h"
#include "fileutility.h"
#include "fluttersolver.h"
#include "geometryview.h"
#include "modalsolver.h"
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

//! View a model
void TestFrontend::testViewModel()
{
    int iSubproject = 1;
    Core::Subproject& subproject = mpMainWindow->project().subprojects()[iSubproject];
    KCL::Model& model = subproject.model();
    mpMainWindow->viewManager()->createView(model);
}

//! View a model geometry
void TestFrontend::testViewGeometry()
{
    int iSubproject = 1;
    int iMode = 8;
    Core::Subproject& subproject = mpMainWindow->project().subprojects()[iSubproject];
    auto solvers = subproject.solvers(Core::ISolver::kModal);
    int numSolvers = solvers.size();
    for (int i = 0; i != numSolvers; ++i)
    {
        auto pSolver = (Core::ModalSolver*) solvers[i];
        VertexField field(pSolver->solution, iMode);
        mpMainWindow->viewManager()->createView(pSolver->solution.geometry, field);
    }
}

//! View a solver log
void TestFrontend::testViewLog()
{
    int iSubproject = 0;
    int iSolver = 1;
    Core::Subproject& subproject = mpMainWindow->project().subprojects()[iSubproject];
    Core::ISolver* pBaseSolver = subproject.solvers()[iSolver];
    QVERIFY(pBaseSolver);
    QString log;
    switch (pBaseSolver->type())
    {
    case Core::ISolver::kModal:
        log = static_cast<Core::ModalSolver*>(pBaseSolver)->log;
        break;
    case Core::ISolver::kFlutter:
        log = static_cast<Core::FlutterSolver*>(pBaseSolver)->log;
        break;
    case Core::ISolver::kOptim:
        log = static_cast<Core::OptimSolver*>(pBaseSolver)->log;
        break;
    }
    mpMainWindow->viewManager()->createView(log);
}

//! Edit elements of different types through the manager
void TestFrontend::testEditorManager()
{
    int iSubproject = 0;
    int iSurface = 0;
    EditorManager* pManager = mpMainWindow->projectBrowser()->editorManager();
    Core::Subproject& subproject = mpMainWindow->project().subprojects()[iSubproject];
    mpModel = new KCL::Model(subproject.model());
    KCL::ElasticSurface& surface = mpModel->surfaces[iSurface];

    // Obtaint the solvers
    auto pModalSolver = (Core::ModalSolver*) subproject.solver(Core::ISolver::kModal);
    auto pFlutterSolver = (Core::FlutterSolver*) subproject.solver(Core::ISolver::kFlutter);
    auto pOptimSolver = (Core::OptimSolver*) subproject.solver(Core::ISolver::kOptim);

    // Introduce dummy elements
    surface.insertElement(KCL::BK);
    surface.insertElement(KCL::PN);
    surface.insertElement(KCL::P4);
    surface.insertElement(KCL::SM);
    surface.insertElement(KCL::DA);
    surface.insertElement(KCL::GS);

    // Create editors of elements
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
    pManager->createEditor(*mpModel, Core::Selection(iSurface, KCL::DA));
    pManager->createEditor(*mpModel, Core::Selection(iSurface, KCL::GS));
    pManager->createEditor(*mpModel, Core::Selection(iSurface, KCL::DE));
    pManager->createEditor(*mpModel, Core::Selection(iSurface, KCL::AE, 1));
    pManager->createEditor(*mpModel, Core::Selection(-1, KCL::TE));
    pManager->createEditor(*mpModel, Core::Selection(iSurface, KCL::PK));

    // Create a model editor
    pManager->createEditor(*mpModel);

    // Create editors of solvers
    if (pModalSolver)
        pManager->createEditor(pModalSolver->options);
    if (pFlutterSolver)
        pManager->createEditor(pFlutterSolver->options);
    if (pOptimSolver)
    {
        pManager->createEditor(pOptimSolver->options);
        pManager->createEditor(pOptimSolver->problem.constraints);
    }
    pManager->setCurrentEditor(pManager->numEditors() - 1);
    if (!mpMainWindow->isVisible())
        pManager->show();
}

TestFrontend::~TestFrontend()
{
    QTest::qWait(30000);
    mpMainWindow->deleteLater();
    delete mpModel;
}

QTEST_MAIN(TestFrontend)
