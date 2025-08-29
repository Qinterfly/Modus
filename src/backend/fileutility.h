
#ifndef FILEUTILITY_H
#define FILEUTILITY_H

#include <Eigen/Core>
#include <QDir>
#include <QFile>
#include <QMetaProperty>
#include <QString>
#include <QXmlStreamWriter>

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

//! Output an Eigen matrix to a string
template<typename Derived>
QString toString(Eigen::MatrixBase<Derived> const& matrix)
{
    static Eigen::IOFormat const kFormatter(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", "; ");
    std::stringstream stream;
    stream << matrix.format(kFormatter);
    return QString(stream.str().data());
}

template<typename T>
void serialize(QXmlStreamWriter& stream, T const& object);
template<>
void serialize(QXmlStreamWriter& stream, KCL::Model const& model);
void serialize(QXmlStreamWriter& stream, QString const& name, QMap<Backend::Core::Selection, bool> const& map);
void serialize(QXmlStreamWriter& stream, QString const& name, QList<QString> const& values);

//! Output pairs to a XML stream
template<typename T, typename M>
void serialize(QXmlStreamWriter& stream, QString const& name, QList<QPair<T, M>> const& pairs)
{
    stream.writeStartElement(name);
    for (auto const& item : pairs)
    {
        QString text = QString("%1; %2").arg(QString::number(item.first), QString::number(item.second));
        stream.writeTextElement("pair", text);
    }
    stream.writeEndElement();
}

//! Output an Eigen matrix to a string
template<typename Derived>
void serialize(QString const& name, QXmlStreamWriter& stream, Eigen::MatrixBase<Derived> const& matrix)
{
    QString text = toString(matrix);
    stream.writeStartElement(name);
    stream.writeAttribute("value", text);
    stream.writeEndElement();
}
}

#endif // FILEUTILITY_H
