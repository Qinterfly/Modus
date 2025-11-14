#ifndef EDITORMANAGER_H
#define EDITORMANAGER_H

#include <Eigen/Core>
#include <QDialog>
#include <QMetaProperty>
#include <QUndoCommand>

#include "kcl/alias.h"

QT_FORWARD_DECLARE_CLASS(QComboBox)

namespace KCL
{
class AbstractElement;
class ElasticSurface;
struct Model;
}

namespace Backend::Core
{
struct Selection;
struct ModalOptions;
struct FlutterOptions;
struct OptimOptions;
class Constraints;
struct ModalSolution;
}

namespace Frontend
{

class EditCommand;

//! Base class for all editors
class Editor : public QWidget
{
    Q_OBJECT

public:
    enum Type
    {
        kRawData,
        kGeneralData,
        kBeam,
        kPanel,
        kMass,
        kConstants,
        kAnalysisParameters,
        kAeroTrapezium,
        kPolyExponents,
        kSpringDamper,
        kModel,
        kModalOptions,
        kFlutterOptions,
        kOptimOptions,
        kConstraints,
        kTarget
    };

    Editor() = delete;
    Editor(Type type, QString const& name, QIcon const& icon = QIcon(), QWidget* pParent = nullptr);
    virtual ~Editor() = default;

    Type type() const;
    QString const& name() const;
    QIcon const& icon() const;

    void setIcon(QIcon const& icon);

    virtual void refresh() = 0;

signals:
    void commandExecuted(Frontend::EditCommand* pCommand);

protected:
    Type const mkType;
    QString mName;
    QIcon mIcon;
};

//! Factory class to create and display editors
class EditorManager : public QDialog
{
    Q_OBJECT

public:
    EditorManager(QWidget* pParent = nullptr);
    virtual ~EditorManager();

    bool isEmpty() const;
    int numEditors() const;

    void clear();
    void createEditor(KCL::Model& model, Backend::Core::Selection const& selection);
    void createEditor(KCL::Model& model);
    void createEditor(Backend::Core::ModalOptions& options);
    void createEditor(Backend::Core::FlutterOptions& options);
    void createEditor(Backend::Core::OptimOptions& options);
    void createEditor(Backend::Core::Constraints& constraints);
    void createEditor(Eigen::VectorXi& indices, Eigen::VectorXd& weights, Backend::Core::ModalSolution& solution);
    void setCurrentEditor(int index);
    void refreshCurrentEditor();

signals:
    void modelEdited(KCL::Model& model);
    void modalOptionsEdited(Backend::Core::ModalOptions& options);
    void flutterOptionsEdited(Backend::Core::FlutterOptions& options);
    void optimOptionsEdited(Backend::Core::OptimOptions& options);
    void constraintsEdited(Backend::Core::Constraints& constraints);

private:
    void createContent();
    void createConnections();
    void addEditor(Editor* pEditor);
    void connectEditCommand(Editor* pEditor, std::function<void()> setEdited);

private:
    Editor* mpCurrentEditor;
    QComboBox* mpEditorsList;
    QList<Editor*> mEditors;
    QUndoStack* mpUndoStack;
};

//! Class to send edit signals
class EditHandler : public QObject
{
    Q_OBJECT

public:
    EditHandler(QObject* pParent = nullptr);
    virtual ~EditHandler() = default;

signals:
    void edited();
};

//! Base command for editing
class EditCommand : public QUndoCommand
{
public:
    EditCommand();
    virtual ~EditCommand();
    void setEdited();

    EditHandler* pHandler;
};

//! Command to edit elements using datasets
class EditElements : public EditCommand
{
public:
    EditElements(QList<KCL::AbstractElement*> elements, QList<KCL::VecN> const& dataSet, QString const& name);
    EditElements(KCL::AbstractElement* pElement, KCL::VecN const& data, QString const& name);
    virtual ~EditElements() = default;

    void undo() override;
    void redo() override;

private:
    QList<KCL::AbstractElement*> mElements;
    QList<KCL::VecN> mOldDataSet;
    QList<KCL::VecN> mNewDataSet;
};

//! Command to edit property of a QObject
template<typename T>
class EditProperty : public EditCommand
{
public:
    EditProperty(T& object, QString const& name, QVariant const& value);
    virtual ~EditProperty() = default;

    void undo() override;
    void redo() override;

private:
    T& mObject;
    QMetaProperty mProperty;
    QVariant mOldValue;
    QVariant mNewValue;
};

//! Command to edit value
template<typename T>
class EditObject : public EditCommand
{
public:
    EditObject(T& object, QString const& name, T const& value);
    virtual ~EditObject() = default;

    void undo() override;
    void redo() override;

private:
    T& mObject;
    T mOldValue;
    T mNewValue;
};
}

#endif // EDITORMANAGER_H
