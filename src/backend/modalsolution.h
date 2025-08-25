#ifndef MODALSOLUTION_H
#define MODALSOLUTION_H

#include <Eigen/Core>
#include <kcl/solver.h>
#include <QDir>
#include <QList>

#include "aliasdata.h"

namespace Backend::Core
{

enum class Direction
{
    kX,
    kY,
    kZ
};

struct Vertex
{
    QString name;
    Eigen::Vector3d position;
};

struct Slave
{
    int slaveIndex;
    Eigen::VectorXi masterIndices;
    Direction direction;
};

struct Geometry
{
    Geometry();
    ~Geometry() = default;

    bool isEmpty() const;
    void move(Eigen::Vector3d const& shift);
    void rotate(double angle, Direction direction);

    QList<Vertex> vertices;
    QList<Slave> slaves;
    Eigen::MatrixXi lines;
    Eigen::MatrixXi triangles;
    Eigen::MatrixXi quadrangles;
};

struct ModalComparison
{
    ModalComparison();
    ~ModalComparison() = default;

    bool isEmpty() const;
    bool isValid() const;
    void resize(int numModes);

    Eigen::VectorXd diffFrequencies;
    Eigen::VectorXd errorFrequencies;
    Eigen::VectorXd errorsMAC;
    ModalPairs pairs;
};

class ModalSolution
{
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
