#include <Eigen/Geometry>

#include "constants.h"
#include "fileutility.h"
#include "geometry.h"

using namespace Backend;
using namespace Backend::Core;
using namespace Eigen;

MatrixXi readPolygons(QTextStream& stream, QMap<QString, int> const& mapVertices);
QList<Slave> readSlaves(QTextStream& stream, QMap<QString, int> const& mapVertices);

Vertex::Vertex()
{
    position.fill(0.0);
}

Vertex::~Vertex()
{
}

bool Vertex::operator==(Vertex const& another) const
{
    return Utility::areEqual(*this, another);
}

bool Vertex::operator!=(Vertex const& another) const
{
    return !(*this == another);
}

void Vertex::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    stream.writeAttribute("name", name);
    Utility::serialize(stream, "position", position);
    stream.writeEndElement();
}

void Vertex::deserialize(QXmlStreamReader& stream)
{
    name = stream.attributes().value("name").toString();
    while (stream.readNextStartElement())
    {
        if (stream.name() == "position")
            Utility::deserialize(stream, position);
        else
            stream.skipCurrentElement();
    }
}

Slave::Slave()
    : slaveIndex(-1)
{
}

Slave::~Slave()
{
}

bool Slave::operator==(Slave const& another) const
{
    return Utility::areEqual(*this, another);
}

bool Slave::operator!=(Slave const& another) const
{
    return !(*this == another);
}

void Slave::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    stream.writeAttribute("slaveIndex", Utility::toString(slaveIndex));
    stream.writeAttribute("direction", Utility::toString((int) direction));
    Utility::serialize(stream, "masterIndices", masterIndices);
    stream.writeEndElement();
}

void Slave::deserialize(QXmlStreamReader& stream)
{
    slaveIndex = stream.attributes().value("slaveIndex").toInt();
    direction = (Direction) stream.attributes().value("direction").toInt();
    while (stream.readNextStartElement())
    {
        if (stream.name() == "masterIndices")
            Utility::deserialize(stream, masterIndices);
        else
            stream.skipCurrentElement();
    }
}

Geometry::Geometry()
{
}

Geometry::~Geometry()
{
}

Geometry::Geometry(KCL::Geometry const& geometry)
{
    int numDOFs = geometry.vertices.rows();
    vertices.resize(numDOFs);
    for (int i = 0; i != numDOFs; ++i)
    {
        Vertex& vertex = vertices[i];
        vertex.name = QString::number(i);
        for (int j = 0; j != Constants::skNumDirections; ++j)
            vertex.position[j] = geometry.vertices(i, j);
    }
    quadrangles = geometry.quadrangles;
}

bool Geometry::isEmpty() const
{
    return numVertices() == 0;
}

int Geometry::numVertices() const
{
    return vertices.size();
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

void Geometry::read(QString const& pathFile)
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

    // Read the vertices
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
            quadrangles = polygons;
        case 3:
            triangles = polygons;
        case 2:
            lines = polygons;
        }
    }

    // Read the vertex dependencies
    slaves = readSlaves(stream, mapVertices);
}

bool Geometry::operator==(Geometry const& another) const
{
    return Utility::areEqual(*this, another);
}

bool Geometry::operator!=(Geometry const& another) const
{
    return !(*this == another);
}

void Geometry::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    Utility::serialize(stream, "vertices", "vertex", vertices);
    Utility::serialize(stream, "slaves", "slave", slaves);
    Utility::serialize(stream, "lines", lines);
    Utility::serialize(stream, "triangles", triangles);
    Utility::serialize(stream, "quadrangles", quadrangles);
    stream.writeEndElement();
}

void Geometry::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "vertices")
            Utility::deserialize(stream, "vertex", vertices);
        else if (stream.name() == "slaves")
            Utility::deserialize(stream, "slave", slaves);
        else if (stream.name() == "lines")
            Utility::deserialize(stream, lines);
        else if (stream.name() == "triangles")
            Utility::deserialize(stream, triangles);
        else if (stream.name() == "quadrangles")
            Utility::deserialize(stream, quadrangles);
        else
            stream.skipCurrentElement();
    }
}

//! Helper function to retrieve the polygon indices from the text stream
MatrixXi readPolygons(QTextStream& stream, QMap<QString, int> const& mapVertices)
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

//! Helper function to retrieve the vertex dependencies
QList<Slave> readSlaves(QTextStream& stream, QMap<QString, int> const& mapVertices)
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
