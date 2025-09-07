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

struct Configuration : public ISerializable
{
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(OptimProblem problem MEMBER problem)
    Q_PROPERTY(OptimOptions options MEMBER options)

public:
    Configuration();
    ~Configuration() = default;

    bool operator==(Configuration const& another) const;
    bool operator!=(Configuration const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

    QString name;
    OptimProblem problem;
    OptimOptions options;
};

class Subproject : public Identifier, public ISerializable
{
    Q_GADGET
    Q_PROPERTY(Configuration configuration MEMBER mConfiguration)
    Q_PROPERTY(KCL::Model model MEMBER mModel)
    Q_PROPERTY(QList<OptimSolution> optimSolutions MEMBER mOptimSolutions)

public:
    Subproject();
    Subproject(QString const& name);
    ~Subproject() = default;

    QString const& name() const;
    Configuration const& configuration() const;
    KCL::Model const& model() const;
    QList<OptimSolution> const& optimSolutions() const;

    Configuration& configuration();
    KCL::Model& model();
    QList<OptimSolution>& optimSolutions();

    void clear();

    bool operator==(Subproject const& another) const;
    bool operator!=(Subproject const& another) const;

    void serialize(QXmlStreamWriter& stream, QString const& elementName) const override;
    void deserialize(QXmlStreamReader& stream) override;

private:
    Configuration mConfiguration;
    KCL::Model mModel;
    QList<OptimSolution> mOptimSolutions;
};

QXmlStreamWriter& operator<<(QXmlStreamWriter& stream, Configuration const& configuration);
QXmlStreamReader& operator>>(QXmlStreamReader& stream, Configuration& configuration);
QXmlStreamWriter& operator<<(QXmlStreamWriter& stream, Subproject const& subproject);
QXmlStreamReader& operator>>(QXmlStreamReader& stream, Subproject& subproject);
}

#endif // SUBPROJECT_H
