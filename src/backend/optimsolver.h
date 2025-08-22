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

struct OptimProblem
{
    OptimProblem();
    ~OptimProblem() = default;

    bool isValid() const;
    void resize(int numModes);

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

struct OptimOptions
{
    OptimOptions();
    ~OptimOptions() = default;

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
};

class OptimSolver
{
public:
    OptimSolver();
    ~OptimSolver() = default;

    void solve(OptimProblem const& problem, OptimOptions const& options);

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
}

#endif // OPTIMSOLVER_H
