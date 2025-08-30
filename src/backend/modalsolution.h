#ifndef MODALSOLUTION_H
#define MODALSOLUTION_H

#include <Eigen/Core>
#include <kcl/solver.h>
#include <QDir>
#include <QList>

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
    ~Vertex() = default;

    bool operator==(Vertex const& another) const;
    bool operator!=(Vertex const& another) const;

    void serialize(QXmlStreamWriter& stream) const override;
    void deserialize(QXmlStreamWriter& stream) override;
    QString elementName() const override;

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
    ~Slave() = default;

    bool operator==(Slave const& another) const;
    bool operator!=(Slave const& another) const;

    void serialize(QXmlStreamWriter& stream) const override;
    void deserialize(QXmlStreamWriter& stream) override;
    QString elementName() const override;

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
    ~Geometry() = default;

    bool isEmpty() const;
    void move(Eigen::Vector3d const& shift);
    void rotate(double angle, Direction direction);

    bool operator==(Geometry const& another) const;
    bool operator!=(Geometry const& another) const;

    void serialize(QXmlStreamWriter& stream) const override;
    void deserialize(QXmlStreamWriter& stream) override;
    QString elementName() const override;

    QList<Vertex> vertices;
    QList<Slave> slaves;
    Eigen::MatrixXi lines;
    Eigen::MatrixXi triangles;
    Eigen::MatrixXi quadrangles;
};

struct ModalComparison : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(Eigen::VectorXd diffFrequencies MEMBER diffFrequencies)
    Q_PROPERTY(Eigen::VectorXd errorFrequencies MEMBER errorFrequencies)
    Q_PROPERTY(Eigen::VectorXd errorsMAC MEMBER errorsMAC)
    Q_PROPERTY(ModalPairs pairs MEMBER pairs)

public:
    ModalComparison();
    ~ModalComparison() = default;

    bool isEmpty() const;
    bool isValid() const;
    void resize(int numModes);

    bool operator==(ModalComparison const& another) const;
    bool operator!=(ModalComparison const& another) const;

    void serialize(QXmlStreamWriter& stream) const override;
    void deserialize(QXmlStreamWriter& stream) override;
    QString elementName() const override;

    Eigen::VectorXd diffFrequencies;
    Eigen::VectorXd errorFrequencies;
    Eigen::VectorXd errorsMAC;
    ModalPairs pairs;
};

class ModalSolution : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(Geometry geometry MEMBER mGeometry)
    Q_PROPERTY(Eigen::VectorXd frequencies MEMBER mFrequencies)
    Q_PROPERTY(QList<Eigen::MatrixXd> modeShapes MEMBER mModeShapes)
    Q_PROPERTY(QList<QString> names MEMBER mNames)

public:
    ModalSolution();
    ModalSolution(Geometry const& geometry, Eigen::VectorXd const& frequencies, QList<Eigen::MatrixXd> const& modeShapes);
    ModalSolution(KCL::EigenSolution const& solution);
    ~ModalSolution() = default;

    int numVertices() const;
    int numModes() const;
    bool isEmpty() const;
    Geometry const& geometry() const;
    Eigen::VectorXd const& frequencies() const;
    QList<Eigen::MatrixXd> const& modeShapes() const;
    ModalComparison compare(ModalSolution const& another, Eigen::VectorXi const& indices, Matches const& matches, double minMAC) const;

    void read(QDir const& directory);
    void readGeometry(QString const& pathFile);
    void readModesets(QString const& pathFile);

    bool operator==(ModalSolution const& another) const;
    bool operator!=(ModalSolution const& another) const;

    void serialize(QXmlStreamWriter& stream) const override;
    void deserialize(QXmlStreamWriter& stream) override;
    QString elementName() const override;

private:
    void resize(int numDOFs, int numModes);
    Eigen::MatrixXi readPolygons(QTextStream& stream, QMap<QString, int> const& mapVertices);
    QList<Slave> readSlaves(QTextStream& stream, QMap<QString, int> const& mapVertices);

private:
    Geometry mGeometry;
    Eigen::VectorXd mFrequencies;
    QList<Eigen::MatrixXd> mModeShapes;
    QList<QString> mNames;
};

}

#endif // MODALSOLUTION_H
