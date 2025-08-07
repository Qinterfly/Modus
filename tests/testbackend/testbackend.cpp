#include "testbackend.h"
#include "config.h"
#include "fileutility.h"
#include "selector.h"

using namespace Tests;
using namespace Backend;
using namespace KCL;

TestBackend::TestBackend()
{

}

//! Load all the models
void TestBackend::testLoadModels()
{
    QMap<ExampleModel, QString> fileNames;
    fileNames[ExampleModel::kSimpleWing] = "DATWEXA.dat";
    fileNames[ExampleModel::kHunterWing] = "DATW70.dat";
    fileNames[ExampleModel::kFullHunterSym] = "DATH70s.dat";
    fileNames[ExampleModel::kFullHunterASym] = "DATH70a.dat";
    for (auto [key, value] : fileNames.asKeyValueRange())
    {
        QString pathFile = Utility::combineFilePath(EXAMPLES_DIR, value);
        Model model(pathFile.toStdString());
        QVERIFY(!model.isEmpty());
        mModels[key] = model;
    }
};

QTEST_MAIN(TestBackend)
