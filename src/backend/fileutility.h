
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

//! Check if metaobjects are equal
template<typename T>
bool areEqual(T const& first, T const& second)
{
    QMetaObject const& metaObject = first.staticMetaObject;
    int numProperties = metaObject.propertyCount();
    QVariant firstValue, secondValue;
    while (numProperties > 0)
    {
        firstValue = metaObject.property(numProperties - 1).readOnGadget(&first);
        secondValue = metaObject.property(numProperties - 1).readOnGadget(&second);
        if (firstValue != secondValue)
            return false;
        --numProperties;
    }
    return true;
}

void serialize(QXmlStreamWriter& stream, QString const& name, QVariant const& variant);
void serialize(QXmlStreamWriter& stream, QString const& name, QList<QString> const& items);
void serialize(QXmlStreamWriter& stream, QString const& name, QMap<Backend::Core::Selection, bool> const& map);
void serialize(QXmlStreamWriter& stream, QString const& name, QList<Eigen::MatrixXd> const& matrices);

template<typename T>
void serialize(QXmlStreamWriter& stream, T const& object)
{
    if (!std::is_base_of<Core::ISerializable, T>())
        return;
    QMetaObject const& metaObject = object.staticMetaObject;
    int numProperties = metaObject.propertyCount();
    stream.writeStartElement(object.elementName());
    for (int i = 0; i != numProperties; ++i)
    {
        QString name = metaObject.property(i).name();
        QVariant variant = metaObject.property(i).readOnGadget(&object);
        serialize(stream, name, variant);
    }
    stream.writeEndElement();
}

template<typename T>
void serialize(QXmlStreamWriter& stream, QString const& name, QList<T> const& objects)
{
    if (!std::is_base_of<Core::ISerializable, T>())
        return;
    stream.writeStartElement(name);
    for (auto const& object : objects)
        serialize(stream, object);
    stream.writeEndElement();
}

template<typename T, typename M>
void serialize(QXmlStreamWriter& stream, QString const& name, QPair<T, M> const& pair)
{
    if (!std::is_arithmetic<T>() || !std::is_arithmetic<M>())
        return;
    stream.writeStartElement(name);
    stream.writeTextElement("first", QString::number(pair.first));
    stream.writeTextElement("second", QString::number(pair.second));
    stream.writeEndElement();
}

template<typename T, typename M>
void serialize(QXmlStreamWriter& stream, QString const& name, QList<QPair<T, M>> const& pairs)
{
    stream.writeStartElement(name);
    for (auto const& item : pairs)
        serialize(stream, "item", item);
    stream.writeEndElement();
}

template<typename T>
void serialize(QXmlStreamWriter& stream, QString const& name, QMap<Backend::Core::VariableType, T> const& map)
{
    stream.writeStartElement(name);
    for (auto const& [key, value] : map.asKeyValueRange())
    {
        stream.writeStartElement("item");
        serialize(stream, "key", (int) key);
        serialize(stream, "value", value);
        stream.writeEndElement();
    }
    stream.writeEndElement();
}

template<typename Derived>
void serialize(QXmlStreamWriter& stream, QString const& name, Eigen::MatrixBase<Derived> const& matrix)
{
    QString text;
    QTextStream textStream(&text);
    stream.writeStartElement(name);
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
            textStream << matrix(i, j);
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
        for (int j = 0; j != numCols; ++j)
            textStream >> matrix(i, j);
}

template<>
void serialize(QXmlStreamWriter& stream, KCL::Model const& model);
void deserialize(QXmlStreamReader& stream, KCL::Model& model);

}

#endif // FILEUTILITY_H
