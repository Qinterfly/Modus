#include <kcl/model.h>

#include "fileutility.h"
#include "project.h"

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

//! Output a metaobject to a XML stream
template<typename T>
void serialize(QXmlStreamWriter& stream, T const& object)
{
    QMetaObject const& metaObject = object.staticMetaObject;
    int numProperties = metaObject.propertyCount();
    QString name;
    QVariant variant;
    QString typeName;
    for (int i = 0; i != numProperties; ++i)
    {
        name = metaObject.property(i).name();
        variant = metaObject.property(i).readOnGadget(&object);
        typeName = variant.typeName();
        if (typeName == "QList<Backend::Core::Subproject>")
        {
            auto const& items = variant.value<QList<Subproject>>();
            for (auto const& item : items)
                item.serialize(stream);
        }
        else if (typeName == "Backend::Core::Configuration")
        {
            variant.value<Configuration>().serialize(stream);
        }
        else if (typeName == "Backend::Core::OptimProblem")
        {
            variant.value<OptimProblem>().serialize(stream);
        }
        else if (typeName == "Backend::Core::OptimOptions")
        {
            variant.value<OptimOptions>().serialize(stream);
        }
        else if (typeName == "Backend::Core::ModalSolution")
        {
            variant.value<ModalSolution>().serialize(stream);
        }
        else if (typeName == "Backend::Core::ModalComparison")
        {
            variant.value<ModalComparison>().serialize(stream);
        }
        else if (typeName == "Backend::Core::Geometry")
        {
            variant.value<Geometry>().serialize(stream);
        }
        else if (typeName == "Backend::Core::Selector")
        {
            variant.value<Selector>().serialize(stream);
        }
        else if (typeName == "Backend::Core::Constraints")
        {
            variant.value<Constraints>().serialize(stream);
        }
        else if (typeName == "KCL::Model")
        {
            serialize(stream, variant.value<KCL::Model>());
        }
        else if (typeName == "QList<Backend::Core::Vertex>")
        {
            auto const& items = variant.value<QList<Vertex>>();
            for (auto const& item : items)
                item.serialize(stream);
        }
        else if (typeName == "QList<Backend::Core::Slave>")
        {
            auto const& items = variant.value<QList<Slave>>();
            for (auto const& item : items)
                item.serialize(stream);
        }
        else if (typeName == "QList<Backend::Core::SelectionSet>")
        {
            auto const& items = variant.value<QList<SelectionSet>>();
            for (auto const& item : items)
                item.serialize(stream);
        }
        else if (typeName == "QList<std::pair<int,int>>")
        {
            serialize(stream, name, variant.value<QList<QPair<int, int>>>());
        }
        else if (typeName == "QMap<Backend::Core::Selection,bool>")
        {
            serialize(stream, name, variant.value<QMap<Selection, bool>>());
        }
        else if (typeName == "QStringList")
        {
            serialize(stream, name, variant.value<QList<QString>>());
        }
        else if (typeName == "Eigen::Matrix<int,-1,1>")
        {
            serialize(name, stream, variant.value<Eigen::VectorXi>());
        }
        else if (typeName == "Eigen::Matrix<double,-1,1>")
        {
            serialize(name, stream, variant.value<Eigen::VectorXd>());
        }
        else if (typeName == "Eigen::Matrix<double,3,1>")
        {
            serialize(name, stream, variant.value<Eigen::Vector3d>());
        }
        else if (typeName == "Eigen::Matrix<int,-1,-1>")
        {
            serialize(name, stream, variant.value<Eigen::MatrixXi>());
        }
        else if (typeName == "Eigen::Matrix<double,-1,-1>")
        {
            serialize(name, stream, variant.value<Eigen::MatrixXd>());
        }
        else if (typeName == "QList<Eigen::Matrix<double,-1,-1>>")
        {
            auto const& items = variant.value<QList<Eigen::MatrixXd>>();
            for (auto const& item : items)
                serialize(name, stream, item);
        }
        else
        {
            stream.writeTextElement(name, variant.toString());
        }
    }
}

//! Output model to a XML stream
template<>
void serialize(QXmlStreamWriter& stream, KCL::Model const& model)
{
    QByteArray data;
    if (!model.isEmpty())
    {
        QString text = model.toString().c_str();
        data = qCompress(text.toUtf8());
    }
    stream.writeStartElement("model");
    stream.writeAttribute("type", "kcl");
    stream.writeAttribute("value", data);
    stream.writeEndElement();
}

//! Output a map of selections to a XML stream
void serialize(QXmlStreamWriter& stream, QString const& name, QMap<Backend::Core::Selection, bool> const& map)
{
    stream.writeStartElement(name);
    for (auto const& [key, value] : map.asKeyValueRange())
    {
        stream.writeStartElement("pair");
        key.serialize(stream);
        stream.writeTextElement("flag", QString::number(value));
        stream.writeEndElement();
    }
    stream.writeEndElement();
}

//! Output a list of strings to a XML stream
void serialize(QXmlStreamWriter& stream, QString const& name, QList<QString> const& values)
{
    stream.writeStartElement(name);
    for (auto const& item : values)
        stream.writeTextElement("value", item);
    stream.writeEndElement();
}

// Explicit template instantiation
template void serialize(QXmlStreamWriter&, Project const&);
template void serialize(QXmlStreamWriter&, Subproject const&);
template void serialize(QXmlStreamWriter&, Configuration const&);
template void serialize(QXmlStreamWriter&, OptimProblem const&);
template void serialize(QXmlStreamWriter&, OptimOptions const&);
template void serialize(QXmlStreamWriter&, OptimSolution const&);
template void serialize(QXmlStreamWriter&, ModalSolution const&);
template void serialize(QXmlStreamWriter&, ModalComparison const&);
template void serialize(QXmlStreamWriter&, Selector const&);
template void serialize(QXmlStreamWriter&, SelectionSet const&);
template void serialize(QXmlStreamWriter&, Selection const&);
template void serialize(QXmlStreamWriter&, Constraints const&);
template void serialize(QXmlStreamWriter&, Geometry const&);
template void serialize(QXmlStreamWriter&, Vertex const&);
template void serialize(QXmlStreamWriter&, Slave const&);
}
