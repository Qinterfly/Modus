#include <QApplication>
#include <QVBoxLayout>

#include "fluttersolver.h"
#include "modalsolver.h"
#include "optimsolver.h"
#include "solveroptionseditor.h"

using namespace Backend;
using namespace Frontend;
using namespace Core;

ModalOptionsEditor::ModalOptionsEditor(ModalOptions& options, QString const& name, QWidget* pParent)
    : Editor(kModalOptions, name, QIcon(":/icons/options.png"), pParent)
    , mOptions(options)
{
    createContent();
    createProperties();
    createConnections();
}

QSize ModalOptionsEditor::sizeHint() const
{
    return QSize(600, 400);
}

//! Update the editor state
void ModalOptionsEditor::refresh()
{
    mpEditor->clear();
    createProperties();
}

//! Create all the widgets
void ModalOptionsEditor::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;
    mpEditor = new CustomPropertyEditor;
    pLayout->addWidget(mpEditor);
    setLayout(pLayout);
}

//! Create interactions between widgets
void ModalOptionsEditor::createConnections()
{
    connect(mpEditor, &CustomPropertyEditor::intValueChanged, this, &ModalOptionsEditor::setIntValue);
    connect(mpEditor, &CustomPropertyEditor::doubleValueChanged, this, &ModalOptionsEditor::setDoubleValue);
}

//! Create the properties to view and edit
void ModalOptionsEditor::createProperties()
{
    mpEditor->createIntProperty(kNumModes, tr("Number of modes"), mOptions.numModes, 1);
    mpEditor->createDoubleProperty(kTimeout, tr("Timeout"), mOptions.timeout, 0.0);
}

//! Process changing of an integer value
void ModalOptionsEditor::setIntValue(QtProperty* pProperty, int value)
{
    if (mpEditor->id(pProperty) == kNumModes)
        emit commandExecuted(new EditProperty<ModalOptions>(mOptions, "numModes", value, this));
}

//! Process changing of a double value
void ModalOptionsEditor::setDoubleValue(QtProperty* pProperty, double value)
{
    if (mpEditor->id(pProperty) == kTimeout)
        emit commandExecuted(new EditProperty<ModalOptions>(mOptions, "timeout", value, this));
}

FlutterOptionsEditor::FlutterOptionsEditor(FlutterOptions& options, QString const& name, QWidget* pParent)
    : Editor(kFlutterOptions, name, QIcon(":/icons/options.png"), pParent)
    , mOptions(options)
{
    createContent();
    createProperties();
    createConnections();
}

QSize FlutterOptionsEditor::sizeHint() const
{
    return QSize(600, 400);
}

//! Update the editor state
void FlutterOptionsEditor::refresh()
{
    mpEditor->clear();
    createProperties();
}

//! Create all the widgets
void FlutterOptionsEditor::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;
    mpEditor = new CustomPropertyEditor;
    pLayout->addWidget(mpEditor);
    setLayout(pLayout);
}

//! Create interactions between widgets
void FlutterOptionsEditor::createConnections()
{
    connect(mpEditor, &CustomPropertyEditor::intValueChanged, this, &FlutterOptionsEditor::setIntValue);
    connect(mpEditor, &CustomPropertyEditor::doubleValueChanged, this, &FlutterOptionsEditor::setDoubleValue);
}

//! Create the properties to view and edit
void FlutterOptionsEditor::createProperties()
{
    mpEditor->createIntProperty(kNumModes, tr("Number of modes"), mOptions.numModes, 1);
    mpEditor->createDoubleProperty(kTimeout, tr("Timeout"), mOptions.timeout, 0.0);
}

//! Process changing of an integer value
void FlutterOptionsEditor::setIntValue(QtProperty* pProperty, int value)
{
    if (mpEditor->id(pProperty) == kNumModes)
        emit commandExecuted(new EditProperty<FlutterOptions>(mOptions, "numModes", value, this));
}

//! Process changing of a double value
void FlutterOptionsEditor::setDoubleValue(QtProperty* pProperty, double value)
{
    if (mpEditor->id(pProperty) == kTimeout)
        emit commandExecuted(new EditProperty<FlutterOptions>(mOptions, "timeout", value, this));
}

OptimOptionsEditor::OptimOptionsEditor(OptimOptions& options, QString const& name, QWidget* pParent)
    : Editor(kOptimOptions, name, QIcon(":/icons/options.png"), pParent)
    , mOptions(options)
{
    createContent();
    createProperties();
    createConnections();
}

QSize OptimOptionsEditor::sizeHint() const
{
    return QSize(600, 400);
}

//! Update the editor state
void OptimOptionsEditor::refresh()
{
    mpEditor->clear();
    createProperties();
}

//! Create all the widgets
void OptimOptionsEditor::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;
    mpEditor = new CustomPropertyEditor;
    pLayout->addWidget(mpEditor);
    setLayout(pLayout);
}

//! Create interactions between widgets
void OptimOptionsEditor::createConnections()
{
    connect(mpEditor, &CustomPropertyEditor::intValueChanged, this, &OptimOptionsEditor::setIntValue);
    connect(mpEditor, &CustomPropertyEditor::doubleValueChanged, this, &OptimOptionsEditor::setDoubleValue);
}

//! Create the properties to view and edit
void OptimOptionsEditor::createProperties()
{
    mpEditor->createIntProperty(kMaxNumIterations, tr("Maximum number of iterations"), mOptions.maxNumIterations, 1);
    mpEditor->createDoubleProperty(kTimeoutIteration, tr("Timeout iteration"), mOptions.timeoutIteration, 0.0);
    mpEditor->createIntProperty(kNumThreads, tr("Number of threads"), mOptions.numThreads, 1);
    mpEditor->createDoubleProperty(kDiffStepSize, tr("Differentiation step size"), mOptions.diffStepSize, 1e-12, 1.0, 6);
    mpEditor->createDoubleProperty(kMinMAC, tr("Minimum MAC"), mOptions.minMAC, 0.0, 1.0);
    mpEditor->createDoubleProperty(kPenaltyMAC, tr("Penalty MAC"), mOptions.penaltyMAC, 0.0);
    mpEditor->createDoubleProperty(kMaxRelError, tr("Maximum relative error"), mOptions.maxRelError, 0.0, 1, 5);
    mpEditor->createIntProperty(kNumModes, tr("Number of modes"), mOptions.numModes, 1);
}

//! Process changing of an integer value
void OptimOptionsEditor::setIntValue(QtProperty* pProperty, int value)
{
    switch (mpEditor->id(pProperty))
    {
    case kMaxNumIterations:
        emit commandExecuted(new EditProperty<OptimOptions>(mOptions, "maxNumIterations", value, this));
        break;
    case kNumThreads:
        emit commandExecuted(new EditProperty<OptimOptions>(mOptions, "numThreads", value, this));
        break;
    case kNumModes:
        emit commandExecuted(new EditProperty<OptimOptions>(mOptions, "numModes", value, this));
        break;
    }
}

//! Process changing of a double value
void OptimOptionsEditor::setDoubleValue(QtProperty* pProperty, double value)
{
    switch (mpEditor->id(pProperty))
    {
    case kTimeoutIteration:
        emit commandExecuted(new EditProperty<OptimOptions>(mOptions, "timeoutIteration", value, this));
        break;
    case kDiffStepSize:
        emit commandExecuted(new EditProperty<OptimOptions>(mOptions, "diffStepSize", value, this));
        break;
    case kMinMAC:
        emit commandExecuted(new EditProperty<OptimOptions>(mOptions, "minMAC", value, this));
        break;
    case kPenaltyMAC:
        emit commandExecuted(new EditProperty<OptimOptions>(mOptions, "penaltyMAC", value, this));
        break;
    case kMaxRelError:
        emit commandExecuted(new EditProperty<OptimOptions>(mOptions, "maxRelError", value, this));
        break;
    }
}
