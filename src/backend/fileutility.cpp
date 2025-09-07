#include <kcl/model.h>
#include <QUuid>

#include "fileutility.h"
#include "selectionset.h"

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
    int type = variant.typeId();

    switch (type)
    {
    case QMetaType::Bool:
        return QString::number(variant.value<int>());
    case QMetaType::Int:
        return QString::number(variant.value<int>());
    case QMetaType::Double:
        return QString::number(variant.value<double>());
    case QMetaType::QString:
        return variant.value<QString>();
    case QMetaType::QUuid:
        return variant.value<QUuid>().toString();
    default:
        break;
    }

    return QString();
}

void serialize(QXmlStreamWriter& stream, QString const& elementName, QString const& objectName, QList<Eigen::MatrixXd> const& matrices)
{
    stream.writeStartElement(elementName);
    for (auto const& matrix : matrices)
        serialize(stream, objectName, matrix);
    stream.writeEndElement();
}

void deserialize(QXmlStreamReader& stream, QString const& objectName, QList<Eigen::MatrixXd>& matrices)
{
    matrices.clear();
    while (stream.readNextStartElement())
    {
        if (stream.name() == objectName)
        {
            Eigen::MatrixXd matrix;
            deserialize(stream, matrix);
            matrices.emplaceBack(std::move(matrix));
        }
        else
        {
            stream.skipCurrentElement();
        }
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
    QByteArray data = QByteArray::fromBase64(text.toLatin1());
    text = QString::fromUtf8(qUncompress(data));
    if (!text.isEmpty())
        model.fromString(text.toStdString());
}
}
