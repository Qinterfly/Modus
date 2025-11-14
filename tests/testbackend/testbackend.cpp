#include <QRandomGenerator>

#include "config.h"
#include "fileutility.h"
#include "fluttersolver.h"
#include "optimsolver.h"
#include "selector.h"
#include "subproject.h"
#include "testbackend.h"

using namespace Tests;
using namespace Backend;
using namespace Backend::Core;
using namespace KCL;

TestBackend::TestBackend()
{
    // Files
    mFileNames[kSimpleWing] = "DATWEXA";
    mFileNames[kHunterWing] = "DATW70";
    mFileNames[kFullHunterSym] = "DATH70s";
    mFileNames[kFullHunterASym] = "DATH70a";
    // Subprojects
    mSubprojectNames[kSimpleWing] = "Simple wing";
    mSubprojectNames[kHunterWing] = "Hunter wing";
    mSubprojectNames[kFullHunterSym] = "Full Hunter (sym)";
    mSubprojectNames[kFullHunterASym] = "Full Hunter (asym)";
}

//! Load all the models and write them to temporary text files
void TestBackend::testLoadModels()
{
    for (auto [key, value] : mFileNames.asKeyValueRange())
    {
        QString inPathFile = Utility::combineFilePath(EXAMPLES_DIR, value + ".dat");
        QString outPathFile = Utility::combineFilePath(TEMPORARY_DIR, value + ".txt");
        Model model(inPathFile.toStdString());
        model.write(outPathFile.toStdString());
        QVERIFY(!model.isEmpty());
        QString subprojectName = mSubprojectNames[key];
        Subproject subproject(subprojectName);
        subproject.model() = model;
        mProject.addSubproject(subproject);
    }
    QVERIFY(mProject.numSubprojects() == mFileNames.size());
};

//! Load the experimental obtained modal solutions
void TestBackend::testLoadModalSolution()
{
    Example const example = Example::kHunterWing;

    ModalSolution solution;
    solution.read(Utility::combineFilePath(EXAMPLES_DIR, mFileNames[example]));
    QVERIFY(solution.numModes() == 8);
}

//! Try to select elements
void TestBackend::testSelector()
{
    Example const example = Example::kSimpleWing;

    // Slice the subproject
    Subproject& subproject = mProject.subprojects()[example];

    // Select all the elements
    Selector selector;
    SelectionSet& set = selector.add(subproject.model(), "all");
    set.selectAll();
    QVERIFY(set.numSelected() == 84);

    // Reset the selection
    set.selectNone();
    QVERIFY(set.numSelected() == 0);

    // Select the first surface
    set.setSelected(0, true);
    QVERIFY(set.numSelected() == 44);

    // Select the bending beams associated with the second surface
    set.selectNone();
    set.setSelected(0, KCL::BI, true);
    QVERIFY(set.numSelected() == 13);

    // Remove the selections
    selector.clear();
    QVERIFY(selector.isEmpty());
}

//! Obtain the modal solution associated with the simple wing
void TestBackend::testModalSolverSimpleWing()
{
    testModalSolver(Example::kSimpleWing, 15);
}

//! Obtain the modal solution associated with the hunter wing
void TestBackend::testModalSolverHunterWing()
{
    testModalSolver(Example::kHunterWing, 30);
}

//! Obtain the modal solution associated with the full hunter using symmetrical spectrum
void TestBackend::testModalSolverFullHunterSym()
{
    testModalSolver(Example::kFullHunterSym, 30);
}

//! Obtain the modal solution associated with the full hunter using asymmetrical spectrum
void TestBackend::testModalSolverFullHunterASym()
{
    testModalSolver(Example::kFullHunterASym, 30);
}

//! Update the model of the simple wing
void TestBackend::testOptimSolverSimpleWing()
{
    Example const example = Example::kSimpleWing;
    int const numModes = 3;
    double const error = 0.01;

    // Slice the subproject
    Subproject& subproject = mProject.subprojects()[example];

    // Obtain the initial solution
    KCL::Model const& model = subproject.model();
    auto eigenSolution = model.solveEigen();

    // Initialize the solver
    OptimSolver* pSolver = (OptimSolver*) subproject.addSolver(ISolver::kOptim);

    // Alias the data
    OptimProblem& problem = pSolver->problem;
    OptimOptions& options = pSolver->options;
    Constraints& constraints = problem.constraints;

    // Set the model
    problem.model = model;

    // Select elements
    SelectionSet& set = problem.selector.add(model, "main");
    set.selectAll();
    set.setSelected(KCL::BI, true);
    set.setSelected(KCL::DB, true);
    set.setSelected(KCL::BK, true);
    set.setSelected(KCL::PR, true);

    // Set the options
    options.maxNumIterations = 32;
    options.diffStepSize = 1e-5;
    options.maxRelError = 1e-1;
    options.penaltyMAC = 0;
    options.numModes = 10;

    // Set the objectives
    problem.resize(numModes);
    problem.targetIndices.setLinSpaced(0, numModes - 1);
    for (int i = 0; i != numModes; ++i)
        problem.targetFrequencies[i] = eigenSolution.frequencies[problem.targetIndices[i]] * (1.0 + generateDouble({-error, error}));
    problem.targetWeights.setOnes();

    // Start the solver
    connect(pSolver, &OptimSolver::logAppended, [](QString message) { std::cout << message.toStdString() << std::endl; });
    pSolver->solve();
    QVERIFY(!pSolver->solutions.isEmpty());
    QVERIFY(pSolver->solutions.last().isSuccess);
}

void TestBackend::testFlutterSolverSimpleWing()
{
    FlutterOptions options;
    options.numModes = 10;
    options.flowStep = 5;
    options.numFlowSteps = 200;
    testFlutterSolver(Example::kSimpleWing, options);
}

//! Solve flutter problem for the hunter wing
void TestBackend::testFlutterSolverHunterWing()
{
    FlutterOptions options;
    options.numModes = 10;
    testFlutterSolver(Example::kHunterWing, options);
}

//! Solve flutter problem for the full hunter using symmetrical spectrum
void TestBackend::testFlutterSolverFullHunterSym()
{
    FlutterOptions options;
    options.numModes = 20;
    testFlutterSolver(Example::kFullHunterSym, options);
}

//! Solve flutter problem for the full hunter using asymmetrical spectrum
void TestBackend::testFlutterSolverFullHunterASym()
{
    FlutterOptions options;
    options.numModes = 20;
    testFlutterSolver(Example::kFullHunterASym, options);
}

//! Write a project consisted of several subprojects to a file
void TestBackend::testWriteProject()
{
    // Write the project to the file
    QString fileName = QString("tests.%1").arg(Project::fileSuffix());
    QString pathFile = Utility::combineFilePath(EXAMPLES_DIR, fileName);
    QVERIFY(mProject.write(pathFile));

    // Read the project from the file
    Project tProject;
    QVERIFY(tProject.read(pathFile));

    // Write the file to check
    fileName = QString("check.%1").arg(Project::fileSuffix());
    pathFile = Utility::combineFilePath(TEMPORARY_DIR, fileName);
    tProject.write(pathFile);

    // Compare the projects
    tProject.setPathFile(mProject.pathFile());
    QVERIFY(mProject == tProject);
}

//! Generate a bounded double value
double TestBackend::generateDouble(QPair<double, double> const& limits)
{
    double value = QRandomGenerator::global()->generateDouble();
    return limits.first + value * (limits.second - limits.first);
}

//! Helper function to obtaim modal solutions
void TestBackend::testModalSolver(Example example, int numModes)
{
    // Slice the subproject
    Subproject& subproject = mProject.subprojects()[example];

    // Initialize the solver
    ModalSolver* pSolver = (ModalSolver*) subproject.addSolver(ISolver::kModal);

    // Set the solver data
    pSolver->options.numModes = numModes;
    pSolver->model = subproject.model();

    // Run the solver
    pSolver->solve();
    QVERIFY(!pSolver->solution.isEmpty());
    QVERIFY(pSolver->solution.numModes() == numModes);
}

//! Helper function to obtain flutter solutions
void TestBackend::testFlutterSolver(Example example, FlutterOptions const& options)
{
    // Slice the subproject
    Subproject& subproject = mProject.subprojects()[example];

    // Initialize the solver
    FlutterSolver* pSolver = (FlutterSolver*) subproject.addSolver(ISolver::kFlutter);

    // Set the solver data
    pSolver->options = options;
    pSolver->model = subproject.model();

    // Run the solver
    pSolver->solve();
    QVERIFY(!pSolver->solution.isEmpty());
}

QTEST_MAIN(TestBackend)
