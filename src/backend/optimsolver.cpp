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
}

//! Perform the updating
void OptimSolver::solve(KCL::Model const& initModel, OptimData const& data, OptimOptions const& options)
{
    // Check if the optimization data is valid
    if (!data.isValid())
    {
        qWarning() << QObject::tr("Optimization data is not valid");
        return;
    }

    // Initialize the fields
    mInitModel = initModel;
    mSelections = data.selector.allSelections();
    mConstraints = data.constraints;

    // Wrap the model
    wrapModel();

    // Change the parameters
    for (double& value : mParamValues)
        value *= 1.1;

    // Unwrap the model
    KCL::Model model = unwrapModel();
}

//! Wrap the model parameters according to the constraints
void OptimSolver::wrapModel()
{
    mParamValues.clear();
    mParamScales.clear();
    mParamBounds.clear();
    auto surfaceElements = getSurfaceElements(mInitModel);
    int numSurfaces = surfaceElements.size();
    auto elementVariables = getElementVariables();
    for (int iSurface = 0; iSurface != numSurfaces; ++iSurface)
    {
        ElementMap elementMap = surfaceElements[iSurface];
        QList<KCL::ElementType> types = elementMap.keys();
        int numTypes = types.size();
        for (int iType = 0; iType != numTypes; ++iType)
        {
            KCL::ElementType type = types[iType];
            QList<AbstractElement*> const& elements = elementMap[type];
            if (elementVariables.contains(type))
            {
                QList<VariableType> const& variables = elementVariables[type];
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

//! Unwrap the model parameters according to the constraints
KCL::Model OptimSolver::unwrapModel()
{
    int iParameter = -1;
    KCL::Model model = mInitModel;
    auto surfaceElements = getSurfaceElements(model);
    int numSurfaces = surfaceElements.size();
    auto elementVariables = getElementVariables();
    for (int iSurface = 0; iSurface != numSurfaces; ++iSurface)
    {
        ElementMap elementMap = surfaceElements[iSurface];
        QList<KCL::ElementType> types = elementMap.keys();
        int numTypes = types.size();
        for (int iType = 0; iType != numTypes; ++iType)
        {
            KCL::ElementType type = types[iType];
            QList<AbstractElement*>& elements = elementMap[type];
            if (elementVariables.contains(type))
            {
                QList<VariableType> const& variables = elementVariables[type];
                int numVariables = variables.size();
                for (int iVariable = 0; iVariable != numVariables; ++iVariable)
                {
                    auto variable = variables[iVariable];
                    MatrixXd initProperties = getProperties(elements, variable);
                    MatrixXd properties = unwrapProperties(iParameter, initProperties, variable);
                    setProperties(properties, elements, variable);
                }
            }
        }
    }
    return model;
}

//! Retrieve element properties by indices
MatrixXd OptimSolver::getProperties(QList<KCL::AbstractElement*> const& elements, VariableType type)
{
    MatrixXd result;
    auto variableIndices = getVariableIndices();
    if (mConstraints.isEnabled(type) && variableIndices.contains(type))
    {
        QList<int> indices = variableIndices[type];
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

//! Set element properties by indices
void OptimSolver::setProperties(Eigen::MatrixXd const& properties, QList<KCL::AbstractElement*>& elements, VariableType type)
{
    auto variableIndices = getVariableIndices();
    QList<int> indices = variableIndices[type];
    int numIndices = indices.size();
    int numElements = elements.size();
    for (int i = 0; i != numElements; ++i)
    {
        VecN values = elements[i]->get();
        for (int j = 0; j != numIndices; ++j)
            values[indices[j]] = properties(i, j);
        elements[i]->set(values);
    }
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
    auto indices = Utility::rowIndicesAbsMax(properties);

    // Slice property values
    QList<double> values;
    int numRows = properties.rows();
    int numCols = properties.cols();
    if (isUnite)
    {
        values.resize(numRows);
        for (int i = 0; i != numRows; ++i)
            values[i] = properties(i, indices[i]);
    }
    else if (isMultiply)
    {
        values.push_back(properties(0, indices[0]));
    }
    else
    {
        values.resize(numRows * numCols);
        int k = 0;
        for (int i = 0; i != numRows; ++i)
        {
            for (int j = 0; j != numCols; ++j)
            {
                values[k] = properties(i, j);
                ++k;
            }
        }
    }

    // Duplicate scales and limits
    int numValues = values.size();
    QList<double> scales(numValues);
    QList<PairDouble> bounds(numValues);
    scales.fill(scale);
    bounds.fill(limits);

    // Apply the scales
    for (int i = 0; i != numValues; ++i)
    {
        double factor = scales[i];
        values[i] *= factor;
        bounds[i].first *= factor;
        bounds[i].second *= factor;
    }

    // Append the result
    mParamValues = Utility::combine(mParamValues, values);
    mParamScales = Utility::combine(mParamScales, scales);
    mParamBounds = Utility::combine(mParamBounds, bounds);
}

//! Unwrap properties from a vector
MatrixXd OptimSolver::unwrapProperties(int& iParameter, MatrixXd const& initProperties, VariableType type)
{
    MatrixXd properties = initProperties;

    // Check if there are any variables to slice
    if (properties.size() == 0)
        return properties;

    // Acquire the state and constraints
    bool isUnite = mConstraints.isUnited(type);
    bool isMultiply = mConstraints.isMultiplied(type);

    // Find the indices of maximum valies
    auto indices = Utility::rowIndicesAbsMax(properties);

    // Slice property values
    int numRows = properties.rows();
    int numCols = properties.cols();
    int iStart = iParameter + 1;
    int iEnd;
    if (isUnite)
    {
        int numValues = numRows;
        iEnd = iParameter + numValues;
        for (int i = 0; i != numValues; ++i)
        {
            double value = mParamValues[iStart + i] / mParamScales[iStart + i];
            double factor = value / properties(i, indices(i));
            for (int j = 0; j != numCols; ++j)
                properties(i, j) *= factor;
        }
    }
    else if (isMultiply)
    {
        iEnd = iStart;
        double value = mParamValues[iEnd] / mParamScales[iEnd];
        double factor = value / properties(0, indices(0));
        properties *= factor;
    }
    else
    {
        int numValues = numRows * numCols;
        iEnd = iParameter + numValues;
        int k = 0;
        for (int i = 0; i != numRows; ++i)
        {
            for (int j = 0; j != numCols; ++j)
            {
                double value = mParamValues[iStart + k] / mParamScales[iStart + k];
                properties(i, j) = value;
                ++k;
            }
        }
    }
    iParameter = iEnd;

    return properties;
}

//! Retrieve surface elements
QList<ElementMap> OptimSolver::getSurfaceElements(KCL::Model& model)
{
    int numSurfaces = model.surfaces.size();
    QList<ElementMap> result(numSurfaces);
    int numSelections = mSelections.size();
    for (int i = 0; i != numSelections; ++i)
    {
        Selection const& selection = mSelections[i];
        AbstractElement* pElement = model.surfaces[selection.iSurface].element(selection.type, selection.iElement);
        result[selection.iSurface][selection.type].push_back(pElement);
    }
    return result;
}

//! Retrieve indices of variable associated data of elements
QMap<VariableType, QList<int>> OptimSolver::getVariableIndices()
{
    QMap<VariableType, QList<int>> result;
    result[VariableType::kBendingStiffness] = {4, 5, 6, 7};
    result[VariableType::kTorsionalStiffness] = {4, 5, 6, 7};
    result[VariableType::kYoungsModulus1] = {12};
    result[VariableType::kYoungsModulus2] = {17};
    result[VariableType::kShearModulus] = {14};
    result[VariableType::kPoissonRatio] = {13};
    return result;
}

//! Retrieve a group of variables associated with an element
QMap<KCL::ElementType, QList<VariableType>> OptimSolver::getElementVariables()
{
    QMap<KCL::ElementType, QList<VariableType>> result;
    result[KCL::BI] = {VariableType::kBendingStiffness};
    result[KCL::DB] = {VariableType::kBendingStiffness};
    result[KCL::BK] = {VariableType::kTorsionalStiffness};
    result[KCL::PN] = {VariableType::kThickness, VariableType::kYoungsModulus1, VariableType::kPoissonRatio};
    result[KCL::OP] = {VariableType::kThickness, VariableType::kYoungsModulus1, VariableType::kYoungsModulus2, VariableType::kShearModulus,
                       VariableType::kPoissonRatio};
    return result;
}
