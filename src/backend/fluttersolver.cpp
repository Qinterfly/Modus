#include "fluttersolver.h"
#include "fileutility.h"
#include "mathutility.h"

using namespace Backend::Core;

FlutterOptions::FlutterOptions()
    : numModes(15)
    , timeout(10.0)
{
}

FlutterOptions::~FlutterOptions()
{
}

bool FlutterOptions::operator==(FlutterOptions const& another) const
{
    return Utility::areEqual(*this, another);
}
bool FlutterOptions::operator!=(FlutterOptions const& another) const
{
    return !(*this == another);
}

void FlutterOptions::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    Utility::serializeProperties(stream, "options", *this);
}

void FlutterOptions::deserialize(QXmlStreamReader& stream)
{
    Utility::deserializeProperties(stream, *this);
}

FlutterSolution::FlutterSolution()
{
}

FlutterSolution::FlutterSolution(KCL::FlutterSolution const& solution)
{
    mGeometry = solution.geometry;
    mFlow = solution.flow;
    mRoots = solution.roots;
    mCritFlow = solution.critFlow;
    mCritSpeed = solution.critSpeed;
    mCritFrequency = solution.critFrequency;
    mCritCircFrequency = solution.critCircFrequency;
    mCritStrouhal = solution.critStrouhal;
    mCritDamping = solution.critDamping;
    int numModes = solution.critModeShapes.size();
    mCritModeShapes.resize(numModes);
    for (int i = 0; i != numModes; ++i)
        mCritModeShapes[i] = solution.critModeShapes[i];
    mCritPartFactor = solution.critPartFactor;
    mCritPartPhase = solution.critPartPhase;
}

FlutterSolution::~FlutterSolution()
{
}

bool FlutterSolution::operator==(FlutterSolution const& another) const
{
    return Utility::areEqual(*this, another);
}

bool FlutterSolution::operator!=(FlutterSolution const& another) const
{
    return !(*this == another);
}

void FlutterSolution::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    mGeometry.serialize(stream, "geometry");
    Utility::serialize(stream, "flow", mFlow);
    Utility::serialize(stream, "roots", mRoots);
    Utility::serialize(stream, "critFlow", mCritFlow);
    Utility::serialize(stream, "critSpeed", mCritSpeed);
    Utility::serialize(stream, "critFrequency", mCritFrequency);
    Utility::serialize(stream, "critCircFrequency", mCritCircFrequency);
    Utility::serialize(stream, "critStrouhal", mCritStrouhal);
    Utility::serialize(stream, "critDamping", mCritDamping);
    Utility::serialize(stream, "critModeShapes", "critModeShape", mCritModeShapes);
    Utility::serialize(stream, "critPartFactor", mCritPartFactor);
    Utility::serialize(stream, "critPartPhase", mCritPartPhase);
    stream.writeEndElement();
}

void FlutterSolution::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "geometry")
            mGeometry.deserialize(stream);
        else if (stream.name() == "flow")
            Utility::deserialize(stream, mFlow);
        else if (stream.name() == "roots")
            Utility::deserialize(stream, mRoots);
        else if (stream.name() == "critFlow")
            Utility::deserialize(stream, mCritFlow);
        else if (stream.name() == "critSpeed")
            Utility::deserialize(stream, mCritSpeed);
        else if (stream.name() == "critFrequency")
            Utility::deserialize(stream, mCritFrequency);
        else if (stream.name() == "critCircFrequency")
            Utility::deserialize(stream, mCritCircFrequency);
        else if (stream.name() == "critStrouhal")
            Utility::deserialize(stream, mCritStrouhal);
        else if (stream.name() == "critDamping")
            Utility::deserialize(stream, mCritDamping);
        else if (stream.name() == "critModeShapes")
            Utility::deserialize(stream, "critModeShape", mCritModeShapes);
        else if (stream.name() == "critPartFactor")
            Utility::deserialize(stream, mCritPartFactor);
        else if (stream.name() == "critPartPhase")
            Utility::deserialize(stream, mCritPartPhase);
        else
            stream.skipCurrentElement();
    }
}

FlutterSolver::FlutterSolver()
{
}

FlutterSolver::~FlutterSolver()
{
}

FlutterSolver::FlutterSolver(FlutterSolver const& another)
    : options(another.options)
    , model(another.model)
    , solution(another.solution)
{
}

FlutterSolver::FlutterSolver(FlutterSolver&& another)
{
    mID = std::move(another.mID);
    options = std::move(another.options);
    model = std::move(another.model);
    solution = std::move(another.solution);
}

FlutterSolver& FlutterSolver::operator=(FlutterSolver const& another)
{
    options = another.options;
    model = another.model;
    solution = another.solution;
    return *this;
}

ISolver::Type FlutterSolver::type() const
{
    return ISolver::kFlutter;
}

ISolver* FlutterSolver::clone() const
{
    FlutterSolver* pSolver = new FlutterSolver;
    *pSolver = *this;
    return pSolver;
}

void FlutterSolver::clear()
{
    options = FlutterOptions();
    model = KCL::Model();
    solution = FlutterSolution();
}

void FlutterSolver::solve()
{
    // Copy the model
    KCL::Model currentModel = model;

    // Set the analysis parameters
    auto pParameters = (KCL::AnalysisParameters*) currentModel.specialSurface.element(KCL::WP);
    pParameters->numLowModes = options.numModes;

    // Create the auxiliary function
    std::function<KCL::FlutterSolution()> fun = [&currentModel]() { return currentModel.solveFlutter(); };

    // Run the solution
    solution = Utility::solve(fun, options.timeout);

    emit solverFinished();
}

void FlutterSolver::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    stream.writeAttribute("type", Utility::toString((int) type()));
    stream.writeTextElement("id", mID.toString());
    Utility::serialize(stream, "model", model);
    options.serialize(stream, "options");
    solution.serialize(stream, "solution");
    stream.writeEndElement();
}

void FlutterSolver::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "id")
            mID = QUuid::fromString(stream.readElementText());
        else if (stream.name() == "model")
            Utility::deserialize(stream, model);
        else if (stream.name() == "options")
            options.deserialize(stream);
        else if (stream.name() == "solution")
            solution.deserialize(stream);
        else
            stream.skipCurrentElement();
    }
}

bool FlutterSolver::operator==(ISolver const* pBaseSolver) const
{
    if (type() != pBaseSolver->type())
        return false;
    FlutterSolver* pSolver = (FlutterSolver*) pBaseSolver;
    return Utility::areEqual(*this, *pSolver);
}

bool FlutterSolver::operator!=(ISolver const* pBaseSolver) const
{
    return !(*this == pBaseSolver);
}
