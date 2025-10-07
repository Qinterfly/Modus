#ifndef EDITORMANAGER_H
#define EDITORMANAGER_H

#include <QDialog>

#include "lineedit.h"

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

//! Base class for all editors
class Editor : public QWidget
{
    Q_OBJECT

public:
    enum Type
    {
        kBeam
    };

    Editor() = delete;
    Editor(Type type, QString const& name, QIcon const& icon = QIcon(), QWidget* pParent = nullptr);
    virtual ~Editor() = default;

    Type type() const;
    QString const& name() const;
    QIcon const& icon() const;

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

private:
    void createContent();
    void createConnections();

private:
    QWidget* mpCurrentWidget;
    QComboBox* mpEditorsList;
    QList<Editor*> mEditors;
};
}

#endif // EDITORMANAGER_H
