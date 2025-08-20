#ifndef OPTIMSOLVER_H
#define OPTIMSOLVER_H

#include <Eigen/Core>

#include "constraints.h"
#include "modalsolution.h"
#include "selector.h"

namespace KCL
{
class Model;
}

namespace Backend::Core
{

struct OptimData
{
    OptimData();
    ~OptimData() = default;

    bool isEmpty() const;
    bool isValid() const;
    void resize(int numModes);

    //! Indices of the modes to be updated
    Eigen::VectorXi indices;

    //! Participation factors of mode residuals
    Eigen::VectorXd weights;

    //! Target modal solution
    ModalSolution targetSolution;

    //! Selection of entities to be updated
    Selector selector;

    //! Optimization constraints
    Constraints constraints;
};

struct OptimOptions
{
    OptimOptions();
    ~OptimOptions() = default;

    //! Flag which disables one mode to be paired several times
    bool isExclusiveMAC;

    //! Residual MAC penalty
    double penaltyMAC;
};

class OptimSolver
{
public:
    OptimSolver();
    ~OptimSolver() = default;

    void solve(KCL::Model const& model, OptimData const& data, OptimOptions const& options);
};
}

#endif // OPTIMSOLVER_H
