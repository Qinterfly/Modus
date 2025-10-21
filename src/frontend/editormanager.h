#ifndef EDITORMANAGER_H
#define EDITORMANAGER_H

#include <QDialog>
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
}

namespace Frontend
{

class EditElements : public QUndoCommand
{
public:
    EditElements(QList<KCL::AbstractElement*> elements, QList<KCL::VecN> const& dataSet, QString const& name);
    EditElements(KCL::AbstractElement* pElement, KCL::VecN const& data, QString const& name);
    ~EditElements() = default;

    void undo() override;
    void redo() override;

private:
    QList<KCL::AbstractElement*> mElements;
    QList<KCL::VecN> mOldDataSet;
    QList<KCL::VecN> mNewDataSet;
};

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
        kPolyExponents
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
    void commandExecuted(QUndoCommand* pCommand);

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
    ~EditorManager();

    bool isEmpty() const;
    int numEditors() const;

    void clear();
    void createEditor(KCL::Model& model, Backend::Core::Selection const& selection);
    void setCurrentEditor(int index);
    void refreshCurrentEditor();

private:
    void createContent();
    void createConnections();
    void addEditor(Editor* pEditor);

private:
    Editor* mpCurrentEditor;
    QComboBox* mpEditorsList;
    QList<Editor*> mEditors;
    QUndoStack* mpUndoStack;
};
}

#endif // EDITORMANAGER_H
