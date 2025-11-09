
#ifndef FILEUTILITY_H
#define FILEUTILITY_H

#include <Eigen/Core>
#include <QDir>
#include <QFile>
#include <QMetaProperty>
#include <QString>
#include <QXmlStreamWriter>

#include "constraints.h"
#include "iserializable.h"
#include "isolver.h"

namespace KCL
{
struct Model;
}

namespace Backend::Core
{
struct Selection;
}

namespace Backend::Utility
{

QSharedPointer<QFile> openFile(QString const& pathFile, QString const& expectedSuffix, QIODevice::OpenModeFlag const& mode);

//! Base case for combining a filepath
template<typename T>
QString combineFilePath(T const& value)
{
    return value;
}

//! Combine several components of a filepath, adding slashes if necessary
template<typename T, typename... Args>
QString combineFilePath(T const& first, Args... args)
{
    return QDir(first).filePath(combineFilePath(args...));
}

void appendLog(QString& log, QString const& message, bool isTime = true);
QString toString(QVariant const& variant);
QString toString(bool value);
QString toString(int value);
QString toString(double value);
QString toString(QUuid const& value);
QString toString(std::complex<double> const& value);

template<typename T, typename M>
QString toString(QPair<T, M> const& pair)
{
    return QString("%1 %2").arg(toString(pair.first), toString(pair.second));
}

template<typename T>
void fromString(QString const& text, T& value)
{
    QVariant variant(text);
    value = variant.value<T>();
}

template<typename T, typename M>
void fromString(QString text, QPair<T, M>& value)
{
    QTextStream stream(&text);
    QString first, second;
    stream >> first >> second;
    fromString(first, value.first);
    fromString(second, value.second);
}

template<>
void fromString(QString const& text, std::complex<double>& value);

template<typename T>
void serializeProperties(QXmlStreamWriter& stream, QString const& elementName, T const& object)
{
    QMetaObject const& metaObject = object.staticMetaObject;
    int numProperties = metaObject.propertyCount();
    stream.writeStartElement(elementName);
    for (int i = 0; i != numProperties; ++i)
    {
        QString name = metaObject.property(i).name();
        QVariant variant = metaObject.property(i).readOnGadget(&object);
        stream.writeTextElement(name, Utility::toString(variant));
    }
    stream.writeEndElement();
}

template<typename T>
void deserializeProperties(QXmlStreamReader& stream, T& object)
{
    QMetaObject const& metaObject = object.staticMetaObject;
    while (stream.readNextStartElement())
    {
        auto name = stream.name().toString();
        int iProperty = metaObject.indexOfProperty(name.toStdString().c_str());
        if (iProperty > -1)
            metaObject.property(iProperty).writeOnGadget(&object, stream.readElementText());
        else
            stream.skipCurrentElement();
    }
}

template<typename T>
void serialize(QXmlStreamWriter& stream, QString const& elementName, QString const& objectName, QList<T> const& objects)
{
    if (!std::is_base_of<Core::ISerializable, T>())
        return;
    stream.writeStartElement(elementName);
    for (auto const& object : objects)
        object.serialize(stream, objectName);
    stream.writeEndElement();
}

template<typename T>
void deserialize(QXmlStreamReader& stream, QString const& objectName, QList<T>& objects)
{
    objects.clear();
    if (!std::is_base_of<Core::ISerializable, T>())
        return;
    while (stream.readNextStartElement())
    {
        if (stream.name() == objectName)
        {
            T object;
            object.deserialize(stream);
            objects.emplaceBack(std::move(object));
        }
        else
        {
            stream.skipCurrentElement();
        }
    }
}

template<typename T, typename M>
void serialize(QXmlStreamWriter& stream, QString const& elementName, QList<QPair<T, M>> const& items)
{
    stream.writeStartElement(elementName);
    for (auto const& item : items)
    {
        stream.writeStartElement("item");
        stream.writeAttribute("first", toString(item.first));
        stream.writeAttribute("second", toString(item.second));
        stream.writeEndElement();
    }
    stream.writeEndElement();
}

template<typename T, typename M>
void deserialize(QXmlStreamReader& stream, QList<QPair<T, M>>& items)
{
    items.clear();
    while (stream.readNextStartElement())
    {
        if (stream.name() == "item")
        {
            T first;
            M second;
            fromString(stream.attributes().value("first").toString(), first);
            fromString(stream.attributes().value("second").toString(), second);
            items.push_back({first, second});
            stream.readNextStartElement();
        }
        else
        {
            stream.skipCurrentElement();
        }
    }
}

template<typename T>
void serialize(QXmlStreamWriter& stream, QString const& elementName, QMap<Backend::Core::VariableType, T> const& map)
{
    stream.writeStartElement(elementName);
    for (auto const& [key, value] : map.asKeyValueRange())
    {
        stream.writeStartElement("item");
        stream.writeAttribute("key", toString((int) key));
        stream.writeAttribute("value", toString(value));
        stream.writeEndElement();
    }
    stream.writeEndElement();
}

template<typename T>
void deserialize(QXmlStreamReader& stream, QMap<Backend::Core::VariableType, T>& map)
{
    map.clear();
    while (stream.readNextStartElement())
    {
        if (stream.name() == "item")
        {
            auto variable = (Backend::Core::VariableType) stream.attributes().value("key").toInt();
            T value;
            fromString(stream.attributes().value("value").toString(), value);
            map[variable] = value;
            stream.readNextStartElement();
        }
    }
}

template<typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
void serialize(QXmlStreamWriter& stream, QString const& elementName, Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols> const& matrix)
{
    QString text;
    QTextStream textStream(&text);
    stream.writeStartElement(elementName);
    int numRows = matrix.rows();
    int numCols = matrix.cols();
    stream.writeAttribute("numRows", QString::number(numRows));
    stream.writeAttribute("numCols", QString::number(numCols));
    for (int i = 0; i != numRows; ++i)
    {
        for (int j = 0; j != numCols; ++j)
        {
            if (i > 0 || j > 0)
                textStream << " ";
            textStream << toString(matrix(i, j));
        }
    }
    stream.writeCharacters(text);
    stream.writeEndElement();
}

template<typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
void deserialize(QXmlStreamReader& stream, Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols>& matrix)
{
    int numRows = stream.attributes().value("numRows").toInt();
    int numCols = stream.attributes().value("numCols").toInt();
    QString text = stream.readElementText();
    QTextStream textStream(&text);
    matrix.resize(numRows, numCols);
    for (int i = 0; i != numRows; ++i)
    {
        for (int j = 0; j != numCols; ++j)
        {
            QString token;
            textStream >> token;
            fromString(token, matrix(i, j));
        }
    }
}

template<typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
void serialize(QXmlStreamWriter& stream, QString const& elementName, QString const& objectName,
               QList<Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols>> const& matrices)
{
    stream.writeStartElement(elementName);
    for (auto const& matrix : matrices)
        serialize(stream, objectName, matrix);
    stream.writeEndElement();
}

template<typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
void deserialize(QXmlStreamReader& stream, QString const& objectName,
                 QList<Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols>>& matrices)
{
    matrices.clear();
    while (stream.readNextStartElement())
    {
        if (stream.name() == objectName)
        {
            Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols> matrix;
            deserialize(stream, matrix);
            matrices.emplaceBack(std::move(matrix));
        }
        else
        {
            stream.skipCurrentElement();
        }
    }
}

void serialize(QXmlStreamWriter& stream, QString const& elementName, QString const& objectName, QList<QString> const& items);
void deserialize(QXmlStreamReader& stream, QString const& objectName, QList<QString>& items);

void serialize(QXmlStreamWriter& stream, QString const& elementName, QMap<Backend::Core::Selection, bool> const& map);
void deserialize(QXmlStreamReader& stream, QMap<Backend::Core::Selection, bool>& map);

void serialize(QXmlStreamWriter& stream, QString const& elementName, KCL::Model const& model);
void deserialize(QXmlStreamReader& stream, KCL::Model& model);

void serialize(QXmlStreamWriter& stream, QString const& elementName, QString const& text);
void deserialize(QXmlStreamReader& stream, QString& text);

//! Check if metaobjects are equal
template<typename T>
bool areEqual(T const& first, T const& second)
{
    QMetaObject const& metaObject = first.staticMetaObject;
    int numProperties = metaObject.propertyCount();
    while (numProperties > 0)
    {
        QMetaProperty metaProperty = metaObject.property(numProperties - 1);
        QVariant firstVariant, secondVariant;
        if constexpr (std::is_base_of<QObject, T>())
        {
            firstVariant = metaProperty.read(&first);
            secondVariant = metaProperty.read(&second);
        }
        else
        {
            firstVariant = metaProperty.readOnGadget(&first);
            secondVariant = metaProperty.readOnGadget(&second);
        }
        if (!areEqual(firstVariant, secondVariant))
            return false;
        --numProperties;
    }
    return true;
}

template<>
bool areEqual(QVariant const& first, QVariant const& second);
template<>
bool areEqual(QList<Core::ISolver*> const& first, QList<Core::ISolver*> const& second);

bool areEqual(double first, double second, double tolerance);
bool areEqual(std::complex<double> first, std::complex<double> second, double tolerance);
bool areEqual(Core::PairDouble const& first, Core::PairDouble const& second, double tolerance);
bool areEqual(Core::ModalPairs const& first, Core::ModalPairs const& second, double tolerance);

template<typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
bool areEqual(Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols> const& first,
              Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols> const& second, double tolerance)
{
    if (first.size() != second.size())
        return false;
    int numRows = first.rows();
    int numCols = second.cols();
    for (int i = 0; i != numRows; ++i)
    {
        for (int j = 0; j != numCols; ++j)
        {
            if (!areEqual(first(i, j), second(i, j), tolerance))
                return false;
        }
    }
    return true;
}

template<typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
bool areEqual(QList<Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols>> const& first,
              QList<Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols>> const& second, double tolerance)
{
    if (first.size() != second.size())
        return false;
    int numValues = first.size();
    for (int k = 0; k != numValues; ++k)
    {
        if (!areEqual(first[k], second[k], tolerance))
            return false;
    }
    return true;
}

template<typename T, typename M>
bool areEqual(QMap<T, M> const& first, QMap<T, M> const& second, double tolerance)
{
    auto const keys = first.keys();
    if (keys != second.keys())
        return false;
    for (auto const& key : keys)
    {
        if (!areEqual(QVariant::fromValue(first[key]), QVariant::fromValue(second[key])))
            return false;
    }
    return true;
}
}

#endif // FILEUTILITY_H
