
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
class Selector;
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
        kOptimConstraints,
        kGroupOptimSolutions,
        kOptimSolution
    };

    HierarchyItem() = delete;
    HierarchyItem(Type itemType);
    HierarchyItem(Type itemType, QString const& text);
    HierarchyItem(Type itemType, QIcon const& icon, QString const& text);
    virtual ~HierarchyItem() = default;

    virtual QUuid id() const;

    int type() const override final;
    void setExpanded(bool flag = true);
    void setSelected(bool flag = true);

protected:
    Type const mkType;
};

class SubprojectHierarchyItem : public HierarchyItem
{
public:
    SubprojectHierarchyItem(Backend::Core::Subproject& subproject);
    ~SubprojectHierarchyItem() = default;

    QUuid id() const override;
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
    ~ModelHierarchyItem() = default;

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
    ~SurfaceHierarchyItem() = default;

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
    ~ElementHierarchyItem() = default;

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
    ~ModalSolverHierarchyItem() = default;

    Backend::Core::ModalSolver* solver();

private:
    void appendChildren();

    Backend::Core::ModalSolver* mpSolver;
};

class ModalOptionsHierarchyItem : public HierarchyItem
{
public:
    ModalOptionsHierarchyItem(Backend::Core::ModalOptions& options);
    ~ModalOptionsHierarchyItem() = default;

    Backend::Core::ModalOptions& options();

private:
    Backend::Core::ModalOptions& mOptions;
};

class ModalSolutionHierarchyItem : public HierarchyItem
{
public:
    ModalSolutionHierarchyItem(Backend::Core::ModalSolution const& solution);
    ~ModalSolutionHierarchyItem() = default;

    Backend::Core::ModalSolution const& solution() const;

private:
    void appendChildren();

    Backend::Core::ModalSolution const& mSolution;
};

class ModalPoleHierarchyItem : public HierarchyItem
{
public:
    ModalPoleHierarchyItem(Backend::Core::Geometry const& geometry, int iMode, double frequency, Eigen::MatrixXd const& modeShape,
                           double damping = 0.0);
    ~ModalPoleHierarchyItem() = default;

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
    ~FlutterSolverHierarchyItem() = default;

    Backend::Core::FlutterSolver* solver();

private:
    void appendChildren();

    Backend::Core::FlutterSolver* mpSolver;
};

class FlutterOptionsHierarchyItem : public HierarchyItem
{
public:
    FlutterOptionsHierarchyItem(Backend::Core::FlutterOptions& options);
    ~FlutterOptionsHierarchyItem() = default;

    Backend::Core::FlutterOptions& options();

private:
    Backend::Core::FlutterOptions& mOptions;
};

class FlutterSolutionHierarchyItem : public HierarchyItem
{
public:
    FlutterSolutionHierarchyItem(Backend::Core::FlutterSolution const& solution);
    ~FlutterSolutionHierarchyItem() = default;

    Backend::Core::FlutterSolution const& solution() const;

private:
    void appendChildren();

    Backend::Core::FlutterSolution const& mSolution;
};

class FlutterRootsHierarchyItem : public HierarchyItem
{
public:
    FlutterRootsHierarchyItem(Eigen::VectorXd const& flow, Eigen::MatrixXcd const& roots);
    ~FlutterRootsHierarchyItem() = default;

    Eigen::VectorXd const& flow() const;
    Eigen::MatrixXcd const& roots() const;

private:
    Eigen::VectorXd const& mFlow;
    Eigen::MatrixXcd const& mRoots;
};

class FlutterCritDataHierarchyItem : public HierarchyItem
{
public:
    FlutterCritDataHierarchyItem(Backend::Core::FlutterSolution const& solution);
    ~FlutterCritDataHierarchyItem() = default;

    Eigen::VectorXd const& flow() const;
    Eigen::VectorXd const& speed() const;
    Eigen::VectorXd const& frequency() const;
    Eigen::VectorXd const& circFrequency() const;
    Eigen::VectorXd const& strouhal() const;
    Eigen::VectorXd const& damping() const;

private:
    Eigen::VectorXd const& mFlow;
    Eigen::VectorXd const& mSpeed;
    Eigen::VectorXd const& mFrequency;
    Eigen::VectorXd const& mCircFrequency;
    Eigen::VectorXd const& mStrouhal;
    Eigen::VectorXd const& mDamping;
};

class OptimSolverHierarchyItem : public HierarchyItem
{
public:
    OptimSolverHierarchyItem(Backend::Core::OptimSolver* pSolver, QString const& defaultName);
    ~OptimSolverHierarchyItem() = default;

    Backend::Core::OptimSolver* solver();

private:
    void appendChildren();

    Backend::Core::OptimSolver* mpSolver;
};

class OptimOptionsHierarchyItem : public HierarchyItem
{
public:
    OptimOptionsHierarchyItem(Backend::Core::OptimOptions& options);
    ~OptimOptionsHierarchyItem() = default;

    Backend::Core::OptimOptions& options();

private:
    Backend::Core::OptimOptions& mOptions;
};

class OptimTargetHierarchyItem : public HierarchyItem
{
public:
    OptimTargetHierarchyItem(Eigen::VectorXi& indices, Eigen::VectorXd& weights, Backend::Core::ModalSolution& solution,
                             Backend::Core::Matches& matches);
    ~OptimTargetHierarchyItem() = default;

    Eigen::VectorXi& indices();
    Eigen::VectorXd& weights();
    Backend::Core::ModalSolution& solution();
    Backend::Core::Matches& matches();

private:
    Eigen::VectorXi& mIndices;
    Eigen::VectorXd& mWeights;
    Backend::Core::ModalSolution& mSolution;
    Backend::Core::Matches& mMatches;
};

class OptimSelectorHierarchyItem : public HierarchyItem
{
public:
    OptimSelectorHierarchyItem(Backend::Core::Selector& selector);
    ~OptimSelectorHierarchyItem() = default;

    Backend::Core::Selector& selector();

private:
    Backend::Core::Selector& mSelector;
};

class OptimConstraintsHierarchyItem : public HierarchyItem
{
public:
    OptimConstraintsHierarchyItem(Backend::Core::Constraints& constraints);
    ~OptimConstraintsHierarchyItem() = default;

    Backend::Core::Constraints& constraints();

private:
    Backend::Core::Constraints& mConstraints;
};

class OptimSolutionHierarchyItem : public HierarchyItem
{
public:
    OptimSolutionHierarchyItem(int iSolution, Backend::Core::OptimSolution& solution);
    ~OptimSolutionHierarchyItem() = default;

    int iSolution() const;
    Backend::Core::OptimSolution const& solution() const;

private:
    void appendChildren();

    int const mkISolution;
    Backend::Core::OptimSolution& mSolution;
};
}

#endif // HIERARCHYITEM_H
