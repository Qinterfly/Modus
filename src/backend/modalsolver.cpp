#include <ostream>
#include <QObject>

#include "constants.h"
#include "fileutility.h"
#include "mathutility.h"
#include "modalsolver.h"

using namespace Backend;
using namespace Backend::Core;
using namespace Eigen;

double const skDummy = std::nan("");

ModalSolution::ModalSolution()
{
}

ModalSolution::ModalSolution(Geometry const& anotherGeometry, Eigen::VectorXd const& anotherFrequencies,
                             QList<Eigen::MatrixXd> const& anotherModeShapes)
    : geometry(anotherGeometry)
    , frequencies(anotherFrequencies)
    , modeShapes(anotherModeShapes)
{
}

ModalSolution::ModalSolution(KCL::EigenSolution const& solution)
{
    // Acquire the dimensions
    int numModes = solution.frequencies.size();
    int numDOFs = solution.geometry.vertices.rows();

    // Allocate the data
    resize(numDOFs, numModes);

    // Copy the geometry
    geometry = solution.geometry;

    // Copy modal data
    for (int i = 0; i != numModes; ++i)
    {
        frequencies[i] = solution.frequencies[i];
        modeShapes[i] = solution.modeShapes[i];
    }
}

ModalSolution::~ModalSolution()
{
}

bool ModalSolution::isEmpty() const
{
    return numModes() == 0;
}

int ModalSolution::numModes() const
{
    return frequencies.size();
}

//! Compare the modal and eigen solutions
ModalComparison ModalSolution::compare(ModalSolution const& another, VectorXi const& indices, Matches const& matches, double minMAC) const
{
    ModalComparison result;

    // Compute MAC table
    int numBaseModes = indices.size();
    int numCompareModes = another.numModes();
    MatrixXd tableMAC(numBaseModes, numCompareModes);
    for (int i = 0; i != numBaseModes; ++i)
    {
        int iBaseMode = indices[i];
        MatrixXd baseModeShape = modeShapes[iBaseMode];
        for (int j = 0; j != numCompareModes; ++j)
        {
            int iCompareMode = j;
            MatrixXd compareModeShape = another.modeShapes[iCompareMode];
            tableMAC(i, j) = Utility::computeMAC(baseModeShape, compareModeShape, matches);
        }
    }

    // Pair the modeshapes
    result.resize(numBaseModes);
    result.pairs = Utility::pairByMAC(tableMAC, minMAC);

    // Compute the errors
    for (int i = 0; i != numBaseModes; ++i)
    {
        int iBaseMode = indices[i];
        int iCompareMode = result.pairs[i].first;
        if (iCompareMode >= 0)
        {
            double baseFrequency = frequencies[iBaseMode];
            if (std::abs(baseFrequency) < std::numeric_limits<double>::epsilon())
                continue;
            double compareFrequency = another.frequencies[iCompareMode];
            result.diffFrequencies[i] = compareFrequency - baseFrequency;
            result.errorFrequencies[i] = result.diffFrequencies[i] / baseFrequency;
            result.errorsMAC[i] = 1.0 - result.pairs[i].second;
        }
    }

    return result;
}

//! Read the files with geometry and modal solution located in the same directory
void ModalSolution::read(QDir const& directory)
{
    geometry.read(Utility::combineFilePath(directory.path(), "model.txt"));
    readModesets(Utility::combineFilePath(directory.path(), "modesets.txt"));
}

bool ModalSolution::operator==(ModalSolution const& another) const
{
    return Utility::areEqual(*this, another);
}

bool ModalSolution::operator!=(ModalSolution const& another) const
{
    return !(*this == another);
}

void ModalSolution::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    geometry.serialize(stream, "geometry");
    Utility::serialize(stream, "frequencies", frequencies);
    Utility::serialize(stream, "modeShapes", "modeShape", modeShapes);
    Utility::serialize(stream, "names", "name", names);
    stream.writeEndElement();
}

void ModalSolution::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "geometry")
            geometry.deserialize(stream);
        else if (stream.name() == "frequencies")
            Utility::deserialize(stream, frequencies);
        else if (stream.name() == "modeShapes")
            Utility::deserialize(stream, "modeShape", modeShapes);
        else if (stream.name() == "names")
            Utility::deserialize(stream, "name", names);
        else
            stream.skipCurrentElement();
    }
}

//! Reallocate the data fields
void ModalSolution::resize(int numDOFs, int numModes)
{
    geometry.vertices.resize(numDOFs);
    frequencies.resize(numModes);
    modeShapes.resize(numModes);
    for (MatrixXd& item : modeShapes)
        item.resize(numDOFs, Constants::skNumDirections);
}

//! Read the file which contains several modesets
void ModalSolution::readModesets(QString const& pathFile)
{
    QMap<QString, Direction> const kMapDirections = {{"X", Direction::kX}, {"Y", Direction::kY}, {"Z", Direction::kZ}};

    QFile file(pathFile);

    // Check if the file exists
    if (!file.exists())
    {
        qWarning() << QObject::tr("The file %1 is not found").arg(pathFile);
        return;
    }

    // Open the file for reading
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << QObject::tr("Could not read the modesets from the file: %1").arg(pathFile);
        return;
    }
    QTextStream stream(&file);

    // Map the vertices
    int numVertices = geometry.vertices.size();
    QMap<QString, int> mapVertices;
    for (int i = 0; i != numVertices; ++i)
        mapVertices[geometry.vertices[i].name] = i;

    // Retrieve the number of modesets
    int numModes;
    stream >> numModes;

    // Loop through all the modes
    frequencies.resize(numModes);
    modeShapes.resize(numModes);
    names.resize(numModes);
    for (int iMode = 0; iMode != numModes; ++iMode)
    {
        // Read the header
        stream.readLine();
        names[iMode] = stream.readLine();
        stream >> frequencies[iMode];
        int numDOFs;
        stream >> numDOFs;

        // Read the modeshape data
        MatrixXd& modeShape = modeShapes[iMode];
        modeShape.resize(numVertices, Constants::skNumDirections);
        modeShape.fill(skDummy);
        for (int iDOF = 0; iDOF != numDOFs; ++iDOF)
        {
            // Parse the vertex name
            QString fullName;
            stream >> fullName;
            int offset = fullName.lastIndexOf(":");
            QString vertexName = fullName.left(offset);

            // Parse the direction
            QString fullDirName = fullName.mid(offset + 1);
            QString dirSignName = fullDirName.left(1);
            int sign = 1;
            if (dirSignName == '-')
                sign = -1;
            QString dirName = fullDirName.right(1);
            Direction direction = kMapDirections[dirName];
            int iDir = (int) direction;

            // Read the value
            double value;
            stream >> value;
            if (mapVertices.contains(vertexName))
            {
                int iVertex = mapVertices[vertexName];
                modeShape(iVertex, iDir) = sign * value;
            }
        }
    }
}

ModalComparison::ModalComparison()
{
}

ModalComparison::~ModalComparison()
{
}

bool ModalComparison::isEmpty() const
{
    return errorFrequencies.size() == 0;
}

bool ModalComparison::isValid() const
{
    int numModes = errorFrequencies.size();
    for (int i = 0; i != numModes; ++i)
    {
        if (std::isnan(diffFrequencies[i]) || std::isnan(errorFrequencies[i]) || std::isnan(errorsMAC[i]))
            return false;
    }
    return true;
}

//! Allocate the modal
void ModalComparison::resize(int numModes)
{
    // Allocate
    diffFrequencies.resize(numModes);
    errorFrequencies.resize(numModes);
    errorsMAC.resize(numModes);
    pairs.resize(numModes);
    // Initialize
    diffFrequencies.fill(skDummy);
    errorFrequencies.fill(skDummy);
    errorsMAC.fill(skDummy);
    pairs.fill({-1, skDummy});
}

bool ModalComparison::operator==(ModalComparison const& another) const
{
    return Utility::areEqual(*this, another);
}

bool ModalComparison::operator!=(ModalComparison const& another) const
{
    return !(*this == another);
}

void ModalComparison::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    Utility::serialize(stream, "diffFrequencies", diffFrequencies);
    Utility::serialize(stream, "errorFrequencies", errorFrequencies);
    Utility::serialize(stream, "errorsMAC", errorsMAC);
    Utility::serialize(stream, "pairs", pairs);
    stream.writeEndElement();
}

void ModalComparison::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "diffFrequencies")
            Utility::deserialize(stream, diffFrequencies);
        else if (stream.name() == "errorFrequencies")
            Utility::deserialize(stream, errorFrequencies);
        else if (stream.name() == "errorsMAC")
            Utility::deserialize(stream, errorsMAC);
        else if (stream.name() == "pairs")
            Utility::deserialize(stream, pairs);
        else
            stream.skipCurrentElement();
    }
}

ModalOptions::ModalOptions()
    : numModes(20)
    , timeout(10.0)
{
}

ModalOptions::~ModalOptions()
{
}

bool ModalOptions::operator==(ModalOptions const& another) const
{
    return Utility::areEqual(*this, another);
}

bool ModalOptions::operator!=(ModalOptions const& another) const
{
    return !(*this == another);
}

void ModalOptions::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    Utility::serializeProperties(stream, elementName, *this);
}

void ModalOptions::deserialize(QXmlStreamReader& stream)
{
    Utility::deserializeProperties(stream, *this);
}

ModalSolver::ModalSolver()
{
}

ModalSolver::~ModalSolver()
{
}

ModalSolver::ModalSolver(ModalSolver const& another)
    : options(another.options)
    , solution(another.solution)
{
}

ModalSolver::ModalSolver(ModalSolver&& another)
{
    options = std::move(another.options);
    solution = std::move(another.solution);
}

ModalSolver& ModalSolver::operator=(ModalSolver const& another)
{
    options = another.options;
    solution = another.solution;
    return *this;
}

ISolver::Type ModalSolver::type() const
{
    return ISolver::kModal;
}

ISolver* ModalSolver::clone() const
{
    ModalSolver* pSolver = new ModalSolver;
    *pSolver = *this;
    return pSolver;
}

void ModalSolver::clear()
{
    options = ModalOptions();
    solution = ModalSolution();
    log = QString();
}

void ModalSolver::solve()
{
    // Copy the model
    KCL::Model currentModel = model;

    // Set the analysis parameters
    auto pParameters = (KCL::AnalysisParameters*) currentModel.specialSurface.element(KCL::WP);
    pParameters->numLowModes = options.numModes;

    // Create the auxiliary function
    std::ostringstream stream;
    std::function<KCL::EigenSolution()> fun = [&currentModel, &stream]() { return currentModel.solveEigen(stream); };

    // Run the solution
    solution = Utility::solve(fun, options.timeout);
    appendLog(stream.str().data());

    emit solverFinished();
}

void ModalSolver::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    stream.writeAttribute("type", Utility::toString((int) type()));
    stream.writeTextElement("id", mID.toString());
    stream.writeTextElement("name", name);
    Utility::serialize(stream, "model", model);
    options.serialize(stream, "options");
    solution.serialize(stream, "solution");
    Utility::serialize(stream, "log", log);
    stream.writeEndElement();
}

void ModalSolver::deserialize(QXmlStreamReader& stream)
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
        else if (stream.name() == "log")
            Utility::deserialize(stream, log);
        else
            stream.skipCurrentElement();
    }
}

bool ModalSolver::operator==(ISolver const* pBaseSolver) const
{
    if (type() != pBaseSolver->type())
        return false;
    auto pSolver = (ModalSolver*) pBaseSolver;
    return Utility::areEqual(*this, *pSolver);
}

bool ModalSolver::operator!=(ISolver const* pBaseSolver) const
{
    return !(this == pBaseSolver);
}

void ModalSolver::appendLog(QString const& message)
{
    Utility::appendLog(log, message);
    emit logAppended(message);
}
