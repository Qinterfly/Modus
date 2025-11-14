#ifndef OPTIMSOLVER_H
#define OPTIMSOLVER_H

#include <Eigen/Core>
#include <ceres/ceres.h>
#include <kcl/model.h>

#include "constraints.h"
#include "isolver.h"
#include "modalsolver.h"
#include "selector.h"

namespace KCL
{
class Model;
}

namespace Backend::Core
{

using UnwrapFun = std::function<KCL::Model(const double* const)>;
using SolverFun = std::function<KCL::EigenSolution(KCL::Model const&)>;
using CompareFun = std::function<ModalComparison(ModalSolution const& solution)>;
using ElementMap = QMap<KCL::ElementType, QList<KCL::AbstractElement*>>;

struct OptimTarget : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(Eigen::VectorXi indices MEMBER indices)
    Q_PROPERTY(Eigen::VectorXd frequencies MEMBER frequencies)
    Q_PROPERTY(Eigen::VectorXd weights MEMBER weights)
    Q_PROPERTY(ModalSolution solution MEMBER solution)
    Q_PROPERTY(Matches matches MEMBER matches)

public:
    OptimTarget();
    virtual ~OptimTarget() = default;

    bool isValid() const;
    void resize(int numModes);

    bool operator==(OptimTarget const& another) const;
    bool operator!=(OptimTarget const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    //! Indices of the modes to be updated
    Eigen::VectorXi indices;

    //! Frequencies of the modes to be updated
    Eigen::VectorXd frequencies;

    //! Participation factors of mode residuals
    Eigen::VectorXd weights;

    //! Target modal solution (optional)
    ModalSolution solution;

    //! Vertex correspondence between model and target solutions
    Matches matches;
};

struct OptimProblem : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(KCL::Model model MEMBER model)
    Q_PROPERTY(OptimTarget target MEMBER target)
    Q_PROPERTY(Selector selector MEMBER selector)
    Q_PROPERTY(Constraints constraints MEMBER constraints)

public:
    OptimProblem();
    virtual ~OptimProblem() = default;

    bool isValid() const;

    bool operator==(OptimProblem const& another) const;
    bool operator!=(OptimProblem const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    //! Model to be updated
    KCL::Model model;

    //! Target values
    OptimTarget target;

    //! Selection of entities to be updated
    Selector selector;

    //! Optimization constraints
    Constraints constraints;
};

struct OptimOptions : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(int maxNumIterations MEMBER maxNumIterations)
    Q_PROPERTY(double timeoutIteration MEMBER timeoutIteration)
    Q_PROPERTY(int numThreads MEMBER numThreads)
    Q_PROPERTY(double diffStepSize MEMBER diffStepSize)
    Q_PROPERTY(double minMAC MEMBER minMAC)
    Q_PROPERTY(double penaltyMAC MEMBER penaltyMAC)
    Q_PROPERTY(double maxRelError MEMBER maxRelError)
    Q_PROPERTY(int numModes MEMBER numModes)

public:
    OptimOptions();
    virtual ~OptimOptions() = default;

    bool operator==(OptimOptions const& another) const;
    bool operator!=(OptimOptions const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    //! Maximum number of iterations of optimization process
    int maxNumIterations;

    //! Maximum duration of each iteration
    double timeoutIteration;

    //! Number of threads used to compute the Jacobian
    int numThreads;

    //! Perturbation step of variables to compute the Jacobian
    double diffStepSize;

    //! Minimum MAC acceptance threshold
    double minMAC;

    //! Residual MAC penalty
    double penaltyMAC;

    //! Maximum relative errors in frequencies
    double maxRelError;

    //! Number of modes to compute
    int numModes;
};

struct OptimSolution : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(int iteration MEMBER iteration)
    Q_PROPERTY(bool isSuccess MEMBER isSuccess)
    Q_PROPERTY(double duration MEMBER duration)
    Q_PROPERTY(double cost MEMBER cost)
    Q_PROPERTY(KCL::Model model MEMBER model)
    Q_PROPERTY(ModalSolution modalSolution MEMBER modalSolution)
    Q_PROPERTY(ModalComparison modalComparison MEMBER modalComparison)
    Q_PROPERTY(QString message MEMBER message)

public:
    OptimSolution();
    virtual ~OptimSolution() = default;

    bool operator==(OptimSolution const& another) const;
    bool operator!=(OptimSolution const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    int iteration;
    bool isSuccess;
    double duration;
    double cost;
    KCL::Model model;
    ModalSolution modalSolution;
    ModalComparison modalComparison;
    QString message;
};

class OptimSolver : public QObject, public ISolver
{
    Q_OBJECT
    Q_PROPERTY(OptimProblem problem MEMBER problem)
    Q_PROPERTY(OptimOptions options MEMBER options)
    Q_PROPERTY(QList<OptimSolution> solutions MEMBER solutions)
    Q_PROPERTY(QString log MEMBER log)

public:
    OptimSolver();
    virtual ~OptimSolver() = default;

    OptimSolver(OptimSolver const& another);
    OptimSolver(OptimSolver&& another);
    OptimSolver& operator=(OptimSolver const& another);

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
    void iterationFinished(Backend::Core::OptimSolution solution);
    void logAppended(QString message);

private:
    // Process targets
    void setTargetSolution(SolverFun solverFun);
    void setTargetMatches();

    // Process model
    void setModelParameters();
    QList<double> wrapModel();
    KCL::Model unwrapModel(QList<double> const& paramValues);

    // Process properties
    Eigen::MatrixXd getProperties(QList<KCL::AbstractElement*> const& elements, VariableType type);
    Eigen::MatrixXd getProperties(KCL::SpringDamper* pElement, QList<bool>& mask);
    void setProperties(Eigen::MatrixXd const& properties, QList<KCL::AbstractElement*>& elements, VariableType type);
    void setProperties(Eigen::MatrixXd const& properties, KCL::SpringDamper* pElement, QList<bool> const& mask);
    void wrapProperties(QList<double>& parameterValues, Eigen::MatrixXd const& properties, VariableType type);
    Eigen::MatrixXd unwrapProperties(int& iParameter, QList<double> const& parameterValues, Eigen::MatrixXd const& initProperties,
                                     VariableType type);

    // Logging
    void printReport(ceres::Solver::Summary const& summary);
    void appendLog(QString const& message, QtMsgType type = QtMsgType::QtInfoMsg);

    // Slicing
    QMap<int, ElementMap> getSurfaceElements(KCL::Model& model);
    QMap<VariableType, QList<int>> getVariableIndices();
    QMap<KCL::ElementType, QList<VariableType>> getElementVariables();

public:
    QString name;
    OptimProblem problem;
    OptimOptions options;
    QList<OptimSolution> solutions;
    QString log;

private:
    KCL::Model mInitModel;
    QList<Selection> mSelections;
    Constraints mConstraints;
    QList<double> mParameterScales;
    QList<PairDouble> mParameterBounds;
    OptimTarget mTarget;
};

//! Functor to compute residuals
class ObjectiveFunctor
{
public:
    ObjectiveFunctor(OptimTarget const& target, OptimOptions const& options, UnwrapFun unwrapFun, SolverFun solverFun, CompareFun compareFun);
    ~ObjectiveFunctor() = default;

    bool operator()(double const* const* parameters, double* residuals) const;

private:
    OptimTarget const& mTarget;
    OptimOptions const& mOptions;
    UnwrapFun mUnwrapFun;
    SolverFun mSolverFun;
    CompareFun mCompareFun;
};

//! Functor to be called after every optimization iteration
class OptimCallback : public QObject, public ceres::IterationCallback
{
    Q_OBJECT

public:
    OptimCallback(QList<double>& parameters, OptimTarget const& target, OptimOptions const& options, UnwrapFun unwrapFun, SolverFun solverFun,
                  CompareFun compareFun);
    ~OptimCallback() = default;

    ceres::CallbackReturnType operator()(ceres::IterationSummary const& summary);

signals:
    void iterationFinished(Backend::Core::OptimSolution solution);
    void logRequested(QString message);

private:
    QList<double>& mParameterValues;
    OptimTarget const& mTarget;
    OptimOptions const& mOptions;
    UnwrapFun mUnwrapFun;
    SolverFun mSolverFun;
    CompareFun mCompareFun;
};
}

#endif // OPTIMSOLVER_H
