#include <QRandomGenerator>

#include "config.h"
#include "fileutility.h"
#include "magicenum/magic_enum.hpp"
#include "selector.h"
#include "subproject.h"
#include "testbackend.h"

using namespace Tests;
using namespace Backend;
using namespace Backend::Core;
using namespace KCL;

TestBackend::TestBackend()
{
    mFileNames[simpleWing] = "DATWEXA";
    mFileNames[hunterWing] = "DATW70";
    mFileNames[fullHunterSym] = "DATH70s";
    mFileNames[fullHunterASym] = "DATH70a";
}

//! Load all the models and write them to temporary text files
void TestBackend::testLoadModels()
{
    for (auto [key, value] : mFileNames.asKeyValueRange())
    {
        QString exampleName = magic_enum::enum_name(key).data();
        QString inPathFile = Utility::combineFilePath(EXAMPLES_DIR, value + ".dat");
        QString outPathFile = Utility::combineFilePath(TEMPORARY_DIR, value + ".txt");
        Model model(inPathFile.toStdString());
        model.write(outPathFile.toStdString());
        QVERIFY(!model.isEmpty());
        Subproject subproject(exampleName);
        subproject.model() = model;
        mProject.addSubproject(subproject);
    }
    QVERIFY(mProject.numSubprojects() == mFileNames.size());
};

//! Load the experimental obtained modal solutions
void TestBackend::testLoadModalSolution()
{
    Example const example = Example::hunterWing;

    // Slice the subproject
    Subproject& subproject = mProject.subprojects()[example];
    auto pSolver = (OptimSolver*) subproject.addSolver(ISolver::kOptim);
    ModalSolution& solution = pSolver->problem.targetSolution;

    // Read the geometry and modal data
    solution.read(Utility::combineFilePath(EXAMPLES_DIR, mFileNames[example]));
    QVERIFY(solution.numModes() == 8);
}

//! Try to select elements
void TestBackend::testSelector()
{
    Example const example = Example::simpleWing;

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
void TestBackend::testModalSolver()
{
    Example const example = Example::simpleWing;
    int const numModes = 15;

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

//! Update the model of the simple wing
void TestBackend::testOptimSolver()
{
    Example const example = Example::simpleWing;
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
    ModalSolution solution(eigenSolution);
    Eigen::VectorXd targetFrequencies = solution.frequencies();
    problem.resize(numModes);
    problem.targetIndices.setLinSpaced(0, numModes - 1);
    problem.targetWeights.setOnes();
    for (double& value : targetFrequencies)
        value *= 1.0 + generateDouble({-error, error});
    problem.targetSolution = ModalSolution(solution.geometry(), targetFrequencies, solution.modeShapes());
    problem.fillMatches();

    // Start the solver
    connect(pSolver, &OptimSolver::log, [](QString message) { std::cout << message.toStdString() << std::endl; });
    pSolver->solve();
    QVERIFY(!pSolver->solutions.isEmpty());
    QVERIFY(pSolver->solutions.last().isSuccess);
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

QTEST_MAIN(TestBackend)
