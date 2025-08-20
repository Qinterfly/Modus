#include <kcl/model.h>
#include <QDebug>
#include <QObject>

#include "optimsolver.h"

using namespace Backend::Core;
using namespace KCL;

OptimData::OptimData()
{
}

bool OptimData::isEmpty() const
{
    return indices.size() == 0;
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
    mElements.clear();

    // Check if the optimization data is valid
    if (!data.isValid())
    {
        qWarning() << QObject::tr("Optimization data is not valid");
        return;
    }

    // Slice the selected elements
    // QList<Selection> selections = data.selector.allSelections();
    // mElements.resize(model.surfaces.size());
    // for (auto const& item : selections)
    // {
    //     AbstractElement* pElement = model.surfaces[item.iSurface].element(item.type, item.iElement);
    //     mElements[selection.iSurface][selection.type].push_back();
    // }

    // Wrap the model

    // wrap(model, data.selector, data.constraints);
}

//! Wrap the model to the vector of parameters
void OptimSolver::wrap(KCL::Model const& model, Selector const& selector, Constraints const& constraints)
{
    int numSets = selector.numSets();
    // Loop through all the selection sets
    for (int iSet = 0; iSet != numSets; ++iSet)
    {
        SelectionSet const& set = selector.get(iSet);
        auto const& selections = set.selections();
        // Loop through all the selections
        for (auto [selection, flag] : selections.asKeyValueRange())
        {
            if (!flag)
                continue;
            AbstractElement const* pElement = model.surfaces[selection.iSurface].element(selection.type, selection.iElement);
        }
    }
}
