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
    mProject.setName("Examples");
    mExampleFileNames[simpleWing] = "DATWEXA";
    mExampleFileNames[hunterWing] = "DATW70";
    mExampleFileNames[fullHunterSym] = "DATH70s";
    mExampleFileNames[fullHunterASym] = "DATH70a";
}

//! Load all the models and write them to temporary text files
void TestBackend::testLoadModels()
{
    for (auto [key, value] : mExampleFileNames.asKeyValueRange())
    {
        QString exampleName = magic_enum::enum_name(key).data();
        QString inPathFile = Utility::combineFilePath(EXAMPLES_DIR, value + ".dat");
        QString outPathFile = Utility::combineFilePath(TEMP_DIR, value + ".txt");
        Model model(inPathFile.toStdString());
        model.write(outPathFile.toStdString());
        QVERIFY(!model.isEmpty());
        Subproject subproject(exampleName);
        subproject.model() = model;
        mProject.addSubproject(subproject);
    }
    QVERIFY(mProject.numSubprojects() == mExampleFileNames.size());
};

//! Load the experimental obtained modal solutions
void TestBackend::testLoadModalSolution()
{
    Example const example = Example::hunterWing;

    // Slice the subproject
    Subproject& subproject = mProject.subprojects()[example];
    ModalSolution& solution = subproject.configuration().problem.targetSolution;

    // Read the geometry and modal data
    solution.read(Utility::combineFilePath(EXAMPLES_DIR, mExampleFileNames[example]));
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

//! Update the model of the simple wing
void TestBackend::testUpdateSimpleWing()
{
    Example const example = Example::simpleWing;
    int numModes = 3;
    double error = 0.01;

    // Slice the subproject
    Subproject& subproject = mProject.subprojects()[example];

    // Obtain the initial solution
    KCL::Model const& model = subproject.model();
    auto eigenSolution = model.solveEigen();

    // Select elements
    Configuration& config = subproject.configuration();
    OptimProblem& problem = config.problem;
    Constraints& constraints = problem.constraints;
    problem.model = model;
    SelectionSet& set = problem.selector.add(model, "main");
    set.selectAll();
    set.setSelected(KCL::BI, true);
    set.setSelected(KCL::DB, true);
    set.setSelected(KCL::BK, true);
    set.setSelected(KCL::PR, true);

    // Set the options
    config.options.diffStepSize = 1e-5;
    config.options.maxRelError = 1e-1;
    config.options.penaltyMAC = 0;

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
    OptimOptions& options = config.options;
    OptimSolver solver;
    connect(&solver, &OptimSolver::log, [](QString message) { std::cout << message.toStdString() << std::endl; });
    auto solutions = solver.solve(problem, options);
}

//! Generate a bounded double value
double TestBackend::generateDouble(QPair<double, double> const& limits)
{
    double value = QRandomGenerator::global()->generateDouble();
    return limits.first + value * (limits.second - limits.first);
}

QTEST_MAIN(TestBackend)
