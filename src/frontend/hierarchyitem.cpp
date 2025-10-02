#include <kcl/model.h>
#include <magicenum/magic_enum.hpp>

#include "fluttersolver.h"
#include "hierarchyitem.h"
#include "modalsolver.h"
#include "optimsolver.h"
#include "subproject.h"
#include "uiutility.h"

using namespace Backend;
using namespace Frontend;

HierarchyItem::HierarchyItem(Type itemType)
    : mkType(itemType)
{
    setEditable(false);
}

HierarchyItem::HierarchyItem(Type itemType, QString const& text)
    : HierarchyItem(itemType)
{
    setText(text);
}

HierarchyItem::HierarchyItem(Type itemType, QIcon const& icon, QString const& text)
    : HierarchyItem(itemType, text)
{
    setIcon(icon);
}

QUuid HierarchyItem::id() const
{
    return QUuid();
}

int HierarchyItem::type() const
{
    return mkType;
}

SubprojectHierarchyItem::SubprojectHierarchyItem(Backend::Core::Subproject& subproject)
    : HierarchyItem(kSubproject, QIcon(":/icons/subproject.svg"), subproject.name())
    , mSubproject(subproject)
{
    setEditable(true);
    appendChildren();
}

//! Represent the subproject content
void SubprojectHierarchyItem::appendChildren()
{
    // Model
    appendRow(new ModelHierarchyItem(mSubproject.model()));

    // Solvers
    int numSolvers = mSubproject.numSolvers();
    QMap<Core::ISolver::Type, int> counters;
    for (int i = 0; i != numSolvers; ++i)
    {
        Core::ISolver* pSolver = mSubproject.solver(i);
        auto type = pSolver->type();
        if (!counters.contains(type))
            counters[type] = 0;
        int k = counters[type];
        switch (type)
        {
        case Core::ISolver::kModal:
            appendRow(new ModalSolverHierarchyItem((Core::ModalSolver*) pSolver, QObject::tr("Modal Solver %1").arg(1 + k)));
            break;
        case Core::ISolver::kFlutter:
            appendRow(new FlutterSolverHierarchyItem((Core::FlutterSolver*) pSolver, QObject::tr("Flutter Solver %1").arg(1 + k)));
            break;
        case Core::ISolver::kOptim:
            appendRow(new OptimSolverHierarchyItem((Core::OptimSolver*) pSolver, QObject::tr("Optim Solver %1").arg(1 + k)));
            break;
        default:
            break;
        }
    }
}

QUuid SubprojectHierarchyItem::id() const
{
    return mSubproject.id();
}

Core::Subproject& SubprojectHierarchyItem::subproject()
{
    return mSubproject;
}

ModelHierarchyItem::ModelHierarchyItem(KCL::Model& model)
    : HierarchyItem(kModel, QIcon(":/icons/model.svg"), QObject::tr("Model"))
    , mModel(model)
{
    appendChildren();
}

void ModelHierarchyItem::appendChildren()
{
    // Elastic surfaces
    QIcon icon = QIcon(":/icons/surface.svg");
    int numSurfaces = mModel.surfaces.size();
    for (int i = 0; i != numSurfaces; ++i)
    {
        QString name = mModel.surfaces[i].name.data();
        if (name.isEmpty())
            name = QObject::tr("Elastic surface: %1").arg(1 + i);
        appendRow(new SurfaceHierarchyItem(mModel.surfaces[i], icon, name));
    }

    // Special surface
    icon = QIcon(":/icons/surface-special.svg");
    appendRow(new SurfaceHierarchyItem(mModel.specialSurface, icon, QObject::tr("Special surface")));
}

KCL::Model& ModelHierarchyItem::model()
{
    return mModel;
}

SurfaceHierarchyItem::SurfaceHierarchyItem(KCL::ElasticSurface& surface, QIcon const& icon, QString const& name)
    : HierarchyItem(kSurface, icon, name)
    , mSurface(surface)
{
    setEditable(true);
    appendChildren();
}

void SurfaceHierarchyItem::appendChildren()
{
    std::vector<KCL::ElementType> types = mSurface.types();
    int numTypes = types.size();
    for (int iType = 0; iType != numTypes; ++iType)
    {
        KCL::ElementType type = types[iType];
        QString typeName = magic_enum::enum_name(type).data();
        int numElements = mSurface.numElements(type);
        if (numElements > 1)
        {
            HierarchyItem* pGroupItem = new HierarchyItem(kGroupElements, typeName);
            pGroupItem->setEditable(false);
            for (int iElement = 0; iElement != numElements; ++iElement)
            {
                QString name = QObject::tr("%1: %2").arg(typeName).arg(1 + iElement);
                ElementHierarchyItem* pElementItem = new ElementHierarchyItem(mSurface.element(type, iElement), name);
                pGroupItem->appendRow(pElementItem);
                if (pGroupItem->icon().isNull())
                    pGroupItem->setIcon(pElementItem->icon());
            }
            appendRow(pGroupItem);
        }
        else if (numElements == 1)
        {
            appendRow(new ElementHierarchyItem(mSurface.element(type), typeName));
        }
    }
}

KCL::ElasticSurface& SurfaceHierarchyItem::surface()
{
    return mSurface;
}

ElementHierarchyItem::ElementHierarchyItem(KCL::AbstractElement* pElement, QString const& name)
    : HierarchyItem(kElement, name)
    , mpElement(pElement)
{
    setIcon(Utility::getIcon(mpElement));
}

KCL::AbstractElement* ElementHierarchyItem::element()
{
    return mpElement;
}

ModalSolverHierarchyItem::ModalSolverHierarchyItem(Core::ModalSolver* pSolver, QString const& defaultName)
    : HierarchyItem(kModalSolver)
    , mpSolver(pSolver)
{
    setEditable(true);
    setText(mpSolver->name.isEmpty() ? defaultName : mpSolver->name);
    setIcon(Utility::getIcon(mpSolver));
    appendChildren();
}

Core::ModalSolver* ModalSolverHierarchyItem::solver()
{
    return mpSolver;
}

void ModalSolverHierarchyItem::appendChildren()
{
    appendRow(new ModalOptionsHierarchyItem(mpSolver->options));
    if (!mpSolver->solution.isEmpty())
        appendRow(new ModalSolutionHierarchyItem(mpSolver->solution));
}

ModalOptionsHierarchyItem::ModalOptionsHierarchyItem(Core::ModalOptions& options)
    : HierarchyItem(kModalOptions, QIcon(":/icons/options.png"), QObject::tr("Options"))
    , mOptions(options)
{
}

Core::ModalOptions& ModalOptionsHierarchyItem::options()
{
    return mOptions;
}

ModalSolutionHierarchyItem::ModalSolutionHierarchyItem(Core::ModalSolution const& solution)
    : HierarchyItem(kModalSolution, QIcon(":/icons/solution.png"), QObject::tr("Modal Solution"))
    , mSolution(solution)
{
    appendChildren();
}

Core::ModalSolution const& ModalSolutionHierarchyItem::solution() const
{
    return mSolution;
}

void ModalSolutionHierarchyItem::appendChildren()
{
    int numModes = mSolution.numModes();
    for (int i = 0; i != numModes; ++i)
        appendRow(new ModalPoleHierarchyItem(mSolution.geometry, i, mSolution.frequencies[i], mSolution.modeShapes[i]));
}

ModalPoleHierarchyItem::ModalPoleHierarchyItem(Core::Geometry const& geometry, int iMode, double frequency, Eigen::MatrixXd const& modeShape,
                                               double damping)
    : HierarchyItem(kModalPole)
    , mGeometry(geometry)
    , mIMode(iMode)
    , mFrequency(frequency)
    , mModeShape(modeShape)
    , mDamping(damping)
{
    QString name = QObject::tr("Mode %1: %2 Hz").arg(1 + mIMode).arg(mFrequency, 0, 'f', 3);
    setText(name);
    setIcon(QIcon(":/icons/mode.png"));
}

Backend::Core::Geometry const& ModalPoleHierarchyItem::geometry() const
{
    return mGeometry;
}

int ModalPoleHierarchyItem::iMode() const
{
    return mIMode;
}

double ModalPoleHierarchyItem::frequency() const
{
    return mFrequency;
}

Eigen::MatrixXd const& ModalPoleHierarchyItem::modeShape() const
{
    return mModeShape;
}

double ModalPoleHierarchyItem::damping() const
{
    return mDamping;
}

FlutterSolverHierarchyItem::FlutterSolverHierarchyItem(Core::FlutterSolver* pSolver, QString const& defaultName)
    : HierarchyItem(kFlutterSolver)
    , mpSolver(pSolver)
{
    setEditable(true);
    setText(mpSolver->name.isEmpty() ? defaultName : mpSolver->name);
    setIcon(Utility::getIcon(mpSolver));
    appendChildren();
}

Core::FlutterSolver* FlutterSolverHierarchyItem::solver()
{
    return mpSolver;
}

void FlutterSolverHierarchyItem::appendChildren()
{
    appendRow(new FlutterOptionsHierarchyItem(mpSolver->options));
    if (!mpSolver->solution.isEmpty())
        appendRow(new FlutterSolutionHierarchyItem(mpSolver->solution));
}

FlutterOptionsHierarchyItem::FlutterOptionsHierarchyItem(Core::FlutterOptions& options)
    : HierarchyItem(kFlutterOptions, QIcon(":/icons/options.png"), QObject::tr("Options"))
    , mOptions(options)
{
}

Core::FlutterOptions& FlutterOptionsHierarchyItem::options()
{
    return mOptions;
}

FlutterSolutionHierarchyItem::FlutterSolutionHierarchyItem(Core::FlutterSolution const& solution)
    : HierarchyItem(kFlutterSolution, QIcon(":/icons/solution.png"), QObject::tr("Flutter Solution"))
    , mSolution(solution)
{
    appendChildren();
}

Core::FlutterSolution const& FlutterSolutionHierarchyItem::solution() const
{
    return mSolution;
}

void FlutterSolutionHierarchyItem::appendChildren()
{
    appendRow(new FlutterRootsHierarchyItem(mSolution.flow, mSolution.roots));
    int numCrit = mSolution.numCrit();
    if (numCrit > 0)
    {
        appendRow(new FlutterCritDataHierarchyItem(mSolution));
        for (int i = 0; i != numCrit; ++i)
        {
            appendRow(new ModalPoleHierarchyItem(mSolution.geometry, i, mSolution.critFrequency[i], mSolution.critModeShapes[i].cwiseAbs(),
                                                 mSolution.critDamping[i]));
        }
    }
}

FlutterRootsHierarchyItem::FlutterRootsHierarchyItem(Eigen::VectorXd const& flow, Eigen::MatrixXcd const& roots)
    : HierarchyItem(kFlutterRoots, QIcon(":/icons/roots.svg"), QObject::tr("Roots"))
    , mFlow(flow)
    , mRoots(roots)
{
}

Eigen::VectorXd const& FlutterRootsHierarchyItem::flow() const
{
    return mFlow;
}

Eigen::MatrixXcd const& FlutterRootsHierarchyItem::roots() const
{
    return mRoots;
}

FlutterCritDataHierarchyItem::FlutterCritDataHierarchyItem(Core::FlutterSolution const& solution)
    : HierarchyItem(kFlutterCritData, QIcon(":/icons/crit.png"), QObject::tr("Critical Data"))
    , mFlow(solution.critFlow)
    , mSpeed(solution.critSpeed)
    , mFrequency(solution.critFrequency)
    , mCircFrequency(solution.critCircFrequency)
    , mStrouhal(solution.critStrouhal)
    , mDamping(solution.critDamping)
{
}

Eigen::VectorXd const& FlutterCritDataHierarchyItem::flow() const
{
    return mFlow;
}

Eigen::VectorXd const& FlutterCritDataHierarchyItem::speed() const
{
    return mSpeed;
}

Eigen::VectorXd const& FlutterCritDataHierarchyItem::frequency() const
{
    return mFrequency;
}

Eigen::VectorXd const& FlutterCritDataHierarchyItem::circFrequency() const
{
    return mCircFrequency;
}

Eigen::VectorXd const& FlutterCritDataHierarchyItem::strouhal() const
{
    return mStrouhal;
}

Eigen::VectorXd const& FlutterCritDataHierarchyItem::damping() const
{
    return mDamping;
}

OptimSolverHierarchyItem::OptimSolverHierarchyItem(Core::OptimSolver* pSolver, QString const& defaultName)
    : HierarchyItem(kOptimSolver)
    , mpSolver(pSolver)
{
    setEditable(true);
    setText(mpSolver->name.isEmpty() ? defaultName : mpSolver->name);
    setIcon(Utility::getIcon(mpSolver));
    appendChildren();
}

Core::OptimSolver* OptimSolverHierarchyItem::solver()
{
    return mpSolver;
}

void OptimSolverHierarchyItem::appendChildren()
{
    Core::OptimProblem& problem = mpSolver->problem;
    appendRow(new OptimOptionsHierarchyItem(mpSolver->options));
    appendRow(new OptimTargetHierarchyItem(problem.targetIndices, problem.targetWeights, problem.targetSolution, problem.targetMatches));
    appendRow(new OptimSelectorHierarchyItem(problem.selector));
    appendRow(new OptimConstraintsHierarchyItem(problem.constraints));
    int numSolutions = mpSolver->solutions.size();
    if (numSolutions > 0)
    {
        HierarchyItem* pGroupSolutions = new HierarchyItem(kGroupOptimSolutions, QIcon(":/icons/iterations.svg"),
                                                           QObject::tr("Optim Iterations"));
        for (int i = 0; i != numSolutions; ++i)
            pGroupSolutions->appendRow(new OptimSolutionHierarchyItem(mpSolver->solutions[i]));
        appendRow(pGroupSolutions);
    }
}

OptimOptionsHierarchyItem::OptimOptionsHierarchyItem(Core::OptimOptions& options)
    : HierarchyItem(kOptimOptions, QIcon(":/icons/options.png"), QObject::tr("Options"))
    , mOptions(options)
{
}

Core::OptimOptions& OptimOptionsHierarchyItem::options()
{
    return mOptions;
}

OptimTargetHierarchyItem::OptimTargetHierarchyItem(Eigen::VectorXi& indices, Eigen::VectorXd& weights, Core::ModalSolution& solution,
                                                   Core::Matches& matches)
    : HierarchyItem(kOptimTarget, QIcon(":/icons/target.svg"), QObject::tr("Target"))
    , mIndices(indices)
    , mWeights(weights)
    , mSolution(solution)
    , mMatches(matches)
{
}

Eigen::VectorXi& OptimTargetHierarchyItem::indices()
{
    return mIndices;
}

Eigen::VectorXd& OptimTargetHierarchyItem::weights()
{
    return mWeights;
}

Core::ModalSolution& OptimTargetHierarchyItem::solution()
{
    return mSolution;
}

Core::Matches& OptimTargetHierarchyItem::matches()
{
    return mMatches;
}

OptimSelectorHierarchyItem::OptimSelectorHierarchyItem(Core::Selector& selector)
    : HierarchyItem(kOptimSelector, QIcon(":/icons/selector.svg"), QObject::tr("Selector"))
    , mSelector(selector)
{
}

Core::Selector& OptimSelectorHierarchyItem::selector()
{
    return mSelector;
}

OptimConstraintsHierarchyItem::OptimConstraintsHierarchyItem(Core::Constraints& constraints)
    : HierarchyItem(kOptimConstraints, QIcon(":/icons/constraints.png"), QObject::tr("Constraints"))
    , mConstraints(constraints)
{
}

Core::Constraints& OptimConstraintsHierarchyItem::constraints()
{
    return mConstraints;
}

OptimSolutionHierarchyItem::OptimSolutionHierarchyItem(Core::OptimSolution& solution)
    : HierarchyItem(kOptimSolution)
    , mSolution(solution)
{
    const double kAcceptThreshold = 0.01;
    const double kCritialThreshold = 0.05;
    double error = mSolution.modalComparison.errorFrequencies.array().abs().maxCoeff();
    QString name = QObject::tr("Iteration %1: %2 %").arg(QString::number(mSolution.iteration), QString::number(error * 100, 'f', 3));
    QIcon icon(QString(":/icons/flag-%1.svg").arg(Utility::errorColorName(error, kAcceptThreshold, kCritialThreshold)));
    setText(name);
    setIcon(icon);
    appendChildren();
}

Core::OptimSolution const& OptimSolutionHierarchyItem::solution() const
{
    return mSolution;
}

void OptimSolutionHierarchyItem::appendChildren()
{
    appendRow(new ModelHierarchyItem(mSolution.model));
    appendRow(new ModalSolutionHierarchyItem(mSolution.modalSolution));
}
