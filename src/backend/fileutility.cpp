#include <kcl/model.h>

#include "fileutility.h"
#include "project.h"

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

//! Output a metaobject to a XML stream
template<typename T>
void serialize(QXmlStreamWriter& stream, T const& object)
{
    QMetaObject const& metaObject = object.staticMetaObject;
    int numProperties = metaObject.propertyCount();
    QString name;
    QVariant value;
    for (int i = 0; i != numProperties; ++i)
    {
        name = metaObject.property(i).name();
        value = metaObject.property(i).readOnGadget(&object);
        stream.writeTextElement(name, value.toString());
    }
}

//! Output a model to a XML stream
template<>
void serialize(QXmlStreamWriter& stream, KCL::Model const& model)
{
    QString text = model.toString().c_str();
    QByteArray data = text.toUtf8();
    data = qCompress(data);
    stream.writeStartElement("model");
    stream.writeAttribute("data", data);
    stream.writeEndElement();
}

// Explicit template instantiation
template void serialize(QXmlStreamWriter&, Backend::Core::Project const&);
}
