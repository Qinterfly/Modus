#include <kcl/model.h>
#include <QDebug>
#include <QObject>
#include <QThread>
#include <QXmlStreamWriter>

#include "constants.h"
#include "fileutility.h"
#include "mathutility.h"
#include "optimsolver.h"

using namespace Backend;
using namespace Backend::Core;
using namespace KCL;

QList<double> getStiffnessVector(SpringDamper const* pElement);

ObjectiveFunctor::ObjectiveFunctor(Eigen::VectorXi const& targetIndices, Eigen::VectorXd const& targetWeights, OptimOptions const& options,
                                   UnwrapFun unwrapFun, SolverFun solverFun, CompareFun compareFun)
    : mTargetIndices(targetIndices)
    , mTargetWeights(targetWeights)
    , mOptions(options)
    , mUnwrapFun(unwrapFun)
    , mSolverFun(solverFun)
    , mCompareFun(compareFun)
{
}

//! Compute the residuals
bool ObjectiveFunctor::operator()(double const* const* parameters, double* residuals) const
{
    Model model = mUnwrapFun(*parameters);

    // Obtain the solution
    ModalSolution solution = mSolverFun(model);
    if (solution.isEmpty())
        return false;

    // Compare the solution with the target one
    ModalComparison comparison = mCompareFun(solution);
    if (!comparison.isValid())
        return false;

    // Set the residuals
    int numTargets = mTargetIndices.size();
    int iResidual = 0;
    for (int i = 0; i != numTargets; ++i)
    {
        double errorFrequency = comparison.errorFrequencies[i];
        double errorMAC = comparison.errorsMAC[i];
        double weight = mTargetWeights[i];
        if (weight > std::numeric_limits<double>::epsilon())
        {
            residuals[iResidual] = weight * (std::pow(errorFrequency, 2.0) + mOptions.penaltyMAC * std::pow(errorMAC, 2.0));
            ++iResidual;
        }
    }

    return true;
}

OptimCallback::OptimCallback(QList<double>& parameterValues, Eigen::VectorXi const& targetIndices, Eigen::VectorXd const& targetWeights,
                             ModalSolution const& targetSolution, OptimOptions const& options, UnwrapFun unwrapFun, SolverFun solverFun,
                             CompareFun compareFun)
    : mParameterValues(parameterValues)
    , mTargetIndices(targetIndices)
    , mTargetWeights(targetWeights)
    , mTargetSolution(targetSolution)
    , mOptions(options)
    , mUnwrapFun(unwrapFun)
    , mSolverFun(solverFun)
    , mCompareFun(compareFun)
{
}

//! Display the iteration information
ceres::CallbackReturnType OptimCallback::operator()(ceres::IterationSummary const& summary)
{
    // Check if the user requested to stop the solver
    if (QThread::currentThread()->isInterruptionRequested())
        return ceres::SOLVER_ABORT;

    // Obtain the solution
    Model model = mUnwrapFun(mParameterValues.data());
    ModalSolution modalSolution = mSolverFun(model);
    if (modalSolution.isEmpty())
        return ceres::SOLVER_CONTINUE;

    // Compare the solution with the target one
    ModalComparison modalComparison = mCompareFun(modalSolution);
    if (!modalComparison.isValid())
        return ceres::SOLVER_CONTINUE;

    // Print the header
    QString message;
    QTextStream stream(&message);
    if (summary.iteration == 0)
        stream << std::format("{:^8} {:>6} {:>11} {:>10} {:>10}", "Iter", "Fun", "Diff", "Grad", "Step").c_str() << Qt::endl;
    auto constexpr headFormat = "{:^7d} {:10.3e} {:10.3e} {:10.3e} {:10.3e}";
    stream << std::format(headFormat, summary.iteration, summary.cost, summary.cost_change, summary.gradient_max_norm, summary.step_norm).c_str();
    stream << Qt::endl << Qt::endl;

    // Print the data
    int numTargets = mTargetIndices.size();
    auto constexpr dataFormat = "  {:^3d} {:^3d} {:^9.3f} {:^6.3g} {:^6.3g} {:10.2e}";
    double maxError = 0;
    for (int i = 0; i != numTargets; ++i)
    {
        int iTargetMode = mTargetIndices[i];
        int iCurrentMode = modalComparison.pairs[i].first;
        double MAC = modalComparison.pairs[i].second;
        double targetFrequency = mTargetSolution.frequencies[i];
        double currentFrequency = modalSolution.frequencies[iCurrentMode];
        double error = modalComparison.errorFrequencies[i] * 100;
        double weight = mTargetWeights[i];
        maxError = std::max(maxError, std::abs(error));
        stream << std::format(dataFormat, 1 + iTargetMode, 1 + iCurrentMode, MAC, currentFrequency, targetFrequency, error).c_str();
        if (weight < std::numeric_limits<double>::epsilon())
            stream << std::format("{:^10}", "skip").c_str();
        stream << Qt::endl;
    }

    // Indicate that the iteration is finished
    OptimSolution solution;
    solution.iteration = summary.iteration;
    solution.isSuccess = summary.step_is_successful;
    solution.duration = summary.iteration_time_in_seconds;
    solution.cost = summary.cost;
    solution.model = model;
    solution.modalSolution = modalSolution;
    solution.modalComparison = modalComparison;
    emit iterationFinished(solution);
    emit logRequested(message);

    if (maxError < mOptions.maxRelError)
        return ceres::SOLVER_TERMINATE_SUCCESSFULLY;
    return ceres::SOLVER_CONTINUE;
}

OptimSolver::OptimSolver()
{
}

OptimSolver::~OptimSolver()
{
}

OptimSolver::OptimSolver(OptimSolver const& another)
    : problem(another.problem)
    , options(another.options)
    , solutions(another.solutions)
{
}

OptimSolver::OptimSolver(OptimSolver&& another)
{
    mID = std::move(another.mID);
    problem = std::move(another.problem);
    options = std::move(another.options);
    solutions = std::move(another.solutions);
}

OptimSolver& OptimSolver::operator=(OptimSolver const& another)
{
    problem = another.problem;
    options = another.options;
    solutions = another.solutions;
    return *this;
}

ISolver::Type OptimSolver::type() const
{
    return ISolver::kOptim;
}

ISolver* OptimSolver::clone() const
{
    OptimSolver* pSolver = new OptimSolver;
    *pSolver = *this;
    return pSolver;
}

void OptimSolver::clear()
{
    mInitModel = KCL::Model();
    mSelections.clear();
    mConstraints = Constraints();
    mParameterScales.clear();
    mParameterBounds.clear();
    log = QString();
}

//! Perform the updating
void OptimSolver::solve()
{
    // Clear the previous solution
    clear();

    // Intialize the resulting set
    appendLog("Solver started\n");
    solutions.clear();
    solutions.reserve(options.maxNumIterations);

    // Check if the optimization data is valid
    if (!problem.isValid())
    {
        appendLog("Optimization data is not valid\n", QtMsgType::QtCriticalMsg);
        return;
    }

    // Initialize the fields
    mInitModel = problem.model;
    mSelections = problem.selector.allSelections();
    mConstraints = problem.constraints;

    // Set the model parameters
    QString message;
    message.append("* Preparing the model parameters to be updated\n");
    setModelParameters();

    // Wrap the model
    QList<double> parameterValues = wrapModel();
    int numParameters = parameterValues.size();
    message.append(QString("Number of parameters: %1\n").arg(numParameters));

    // Count the number of residuals
    int numResiduals = 0;
    for (auto weight : problem.targetWeights)
    {
        if (weight > std::numeric_limits<double>::epsilon())
            ++numResiduals;
    }
    message.append(QString("Number of residuals: %1\n").arg(numResiduals));
    appendLog(message);

    // Create the auxiliary function
    UnwrapFun unwrapFun = [this, &numParameters](const double* const x)
    {
        QList<double> params(x, x + numParameters);
        return unwrapModel(params);
    };
    SolverFun solverFun = [this](Model const& model)
    {
        std::ostringstream stream;
        std::function<EigenSolution()> fun = [&model, &stream]() { return model.solveEigen(stream); };
        return Utility::solve(fun, options.timeoutIteration);
    };
    CompareFun compareFun = [this](ModalSolution const& solution)
    { return mTargetSolution.compare(solution, problem.targetIndices, mTargetMatches, options.minMAC); };

    // Set up the targets
    setTargetSolution(solverFun);
    if (mTargetSolution.isEmpty())
    {
        emit solverFinished();
        return;
    }
    setTargetMatches();

    // Assign options to compute Jacobian
    ceres::NumericDiffOptions diffOptions;
    diffOptions.relative_step_size = options.diffStepSize;

    // Create the cost function
    appendLog("* Constructing the cost function\n");
    ObjectiveFunctor functor(problem.targetIndices, problem.targetWeights, options, unwrapFun, solverFun, compareFun);
    auto* costFunction = new ceres::DynamicNumericDiffCostFunction<ObjectiveFunctor, ceres::FORWARD>(&functor, ceres::DO_NOT_TAKE_OWNERSHIP,
                                                                                                     diffOptions);
    costFunction->AddParameterBlock(numParameters);
    costFunction->SetNumResiduals(numResiduals);

    // Set the problem
    double* values = parameterValues.data();
    ceres::Problem ceresProblem;
    ceresProblem.AddResidualBlock(costFunction, nullptr, values);

    // Set the boundaries
    for (int i = 0; i != numParameters; ++i)
    {
        PairDouble const& bounds = mParameterBounds[i];
        ceresProblem.SetParameterLowerBound(values, i, bounds.first);
        ceresProblem.SetParameterUpperBound(values, i, bounds.second);
    }

    // Assign the solver settings
    ceres::Solver::Options ceresOptions;
    ceresOptions.max_num_iterations = options.maxNumIterations;
    ceresOptions.num_threads = options.numThreads;
    ceresOptions.minimizer_type = ceres::TRUST_REGION;
    ceresOptions.linear_solver_type = ceres::DENSE_QR;
    ceresOptions.use_nonmonotonic_steps = true;
    ceresOptions.logging_type = ceres::SILENT;
    ceresOptions.minimizer_progress_to_stdout = false;

    // Set the callback functions
    ceresOptions.update_state_every_iteration = true;
    OptimCallback callback(parameterValues, problem.targetIndices, problem.targetWeights, mTargetSolution, options, unwrapFun, solverFun,
                           compareFun);
    connect(&callback, &OptimCallback::iterationFinished, this,
            [this](OptimSolution solution)
            {
                solutions.push_back(solution);
                emit iterationFinished(solution);
            });
    connect(&callback, &OptimCallback::logRequested, this, [this](QString const& message) { appendLog(message); });
    ceresOptions.callbacks.push_back(&callback);

    // Solve the problem
    appendLog("* Running optimization process\n");
    ceres::Solver::Summary ceresSummary;
    ceres::Solve(ceresOptions, &ceresProblem, &ceresSummary);
    if (!solutions.empty())
    {
        OptimSolution& lastSolution = solutions.back();
        lastSolution.isSuccess = ceresSummary.IsSolutionUsable();
        lastSolution.message = ceresSummary.message.c_str();
    }
    appendLog("Solver terminated successfully\n");

    // Log the report
    printReport(ceresSummary);

    emit solverFinished();
}

//! Set the target modal solution (compute, if necessary)
void OptimSolver::setTargetSolution(SolverFun solverFun)
{
    mTargetSolution = problem.targetSolution;
    if (mTargetSolution.isEmpty())
    {
        appendLog("* Evaluating the target solution\n");
        mTargetSolution = solverFun(mInitModel);
        int numModes = mTargetSolution.numModes();
        int numTargets = problem.targetIndices.size();
        for (int i = 0; i != numTargets; ++i)
        {
            int iTarget = problem.targetIndices[i];
            if (iTarget >= 0 && iTarget < numModes)
            {
                mTargetSolution.frequencies[iTarget] = problem.targetFrequencies[i];
            }
            else
            {
                appendLog("Could not set the target modal solution\n", QtMsgType::QtCriticalMsg);
                break;
            }
        }
    }
}

//! Set the target matches (fill, if necessary)
void OptimSolver::setTargetMatches()
{
    mTargetMatches = problem.targetMatches;
    if (mTargetMatches.isEmpty())
    {
        int numVertices = mTargetSolution.geometry.numVertices();
        mTargetMatches.resize(numVertices);
        for (int i = 0; i != numVertices; ++i)
            mTargetMatches[i] = {i, i};
    }
}

//! Set model parameters for further updating
void OptimSolver::setModelParameters()
{
    auto pParameters = (AnalysisParameters*) mInitModel.specialSurface.element(KCL::WP);
    pParameters->numLowModes = options.numModes;
}

//! Wrap the model parameters according to the constraints
QList<double> OptimSolver::wrapModel()
{
    QList<double> parameterValues;

    // Clear the previous parameters
    mParameterScales.clear();
    mParameterBounds.clear();

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
        QList<ElementType> types = elementMap.keys();
        int numTypes = types.size();
        for (int iType = 0; iType != numTypes; ++iType)
        {
            ElementType type = types[iType];
            QList<AbstractElement*> const& elements = elementMap[type];
            if (elementVariables.contains(type))
            {
                QList<VariableType> const& variables = elementVariables[type];
                int numVariables = variables.size();
                for (int iVariable = 0; iVariable != numVariables; ++iVariable)
                {
                    auto variable = variables[iVariable];
                    MatrixXd properties = getProperties(elements, variable);
                    wrapProperties(parameterValues, properties, variable);
                }
            }
        }
    }

    // Process the special surface
    if (surfaceElements.contains(Constants::skISpecialSurface))
    {
        ElementMap elementMap = surfaceElements[Constants::skISpecialSurface];
        if (elementMap.contains(PR))
        {
            QList<AbstractElement*> const& elements = elementMap[PR];
            int numElements = elements.size();
            for (int iElement = 0; iElement != numElements; ++iElement)
            {
                SpringDamper* pElement = (SpringDamper*) elements[iElement];
                QList<bool> mask;
                MatrixXd properties = getProperties(pElement, mask);
                wrapProperties(parameterValues, properties, VariableType::kSpringStiffness);
            }
        }
    }
    return parameterValues;
}

//! Unwrap the model parameters according to the constraints
Model OptimSolver::unwrapModel(QList<double> const& parameterValues)
{
    int iParameter = -1;
    Model model = mInitModel;

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
        QList<ElementType> types = elementMap.keys();
        int numTypes = types.size();
        for (int iType = 0; iType != numTypes; ++iType)
        {
            ElementType type = types[iType];
            QList<AbstractElement*>& elements = elementMap[type];
            if (elementVariables.contains(type))
            {
                QList<VariableType> const& variables = elementVariables[type];
                int numVariables = variables.size();
                for (int iVariable = 0; iVariable != numVariables; ++iVariable)
                {
                    auto variable = variables[iVariable];
                    MatrixXd initProperties = getProperties(elements, variable);
                    MatrixXd properties = unwrapProperties(iParameter, parameterValues, initProperties, variable);
                    setProperties(properties, elements, variable);
                }
            }
        }
    }

    // Process the special surface
    if (surfaceElements.contains(Constants::skISpecialSurface))
    {
        ElementMap elementMap = surfaceElements[Constants::skISpecialSurface];
        if (elementMap.contains(PR))
        {
            QList<AbstractElement*> const& elements = elementMap[PR];
            int numElements = elements.size();
            for (int iElement = 0; iElement != numElements; ++iElement)
            {
                SpringDamper* pElement = (SpringDamper*) elements[iElement];
                QList<bool> mask;
                MatrixXd initProperties = getProperties(pElement, mask);
                MatrixXd properties = unwrapProperties(iParameter, parameterValues, initProperties, VariableType::kSpringStiffness);
                setProperties(properties, pElement, mask);
            }
        }
    }

    // Check if all the parameters are processed
    if (iParameter != parameterValues.size() - 1)
        qWarning() << QObject::tr("Some parameters were not unwrapped during updating. Check the results carefully");
    return model;
}

//! Retrieve element properties by indices
MatrixXd OptimSolver::getProperties(QList<AbstractElement*> const& elements, VariableType type)
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
MatrixXd OptimSolver::getProperties(SpringDamper* pElement, QList<bool>& mask)
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
    PairDouble bounds = mConstraints.bounds(type);
    mask.resize(numValues);
    int numProperties = 0;
    for (int i = 0; i != numValues; ++i)
    {
        double value = values[i];
        bool flag = false;
        if (value <= bounds.second)
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
void OptimSolver::setProperties(Eigen::MatrixXd const& properties, QList<AbstractElement*>& elements, VariableType type)
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
void OptimSolver::setProperties(Eigen::MatrixXd const& properties, SpringDamper* pElement, QList<bool> const& mask)
{
    Mat6x6& stiffness = pElement->stiffness;
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
        int k = 0;
        for (int i = 0; i != numMat; ++i)
        {
            for (int j = 0; j != numMat; ++j)
            {
                if (mask[k])
                {
                    stiffness[i][j] = properties(0, iSlice);
                    ++iSlice;
                }
                ++k;
            }
        }
    }
}

//! Vectorize properties
void OptimSolver::wrapProperties(QList<double>& parameterValues, Eigen::MatrixXd const& properties, VariableType type)
{
    // Check if there are any properties to vectorize
    if (properties.size() == 0)
        return;

    // Acquire the state and constraints
    bool isUnite = mConstraints.isUnited(type);
    bool isMultiply = mConstraints.isMultiplied(type);
    bool isNonzero = mConstraints.isNonzero(type);
    double propertyScale = mConstraints.scale(type);
    PairDouble propertyBounds = mConstraints.bounds(type);

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
        for (int i = 0; i != numRows; ++i)
        {
            for (int j = 0; j != numCols; ++j)
            {
                bool isInsert = true;
                double value = properties(i, j);
                if (isNonzero && std::abs(value) <= std::numeric_limits<double>::epsilon())
                    isInsert = false;
                if (isInsert)
                    values.push_back(value);
            }
        }
    }

    // Duplicate scales and limits
    int numValues = values.size();
    QList<double> scales(numValues);
    QList<PairDouble> bounds(numValues);
    scales.fill(propertyScale);
    bounds.fill(propertyBounds);

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
    parameterValues = Utility::combine(parameterValues, values);
    mParameterScales = Utility::combine(mParameterScales, scales);
    mParameterBounds = Utility::combine(mParameterBounds, bounds);
}

//! Unwrap properties from a vector
MatrixXd OptimSolver::unwrapProperties(int& iParameter, QList<double> const& parameterValues, MatrixXd const& initProperties, VariableType type)
{
    MatrixXd properties = initProperties;

    // Check if there are any variables to slice
    if (properties.size() == 0)
        return properties;

    // Acquire the state and constraints
    bool isUnite = mConstraints.isUnited(type);
    bool isMultiply = mConstraints.isMultiplied(type);
    bool isNonzero = mConstraints.isNonzero(type);

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
            double scale = mParameterScales[iStart + i];
            double value = parameterValues[iStart + i];
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
        double scale = mParameterScales[iEnd];
        double value = parameterValues[iEnd];
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
                bool isInsert = true;
                if (isNonzero && std::abs(initProperties(i, j)) <= std::numeric_limits<double>::epsilon())
                    isInsert = false;
                if (isInsert)
                {
                    double scale = mParameterScales[iStart + k];
                    double value = parameterValues[iStart + k];
                    if (scale != 0)
                        value /= scale;
                    else
                        value = std::pow(10, value);
                    properties(i, j) = value;
                    ++k;
                }
            }
        }
    }
    iParameter = iEnd;

    return properties;
}

//! Output the report to log
void OptimSolver::printReport(ceres::Solver::Summary const& summary)
{
    QString message;
    QTextStream stream(&message);
    stream << tr("Ceres Solver Report") << Qt::endl;
    stream << tr("-> Iterations:   %1").arg(summary.iterations.size()) << Qt::endl;
    stream << tr("-> Initial cost: %1").arg(QString::number(summary.initial_cost, 'e', 3)) << Qt::endl;
    stream << tr("-> Final cost:   %1").arg(QString::number(summary.final_cost, 'e', 3)) << Qt::endl;
    stream << tr("-> Duration:     %1 s").arg(QString::number(summary.total_time_in_seconds, 'f', 3)) << Qt::endl;
    stream << tr("-> Termination:  %1").arg(ceres::TerminationTypeToString(summary.termination_type)) << Qt::endl;
    appendLog(message);
}

//! Add a message to a log
void OptimSolver::appendLog(QString const& message, QtMsgType type)
{
    Utility::appendLog(log, message, type);
    emit logAppended(message);
}

//! Retrieve surface elements
QMap<int, ElementMap> OptimSolver::getSurfaceElements(Model& model)
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
QMap<ElementType, QList<VariableType>> OptimSolver::getElementVariables()
{
    QMap<ElementType, QList<VariableType>> result;
    result[BI] = {VariableType::kBeamStiffness};
    result[DB] = {VariableType::kBeamStiffness};
    result[BK] = {VariableType::kBeamStiffness};
    result[PN] = {VariableType::kThickness, VariableType::kYoungsModulus1, VariableType::kPoissonRatio};
    result[OP] = {VariableType::kThickness, VariableType::kYoungsModulus1, VariableType::kYoungsModulus2, VariableType::kShearModulus,
                  VariableType::kPoissonRatio};
    return result;
}

void OptimSolver::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    stream.writeAttribute("type", Utility::toString((int) type()));
    stream.writeTextElement("id", mID.toString());
    stream.writeTextElement("name", name);
    problem.serialize(stream, "problem");
    options.serialize(stream, "options");
    Utility::serialize(stream, "solutions", "solution", solutions);
    Utility::serialize(stream, "log", log);
    stream.writeEndElement();
}

void OptimSolver::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "id")
            mID = QUuid::fromString(stream.readElementText());
        else if (stream.name() == "name")
            name = stream.readElementText();
        else if (stream.name() == "problem")
            problem.deserialize(stream);
        else if (stream.name() == "options")
            options.deserialize(stream);
        else if (stream.name() == "solutions")
            Utility::deserialize(stream, "solution", solutions);
        else if (stream.name() == "log")
            Utility::deserialize(stream, log);
        else
            stream.skipCurrentElement();
    }
}

bool OptimSolver::operator==(ISolver const* pBaseSolver) const
{
    if (type() != pBaseSolver->type())
        return false;
    OptimSolver* pSolver = (OptimSolver*) pBaseSolver;
    return problem == pSolver->problem && options == pSolver->options && solutions == pSolver->solutions;
}

bool OptimSolver::operator!=(ISolver const* pBaseSolver) const
{
    return !(*this == pBaseSolver);
}

OptimProblem::OptimProblem()
{
}

OptimProblem::~OptimProblem()
{
}

bool OptimProblem::isValid() const
{
    int numModes = targetIndices.size();
    if (model.isEmpty() || numModes == 0)
        return false;
    if (targetSolution.isEmpty())
        return numModes == targetWeights.size() && numModes == targetFrequencies.size();
    else
        return numModes == targetWeights.size() && numModes <= targetSolution.frequencies.size() && numModes <= targetSolution.modeShapes.size();
    return true;
}

void OptimProblem::resize(int numModes)
{
    targetIndices.resize(numModes);
    targetFrequencies.resize(numModes);
    targetWeights.resize(numModes);
}

bool OptimProblem::operator==(OptimProblem const& another) const
{
    return Utility::areEqual(*this, another);
}

bool OptimProblem::operator!=(OptimProblem const& another) const
{
    return !(*this == another);
}

void OptimProblem::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    Utility::serialize(stream, "model", model);
    Utility::serialize(stream, "targetIndices", targetIndices);
    Utility::serialize(stream, "targetFrequencies", targetFrequencies);
    Utility::serialize(stream, "targetWeights", targetWeights);
    targetSolution.serialize(stream, "targetSolution");
    Utility::serialize(stream, "targetMatches", targetMatches);
    selector.serialize(stream, "selector");
    constraints.serialize(stream, "constraints");
    stream.writeEndElement();
}

void OptimProblem::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "model")
            Utility::deserialize(stream, model);
        else if (stream.name() == "targetIndices")
            Utility::deserialize(stream, targetIndices);
        else if (stream.name() == "targetFrequencies")
            Utility::deserialize(stream, targetFrequencies);
        else if (stream.name() == "targetWeights")
            Utility::deserialize(stream, targetWeights);
        else if (stream.name() == "targetSolution")
            targetSolution.deserialize(stream);
        else if (stream.name() == "targetMatches")
            Utility::deserialize(stream, targetMatches);
        else if (stream.name() == "selector")
            selector.deserialize(stream);
        else if (stream.name() == "constraints")
            constraints.deserialize(stream);
        else
            stream.skipCurrentElement();
    }
}

OptimOptions::OptimOptions()
    : maxNumIterations(256)
    , timeoutIteration(10.0)
    , numThreads(1)
    , diffStepSize(1.0e-5)
    , minMAC(0)
    , penaltyMAC(0.1)
    , maxRelError(1e-3)
    , numModes(20)
{
}

OptimOptions::~OptimOptions()
{
}

bool OptimOptions::operator==(OptimOptions const& another) const
{
    return Utility::areEqual(*this, another);
}

bool OptimOptions::operator!=(OptimOptions const& another) const
{
    return !(*this == another);
}

void OptimOptions::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    Utility::serializeProperties(stream, elementName, *this);
}

void OptimOptions::deserialize(QXmlStreamReader& stream)
{
    Utility::deserializeProperties(stream, *this);
}

OptimSolution::OptimSolution()
{
}

OptimSolution::~OptimSolution()
{
}

bool OptimSolution::operator==(OptimSolution const& another) const
{
    return Utility::areEqual(*this, another);
}

bool OptimSolution::operator!=(OptimSolution const& another) const
{
    return !(*this == another);
}

void OptimSolution::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    stream.writeAttribute("iteration", Utility::toString(iteration));
    stream.writeAttribute("isSuccess", Utility::toString(isSuccess));
    stream.writeAttribute("duration", Utility::toString(duration));
    stream.writeAttribute("cost", Utility::toString(cost));
    Utility::serialize(stream, "model", model);
    modalSolution.serialize(stream, "modalSolution");
    modalComparison.serialize(stream, "modalComparison");
    stream.writeTextElement("message", message);
    stream.writeEndElement();
}

void OptimSolution::deserialize(QXmlStreamReader& stream)
{
    iteration = stream.attributes().value("iteration").toInt();
    isSuccess = stream.attributes().value("isSuccess").toInt();
    duration = stream.attributes().value("duration").toDouble();
    cost = stream.attributes().value("cost").toDouble();
    while (stream.readNextStartElement())
    {
        if (stream.name() == "model")
            Utility::deserialize(stream, model);
        else if (stream.name() == "modalSolution")
            modalSolution.deserialize(stream);
        else if (stream.name() == "modalComparison")
            modalComparison.deserialize(stream);
        else if (stream.name() == "message")
            message = stream.readElementText();
        else
            stream.skipCurrentElement();
    }
}
