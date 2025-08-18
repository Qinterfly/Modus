
#ifndef TESTBACKEND_H
#define TESTBACKEND_H

#include <QTest>

#include "project.h"

namespace Tests
{

class TestBackend : public QObject
{
    Q_OBJECT

public:
    TestBackend();
    virtual ~TestBackend() = default;

private slots:
    void testLoadModels();
    void testSelector();
    void testUpdateSimpleWing();

private:
    double generateDouble(QPair<double, double> const& limits);

private:
    Backend::Core::Project mProject;
};

}

#endif // TESTBACKEND_H
