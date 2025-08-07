
#ifndef TESTBACKEND_H
#define TESTBACKEND_H

#include <kcl/model.h>
#include <QTest>

namespace Tests
{

enum class ExampleModel
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
    void testLoadModels();

private:
    QMap<ExampleModel, KCL::Model> mModels;
};

}

#endif // TESTBACKEND_H
