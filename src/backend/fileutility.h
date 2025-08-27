
#ifndef FILEUTILITY_H
#define FILEUTILITY_H

#include <QDir>
#include <QFile>
#include <QMetaProperty>
#include <QString>
#include <QXmlStreamWriter>

namespace KCL
{
struct Model;
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

//! Output a metaobject to a binary stream
template<typename T>
void serialize(QXmlStreamWriter& stream, T const& object);
template<>
void serialize(QXmlStreamWriter& stream, KCL::Model const& model);

}

#endif // FILEUTILITY_H
