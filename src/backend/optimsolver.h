#ifndef OPTIMSOLVER_H
#define OPTIMSOLVER_H

#include <Eigen/Core>
#include <ceres/ceres.h>
#include <kcl/model.h>

#include "constraints.h"
#include "modalsolution.h"
#include "selector.h"

namespace KCL
{
class Model;
}

namespace Backend::Core
{

using UnwrapFun = std::function<KCL::Model(const double* const)>;
using SolverFun = std::function<KCL::EigenSolution(KCL::Model const&)>;
using ElementMap = QMap<KCL::ElementType, QList<KCL::AbstractElement*>>;

struct OptimSolution : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(int iteration MEMBER iteration)
    Q_PROPERTY(bool isSuccess MEMBER isSuccess)
    Q_PROPERTY(double duration MEMBER duration)
    Q_PROPERTY(double cost MEMBER cost)
    Q_PROPERTY(KCL::Model model MEMBER model)
    Q_PROPERTY(ModalSolution modalSolution MEMBER modalSolution)
    Q_PROPERTY(ModalComparison comparison MEMBER comparison)
    Q_PROPERTY(QString message MEMBER message)

public:
    OptimSolution();
    ~OptimSolution() = default;

    bool operator==(OptimSolution const& another) const;
    bool operator!=(OptimSolution const& another) const;

    void serialize(QXmlStreamWriter& stream) const override;
    void deserialize(QXmlStreamWriter& stream) override;

    int iteration;
    bool isSuccess;
    double duration;
    double cost;
    KCL::Model model;
    ModalSolution modalSolution;
    ModalComparison comparison;
    QString message;
};

struct OptimProblem : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(KCL::Model model MEMBER model)
    Q_PROPERTY(Eigen::VectorXi targetIndices MEMBER targetIndices)
    Q_PROPERTY(Eigen::VectorXd targetWeights MEMBER targetWeights)
    Q_PROPERTY(ModalSolution targetSolution MEMBER targetSolution)
    Q_PROPERTY(Matches targetMatches MEMBER targetMatches)
    Q_PROPERTY(Selector selector MEMBER selector)
    Q_PROPERTY(Constraints constraints MEMBER constraints)

public:
    OptimProblem();
    ~OptimProblem() = default;

    bool isValid() const;
    void resize(int numModes);
    void fillMatches();

    bool operator==(OptimProblem const& another) const;
    bool operator!=(OptimProblem const& another) const;

    void serialize(QXmlStreamWriter& stream) const override;
    void deserialize(QXmlStreamWriter& stream) override;

    //! Model to be updated
    KCL::Model model;

    //! Indices of the modes to be updated
    Eigen::VectorXi targetIndices;

    //! Participation factors of mode residuals
    Eigen::VectorXd targetWeights;

    //! Target modal solution
    ModalSolution targetSolution;

    //! Vertex correspondence between model and target solutions
    Matches targetMatches;

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

public:
    OptimOptions();
    ~OptimOptions() = default;

    bool operator==(OptimOptions const& another) const;
    bool operator!=(OptimOptions const& another) const;

    void serialize(QXmlStreamWriter& stream) const override;
    void deserialize(QXmlStreamWriter& stream) override;

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
};

class OptimSolver : public QObject
{
    Q_OBJECT

public:
    OptimSolver();
    ~OptimSolver() = default;

    QList<OptimSolution> solve(OptimProblem const& problem, OptimOptions const& options);

signals:
    void iterationFinished(Backend::Core::OptimSolution solution);
    void log(QString message);

private:
    QList<double> wrapModel();
    KCL::Model unwrapModel(QList<double> const& paramValues);
    Eigen::MatrixXd getProperties(QList<KCL::AbstractElement*> const& elements, VariableType type);
    Eigen::MatrixXd getProperties(KCL::SpringDamper* pElement, QList<bool>& mask);
    void setProperties(Eigen::MatrixXd const& properties, QList<KCL::AbstractElement*>& elements, VariableType type);
    void setProperties(Eigen::MatrixXd const& properties, KCL::SpringDamper* pElement, QList<bool> const& mask);
    void wrapProperties(QList<double>& parameterValues, Eigen::MatrixXd const& properties, VariableType type);
    Eigen::MatrixXd unwrapProperties(int& iParameter, QList<double> const& parameterValues, Eigen::MatrixXd const& initProperties,
                                     VariableType type);
    void printReport(ceres::Solver::Summary const& summary);
    QMap<int, ElementMap> getSurfaceElements(KCL::Model& model);
    QMap<VariableType, QList<int>> getVariableIndices();
    QMap<KCL::ElementType, QList<VariableType>> getElementVariables();

private:
    KCL::Model mInitModel;
    QList<Selection> mSelections;
    Constraints mConstraints;
    QList<double> mParameterScales;
    QList<PairDouble> mParameterBounds;
};

//! Functor to compute residuals
class ObjectiveFunctor
{
public:
    ObjectiveFunctor(OptimProblem const& problem, OptimOptions const& options, UnwrapFun unwrapFun, SolverFun solverFun);
    ~ObjectiveFunctor() = default;
    bool operator()(double const* const* parameters, double* residuals) const;

private:
    OptimProblem const& mProblem;
    OptimOptions const& mOptions;
    UnwrapFun mUnwrapFun;
    SolverFun mSolverFun;
};

//! Functor to be called after every optimization iteration
class OptimCallback : public QObject, public ceres::IterationCallback
{
    Q_OBJECT

public:
    OptimCallback(QList<double>& parameters, OptimProblem const& problem, OptimOptions const& options, UnwrapFun unwrapFun, SolverFun solverFun);
    ceres::CallbackReturnType operator()(ceres::IterationSummary const& summary);

signals:
    void iterationFinished(Backend::Core::OptimSolution solution);
    void log(QString message);

private:
    QList<double>& mParameterValues;
    OptimProblem const& mProblem;
    OptimOptions const& mOptions;
    UnwrapFun mUnwrapFun;
    SolverFun mSolverFun;
};
}

#endif // OPTIMSOLVER_H
