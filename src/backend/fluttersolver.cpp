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
    geometry = solution.geometry;
    flow = solution.flow;
    roots = solution.roots;
    critFlow = solution.critFlow;
    critSpeed = solution.critSpeed;
    critFrequency = solution.critFrequency;
    critCircFrequency = solution.critCircFrequency;
    critStrouhal = solution.critStrouhal;
    critDamping = solution.critDamping;
    int numModes = solution.critModeShapes.size();
    critModeShapes.resize(numModes);
    for (int i = 0; i != numModes; ++i)
        critModeShapes[i] = solution.critModeShapes[i];
    critPartFactor = solution.critPartFactor;
    critPartPhase = solution.critPartPhase;
}

FlutterSolution::~FlutterSolution()
{
}

bool FlutterSolution::isEmpty() const
{
    return flow.size() == 0;
}

int FlutterSolution::numCrit() const
{
    return critFlow.size();
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
    geometry.serialize(stream, "geometry");
    Utility::serialize(stream, "flow", flow);
    Utility::serialize(stream, "roots", roots);
    Utility::serialize(stream, "critFlow", critFlow);
    Utility::serialize(stream, "critSpeed", critSpeed);
    Utility::serialize(stream, "critFrequency", critFrequency);
    Utility::serialize(stream, "critCircFrequency", critCircFrequency);
    Utility::serialize(stream, "critStrouhal", critStrouhal);
    Utility::serialize(stream, "critDamping", critDamping);
    Utility::serialize(stream, "critModeShapes", "critModeShape", critModeShapes);
    Utility::serialize(stream, "critPartFactor", critPartFactor);
    Utility::serialize(stream, "critPartPhase", critPartPhase);
    stream.writeEndElement();
}

void FlutterSolution::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "geometry")
            geometry.deserialize(stream);
        else if (stream.name() == "flow")
            Utility::deserialize(stream, flow);
        else if (stream.name() == "roots")
            Utility::deserialize(stream, roots);
        else if (stream.name() == "critFlow")
            Utility::deserialize(stream, critFlow);
        else if (stream.name() == "critSpeed")
            Utility::deserialize(stream, critSpeed);
        else if (stream.name() == "critFrequency")
            Utility::deserialize(stream, critFrequency);
        else if (stream.name() == "critCircFrequency")
            Utility::deserialize(stream, critCircFrequency);
        else if (stream.name() == "critStrouhal")
            Utility::deserialize(stream, critStrouhal);
        else if (stream.name() == "critDamping")
            Utility::deserialize(stream, critDamping);
        else if (stream.name() == "critModeShapes")
            Utility::deserialize(stream, "critModeShape", critModeShapes);
        else if (stream.name() == "critPartFactor")
            Utility::deserialize(stream, critPartFactor);
        else if (stream.name() == "critPartPhase")
            Utility::deserialize(stream, critPartPhase);
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
    stream.writeTextElement("name", name);
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
        else if (stream.name() == "name")
            name = stream.readElementText();
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
