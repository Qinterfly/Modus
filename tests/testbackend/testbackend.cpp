#include <QRandomGenerator>

#include "config.h"
#include "fileutility.h"
#include "kclsubproject.h"
#include "mathutility.h"
#include "selector.h"
#include "testbackend.h"

using namespace Tests;
using namespace Backend;
using namespace Backend::Core;
using namespace KCL;

TestBackend::TestBackend()
{
    mProject.setName("Examples");
}

//! Load all the models and write them to temporary text files
void TestBackend::testLoadModels()
{
    QStringList const fileNames = {"DATWEXA", "DATW70", "DATH70s", "DATH70a"};
    QStringList const modelNames = {"SimpleWing", "HunterWing", "FullHunterSym", "FullHunterASym"};
    int numModels = modelNames.size();
    for (int i = 0; i != numModels; ++i)
    {
        QString const& fileName = fileNames[i];
        QString inPathFile = Utility::combineFilePath(EXAMPLES_DIR, fileName + ".dat");
        QString outPathFile = Utility::combineFilePath(TEMP_DIR, fileName + ".txt");
        Model model(inPathFile.toStdString());
        model.write(outPathFile.toStdString());
        QVERIFY(!model.isEmpty());
        KCLSubproject* pSubproject = new KCLSubproject(modelNames[i]);
        pSubproject->model() = model;
        mProject.addSubproject(pSubproject);
    }
    QVERIFY(mProject.numSubprojects() == fileNames.size());
};

//! Try to select model entities
void TestBackend::testSelector()
{
    QString const name = "SimpleWing";

    // Slice the subproject
    int iSubproject = Utility::getIndexByName(mProject.subprojects(), name);
    KCLSubproject* pSubproject = (KCLSubproject*) mProject.subprojects()[iSubproject];

    // Select all the elements
    Selector selector(&pSubproject->model());
    SelectionSet& set = selector.add();
    set.selectAll();
    QVERIFY(set.numSelected() == 78);

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
    int numModes = 3;
    double error = 0.05;
    QString const name = "SimpleWing";

    // Slice the subproject
    int iSubproject = Utility::getIndexByName(mProject.subprojects(), name);
    KCLSubproject* pSubproject = (KCLSubproject*) mProject.subprojects()[iSubproject];

    // Obtain the initial solution
    KCL::Model const& model = pSubproject->model();
    auto initSolution = model.solveEigen();

    // Select all the entities
    KCLConfiguration& config = pSubproject->configuration();
    OptimOptions& options = config.options;
    options.selector.add().selectAll();

    // Set the objectives
    int numDOFs = initSolution.geometry.vertices.rows();
    options.resize(numDOFs, numModes);
    for (int i = 0; i != numModes; ++i)
    {
        options.indices[i] = i;
        options.frequencies[i] = initSolution.frequencies[i] * (1 + generateDouble({-error / 2.0, error / 2.0}));
        options.modeShapes[i] = initSolution.modeShapes[i];
        options.weights[i] = 1.0;
    }

    // Start the solver
    OptimSolver solver;
    solver.solve(model, options);
}

//! Generate a bounded double value
double TestBackend::generateDouble(QPair<double, double> const& limits)
{
    double value = QRandomGenerator::global()->generateDouble();
    return limits.first + value * (limits.second - limits.first);
}

QTEST_MAIN(TestBackend)
