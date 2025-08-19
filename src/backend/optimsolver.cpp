#include <kcl/model.h>
#include <QDebug>
#include <QObject>

#include "optimsolver.h"

using namespace Backend::Core;

OptimData::OptimData()
{
}

bool OptimData::isEmpty() const
{
    return indices.size() > 0;
}

bool OptimData::isValid() const
{
    int numModes = indices.size();
    bool isSlice = numModes == weights.size() && numModes <= targetSolution.frequencies().size()
                   && numModes <= targetSolution.modeShapes().size();
    return !isEmpty() && isSlice;
}

void OptimData::resize(int numModes)
{
    indices.resize(numModes);
    weights.resize(numModes);
}

OptimOptions::OptimOptions()
    : isExclusiveMAC(true)
    , penaltyMAC(20)
{
}

OptimSolver::OptimSolver()
{
}

void OptimSolver::solve(KCL::Model const& model, OptimData const& data, OptimOptions const& options)
{
    // Check if the optimization data is valid
    if (!data.isValid())
    {
        qWarning() << QObject::tr("Optimization data is not valid");
        return;
    }

    // TODO
}
