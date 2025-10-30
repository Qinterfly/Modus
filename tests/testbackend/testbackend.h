
#ifndef TESTBACKEND_H
#define TESTBACKEND_H

#include <QTest>

#include "project.h"

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
    void testFlutterSolverHunterWing();

    // Project
    void testWriteProject();

private:
    double generateDouble(QPair<double, double> const& limits);
    void testModalSolver(Example example, int numModes);

private:
    Backend::Core::Project mProject;
    QMap<Example, QString> mFileNames;
    QMap<Example, QString> mSubprojectNames;
};

}

#endif // TESTBACKEND_H
