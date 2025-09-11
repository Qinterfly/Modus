#ifndef FLUTTERSOLVER_H
#define FLUTTERSOLVER_H

#include "isolver.h"
#include "kcl/model.h"

#include "geometry.h"

namespace Backend::Core
{

struct FlutterOptions : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(double timeout MEMBER timeout)
    Q_PROPERTY(int numModes MEMBER numModes)

public:
    FlutterOptions();
    ~FlutterOptions();

    bool operator==(FlutterOptions const& another) const;
    bool operator!=(FlutterOptions const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    //! Maximum duration of solution
    double timeout;

    //! Number of modes to compute
    int numModes;
};

class FlutterSolution : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(Geometry geometry MEMBER mGeometry)
    Q_PROPERTY(Eigen::VectorXd flow MEMBER mFlow)
    Q_PROPERTY(Eigen::MatrixXcd roots MEMBER mRoots)
    Q_PROPERTY(Eigen::VectorXd critFlow MEMBER mCritFlow)
    Q_PROPERTY(Eigen::VectorXd critSpeed MEMBER mCritSpeed)
    Q_PROPERTY(Eigen::VectorXd critFrequency MEMBER mCritFrequency)
    Q_PROPERTY(Eigen::VectorXd critCircFrequency MEMBER mCritCircFrequency)
    Q_PROPERTY(Eigen::VectorXd critStrouhal MEMBER mCritStrouhal)
    Q_PROPERTY(Eigen::VectorXd critDamping MEMBER mCritDamping)
    Q_PROPERTY(QList<Eigen::MatrixXcd> critModeShapes MEMBER mCritModeShapes)
    Q_PROPERTY(Eigen::MatrixXd critPartFactor MEMBER mCritPartFactor)
    Q_PROPERTY(Eigen::MatrixXd critPartPhase MEMBER mCritPartPhase)

public:
    FlutterSolution();
    FlutterSolution(KCL::FlutterSolution const& solution);
    ~FlutterSolution();

    bool isEmpty() const;
    int numCrit() const;

    Geometry const& geometry() const;
    Eigen::VectorXd const& flow() const;
    Eigen::MatrixXcd const& roots() const;
    Eigen::VectorXd const& critFlow() const;
    Eigen::VectorXd const& critSpeed() const;
    Eigen::VectorXd const& critFrequency() const;
    Eigen::VectorXd const& critCircFrequency() const;
    Eigen::VectorXd const& critStrouhal() const;
    Eigen::VectorXd const& critDamping() const;
    QList<Eigen::MatrixXcd> const& critModeShapes() const;
    Eigen::MatrixXd const& critPartFactor() const;
    Eigen::MatrixXd const& critPartPhase() const;

    bool operator==(FlutterSolution const& another) const;
    bool operator!=(FlutterSolution const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

private:
    Geometry mGeometry;
    Eigen::VectorXd mFlow;
    Eigen::MatrixXcd mRoots;
    Eigen::VectorXd mCritFlow;
    Eigen::VectorXd mCritSpeed;
    Eigen::VectorXd mCritFrequency;
    Eigen::VectorXd mCritCircFrequency;
    Eigen::VectorXd mCritStrouhal;
    Eigen::VectorXd mCritDamping;
    QList<Eigen::MatrixXcd> mCritModeShapes;
    Eigen::MatrixXd mCritPartFactor;
    Eigen::MatrixXd mCritPartPhase;
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
    KCL::Model model;
    FlutterOptions options;
    FlutterSolution solution;
};
}

#endif // FLUTTERSOLVER_H
