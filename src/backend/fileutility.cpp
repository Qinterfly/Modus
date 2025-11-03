#include <kcl/model.h>
#include <QUuid>

#include "fileutility.h"
#include "fluttersolver.h"
#include "optimsolver.h"
#include "selectionset.h"
#include "subproject.h"

using namespace Backend::Core;

namespace Backend::Utility
{

//! Open a file and check its extension
QSharedPointer<QFile> openFile(QString const& pathFile, QString const& expectedSuffix, QIODevice::OpenModeFlag const& mode)
{
    // Check if the output file has the correct extension
    QFileInfo info(pathFile);
    if (info.suffix() != expectedSuffix)
    {
        qWarning() << QObject::tr("Unknown extension was specified for the file: %1").arg(pathFile);
        return nullptr;
    }

    // Open the file for the specified mode
    QSharedPointer<QFile> pFile(new QFile(pathFile));
    if (!pFile->open(mode))
    {
        qWarning() << QObject::tr("Could not open the file: %1").arg(pathFile);
        return nullptr;
    }
    return pFile;
}

QString toString(QVariant const& variant)
{
    int const kPrecision = 10;

    int type = variant.typeId();
    switch (type)
    {
    case QMetaType::Bool:
        return toString(variant.value<int>());
    case QMetaType::Int:
        return toString(variant.value<int>());
    case QMetaType::Double:
        return toString(variant.value<double>());
    case QMetaType::QString:
        return variant.value<QString>();
    case QMetaType::QUuid:
        return toString(variant.value<QUuid>());
    default:
        break;
    }

    return QString();
}

QString toString(bool value)
{
    return QString::number(value);
}

QString toString(int value)
{
    return QString::number(value);
}

QString toString(double value)
{
    return QString::number(value, 'g', 12);
}

QString toString(QUuid const& value)
{
    return value.toString();
}

QString toString(std::complex<double> const& value)
{
    return QString("(%1,%2)").arg(toString(value.real()), toString(value.imag()));
}

template<>
void fromString(QString const& text, std::complex<double>& value)
{
    QString parsedText = text;
    parsedText.remove('(');
    parsedText.remove(')');
    QStringList tokens = parsedText.split(",");
    if (tokens.size() == 2)
    {
        double real, imag;
        fromString(tokens[0], real);
        fromString(tokens[1], imag);
        value = {real, imag};
    }
}

void serialize(QXmlStreamWriter& stream, QString const& elementName, QString const& objectName, QList<QString> const& items)
{
    stream.writeStartElement(elementName);
    for (auto const& item : items)
        stream.writeTextElement(objectName, item);
    stream.writeEndElement();
}

void deserialize(QXmlStreamReader& stream, QString const& objectName, QList<QString>& items)
{
    items.clear();
    while (stream.readNextStartElement())
    {
        if (stream.name() == objectName)
            items.emplaceBack(stream.readElementText());
        else
            stream.skipCurrentElement();
    }
}

void serialize(QXmlStreamWriter& stream, QString const& elementName, QMap<Backend::Core::Selection, bool> const& map)
{
    stream.writeStartElement(elementName);
    for (auto const& [key, value] : map.asKeyValueRange())
    {
        stream.writeStartElement("item");
        key.serialize(stream, "selection");
        stream.writeTextElement("flag", toString(value));
        stream.writeEndElement();
    }
    stream.writeEndElement();
}

void deserialize(QXmlStreamReader& stream, QMap<Backend::Core::Selection, bool>& map)
{
    map.clear();
    while (stream.readNextStartElement())
    {
        if (stream.name() == "item")
        {
            stream.readNextStartElement();
            if (stream.name() == "selection")
            {
                Selection key;
                key.deserialize(stream);
                stream.readNextStartElement();
                if (stream.name() == "flag")
                {
                    bool flag = stream.readElementText().toInt();
                    map[key] = flag;
                }
            }
            stream.readNextStartElement();
        }
        else
        {
            stream.skipCurrentElement();
        }
    }
}

void serialize(QXmlStreamWriter& stream, QString const& elementName, KCL::Model const& model)
{
    QString text;
    if (!model.isEmpty())
    {
        text = model.toString().c_str();
        QByteArray data(text.toUtf8());
        data = qCompress(data).toBase64();
        text = QString::fromLatin1(data);
    }
    stream.writeStartElement(elementName);
    stream.writeCharacters(text);
    stream.writeEndElement();
}

void deserialize(QXmlStreamReader& stream, KCL::Model& model)
{
    QString text = stream.readElementText();
    if (!text.isEmpty())
    {
        QByteArray data = QByteArray::fromBase64(text.toLatin1());
        text = QString::fromUtf8(qUncompress(data));
        model.fromString(text.toStdString());
    }
}

template<>
bool areEqual(QVariant const& first, QVariant const& second)
{
    double const kTolerance = 1e-6;

    if (first.typeId() != second.typeId())
        return false;

    // Handle standard types
    int type = first.typeId();
    switch (type)
    {
    case QMetaType::Bool:
        return first.value<int>() == second.value<int>();
    case QMetaType::Int:
        return first.value<int>() == second.value<int>();
    case QMetaType::Double:
        return areEqual(first.value<double>(), second.value<double>(), kTolerance);
    case QMetaType::QString:
        return first.value<QString>() == second.value<QString>();
    case QMetaType::QUuid:
        return first.value<QUuid>() == second.value<QUuid>();
    default:
        break;
    }

    // Handle custom types
    bool flag = true;
    if (type == qMetaTypeId<QPair<double, double>>())
        flag = areEqual(first.value<PairDouble>(), second.value<PairDouble>(), kTolerance);
    else if (type == qMetaTypeId<QStringList>())
        flag = first.value<QStringList>() == second.value<QStringList>();
    else if (type == qMetaTypeId<KCL::Model>())
        flag = first.value<KCL::Model>() == second.value<KCL::Model>();
    else if (type == qMetaTypeId<Direction>())
        flag = first.value<Direction>() == second.value<Direction>();
    else if (type == qMetaTypeId<Geometry>())
        flag = first.value<Geometry>() == second.value<Geometry>();
    else if (type == qMetaTypeId<QList<Vertex>>())
        flag = first.value<QList<Vertex>>() == second.value<QList<Vertex>>();
    else if (type == qMetaTypeId<QList<Slave>>())
        flag = first.value<QList<Slave>>() == second.value<QList<Slave>>();
    else if (type == qMetaTypeId<ModalOptions>())
        flag = first.value<ModalOptions>() == second.value<ModalOptions>();
    else if (type == qMetaTypeId<ModalSolution>())
        flag = first.value<ModalSolution>() == second.value<ModalSolution>();
    else if (type == qMetaTypeId<ModalComparison>())
        flag = first.value<ModalComparison>() == second.value<ModalComparison>();
    else if (type == qMetaTypeId<Constraints>())
        flag = first.value<Constraints>() == second.value<Constraints>();
    else if (type == qMetaTypeId<Selector>())
        flag = first.value<Selector>() == second.value<Selector>();
    else if (type == qMetaTypeId<OptimProblem>())
        flag = first.value<OptimProblem>() == second.value<OptimProblem>();
    else if (type == qMetaTypeId<OptimOptions>())
        flag = first.value<OptimOptions>() == second.value<OptimOptions>();
    else if (type == qMetaTypeId<FlutterOptions>())
        flag = first.value<FlutterOptions>() == second.value<FlutterOptions>();
    else if (type == qMetaTypeId<FlutterSolution>())
        flag = first.value<FlutterSolution>() == second.value<FlutterSolution>();
    else if (type == qMetaTypeId<QList<SelectionSet>>())
        flag = first.value<QList<SelectionSet>>() == second.value<QList<SelectionSet>>();
    else if (type == qMetaTypeId<QList<OptimSolution>>())
        flag = first.value<QList<OptimSolution>>() == second.value<QList<OptimSolution>>();
    else if (type == qMetaTypeId<QList<Subproject>>())
        flag = first.value<QList<Subproject>>() == second.value<QList<Subproject>>();
    else if (type == qMetaTypeId<QList<ISolver*>>())
        flag = areEqual(first.value<QList<ISolver*>>(), second.value<QList<ISolver*>>());
    else if (type == qMetaTypeId<QList<PairInt>>())
        flag = first.value<QList<PairInt>>() == second.value<QList<PairInt>>();
    else if (type == qMetaTypeId<ModalPairs>())
        flag = areEqual(first.value<ModalPairs>(), second.value<ModalPairs>(), kTolerance);
    else if (type == qMetaTypeId<QMap<Selection, bool>>())
        flag = areEqual(first.value<QMap<Selection, bool>>(), second.value<QMap<Selection, bool>>(), kTolerance);
    else if (type == qMetaTypeId<QMap<VariableType, bool>>())
        flag = areEqual(first.value<QMap<VariableType, bool>>(), second.value<QMap<VariableType, bool>>(), kTolerance);
    else if (type == qMetaTypeId<QMap<VariableType, double>>())
        flag = areEqual(first.value<QMap<VariableType, double>>(), second.value<QMap<VariableType, double>>(), kTolerance);
    else if (type == qMetaTypeId<QMap<VariableType, PairDouble>>())
        flag = areEqual(first.value<QMap<VariableType, PairDouble>>(), second.value<QMap<VariableType, PairDouble>>(), kTolerance);
    else if (type == qMetaTypeId<QList<Eigen::MatrixXd>>())
        flag = areEqual(first.value<QList<Eigen::MatrixXd>>(), second.value<QList<Eigen::MatrixXd>>(), kTolerance);
    else if (type == qMetaTypeId<QList<Eigen::MatrixXcd>>())
        flag = areEqual(first.value<QList<Eigen::MatrixXcd>>(), second.value<QList<Eigen::MatrixXcd>>(), kTolerance);
    else if (type == qMetaTypeId<Eigen::MatrixXi>())
        flag = first.value<Eigen::MatrixXi>() == second.value<Eigen::MatrixXi>();
    else if (type == qMetaTypeId<Eigen::VectorXi>())
        flag = first.value<Eigen::VectorXi>() == second.value<Eigen::VectorXi>();
    else if (type == qMetaTypeId<Eigen::MatrixXd>())
        flag = areEqual(first.value<Eigen::MatrixXd>(), second.value<Eigen::MatrixXd>(), kTolerance);
    else if (type == qMetaTypeId<Eigen::VectorXd>())
        flag = areEqual(first.value<Eigen::VectorXd>(), second.value<Eigen::VectorXd>(), kTolerance);
    else if (type == qMetaTypeId<Eigen::Vector3d>())
        flag = areEqual(first.value<Eigen::Vector3d>(), second.value<Eigen::Vector3d>(), kTolerance);
    else if (type == qMetaTypeId<Eigen::MatrixXcd>())
        flag = areEqual(first.value<Eigen::MatrixXcd>(), second.value<Eigen::MatrixXcd>(), kTolerance);
    else
        flag = false;
    if (!flag)
        qWarning() << QObject::tr("Values of the type %1 are not the same").arg(first.typeName());
    return flag;
}

template<>
bool areEqual(QList<Core::ISolver*> const& first, QList<Core::ISolver*> const& second)
{
    if (first.size() != second.size())
        return false;
    int numValues = first.size();
    for (int i = 0; i != numValues; ++i)
    {
        auto type = first[i]->type();
        if (type != second[i]->type())
            return false;
        switch (type)
        {
        case Core::ISolver::kModal:
        {
            auto pFirstSolver = (Core::ModalSolver*) first[i];
            auto pSecondSolver = (Core::ModalSolver*) second[i];
            if (!areEqual(*pFirstSolver, *pSecondSolver))
                return false;
            break;
        }
        case Core::ISolver::kOptim:
        {
            auto pFirstSolver = (Core::OptimSolver*) first[i];
            auto pSecondSolver = (Core::OptimSolver*) second[i];
            if (!areEqual(*pFirstSolver, *pSecondSolver))
                return false;
            break;
        }
        case Core::ISolver::kFlutter:
        {
            auto pFirstSolver = (Core::FlutterSolver*) first[i];
            auto pSecondSolver = (Core::FlutterSolver*) second[i];
            if (!areEqual(*pFirstSolver, *pSecondSolver))
                return false;
            break;
        }
        }
    }
    return true;
}

bool areEqual(double first, double second, double tolerance)
{
    if (std::isinf(first) && std::isinf(second) && std::signbit(first) == std::signbit(second))
        return true;
    if (std::isnan(first) && std::isnan(second))
        return true;
    if (std::abs(first - second) <= tolerance)
        return true;
    return false;
}

bool areEqual(std::complex<double> first, std::complex<double> second, double tolerance)
{
    bool isReal = areEqual(first.real(), second.real(), tolerance);
    bool isImag = areEqual(first.imag(), second.imag(), tolerance);
    return isReal && isImag;
}

bool areEqual(PairDouble const& first, PairDouble const& second, double tolerance)
{
    bool isFirst = areEqual(first.first, second.first, tolerance);
    bool isSecond = areEqual(first.second, second.second, tolerance);
    return isFirst && isSecond;
}

bool areEqual(Core::ModalPairs const& first, Core::ModalPairs const& second, double tolerance)
{
    if (first.size() != second.size())
        return false;
    int numValues = first.size();
    for (int k = 0; k != numValues; ++k)
    {
        if (first[k].first - second[k].first != 0)
            return false;
        if (!areEqual(first[k].second, second[k].second, tolerance))
            return false;
    }
    return true;
}
}
