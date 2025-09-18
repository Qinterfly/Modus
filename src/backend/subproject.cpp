#include "subproject.h"
#include "fileutility.h"
#include "fluttersolver.h"

using namespace Backend::Core;

ISolver* createSolver(ISolver::Type type);

Subproject::Subproject()
{
}

Subproject::Subproject(QString const& name)
    : mName(name)
{
}

Subproject::~Subproject()
{
    clear();
}

Subproject::Subproject(Subproject const& another)
    : mName(another.mName)
    , mModel(another.mModel)
{
    for (ISolver const* pSolver : another.mSolvers)
        mSolvers.push_back(pSolver->clone());
}

Subproject::Subproject(Subproject&& another)
{
    mID = std::move(another.mID);
    mName = std::move(another.mName);
    mModel = std::move(another.mModel);
    mSolvers = std::move(another.mSolvers);
}

Subproject& Subproject::operator=(Subproject const& another)
{
    clear();
    mName = another.mName;
    mModel = another.mModel;
    for (ISolver const* pSolver : another.mSolvers)
        mSolvers.push_back(pSolver->clone());
    return *this;
}

QString const& Subproject::name() const
{
    return mName;
}

KCL::Model const& Subproject::model() const
{
    return mModel;
}

QList<ISolver*> const& Subproject::solvers() const
{
    return mSolvers;
}

int Subproject::numSolvers() const
{
    return mSolvers.size();
}

QString& Subproject::name()
{
    return mName;
}

KCL::Model& Subproject::model()
{
    return mModel;
}

QList<ISolver*>& Subproject::solvers()
{
    return mSolvers;
}

ISolver* Subproject::solver(int index)
{
    ISolver* pSolver = nullptr;
    if (index >= 0 && index < mSolvers.size())
        pSolver = mSolvers[index];
    return pSolver;
}

ISolver* Subproject::addSolver(ISolver::Type type)
{
    ISolver* pSolver = createSolver(type);
    if (pSolver)
        mSolvers.push_back(pSolver);
    return pSolver;
}

void Subproject::removeSolver(int index)
{
    if (index >= 0 && index < mSolvers.size())
    {
        delete mSolvers[index];
        mSolvers.remove(index);
    }
}

void Subproject::removeAllSolvers()
{
    int numSolvers = mSolvers.size();
    for (int i = 0; i != numSolvers; ++i)
        delete mSolvers[i];
    mSolvers.clear();
}

void Subproject::clear()
{
    mName = QString();
    mModel = KCL::Model();
    removeAllSolvers();
}

bool Subproject::operator==(Subproject const& another) const
{
    return Utility::areEqual(*this, another);
}

bool Subproject::operator!=(Subproject const& another) const
{
    return !(*this == another);
}

void Subproject::serialize(QXmlStreamWriter& stream, QString const& elementName) const
{
    stream.writeStartElement(elementName);
    stream.writeTextElement("id", mID.toString());
    stream.writeTextElement("name", mName);
    Utility::serialize(stream, "model", mModel);
    for (ISolver* pSolver : mSolvers)
        pSolver->serialize(stream, "solver");
    stream.writeEndElement();
}

void Subproject::deserialize(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "id")
        {
            mID = QUuid::fromString(stream.readElementText());
        }
        else if (stream.name() == "name")
        {
            mName = stream.readElementText();
        }
        else if (stream.name() == "model")
        {
            Utility::deserialize(stream, mModel);
        }
        else if (stream.name() == "solver")
        {
            ISolver::Type type = (ISolver::Type) stream.attributes().value("type").toInt();
            ISolver* pSolver = createSolver(type);
            if (pSolver)
            {
                pSolver->deserialize(stream);
                mSolvers.push_back(pSolver);
            }
        }
        else
        {
            stream.skipCurrentElement();
        }
    }
}

//! Helper function to create solvers of specified type
ISolver* createSolver(ISolver::Type type)
{
    ISolver* pSolver = nullptr;
    switch (type)
    {
    case ISolver::kModal:
        pSolver = new ModalSolver;
        break;
    case ISolver::kOptim:
        pSolver = new OptimSolver;
        break;
    case ISolver::kFlutter:
        pSolver = new FlutterSolver;
        break;
    }
    return pSolver;
}
