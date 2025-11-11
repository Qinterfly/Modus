
#ifndef TESTBACKEND_H
#define TESTBACKEND_H

#include <QTest>

#include "project.h"

namespace Backend::Core
{
struct FlutterOptions;
}

namespace Tests
{

enum Example
{
    kSimpleWing,
    kHunterWing,
    kFullHunterSym,
    kFullHunterASym
};

class TestBackend : public QObject
{
    Q_OBJECT

public:
    TestBackend();
    virtual ~TestBackend() = default;

private slots:
    // Models
    void testLoadModels();
    void testLoadModalSolution();
    void testSelector();

    // Modal solvers
    void testModalSolverSimpleWing();
    void testModalSolverHunterWing();
    void testModalSolverFullHunterSym();
    void testModalSolverFullHunterASym();

    // Optimization solvers
    void testOptimSolverSimpleWing();

    // Flutter solvers
    void testFlutterSolverSimpleWing();
    void testFlutterSolverHunterWing();
    void testFlutterSolverFullHunterSym();
    void testFlutterSolverFullHunterASym();

    // Project
    void testWriteProject();

private:
    double generateDouble(QPair<double, double> const& limits);
    void testModalSolver(Example example, int numModes);
    void testFlutterSolver(Example example, Backend::Core::FlutterOptions const& options);

private:
    Backend::Core::Project mProject;
    QMap<Example, QString> mFileNames;
    QMap<Example, QString> mSubprojectNames;
};

}

#endif // TESTBACKEND_H
