#include <Eigen/Geometry>
#include <QObject>

#include "constants.h"
#include "fileutility.h"
#include "mathutility.h"
#include "modalsolution.h"

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

int ModalSolution::numVertices() const
{
    return mGeometry.vertices.size();
}

int ModalSolution::numModes() const
{
    return mFrequencies.size();
}

bool ModalSolution::isEmpty() const
{
    return mFrequencies.size() == 0 && mModeShapes.size() == 0;
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
    readGeometry(Utility::combineFilePath(directory.path(), "model.txt"));
    readModesets(Utility::combineFilePath(directory.path(), "modesets.txt"));
}

//! Read the geometry file
void ModalSolution::readGeometry(QString const& pathFile)
{
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
        qWarning() << QObject::tr("Could not read the model geometry from the file: %1").arg(pathFile);
        return;
    }
    QTextStream stream(&file);
    Geometry result;

    // Read the vertices
    QList<Vertex>& vertices = result.vertices;
    QMap<QString, int> mapVertices;
    int numVertices;
    stream >> numVertices;
    vertices.resize(numVertices);
    for (int i = 0; i != numVertices; ++i)
    {
        Vertex& vertex = vertices[i];
        stream >> vertex.name;
        mapVertices[vertex.name] = i;
        for (int j = 0; j != Constants::skNumDirections; ++j)
            stream >> vertex.position[j];
    }

    // Read the polygons
    int numPolygonSets;
    stream >> numPolygonSets;
    while (numPolygonSets-- > 0)
    {
        MatrixXi polygons = readPolygons(stream, mapVertices);
        switch (polygons.cols())
        {
        case 4:
            result.quadrangles = polygons;
        case 3:
            result.triangles = polygons;
        case 2:
            result.lines = polygons;
        }
    }

    // Read the vertex dependencies
    result.slaves = readSlaves(stream, mapVertices);

    // Assign the result
    mGeometry = result;
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

//! Reallocate the data fields
void ModalSolution::resize(int numDOFs, int numModes)
{
    mGeometry.vertices.resize(numDOFs);
    mFrequencies.resize(numModes);
    mModeShapes.resize(numModes);
    for (MatrixXd& item : mModeShapes)
        item.resize(numDOFs, Constants::skNumDirections);
}

//! Retrieve the polygon indices from the text stream
MatrixXi ModalSolution::readPolygons(QTextStream& stream, QMap<QString, int> const& mapVertices)
{
    int numPolygons, numIndices;
    stream >> numPolygons >> numIndices;
    MatrixXi result(numPolygons, numIndices);
    int numRows = 0;
    for (int i = 0; i != numPolygons; ++i)
    {
        VectorXi indices(numIndices);
        bool isFound = true;
        for (int j = 0; j != numIndices; ++j)
        {
            QString name;
            stream >> name;
            if (mapVertices.contains(name))
            {
                indices[j] = mapVertices[name];
            }
            else
            {
                isFound = false;
                break;
            }
        }
        if (isFound)
        {
            result.row(numRows) = indices;
            ++numRows;
        }
    }
    return result(seq(0, numRows - 1), all);
}

//! Retrieve the vertex dependencies
QList<Slave> ModalSolution::readSlaves(QTextStream& stream, QMap<QString, int> const& mapVertices)
{
    int numSlaves;
    stream >> numSlaves;
    QList<Slave> result;
    result.resize(numSlaves);
    int numRows = 0;
    for (int i = 0; i != numSlaves; ++i)
    {
        QString name;
        stream >> name;
        if (mapVertices.contains(name))
        {
            // Acquire the slave index
            int slaveIndex = mapVertices[name];

            // Get the master indices
            Vector4i masterIndices;
            int numIndices = 0;
            for (int j = 0; j != masterIndices.size(); ++j)
            {
                stream >> name;
                if (mapVertices.contains(name))
                {
                    masterIndices[numIndices] = mapVertices[name];
                    ++numIndices;
                }
            }

            // Get the direction
            int flag;
            int index = -1;
            for (int j = 0; j != Constants::skNumDirections; ++j)
            {
                stream >> flag;
                if (flag == 1)
                {
                    index = j;
                    break;
                }
            }
            if (index < 0)
                return QList<Slave>();
            Direction direction = (Direction) index;

            // Insert the result
            Slave item;
            item.slaveIndex = slaveIndex;
            item.masterIndices = masterIndices(seq(0, numIndices - 1));
            item.direction = direction;
            result[numRows] = item;
            ++numRows;
        }
    }
    result.resize(numRows);
    return result;
}

Geometry::Geometry()
{
}

bool Geometry::isEmpty() const
{
    return vertices.empty();
}

//! Shift the geometry
void Geometry::move(Eigen::Vector3d const& shift)
{
    for (Vertex& vertex : vertices)
        vertex.position += shift;
}

//! Rotate the geometry around the specified axis
void Geometry::rotate(double angle, Direction direction)
{
    Vector3d axis;
    switch (direction)
    {
    case Direction::kX:
        axis = Vector3d::UnitX();
        break;
    case Direction::kY:
        axis = Vector3d::UnitY();
        break;
    case Direction::kZ:
        axis = Vector3d::UnitZ();
        break;
    }
    auto transformation = AngleAxisd(angle, axis);
    for (Vertex& vertex : vertices)
        vertex.position = transformation * vertex.position;
}

ModalComparison::ModalComparison()
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
