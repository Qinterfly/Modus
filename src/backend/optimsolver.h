#ifndef OPTIMSOLVER_H
#define OPTIMSOLVER_H

#include <Eigen/Core>
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

using ElementMap = QMap<KCL::ElementType, QList<KCL::AbstractElement*>>;

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

    void solve(KCL::Model const& initModel, OptimData const& data, OptimOptions const& options);

private:
    void wrapModel();
    KCL::Model unwrapModel();
    Eigen::MatrixXd getProperties(QList<KCL::AbstractElement*> const& elements, VariableType type);
    void setProperties(Eigen::MatrixXd const& properties, QList<KCL::AbstractElement*>& elements, VariableType type);
    void wrapProperties(Eigen::MatrixXd const& properties, VariableType type);
    Eigen::MatrixXd unwrapProperties(int& iParameter, Eigen::MatrixXd const& initProperties, VariableType type);
    QList<ElementMap> getSurfaceElements(KCL::Model& model);
    QMap<VariableType, QList<int>> getVariableIndices();
    QMap<KCL::ElementType, QList<VariableType>> getElementVariables();

private:
    KCL::Model mInitModel;
    QList<Selection> mSelections;
    Constraints mConstraints;
    QList<double> mParamValues;
    QList<double> mParamScales;
    QList<PairDouble> mParamBounds;
};
}

#endif // OPTIMSOLVER_H
