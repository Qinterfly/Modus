#include <kcl/model.h>
#include <QDebug>
#include <QObject>

#include "mathutility.h"
#include "optimsolver.h"

using namespace Backend;
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
    mVariableIndices[VariableType::kBendingStiffness] = {4, 5, 6, 7};
    mVariableIndices[VariableType::kTorsionalStiffness] = {4, 5, 6, 7};
    mVariableIndices[VariableType::kYoungsModulus1] = {12};
    mVariableIndices[VariableType::kYoungsModulus2] = {17};
    mVariableIndices[VariableType::kShearModulus] = {14};
    mVariableIndices[VariableType::kPoissonRatio] = {13};
    mElementVariables[KCL::BI] = {VariableType::kBendingStiffness};
    mElementVariables[KCL::DB] = {VariableType::kBendingStiffness};
    mElementVariables[KCL::BK] = {VariableType::kTorsionalStiffness};
    mElementVariables[KCL::PN] = {VariableType::kThickness, VariableType::kYoungsModulus1, VariableType::kPoissonRatio};
    mElementVariables[KCL::OP] = {VariableType::kThickness, VariableType::kYoungsModulus1, VariableType::kYoungsModulus2,
                                  VariableType::kShearModulus, VariableType::kPoissonRatio};
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
    mParamValues.clear();
    mParamScales.clear();
    mParamBounds.clear();
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
            if (mElementVariables.contains(type))
            {
                QList<VariableType> const& variables = mElementVariables[type];
                int numVariables = variables.size();
                for (int iVariable = 0; iVariable != numVariables; ++iVariable)
                {
                    auto variable = variables[iVariable];
                    MatrixXd properties = getProperties(elements, variable);
                    wrapProperties(properties, variable);
                }
            }
        }
    }
}

//! Retrieve element properties by indices
MatrixXd OptimSolver::getProperties(QList<KCL::AbstractElement*> const& elements, VariableType type)
{
    MatrixXd result;
    if (mConstraints.isEnabled(type) && mVariableIndices.contains(type))
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

//! Vectorize properties
void OptimSolver::wrapProperties(Eigen::MatrixXd const& properties, VariableType type)
{
    // Check if there are any properties to vectorize
    if (properties.size() == 0)
        return;

    // Acquire the state and constraints
    bool isUnite = mConstraints.isUnited(type);
    bool isMultiply = mConstraints.isMultiplied(type);
    double scale = mConstraints.scale(type);
    PairDouble limits = mConstraints.limits(type);

    // Find the indices of maximum valies
    int numRows = properties.rows();
    int numCols = properties.cols();
    QList<int> indicesRowMax(numRows);
    for (int i = 0; i != numRows; ++i)
    {
        int iMax = -1;
        double absMax = 0.0;
        for (int j = 0; j != numCols; ++j)
        {
            double absValue = std::abs(properties(i, j));
            if (absValue > absMax)
            {
                absMax = absValue;
                iMax = j;
            }
        }
        indicesRowMax[i] = iMax;
    }

    // Slice property values
    QList<double> values;
    if (isUnite)
    {
        values.resize(numRows);
        for (int i = 0; i != numRows; ++i)
            values[i] = properties(i, indicesRowMax[i]);
    }
    else if (isMultiply)
    {
        values.push_back(properties(0, indicesRowMax[0]));
    }
    else
    {
        auto vecProperties = properties.reshaped<RowMajor>();
        values = QList<double>(vecProperties.begin(), vecProperties.end());
    }

    // Duplicate scales and limits
    int numValues = values.size();
    QList<double> scales(numValues);
    QList<PairDouble> bounds(numValues);
    scales.fill(scale);
    bounds.fill(limits);

    // Append the result
    mParamValues = Utility::combine(mParamValues, values);
    mParamScales = Utility::combine(mParamScales, scales);
    mParamBounds = Utility::combine(mParamBounds, bounds);
}
