
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

    // Solvers
    void testModalSolver();
    void testOptimSolver();
    void testFlutterSolver();

    // Project
    void testWriteProject();

private:
    double generateDouble(QPair<double, double> const& limits);

private:
    Backend::Core::Project mProject;
    QMap<Example, QString> mFileNames;
    QMap<Example, QString> mSubprojectNames;
};

}

#endif // TESTBACKEND_H
