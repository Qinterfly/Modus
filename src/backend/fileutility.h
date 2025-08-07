
#ifndef FILEUTILITY_H
#define FILEUTILITY_H

#include <QDir>
#include <QFile>
#include <QString>

namespace Backend::Utility
{

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
}

#endif // FILEUTILITY_H
