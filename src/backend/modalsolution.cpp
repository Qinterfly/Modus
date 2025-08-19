#include <QObject>

#include "fileutility.h"
#include "modalsolution.h"

static int const skNumDirections = 3;

using namespace Backend;
using namespace Backend::Core;
using namespace Eigen;

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
        for (int j = 0; j != skNumDirections; ++j)
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
        for (int j = 0; j != skNumDirections; ++j)
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
    // TODO
}

//! Reallocate the data fields
void ModalSolution::resize(int numDOFs, int numModes)
{
    mGeometry.vertices.resize(numDOFs);
    mFrequencies.resize(numModes);
    mModeShapes.resize(numModes);
    for (MatrixXd& item : mModeShapes)
        item.resize(numDOFs, skNumDirections);
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
            for (int j = 0; j != skNumDirections; ++j)
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
