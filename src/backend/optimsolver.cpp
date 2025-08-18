#include <kcl/model.h>
#include <QDebug>
#include <QObject>

#include "optimsolver.h"

using namespace Backend::Core;

static int const skNumDirections = 3;

OptimOptions::OptimOptions()
{
    isExclusiveMAC = true;
    penaltyMAC = 20;
}

bool OptimOptions::isValid() const
{
    int numIndices = indices.size();
    return numIndices == frequencies.size() && numIndices == modeShapes.size() && numIndices == weights.size();
}

void OptimOptions::resize(int numDOFs, int numModes)
{
    indices.resize(numModes);
    frequencies.resize(numModes);
    modeShapes.resize(numModes);
    for (Eigen::MatrixXd& item : modeShapes)
        item.resize(numDOFs, skNumDirections);
    weights.resize(numModes);
}

OptimSolver::OptimSolver()
{
}

void OptimSolver::solve(KCL::Model const& model, OptimOptions const& options)
{
    // Check if the options are valid
    if (!options.isValid())
    {
        qWarning() << QObject::tr("The optimization options are not valid");
        return;
    }
}
