#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <Eigen/Core>

#include "aliasdata.h"
#include "iserializable.h"

namespace Backend::Core
{

enum class Direction
{
    kX,
    kY,
    kZ
};

struct Vertex : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(Eigen::Vector3d position MEMBER position)

public:
    Vertex();
    ~Vertex();

    bool operator==(Vertex const& another) const;
    bool operator!=(Vertex const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    QString name;
    Eigen::Vector3d position;
};

struct Slave : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(int slaveIndex MEMBER slaveIndex)
    Q_PROPERTY(Eigen::VectorXi masterIndices MEMBER masterIndices)
    Q_PROPERTY(Direction direction MEMBER direction)

public:
    Slave();
    ~Slave();

    bool operator==(Slave const& another) const;
    bool operator!=(Slave const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    int slaveIndex;
    Eigen::VectorXi masterIndices;
    Direction direction;
};

struct Geometry : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(QList<Vertex> vertices MEMBER vertices)
    Q_PROPERTY(QList<Slave> slaves MEMBER slaves)
    Q_PROPERTY(Eigen::MatrixXi lines MEMBER lines)
    Q_PROPERTY(Eigen::MatrixXi triangles MEMBER triangles)
    Q_PROPERTY(Eigen::MatrixXi quadrangles MEMBER quadrangles)

public:
    Geometry();
    ~Geometry();

    bool isEmpty() const;
    int numVertices() const;
    void move(Eigen::Vector3d const& shift);
    void rotate(double angle, Direction direction);
    void read(QString const& pathFile);

    bool operator==(Geometry const& another) const;
    bool operator!=(Geometry const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    QList<Vertex> vertices;
    QList<Slave> slaves;
    Eigen::MatrixXi lines;
    Eigen::MatrixXi triangles;
    Eigen::MatrixXi quadrangles;
};

}
#endif // GEOMETRY_H
