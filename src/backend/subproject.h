#ifndef SUBPROJECT_H
#define SUBPROJECT_H

#include <kcl/model.h>
#include <QString>

#include "identifier.h"
#include "iserializable.h"
#include "optimsolver.h"
#include "selector.h"

namespace Backend::Core
{

class Subproject : public Identifier, public ISerializable
{
    Q_GADGET
    Q_PROPERTY(QUuid id MEMBER mID)
    Q_PROPERTY(QString name MEMBER mName)
    Q_PROPERTY(KCL::Model model MEMBER mModel)
    Q_PROPERTY(QList<ISolver*> solvers MEMBER mSolvers)

public:
    Subproject();
    Subproject(QString const& name);
    ~Subproject();

    Subproject(Subproject const& another);
    Subproject(Subproject&& another);
    Subproject& operator=(Subproject const& another);

    QString const& name() const;
    KCL::Model const& model() const;
    QList<ISolver*> const& solvers() const;
    int numSolvers() const;

    QString& name();
    KCL::Model& model();
    QList<ISolver*>& solvers();

    ISolver* solver(int index);
    ISolver* addSolver(ISolver::Type type);
    void removeSolver(int index);
    void removeAllSolvers();
    void clear();

    bool operator==(Subproject const& another) const;
    bool operator!=(Subproject const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

private:
    QString mName;
    KCL::Model mModel;
    QList<ISolver*> mSolvers;
};
}

#endif // SUBPROJECT_H
