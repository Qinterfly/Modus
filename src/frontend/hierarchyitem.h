
#ifndef HIERARCHYITEM_H
#define HIERARCHYITEM_H

#include <Eigen/Core>
#include <QStandardItem>
#include <QUuid>

#include "aliasdata.h"

namespace KCL
{
class ElasticSurface;
class Model;
class AbstractElement;
}

namespace Backend::Core
{
struct Geometry;
class Subproject;

class ModalSolver;
struct ModalOptions;
struct ModalSolution;

class FlutterSolver;
struct FlutterOptions;
struct FlutterSolution;

class OptimSolver;
struct OptimProblem;
struct OptimOptions;
struct OptimSolution;
struct OptimTarget;
class Selector;
class SelectionSet;
class Constraints;
struct Selection;
}

namespace Frontend
{

class HierarchyItem : public QStandardItem
{
public:
    enum Type
    {
        kSubproject = QStandardItem::UserType,
        kModel,
        kSurface,
        kGroupElements,
        kElement,
        kModalSolver,
        kModalOptions,
        kModalSolution,
        kModalFrequencies,
        kModalPole,
        kFlutterSolver,
        kFlutterOptions,
        kFlutterSolution,
        kFlutterRoots,
        kFlutterCritData,
        kOptimSolver,
        kOptimOptions,
        kOptimTarget,
        kOptimSelector,
        kOptimSelectionSet,
        kOptimConstraints,
        kGroupOptimSolutions,
        kOptimSolution,
        kLog
    };

    HierarchyItem() = delete;
    HierarchyItem(Type itemType);
    HierarchyItem(Type itemType, QString const& text);
    HierarchyItem(Type itemType, QIcon const& icon, QString const& text);
    virtual ~HierarchyItem() = default;

    QString const& id();
    int type() const override final;
    void setExpanded(bool flag = true);
    void setSelected(bool flag = true);

    static bool isValid(int iType);

private:
    void evaluateID();

protected:
    Type const mkType;
    QString mID;
};

class SubprojectHierarchyItem : public HierarchyItem
{
public:
    SubprojectHierarchyItem(Backend::Core::Subproject& subproject);
    virtual ~SubprojectHierarchyItem() = default;

    Backend::Core::Subproject& subproject();

    void selectItems(KCL::Model const& kclModel, QList<Backend::Core::Selection> const& selections);

private:
    void appendChildren();

    Backend::Core::Subproject& mSubproject;
};

class ModelHierarchyItem : public HierarchyItem
{
public:
    ModelHierarchyItem(KCL::Model& model);
    virtual ~ModelHierarchyItem() = default;

    Backend::Core::Subproject* subproject();
    KCL::Model& kclModel();

    void selectItems(QList<Backend::Core::Selection> const& selections);

private:
    void appendChildren();

    KCL::Model& mModel;
};

class SurfaceHierarchyItem : public HierarchyItem
{
public:
    SurfaceHierarchyItem(int iSurface, KCL::ElasticSurface& surface, QIcon const& icon, QString const& name);
    virtual ~SurfaceHierarchyItem() = default;

    int iSurface() const;
    KCL::ElasticSurface& surface();
    KCL::Model* kclModel();

    void selectItems(QList<Backend::Core::Selection> const& selections);
    void selectItem(HierarchyItem* pBaseItem, QSet<Backend::Core::Selection> const& selectionSet);

private:
    void appendChildren();
    bool isInsertable(KCL::AbstractElement* pElement);

    int const mkISurface;
    KCL::ElasticSurface& mSurface;
};

class ElementHierarchyItem : public HierarchyItem
{
public:
    ElementHierarchyItem(int iElement, KCL::AbstractElement* pElement, QString const& name);
    virtual ~ElementHierarchyItem() = default;

    int iSurface();
    int iElement() const;
    KCL::AbstractElement* element();
    KCL::Model* kclModel();
    Backend::Core::Subproject* subproject();

private:
    int const mkIElement;
    KCL::AbstractElement* mpElement;
};

class ModalSolverHierarchyItem : public HierarchyItem
{
public:
    ModalSolverHierarchyItem(Backend::Core::ModalSolver* pSolver, QString const& defaultName);
    virtual ~ModalSolverHierarchyItem() = default;

    Backend::Core::ModalSolver* solver();

private:
    void appendChildren();

    Backend::Core::ModalSolver* mpSolver;
};

class ModalOptionsHierarchyItem : public HierarchyItem
{
public:
    ModalOptionsHierarchyItem(Backend::Core::ModalOptions& options);
    virtual ~ModalOptionsHierarchyItem() = default;

    Backend::Core::ModalOptions& options();

private:
    Backend::Core::ModalOptions& mOptions;
};

class ModalSolutionHierarchyItem : public HierarchyItem
{
public:
    ModalSolutionHierarchyItem(Backend::Core::ModalSolution const& solution);
    ModalSolutionHierarchyItem(Backend::Core::ModalSolution const& solution, QString const& name);
    virtual ~ModalSolutionHierarchyItem() = default;

    Backend::Core::ModalSolution const& solution() const;

private:
    void appendChildren();

    Backend::Core::ModalSolution const& mSolution;
};

class ModalFrequenciesHierarchyItem : public HierarchyItem
{
public:
    ModalFrequenciesHierarchyItem(Backend::Core::ModalSolution const& solution);
    virtual ~ModalFrequenciesHierarchyItem() = default;

    Eigen::VectorXd const& frequencies() const;
    Backend::Core::ModalSolution const& solution() const;

private:
    Backend::Core::ModalSolution const& mSolution;
};

class ModalPoleHierarchyItem : public HierarchyItem
{
public:
    ModalPoleHierarchyItem(Backend::Core::Geometry const& geometry, int iMode, double frequency, Eigen::MatrixXd const& modeShape,
                           double damping = 0.0, QString const& postfix = QString());
    virtual ~ModalPoleHierarchyItem() = default;

    Backend::Core::Geometry const& geometry() const;
    int iMode() const;
    double frequency() const;
    Eigen::MatrixXd const& modeShape() const;
    double damping() const;
    Backend::Core::Subproject* subproject();

private:
    Backend::Core::Geometry const& mGeometry;
    int mIMode;
    double mFrequency;
    Eigen::MatrixXd mModeShape;
    double mDamping;
};

class FlutterSolverHierarchyItem : public HierarchyItem
{
public:
    FlutterSolverHierarchyItem(Backend::Core::FlutterSolver* pSolver, QString const& defaultName);
    virtual ~FlutterSolverHierarchyItem() = default;

    Backend::Core::FlutterSolver* solver();

private:
    void appendChildren();

    Backend::Core::FlutterSolver* mpSolver;
};

class FlutterOptionsHierarchyItem : public HierarchyItem
{
public:
    FlutterOptionsHierarchyItem(Backend::Core::FlutterOptions& options);
    virtual ~FlutterOptionsHierarchyItem() = default;

    Backend::Core::FlutterOptions& options();

private:
    Backend::Core::FlutterOptions& mOptions;
};

class FlutterSolutionHierarchyItem : public HierarchyItem
{
public:
    FlutterSolutionHierarchyItem(Backend::Core::FlutterSolution const& solution);
    virtual ~FlutterSolutionHierarchyItem() = default;

    Backend::Core::FlutterSolution const& solution() const;

private:
    void appendChildren();

    Backend::Core::FlutterSolution const& mSolution;
};

class FlutterRootsHierarchyItem : public HierarchyItem
{
public:
    FlutterRootsHierarchyItem(Backend::Core::FlutterSolution const& solution);
    virtual ~FlutterRootsHierarchyItem() = default;

    Eigen::VectorXd const& flow() const;
    Eigen::MatrixXcd const& roots() const;
    Backend::Core::FlutterSolution const& solution() const;

private:
    Backend::Core::FlutterSolution const& mSolution;
};

class FlutterCritDataHierarchyItem : public HierarchyItem
{
public:
    FlutterCritDataHierarchyItem(Backend::Core::FlutterSolution const& solution);
    virtual ~FlutterCritDataHierarchyItem() = default;

    Eigen::VectorXd const& flow() const;
    Eigen::VectorXd const& speed() const;
    Eigen::VectorXd const& frequency() const;
    Eigen::VectorXd const& circFrequency() const;
    Eigen::VectorXd const& strouhal() const;
    Eigen::VectorXd const& damping() const;
    Backend::Core::FlutterSolution const& solution() const;

private:
    Backend::Core::FlutterSolution const& mSolution;
};

class OptimSolverHierarchyItem : public HierarchyItem
{
public:
    OptimSolverHierarchyItem(Backend::Core::OptimSolver* pSolver, QString const& defaultName);
    virtual ~OptimSolverHierarchyItem() = default;

    Backend::Core::OptimSolver* solver();

private:
    void appendChildren();

    Backend::Core::OptimSolver* mpSolver;
};

class OptimOptionsHierarchyItem : public HierarchyItem
{
public:
    OptimOptionsHierarchyItem(Backend::Core::OptimOptions& options);
    virtual ~OptimOptionsHierarchyItem() = default;

    Backend::Core::OptimOptions& options();

private:
    Backend::Core::OptimOptions& mOptions;
};

class OptimTargetHierarchyItem : public HierarchyItem
{
public:
    OptimTargetHierarchyItem(Backend::Core::OptimTarget& target);
    virtual ~OptimTargetHierarchyItem() = default;

    Backend::Core::OptimTarget& target();

private:
    Backend::Core::OptimTarget& mTarget;
};

class OptimSelectorHierarchyItem : public HierarchyItem
{
public:
    OptimSelectorHierarchyItem(Backend::Core::Selector& selector);
    virtual ~OptimSelectorHierarchyItem() = default;

    Backend::Core::Selector& selector();

private:
    void appendChildren();

    Backend::Core::Selector& mSelector;
};

class OptimSelectionSetHierarchyItem : public HierarchyItem
{
public:
    OptimSelectionSetHierarchyItem(Backend::Core::SelectionSet& selectionSet, QString const& name);
    virtual ~OptimSelectionSetHierarchyItem() = default;

    Backend::Core::SelectionSet& selectionSet();
    KCL::Model* kclModel();

private:
    Backend::Core::SelectionSet& mSelectionSet;
};

class OptimConstraintsHierarchyItem : public HierarchyItem
{
public:
    OptimConstraintsHierarchyItem(Backend::Core::Constraints& constraints);
    virtual ~OptimConstraintsHierarchyItem() = default;

    Backend::Core::Constraints& constraints();

private:
    Backend::Core::Constraints& mConstraints;
};

class OptimSolutionHierarchyItem : public HierarchyItem
{
public:
    OptimSolutionHierarchyItem(int iSolution, Backend::Core::OptimSolution& solution);
    virtual ~OptimSolutionHierarchyItem() = default;

    int iSolution() const;
    Backend::Core::OptimSolution const& solution() const;

private:
    void appendChildren();

    int const mkISolution;
    Backend::Core::OptimSolution& mSolution;
};

class LogHierarchyItem : public QObject, public HierarchyItem
{
public:
    LogHierarchyItem(QString& log);
    virtual ~LogHierarchyItem() = default;

    QString const& log() const;

private:
    QString& mLog;
};
}

#endif // HIERARCHYITEM_H
