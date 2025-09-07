
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

QString toString(QVariant const& variant);

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

template<typename Derived>
void serialize(QXmlStreamWriter& stream, QString const& elementName, Eigen::MatrixBase<Derived> const& matrix)
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
        for (int j = 0; j != numCols; ++j)
            textStream >> matrix(i, j);
}

void serialize(QXmlStreamWriter& stream, QString const& elementName, QString const& objectName, QList<Eigen::MatrixXd> const& matrices);
void deserialize(QXmlStreamReader& stream, QString const& objectName, QList<Eigen::MatrixXd>& matrices);

void serialize(QXmlStreamWriter& stream, QString const& elementName, QString const& objectName, QList<QString> const& items);
void deserialize(QXmlStreamReader& stream, QString const& objectName, QList<QString>& items);

void serialize(QXmlStreamWriter& stream, QString const& elementName, QMap<Backend::Core::Selection, bool> const& map);
void deserialize(QXmlStreamReader& stream, QMap<Backend::Core::Selection, bool>& map);

void serialize(QXmlStreamWriter& stream, QString const& elementName, KCL::Model const& model);
void deserialize(QXmlStreamReader& stream, KCL::Model& model);

bool areEqual(double first, double second, double tolerance);

//! Check if two matrices are equal
template<typename Derived>
bool areEqual(Eigen::MatrixBase<Derived> const& first, Eigen::MatrixBase<Derived> const& second, double tolerance)
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

//! Check if metaobjects are equal
template<typename T>
bool areEqual(T const& first, T const& second)
{
    double const kTolerance = 1e-6;
    QMetaObject const& metaObject = first.staticMetaObject;
    int numProperties = metaObject.propertyCount();
    while (numProperties > 0)
    {
        QVariant firstVariant = metaObject.property(numProperties - 1).readOnGadget(&first);
        QVariant secondVariant = metaObject.property(numProperties - 1).readOnGadget(&second);
        if (firstVariant != secondVariant)
        {
            QString type = firstVariant.typeName();
            if (type == "double")
            {
                double firstValue = firstVariant.value<double>();
                double secondValue = secondVariant.value<double>();
                if (!areEqual(firstValue, secondValue, kTolerance))
                    return false;
            }
            else if (type == "QList<std::pair<int,double>>")
            {
                auto firstValue = firstVariant.value<Core::ModalPairs>();
                auto secondValue = secondVariant.value<Core::ModalPairs>();
                if (firstValue.size() != secondValue.size())
                    return false;
                int numValues = firstValue.size();
                for (int k = 0; k != numValues; ++k)
                {
                    if (firstValue[k].first - secondValue[k].first != 0)
                        return false;
                    if (!areEqual(firstValue[k].second, secondValue[k].second, kTolerance))
                        return false;
                }
            }
            else if (type == "QList<Eigen::Matrix<double,-1,-1>>")
            {
                auto firstValue = firstVariant.value<QList<Eigen::MatrixXd>>();
                auto secondValue = secondVariant.value<QList<Eigen::MatrixXd>>();
                if (firstValue.size() != secondValue.size())
                    return false;
                int numValues = firstValue.size();
                for (int k = 0; k != numValues; ++k)
                {
                    if (!areEqual(firstValue[k], secondValue[k], kTolerance))
                        return false;
                }
            }
            else if (type == "Eigen::Matrix<double,-1,-1>")
            {
                if (!areEqual(firstVariant.value<Eigen::MatrixXd>(), secondVariant.value<Eigen::MatrixXd>(), kTolerance))
                    return false;
            }
            else if (type == "Eigen::Matrix<double,-1,1>")
            {
                if (!areEqual(firstVariant.value<Eigen::VectorXd>(), secondVariant.value<Eigen::VectorXd>(), kTolerance))
                    return false;
            }
            else if (type == "Eigen::Matrix<double,3,1>")
            {
                if (!areEqual(firstVariant.value<Eigen::Vector3d>(), secondVariant.value<Eigen::Vector3d>(), kTolerance))
                    return false;
            }
            else
            {
                return false;
            }
        }
        --numProperties;
    }
    return true;
}
}

#endif // FILEUTILITY_H
