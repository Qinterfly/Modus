#ifndef MODALSOLVER_H
#define MODALSOLVER_H

#include <kcl/model.h>
#include <kcl/solver.h>
#include <QDir>
#include <QList>

#include "aliasdata.h"
#include "geometry.h"
#include "iserializable.h"
#include "isolver.h"

namespace Backend::Core
{

struct ModalComparison;

struct ModalSolution : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(Geometry geometry MEMBER geometry)
    Q_PROPERTY(Eigen::VectorXd frequencies MEMBER frequencies)
    Q_PROPERTY(QList<Eigen::MatrixXd> modeShapes MEMBER modeShapes)
    Q_PROPERTY(QList<QString> names MEMBER names)

public:
    ModalSolution();
    ModalSolution(Geometry const& anotherGeometry, Eigen::VectorXd const& anotherFrequencies, QList<Eigen::MatrixXd> const& anotherModeShapes);
    ModalSolution(KCL::EigenSolution const& solution);
    ~ModalSolution();

    bool isEmpty() const;
    int numModes() const;
    ModalComparison compare(ModalSolution const& another, Eigen::VectorXi const& indices, Matches const& matches, double minMAC) const;

    void read(QDir const& directory);

    bool operator==(ModalSolution const& another) const;
    bool operator!=(ModalSolution const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    void resize(int numDOFs, int numModes);
    void readModesets(QString const& pathFile);

    Geometry geometry;
    Eigen::VectorXd frequencies;
    QList<Eigen::MatrixXd> modeShapes;
    QList<QString> names;
};

struct ModalComparison : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(Eigen::VectorXd diffFrequencies MEMBER diffFrequencies)
    Q_PROPERTY(Eigen::VectorXd errorFrequencies MEMBER errorFrequencies)
    Q_PROPERTY(Eigen::VectorXd errorsMAC MEMBER errorsMAC)
    Q_PROPERTY(ModalPairs pairs MEMBER pairs)

public:
    ModalComparison();
    ~ModalComparison();

    bool isEmpty() const;
    bool isValid() const;
    void resize(int numModes);

    bool operator==(ModalComparison const& another) const;
    bool operator!=(ModalComparison const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    Eigen::VectorXd diffFrequencies;
    Eigen::VectorXd errorFrequencies;
    Eigen::VectorXd errorsMAC;
    ModalPairs pairs;
};

struct ModalOptions : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(int numModes MEMBER numModes)
    Q_PROPERTY(double timeout MEMBER timeout)

public:
    ModalOptions();
    ~ModalOptions();

    bool operator==(ModalOptions const& another) const;
    bool operator!=(ModalOptions const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    //! Number of modes to compute
    int numModes;

    //! Maximum duration of solution
    double timeout;
};

class ModalSolver : public QObject, public ISolver
{
    Q_OBJECT
    Q_PROPERTY(KCL::Model model MEMBER model)
    Q_PROPERTY(ModalOptions options MEMBER options)
    Q_PROPERTY(ModalSolution solution MEMBER solution)
    Q_PROPERTY(QString log MEMBER log)

public:
    ModalSolver();
    ~ModalSolver();
    ModalSolver(ModalSolver const& another);
    ModalSolver(ModalSolver&& another);
    ModalSolver& operator=(ModalSolver const& another);

    ISolver::Type type() const override;
    ISolver* clone() const override;

    void clear() override;
    void solve() override;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    bool operator==(ISolver const* pBaseSolver) const override;
    bool operator!=(ISolver const* pBaseSolver) const override;

signals:
    void solverFinished();

public:
    QString name;
    KCL::Model model;
    ModalOptions options;
    ModalSolution solution;
    QString log;
};
}

#endif // MODALSOLVER_H
