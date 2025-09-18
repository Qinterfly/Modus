#include <kcl/model.h>
#include <magicenum/magic_enum.hpp>

#include "fluttersolver.h"
#include "hierarchyitem.h"
#include "modalsolver.h"
#include "optimsolver.h"
#include "subproject.h"

using namespace Backend;
using namespace Frontend;

QIcon getIcon(KCL::AbstractElement const* pElement);
QIcon getIcon(Core::ISolver const* pSolver);

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
        QString name = QObject::tr("Elastic surface: %1").arg(1 + i);
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
    setIcon(getIcon(mpElement));
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
    setIcon(getIcon(mpSolver));
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
        appendRow(new ModalSolutionHierarchyItem(mpSolver->solution, QObject::tr("Solution")));
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

ModalSolutionHierarchyItem::ModalSolutionHierarchyItem(Core::ModalSolution const& solution, QString const& name)
    : HierarchyItem(kModalSolution, QIcon(":/icons/solution.png"), name)
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
        appendRow(new ModalPoleHierarchyItem(i, mSolution.frequencies()[i], mSolution.modeShapes()[i]));
}

ModalPoleHierarchyItem::ModalPoleHierarchyItem(int iMode, double frequency, Eigen::MatrixXd const& modeShape)
    : HierarchyItem(kModalPole)
    , mIMode(iMode)
    , mFrequency(frequency)
    , mModeShape(modeShape)
{
    QString name = QObject::tr("Mode %1: %2 Hz").arg(1 + mIMode).arg(mFrequency, 0, 'f', 3);
    setText(name);
    setIcon(QIcon(":/icons/mode.png"));
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

FlutterSolverHierarchyItem::FlutterSolverHierarchyItem(Core::FlutterSolver* pSolver, QString const& defaultName)
    : HierarchyItem(kFlutterSolver)
    , mpSolver(pSolver)
{
    setEditable(true);
    setText(mpSolver->name.isEmpty() ? defaultName : mpSolver->name);
    setIcon(getIcon(mpSolver));
}

Core::FlutterSolver* FlutterSolverHierarchyItem::solver()
{
    return mpSolver;
}

OptimSolverHierarchyItem::OptimSolverHierarchyItem(Core::OptimSolver* pSolver, QString const& defaultName)
    : HierarchyItem(kOptimSolver)
    , mpSolver(pSolver)
{
    setEditable(true);
    setText(mpSolver->name.isEmpty() ? defaultName : mpSolver->name);
    setIcon(getIcon(mpSolver));
}

Core::OptimSolver* OptimSolverHierarchyItem::solver()
{
    return mpSolver;
}

QIcon getIcon(KCL::AbstractElement const* pElement)
{
    switch (pElement->type())
    {
    case KCL::OD:
        return QIcon(":/icons/configuration.png");
    case KCL::SM:
        return QIcon(":/icons/mass.png");
    case KCL::BI:
        return QIcon(":/icons/beam-bending.png");
    case KCL::PN:
        return QIcon(":/icons/panel.png");
    case KCL::EL:
        return QIcon(":/icons/aileron.png");
    case KCL::DE:
        return QIcon(":/icons/aileron.png");
    case KCL::M3:
        return QIcon(":/icons/mass.png");
    case KCL::OP:
        return QIcon(":/icons/layer.png");
    case KCL::BK:
        return QIcon(":/icons/beam-torsion.png");
    case KCL::AE:
        return QIcon(":/icons/trapezium.png");
    case KCL::DQ:
        return QIcon(":/icons/function.png");
    case KCL::DA:
        return QIcon(":/icons/trapezium.png");
    case KCL::DB:
        return QIcon(":/icons/beam-bending.png");
    case KCL::PK:
        return QIcon(":/icons/function.png");
    case KCL::QK:
        return QIcon(":/icons/function.png");
    case KCL::WP:
        return QIcon(":/icons/setup.png");
    case KCL::PR:
        return QIcon(":/icons/spring.png");
    case KCL::TE:
        return QIcon(":/icons/damper.png");
    case KCL::CO:
        return QIcon(":/icons/constants.png");
    default:
        break;
    }
    return QIcon();
}

QIcon getIcon(Core::ISolver const* pSolver)
{
    switch (pSolver->type())
    {
    case Core::ISolver::kModal:
        return QIcon(":/icons/spectrum.png");
    case Core::ISolver::kFlutter:
        return QIcon(":/icons/flutter.png");
    case Core::ISolver::kOptim:
        return QIcon(":/icons/optimization.png");
    }
    return QIcon();
}
