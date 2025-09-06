#include <kcl/model.h>

#include "fileutility.h"
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

void serialize(QXmlStreamWriter& stream, QString const& name, QVariant const& variant)
{
    int type = variant.typeId();
    int lastPos = stream.device()->pos();

    // Process standard types
    switch (type)
    {
    case QMetaType::Bool:
        stream.writeTextElement(name, variant.toBool() ? "true" : "false");
        return;
    case QMetaType::Int:
        stream.writeTextElement(name, QString::number(variant.toInt()));
        return;
    case QMetaType::Double:
        stream.writeTextElement(name, QString::number(variant.toDouble()));
        return;
    case QMetaType::QString:
        stream.writeTextElement(name, variant.toString());
        return;
    case QMetaType::QStringList:
        serialize(stream, name, variant.toStringList());
        return;
    case QMetaType::QUuid:
        stream.writeTextElement(name, variant.value<QUuid>().toString());
        return;
    default:
        break;
    }

    // Process custom types
    if (type == qMetaTypeId<Subproject>())
        serialize(stream, variant.value<Subproject>());
    else if (type == qMetaTypeId<Configuration>())
        serialize(stream, variant.value<Configuration>());
    else if (type == qMetaTypeId<OptimProblem>())
        serialize(stream, variant.value<OptimProblem>());
    else if (type == qMetaTypeId<OptimOptions>())
        serialize(stream, variant.value<OptimOptions>());
    else if (type == qMetaTypeId<ModalSolution>())
        serialize(stream, variant.value<ModalSolution>());
    else if (type == qMetaTypeId<ModalComparison>())
        serialize(stream, variant.value<ModalComparison>());
    else if (type == qMetaTypeId<Geometry>())
        serialize(stream, variant.value<Geometry>());
    else if (type == qMetaTypeId<Selector>())
        serialize(stream, variant.value<Selector>());
    else if (type == qMetaTypeId<Constraints>())
        serialize(stream, variant.value<Constraints>());
    else if (type == qMetaTypeId<SelectionSet>())
        serialize(stream, variant.value<SelectionSet>());
    else if (type == qMetaTypeId<KCL::Model>())
        serialize(stream, variant.value<KCL::Model>());
    else if (type == qMetaTypeId<KCL::ElementType>())
        serialize(stream, name, variant.toInt());
    else if (type == qMetaTypeId<QList<Subproject>>())
        serialize(stream, name, variant.value<QList<Subproject>>());
    else if (type == qMetaTypeId<QList<Vertex>>())
        serialize(stream, name, variant.value<QList<Vertex>>());
    else if (type == qMetaTypeId<QList<Slave>>())
        serialize(stream, name, variant.value<QList<Slave>>());
    else if (type == qMetaTypeId<QList<SelectionSet>>())
        serialize(stream, name, variant.value<QList<SelectionSet>>());
    else if (type == qMetaTypeId<QList<PairInt>>())
        serialize(stream, name, variant.value<QList<PairInt>>());
    else if (type == qMetaTypeId<QMap<Selection, bool>>())
        serialize(stream, name, variant.value<QMap<Selection, bool>>());
    else if (type == qMetaTypeId<QMap<VariableType, bool>>())
        serialize(stream, name, variant.value<QMap<VariableType, bool>>());
    else if (type == qMetaTypeId<QMap<VariableType, double>>())
        serialize(stream, name, variant.value<QMap<VariableType, double>>());
    else if (type == qMetaTypeId<QMap<VariableType, PairDouble>>())
        serialize(stream, name, variant.value<QMap<VariableType, PairDouble>>());
    else if (type == qMetaTypeId<PairDouble>())
        serialize(stream, name, variant.value<PairDouble>());
    else if (type == qMetaTypeId<Eigen::Vector3d>())
        serialize(stream, name, variant.value<Eigen::Vector3d>());
    else if (type == qMetaTypeId<Eigen::VectorXi>())
        serialize(stream, name, variant.value<Eigen::VectorXi>());
    else if (type == qMetaTypeId<Eigen::VectorXd>())
        serialize(stream, name, variant.value<Eigen::VectorXd>());
    else if (type == qMetaTypeId<Eigen::MatrixXi>())
        serialize(stream, name, variant.value<Eigen::MatrixXi>());
    else if (type == qMetaTypeId<Eigen::MatrixXd>())
        serialize(stream, name, variant.value<Eigen::MatrixXd>());
    else if (type == qMetaTypeId<QList<Eigen::MatrixXd>>())
        serialize(stream, name, variant.value<QList<Eigen::MatrixXd>>());
    else if (type == qMetaTypeId<Direction>())
        serialize(stream, name, (int) variant.value<Direction>());

    // Process elements which cannot be serialized
    if (stream.device()->pos() == lastPos)
    {
        stream.writeEmptyElement(name);
        qWarning() << QObject::tr("Could not serialize %1").arg(name);
    }
}

void serialize(QXmlStreamWriter& stream, QString const& name, QList<QString> const& items)
{
    stream.writeStartElement(name);
    for (auto const& item : items)
        stream.writeTextElement("value", item);
    stream.writeEndElement();
}

void serialize(QXmlStreamWriter& stream, QString const& name, QMap<Backend::Core::Selection, bool> const& map)
{
    stream.writeStartElement(name);
    for (auto const& [key, value] : map.asKeyValueRange())
    {
        stream.writeStartElement("item");
        serialize(stream, key);
        serialize(stream, "value", value);
        stream.writeEndElement();
    }
    stream.writeEndElement();
}

void serialize(QXmlStreamWriter& stream, QString const& name, QList<Eigen::MatrixXd> const& matrices)
{
    stream.writeStartElement(name);
    for (auto const& matrix : matrices)
        serialize(stream, "value", matrix);
    stream.writeEndElement();
}

template<>
void serialize(QXmlStreamWriter& stream, KCL::Model const& model)
{
    QString text;
    if (!model.isEmpty())
    {
        text = model.toString().c_str();
        QByteArray data(text.toUtf8());
        data = qCompress(data).toBase64();
        text = QString::fromLatin1(data);
    }
    stream.writeStartElement("model");
    stream.writeCharacters(text);
    stream.writeEndElement();
}

void deserialize(QXmlStreamReader& stream, KCL::Model& model)
{
    QString value = stream.readElementText();
    QByteArray data = QByteArray::fromBase64(value.toLatin1());
    QString text = QString::fromUtf8(qUncompress(data));
    if (!text.isEmpty())
        model.fromString(text.toStdString());
}
}
