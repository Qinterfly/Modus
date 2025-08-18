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

//! Load all the models and write them to temporary text files
void TestBackend::testLoadModels()
{
    QMap<ExampleModel, QString> fileNames;
    fileNames[ExampleModel::kSimpleWing] = "DATWEXA";
    fileNames[ExampleModel::kHunterWing] = "DATW70";
    fileNames[ExampleModel::kFullHunterSym] = "DATH70s";
    fileNames[ExampleModel::kFullHunterASym] = "DATH70a";
    for (auto [key, value] : fileNames.asKeyValueRange())
    {
        QString inPathFile = Utility::combineFilePath(EXAMPLES_DIR, value + ".dat");
        QString outPathFile = Utility::combineFilePath(TEMP_DIR, value + ".txt");
        Model model(inPathFile.toStdString());
        model.write(outPathFile.toStdString());
        QVERIFY(!model.isEmpty());
        mModels[key] = model;
    }
};

QTEST_MAIN(TestBackend)
