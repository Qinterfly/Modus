#include <QSortFilterProxyModel>
#include <QTreeView>

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

// Helper functions
Core::Subproject* getSubproject(HierarchyItem* pItem);
KCL::Model* getModel(HierarchyItem* pItem);

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

QString const& HierarchyItem::id()
{
    if (mID.isEmpty())
        evaluateID();
    return mID;
}

int HierarchyItem::type() const
{
    return mkType;
}

//! Compute item identifier (might not be unique)
void HierarchyItem::evaluateID()
{
    // Constants
    QString const kDelimiter = "/";

    // Retrieve the parent identifier
    QString parentKey;
    QStandardItem* pParent = parent();
    if (pParent && HierarchyItem::isValid(pParent->type()))
        parentKey = static_cast<HierarchyItem*>(pParent)->id();

    // Retrieve the object identifier
    QString objectKey = data(Qt::DisplayRole).toString();

    // Build up the id
    mID = QString("%1%3%2").arg(parentKey, objectKey, kDelimiter);
}

//! Set the expanded state of the hierarchy item
void HierarchyItem::setExpanded(bool flag)
{
    // Retrieve the parent widget
    if (!model()->parent())
        return;
    QTreeView* pView = (QTreeView*) model()->parent();

    // Get the item model
    QSortFilterProxyModel* pProxyModel = qobject_cast<QSortFilterProxyModel*>(pView->model());
    if (!pProxyModel)
        return;

    // Set the expanded state
    auto proxyIndex = pProxyModel->mapFromSource(index());
    if (flag)
        pView->expand(proxyIndex);
    else
        pView->collapse(proxyIndex);
}

//! Set the selected state of the hierarchy item
void HierarchyItem::setSelected(bool flag)
{
    // Retrieve the parent widget
    if (!model()->parent())
        return;
    QTreeView* pView = (QTreeView*) model()->parent();

    // Get the item models
    QSortFilterProxyModel* pProxyModel = qobject_cast<QSortFilterProxyModel*>(pView->model());
    if (!pProxyModel)
        return;
    QItemSelectionModel* pSelectionModel = pView->selectionModel();

    // Set the selected state
    auto proxyIndex = pProxyModel->mapFromSource(index());
    if (flag)
        pSelectionModel->select(proxyIndex, QItemSelectionModel::Select);
    else
        pSelectionModel->select(proxyIndex, QItemSelectionModel::Deselect);
}

//! Check if the type is hierarchial
bool HierarchyItem::isValid(int iType)
{
    return iType >= Qt::UserRole;
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

Core::Subproject& SubprojectHierarchyItem::subproject()
{
    return mSubproject;
}

//! Select items associated with the model
void SubprojectHierarchyItem::selectItems(KCL::Model const& kclModel, QList<Core::Selection> const& selections)
{
    QList<HierarchyItem*> foundItems;
    Utility::findItems(this, HierarchyItem::kModel, foundItems);
    if (!foundItems.empty())
    {
        int numFound = foundItems.size();
        for (int i = 0; i != numFound; ++i)
        {
            ModelHierarchyItem* pModelItem = (ModelHierarchyItem*) foundItems[i];
            if (&pModelItem->kclModel() == &kclModel)
                pModelItem->selectItems(selections);
        }
    }
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
        appendRow(new SurfaceHierarchyItem(i, mModel.surfaces[i], icon, name));
    }

    // Special surface
    icon = QIcon(":/icons/surface-special.svg");
    appendRow(new SurfaceHierarchyItem(-1, mModel.specialSurface, icon, QObject::tr("Special surface")));
}

Core::Subproject* ModelHierarchyItem::subproject()
{
    return getSubproject(this);
}

KCL::Model& ModelHierarchyItem::kclModel()
{
    return mModel;
}

//! Select model elements associated with surfaces
void ModelHierarchyItem::selectItems(QList<Core::Selection> const& selections)
{
    int numChildren = rowCount();
    for (int i = 0; i != numChildren; ++i)
    {
        HierarchyItem* pBaseItem = (HierarchyItem*) child(i);
        if (pBaseItem->type() == HierarchyItem::kSurface)
        {
            SurfaceHierarchyItem* pItem = (SurfaceHierarchyItem*) pBaseItem;
            pItem->selectItems(selections);
        }
    }
}

SurfaceHierarchyItem::SurfaceHierarchyItem(int iSurface, KCL::ElasticSurface& surface, QIcon const& icon, QString const& name)
    : HierarchyItem(kSurface, icon, name)
    , mkISurface(iSurface)
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
            int numInsert = 0;
            for (int iElement = 0; iElement != numElements; ++iElement)
            {
                KCL::AbstractElement* pElement = mSurface.element(type, iElement);
                if (!isInsertable(pElement))
                    continue;
                QString name = QObject::tr("%1: %2").arg(typeName).arg(1 + numInsert);
                ElementHierarchyItem* pElementItem = new ElementHierarchyItem(iElement, pElement, name);
                pGroupItem->appendRow(pElementItem);
                if (pGroupItem->icon().isNull())
                    pGroupItem->setIcon(pElementItem->icon());
                ++numInsert;
            }
            if (pGroupItem->hasChildren())
                appendRow(pGroupItem);
        }
        else if (numElements == 1)
        {
            KCL::AbstractElement* pElement = mSurface.element(type);
            if (isInsertable(pElement))
                appendRow(new ElementHierarchyItem(0, pElement, typeName));
        }
    }
}

bool SurfaceHierarchyItem::isInsertable(KCL::AbstractElement* pElement)
{
    return pElement && pElement->subType() != KCL::AE1;
}

int SurfaceHierarchyItem::iSurface() const
{
    return mkISurface;
}

KCL::ElasticSurface& SurfaceHierarchyItem::surface()
{
    return mSurface;
}

KCL::Model* SurfaceHierarchyItem::kclModel()
{
    return getModel(this);
}

//! Select items excluding duplicate entities
void SurfaceHierarchyItem::selectItems(QList<Core::Selection> const& selections)
{
    // Create the mask of the selected elements
    int numSelections = selections.size();
    QSet<Core::Selection> selectionSet;
    for (int i = 0; i != numSelections; ++i)
        selectionSet.insert(selections[i]);

    // Loop through all the elements associated with the surface
    int numChildren = rowCount();
    for (int i = 0; i != numChildren; ++i)
        selectItem((HierarchyItem*) child(i), selectionSet);
}

//! Select model elements associated with the given item using the selection set
void SurfaceHierarchyItem::selectItem(HierarchyItem* pBaseItem, QSet<Core::Selection> const& selectionSet)
{
    if (pBaseItem->type() == HierarchyItem::kElement)
    {
        ElementHierarchyItem* pItem = (ElementHierarchyItem*) pBaseItem;
        Core::Selection key(mkISurface, pItem->element()->type(), pItem->iElement());
        if (selectionSet.contains(key))
            pItem->setSelected();
    }
    else if (pBaseItem->type() == HierarchyItem::kGroupElements)
    {
        int numChildren = pBaseItem->rowCount();
        for (int i = 0; i != numChildren; ++i)
            selectItem((HierarchyItem*) pBaseItem->child(i), selectionSet);
    }
}

ElementHierarchyItem::ElementHierarchyItem(int iElement, KCL::AbstractElement* pElement, QString const& name)
    : HierarchyItem(kElement, name)
    , mkIElement(iElement)
    , mpElement(pElement)
{
    setIcon(Utility::getIcon(mpElement));
}

int ElementHierarchyItem::iSurface()
{
    HierarchyItem* pItem = Utility::findParentByType(this, HierarchyItem::kSurface);
    if (pItem)
        return static_cast<SurfaceHierarchyItem*>(pItem)->iSurface();
    return std::numeric_limits<int>::min();
}

int ElementHierarchyItem::iElement() const
{
    return mkIElement;
}

KCL::AbstractElement* ElementHierarchyItem::element()
{
    return mpElement;
}

KCL::Model* ElementHierarchyItem::kclModel()
{
    return getModel(this);
}

Core::Subproject* ElementHierarchyItem::subproject()
{
    return getSubproject(this);
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

Core::Subproject* ModalPoleHierarchyItem::subproject()
{
    return getSubproject(this);
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
            pGroupSolutions->appendRow(new OptimSolutionHierarchyItem(i, mpSolver->solutions[i]));
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
    appendRow(new ModalSolutionHierarchyItem(solution));
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

OptimSolutionHierarchyItem::OptimSolutionHierarchyItem(int iSolution, Core::OptimSolution& solution)
    : HierarchyItem(kOptimSolution)
    , mkISolution(iSolution)
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

int OptimSolutionHierarchyItem::iSolution() const
{
    return mkISolution;
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

//! Helper function to find subproject which contains the current item
Core::Subproject* getSubproject(HierarchyItem* pItem)
{
    HierarchyItem* pFoundItem = Utility::findParentByType(pItem, HierarchyItem::kSubproject);
    if (pFoundItem)
        return &static_cast<SubprojectHierarchyItem*>(pFoundItem)->subproject();
    return nullptr;
}

//! Helper function to find model which contains the current item
KCL::Model* getModel(HierarchyItem* pItem)
{
    HierarchyItem* pFoundItem = Utility::findParentByType(pItem, HierarchyItem::kModel);
    if (pFoundItem)
        return &static_cast<ModelHierarchyItem*>(pFoundItem)->kclModel();
    return nullptr;
}
