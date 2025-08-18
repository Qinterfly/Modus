#ifndef OPTIMSOLVER_H
#define OPTIMSOLVER_H

#include <Eigen/Core>

#include "constraints.h"
#include "selector.h"

namespace KCL
{
class Model;
}

namespace Backend::Core
{

//! Optimization parameters
struct OptimOptions
{
    OptimOptions();
    ~OptimOptions() = default;

    bool isValid() const;
    void resize(int numDOFs, int numModes);

    //! Indices of the modes to be updated
    Eigen::VectorXi indices;

    //! Target frequencies
    Eigen::VectorXd frequencies;

    //! Target modeshapes
    QList<Eigen::MatrixXd> modeShapes;

    //! Participation factors of mode residuals
    Eigen::VectorXd weights;

    //! Selection of entities to be updated
    Selector selector;

    //! Optimization constraints
    Constraints constraints;

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

    void solve(KCL::Model const& model, OptimOptions const& options);
};

}

#endif // OPTIMSOLVER_H
