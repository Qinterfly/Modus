#ifndef FLUTTERSOLVER_H
#define FLUTTERSOLVER_H

#include <kcl/model.h>

#include "isolver.h"
#include "geometry.h"

namespace Backend::Core
{

struct FlutterOptions : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(int numModes MEMBER numModes)
    Q_PROPERTY(double timeout MEMBER timeout)

public:
    FlutterOptions();
    ~FlutterOptions();

    bool operator==(FlutterOptions const& another) const;
    bool operator!=(FlutterOptions const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    //! Number of modes to compute
    int numModes;

    //! Maximum duration of solution
    double timeout;
};

struct FlutterSolution : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(Geometry geometry MEMBER geometry)
    Q_PROPERTY(Eigen::VectorXd flow MEMBER flow)
    Q_PROPERTY(Eigen::MatrixXcd roots MEMBER roots)
    Q_PROPERTY(Eigen::VectorXd critFlow MEMBER critFlow)
    Q_PROPERTY(Eigen::VectorXd critSpeed MEMBER critSpeed)
    Q_PROPERTY(Eigen::VectorXd critFrequency MEMBER critFrequency)
    Q_PROPERTY(Eigen::VectorXd critCircFrequency MEMBER critCircFrequency)
    Q_PROPERTY(Eigen::VectorXd critStrouhal MEMBER critStrouhal)
    Q_PROPERTY(Eigen::VectorXd critDamping MEMBER critDamping)
    Q_PROPERTY(QList<Eigen::MatrixXcd> critModeShapes MEMBER critModeShapes)
    Q_PROPERTY(Eigen::MatrixXd critPartFactor MEMBER critPartFactor)
    Q_PROPERTY(Eigen::MatrixXd critPartPhase MEMBER critPartPhase)

public:
    FlutterSolution();
    FlutterSolution(KCL::FlutterSolution const& solution);
    ~FlutterSolution();

    bool isEmpty() const;
    int numCrit() const;

    bool operator==(FlutterSolution const& another) const;
    bool operator!=(FlutterSolution const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    Geometry geometry;
    Eigen::VectorXd flow;
    Eigen::MatrixXcd roots;
    Eigen::VectorXd critFlow;
    Eigen::VectorXd critSpeed;
    Eigen::VectorXd critFrequency;
    Eigen::VectorXd critCircFrequency;
    Eigen::VectorXd critStrouhal;
    Eigen::VectorXd critDamping;
    QList<Eigen::MatrixXcd> critModeShapes;
    Eigen::MatrixXd critPartFactor;
    Eigen::MatrixXd critPartPhase;
};

class FlutterSolver : public QObject, public ISolver
{
    Q_OBJECT
    Q_PROPERTY(KCL::Model model MEMBER model)
    Q_PROPERTY(FlutterOptions options MEMBER options)
    Q_PROPERTY(FlutterSolution solution MEMBER solution)

public:
    FlutterSolver();
    ~FlutterSolver();
    FlutterSolver(FlutterSolver const& another);
    FlutterSolver(FlutterSolver&& another);
    FlutterSolver& operator=(FlutterSolver const& another);

    ISolver::Type type() const override;
    ISolver* clone() const override;

    void clear() override;
    void solve() override;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    bool operator==(ISolver const* pBaseSolver) const override;
    bool operator!=(ISolver const* pBaseSolver) const override;

signals:
    void solverFinished();

public:
    QString name;
    KCL::Model model;
    FlutterOptions options;
    FlutterSolution solution;
};
}

#endif // FLUTTERSOLVER_H
