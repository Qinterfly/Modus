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

ModalSolution::ModalSolution(Geometry const& geometry, Eigen::VectorXd const& frequencies, QList<Eigen::MatrixXd> const& modeShapes)
    : mGeometry(geometry)
    , mFrequencies(frequencies)
    , mModeShapes(modeShapes)
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
    for (int i = 0; i != numDOFs; ++i)
    {
        Vertex& vertex = mGeometry.vertices[i];
        vertex.name = QString::number(i);
        for (int j = 0; j != Constants::skNumDirections; ++j)
            vertex.position[j] = solution.geometry.vertices(i, j);
    }
    mGeometry.quadrangles = solution.geometry.quadrangles;

    // Copy modal data
    for (int i = 0; i != numModes; ++i)
    {
        mFrequencies[i] = solution.frequencies[i];
        mModeShapes[i] = solution.modeShapes[i];
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
    return mFrequencies.size();
}

Geometry const& ModalSolution::geometry() const
{
    return mGeometry;
}

Eigen::VectorXd const& ModalSolution::frequencies() const
{
    return mFrequencies;
}

QList<Eigen::MatrixXd> const& ModalSolution::modeShapes() const
{
    return mModeShapes;
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
        MatrixXd baseModeShape = mModeShapes[iBaseMode];
        for (int j = 0; j != numCompareModes; ++j)
        {
            int iCompareMode = j;
            MatrixXd compareModeShape = another.mModeShapes[iCompareMode];
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
            double baseFrequency = mFrequencies[iBaseMode];
            if (std::abs(baseFrequency) < std::numeric_limits<double>::epsilon())
                continue;
            double compareFrequency = another.mFrequencies[iCompareMode];
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
    mGeometry.read(Utility::combineFilePath(directory.path(), "model.txt"));
    readModesets(Utility::combineFilePath(directory.path(), "modesets.txt"));
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
    int numVertices = mGeometry.vertices.size();
    QMap<QString, int> mapVertices;
    for (int i = 0; i != numVertices; ++i)
        mapVertices[mGeometry.vertices[i].name] = i;

    // Retrieve the number of modesets
    int numModes;
    stream >> numModes;

    // Loop through all the modes
    mFrequencies.resize(numModes);
    mModeShapes.resize(numModes);
    mNames.resize(numModes);
    for (int iMode = 0; iMode != numModes; ++iMode)
    {
        // Read the header
        stream.readLine();
        mNames[iMode] = stream.readLine();
        stream >> mFrequencies[iMode];
        int numDOFs;
        stream >> numDOFs;

        // Read the modeshape data
        MatrixXd& modeShape = mModeShapes[iMode];
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
    mGeometry.serialize(stream, "geometry");
    Utility::serialize(stream, "frequencies", mFrequencies);
    Utility::serialize(stream, "modeShapes", "modeShape", mModeShapes);
    Utility::serialize(stream, "names", "name", mNames);
    stream.writeEndElement();
}

void ModalSolution::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "geometry")
            mGeometry.deserialize(stream);
        else if (stream.name() == "frequencies")
            Utility::deserialize(stream, mFrequencies);
        else if (stream.name() == "modeShapes")
            Utility::deserialize(stream, "modeShape", mModeShapes);
        else if (stream.name() == "names")
            Utility::deserialize(stream, "name", mNames);
        else
            stream.skipCurrentElement();
    }
}

//! Reallocate the data fields
void ModalSolution::resize(int numDOFs, int numModes)
{
    mGeometry.vertices.resize(numDOFs);
    mFrequencies.resize(numModes);
    mModeShapes.resize(numModes);
    for (MatrixXd& item : mModeShapes)
        item.resize(numDOFs, Constants::skNumDirections);
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
}

void ModalSolver::solve()
{
    // Copy the model
    KCL::Model currentModel = model;

    // Set the analysis parameters
    auto pParameters = (KCL::AnalysisParameters*) currentModel.specialSurface.element(KCL::WP);
    pParameters->numLowModes = options.numModes;

    // Create the auxiliary function
    std::function<KCL::EigenSolution()> fun = [&currentModel]() { return currentModel.solveEigen(); };

    // Run the solution
    solution = Utility::solve(fun, options.timeout);

    emit solverFinished();
}

void ModalSolver::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    stream.writeAttribute("type", Utility::toString((int) type()));
    stream.writeTextElement("id", mID.toString());
    Utility::serialize(stream, "model", model);
    options.serialize(stream, "options");
    solution.serialize(stream, "solution");
    stream.writeEndElement();
}

void ModalSolver::deserialize(QXmlStreamReader& stream)
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

bool ModalSolver::operator==(ISolver const* pBaseSolver) const
{
    if (type() != pBaseSolver->type())
        return false;
    ModalSolver* pSolver = (ModalSolver*) pBaseSolver;
    return Utility::areEqual(*this, *pSolver);
}

bool ModalSolver::operator!=(ISolver const* pBaseSolver) const
{
    return !(this == pBaseSolver);
}
