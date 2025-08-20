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
    QList<int> const beamIndices = {4, 5, 6, 7};
    mVariableIndices[VariableType::kBendingStiffness] = beamIndices;
    mVariableIndices[VariableType::kTorsionalStiffness] = beamIndices;
    mVariableIndices[VariableType::kYoungsModulus1] = {12};
    mVariableIndices[VariableType::kYoungsModulus2] = {17};
    mVariableIndices[VariableType::kShearModulus] = {14};
    mVariableIndices[VariableType::kPoissonRatio] = {13};
    mVariableIndices[VariableType::kSpringStiffness] = {};
}

void OptimSolver::solve(KCL::Model const& model, OptimData const& data, OptimOptions const& options)
{
    // Check if the optimization data is valid
    if (!data.isValid())
    {
        qWarning() << QObject::tr("Optimization data is not valid");
        return;
    }

    // Initialize the fields
    mModel = model;
    mConstraints = data.constraints;

    // Slice the elements for updating
    mSurfaceElements.clear();
    mSurfaceElements.resize(mModel.surfaces.size());
    QList<Selection> const selections = data.selector.allSelections();
    for (auto const& selection : selections)
    {
        AbstractElement* pElement = mModel.surfaces[selection.iSurface].element(selection.type, selection.iElement);
        mSurfaceElements[selection.iSurface][selection.type].push_back(pElement);
    }

    // Wrap the model
    wrapModel();
}

//! Wrap the model parameters according to the constraints
void OptimSolver::wrapModel()
{
    int numSurfaces = mSurfaceElements.size();
    for (int iSurface = 0; iSurface != numSurfaces; ++iSurface)
    {
        ElementMap elementMap = mSurfaceElements[iSurface];
        QList<KCL::ElementType> types = elementMap.keys();
        int numTypes = types.size();
        for (int iType = 0; iType != numTypes; ++iType)
        {
            KCL::ElementType type = types[iType];
            QList<AbstractElement*> const& elements = elementMap[type];
            switch (type)
            {
            case KCL::BI:
                getProperties(elements, VariableType::kBendingStiffness);
                break;
            default:
                break;
            }
        }
    }
}

//! Retrieve element properties by indices
MatrixXd OptimSolver::getProperties(QList<KCL::AbstractElement*> const& elements, VariableType type)
{
    MatrixXd result;
    if (mConstraints.isEnabled(type))
    {
        QList<int> indices = mVariableIndices[type];
        int numIndices = indices.size();
        int numElements = elements.size();
        result.resize(numElements, numIndices);
        for (int i = 0; i != numElements; ++i)
        {
            VecN values = elements[i]->get();
            for (int j = 0; j != numIndices; ++j)
                result(i, j) = values[indices[j]];
        }
    }
    return result;
}
