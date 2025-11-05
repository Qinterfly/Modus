#include <QVBoxLayout>

#include "optimoptionseditor.h"

using namespace Frontend;

OptimOptionsEditor::OptimOptionsEditor(Backend::Core::OptimOptions& options, QString const& name, QWidget* pParent)
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
        emit commandExecuted(new EditProperty<Backend::Core::OptimOptions>(mOptions, "maxNumIterations", value));
        break;
    case kNumThreads:
        emit commandExecuted(new EditProperty<Backend::Core::OptimOptions>(mOptions, "numThreads", value));
        break;
    case kNumModes:
        emit commandExecuted(new EditProperty<Backend::Core::OptimOptions>(mOptions, "numModes", value));
        break;
    }
}

//! Process changing of a double value
void OptimOptionsEditor::setDoubleValue(QtProperty* pProperty, double value)
{
    switch (mpEditor->id(pProperty))
    {
    case kTimeoutIteration:
        emit commandExecuted(new EditProperty<Backend::Core::OptimOptions>(mOptions, "timeoutIteration", value));
        break;
    case kDiffStepSize:
        emit commandExecuted(new EditProperty<Backend::Core::OptimOptions>(mOptions, "diffStepSize", value));
        break;
    case kMinMAC:
        emit commandExecuted(new EditProperty<Backend::Core::OptimOptions>(mOptions, "minMAC", value));
        break;
    case kPenaltyMAC:
        emit commandExecuted(new EditProperty<Backend::Core::OptimOptions>(mOptions, "penaltyMAC", value));
        break;
    case kMaxRelError:
        emit commandExecuted(new EditProperty<Backend::Core::OptimOptions>(mOptions, "maxRelError", value));
        break;
    }
}
