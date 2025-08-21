#include <kcl/model.h>
#include <QDebug>
#include <QObject>

#include "constants.h"
#include "mathutility.h"
#include "optimsolver.h"

using namespace Backend;
using namespace Backend::Core;
using namespace KCL;

QList<double> getStiffnessVector(KCL::SpringDamper const* pElement);

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

    // Unwrap the model
    KCL::Model model = unwrapModel();
}

//! Wrap the model parameters according to the constraints
void OptimSolver::wrapModel()
{
    // Clear the previous parameters
    mParamValues.clear();
    mParamScales.clear();
    mParamBounds.clear();

    // Obtain the selected elements
    auto surfaceElements = getSurfaceElements(mInitModel);

    // Process the elastic surfaces
    int numSurfaces = surfaceElements.size();
    auto elementVariables = getElementVariables();
    for (int iSurface = 0; iSurface != numSurfaces; ++iSurface)
    {
        if (!surfaceElements.contains(iSurface))
            continue;
        ElementMap const& elementMap = surfaceElements[iSurface];
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

    // Process the special surface
    if (surfaceElements.contains(Constants::skISpecialSurface))
    {
        ElementMap elementMap = surfaceElements[Constants::skISpecialSurface];
        if (elementMap.contains(KCL::PR))
        {
            QList<AbstractElement*> const& elements = elementMap[KCL::PR];
            int numElements = elements.size();
            for (int iElement = 0; iElement != numElements; ++iElement)
            {
                SpringDamper* pElement = (SpringDamper*) elements[iElement];
                QList<bool> mask;
                MatrixXd properties = getProperties(pElement, mask);
                wrapProperties(properties, VariableType::kSpringStiffness);
            }
        }
    }
}

//! Unwrap the model parameters according to the constraints
KCL::Model OptimSolver::unwrapModel()
{
    int iParameter = -1;
    KCL::Model model = mInitModel;

    // Obtain the selected elements
    auto surfaceElements = getSurfaceElements(model);

    // Process the elastic surfaces
    int numSurfaces = surfaceElements.size();
    auto elementVariables = getElementVariables();
    for (int iSurface = 0; iSurface != numSurfaces; ++iSurface)
    {
        if (!surfaceElements.contains(iSurface))
            continue;
        ElementMap& elementMap = surfaceElements[iSurface];
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

    // Process the special surface
    if (surfaceElements.contains(Constants::skISpecialSurface))
    {
        ElementMap elementMap = surfaceElements[Constants::skISpecialSurface];
        if (elementMap.contains(KCL::PR))
        {
            QList<AbstractElement*> const& elements = elementMap[KCL::PR];
            int numElements = elements.size();
            for (int iElement = 0; iElement != numElements; ++iElement)
            {
                SpringDamper* pElement = (SpringDamper*) elements[iElement];
                QList<bool> mask;
                MatrixXd initProperties = getProperties(pElement, mask);
                MatrixXd properties = unwrapProperties(iParameter, initProperties, VariableType::kSpringStiffness);
                setProperties(properties, pElement, mask);
            }
        }
    }

    // Check if all the parameters are processed
    if (iParameter != mParamValues.size() - 1)
        qWarning() << QObject::tr("Some parameters were not unwrapped during updating. Check the results carefully");
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

//! Retrieve spring properties
MatrixXd OptimSolver::getProperties(KCL::SpringDamper* pElement, QList<bool>& mask)
{
    MatrixXd result;
    VariableType type = VariableType::kSpringStiffness;

    // Check if springs are enabled for updating
    if (!mConstraints.isEnabled(type))
        return result;

    // Retrieve the stiffness matrix
    auto stiffness = pElement->stiffness;
    int numMat = stiffness.size();
    int numValues = pElement->iSwitch;

    // Slice the values
    QList<double> values(numValues);
    if (numValues == numMat)
    {
        for (int i = 0; i != numMat; ++i)
            values[i] = stiffness[i][i];
    }
    else
    {
        int k = 0;
        for (int i = 0; i != numMat; ++i)
        {
            for (int j = 0; j != numMat; ++j)
            {
                values[k] = stiffness[i][j];
                ++k;
            }
        }
    }

    // Build up the mask of stiffness values
    auto limits = mConstraints.limits(type);
    mask.resize(numValues);
    int numProperties = 0;
    for (int i = 0; i != numValues; ++i)
    {
        double value = values[i];
        bool flag = false;
        if (value <= limits.second)
        {
            if (mConstraints.isNonzero(type))
            {
                if (value > std::numeric_limits<double>::epsilon())
                    flag = true;
            }
            else
            {
                flag = true;
            }
        }
        if (flag)
            ++numProperties;
        mask[i] = flag;
    }

    // Slice the enabled values
    result.resize(1, numProperties);
    numProperties = 0;
    for (int i = 0; i != numValues; ++i)
    {
        if (mask[i])
        {
            result(0, numProperties) = values[i];
            ++numProperties;
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

//! Set spring properties by mask
void OptimSolver::setProperties(Eigen::MatrixXd const& properties, KCL::SpringDamper* pElement, QList<bool> const& mask)
{
    KCL::Mat6x6& stiffness = pElement->stiffness;
    int numMat = stiffness.size();
    int iSlice = 0;
    if (mask.size() == numMat)
    {
        for (int i = 0; i != numMat; ++i)
        {
            if (mask[i])
            {
                stiffness[i][i] = properties(0, iSlice);
                ++iSlice;
            }
        }
    }
    else
    {
        int iMask = 0;
        for (int i = 0; i != numMat; ++i)
        {
            for (int j = 0; j != numMat; ++j)
            {
                if (mask[iMask])
                {
                    stiffness[i][j] = properties(0, iSlice);
                    ++iSlice;
                }
                ++iMask;
            }
        }
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

    // Check if the logarithmic scale could be applied
    for (int i = 0; i != numValues; ++i)
    {
        if (scales[i] == 0.0 && values[i] <= std::numeric_limits<double>::epsilon())
            scales[i] = 1.0;
    }

    // Apply the scales
    for (int i = 0; i != numValues; ++i)
    {
        double factor = scales[i];
        if (factor != 0)
        {
            values[i] *= factor;
            bounds[i].first *= factor;
            bounds[i].second *= factor;
        }
        else
        {
            values[i] = log10(values[i]);
            bounds[i].first = log10(bounds[i].first);
            bounds[i].second = log10(bounds[i].second);
        }
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
            double scale = mParamScales[iStart + i];
            double value = mParamValues[iStart + i];
            if (scale != 0)
                value /= scale;
            else
                value = std::pow(10, value);
            double factor = value / properties(i, indices(i));
            for (int j = 0; j != numCols; ++j)
                properties(i, j) *= factor;
        }
    }
    else if (isMultiply)
    {
        iEnd = iStart;
        double scale = mParamScales[iEnd];
        double value = mParamValues[iEnd];
        if (scale != 0)
            value /= scale;
        else
            value = std::pow(10, value);
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
                double scale = mParamScales[iStart + k];
                double value = mParamValues[iStart + k];
                if (scale != 0)
                    value /= scale;
                else
                    value = std::pow(10, value);
                properties(i, j) = value;
                ++k;
            }
        }
    }
    iParameter = iEnd;

    return properties;
}

//! Retrieve surface elements
QMap<int, ElementMap> OptimSolver::getSurfaceElements(KCL::Model& model)
{
    QMap<int, ElementMap> result;
    int numSelections = mSelections.size();
    for (int i = 0; i != numSelections; ++i)
    {
        Selection const& selection = mSelections[i];
        AbstractElement* pElement = nullptr;
        if (selection.iSurface == Constants::skISpecialSurface)
            pElement = model.specialSurface.element(selection.type, selection.iElement);
        else
            pElement = model.surfaces[selection.iSurface].element(selection.type, selection.iElement);
        if (pElement)
            result[selection.iSurface][selection.type].push_back(pElement);
    }
    return result;
}

//! Retrieve indices of variable associated data of elements
QMap<VariableType, QList<int>> OptimSolver::getVariableIndices()
{
    QMap<VariableType, QList<int>> result;
    result[VariableType::kBeamStiffness] = {4, 5, 6, 7};
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
    result[KCL::BI] = {VariableType::kBeamStiffness};
    result[KCL::DB] = {VariableType::kBeamStiffness};
    result[KCL::BK] = {VariableType::kBeamStiffness};
    result[KCL::PN] = {VariableType::kThickness, VariableType::kYoungsModulus1, VariableType::kPoissonRatio};
    result[KCL::OP] = {VariableType::kThickness, VariableType::kYoungsModulus1, VariableType::kYoungsModulus2, VariableType::kShearModulus,
                       VariableType::kPoissonRatio};
    return result;
}
