#include <QComboBox>
#include <QLabel>
#include <QToolBar>
#include <QVBoxLayout>

#include <kcl/model.h>

#include "aerotrapeziumeditor.h"
#include "analysisparameterseditor.h"
#include "beameditor.h"
#include "constantseditor.h"
#include "constraintseditor.h"
#include "editormanager.h"
#include "fluttersolver.h"
#include "generaldataeditor.h"
#include "masseditor.h"
#include "modalsolver.h"
#include "modeleditor.h"
#include "optimsolver.h"
#include "paneleditor.h"
#include "polyexponentseditor.h"
#include "rawdataeditor.h"
#include "selectionset.h"
#include "solveroptionseditor.h"
#include "springdampereditor.h"
#include "targeteditor.h"
#include "uiutility.h"

using namespace Backend;
using namespace Frontend;
using namespace Eigen;

Editor::Editor(Type type, QString const& name, QIcon const& icon, QWidget* pParent)
    : QWidget(pParent)
    , mkType(type)
    , mName(name)
    , mIcon(icon)
{
}

Editor::Type Editor::type() const
{
    return mkType;
}

QString const& Editor::name() const
{
    return mName;
}

QIcon const& Editor::icon() const
{
    return mIcon;
}

void Editor::setIcon(QIcon const& icon)
{
    mIcon = icon;
}

EditorManager::EditorManager(QWidget* pParent)
    : QDialog(pParent)
{
    setWindowTitle(tr("Editor Manager"));
    setModal(false);
    createContent();
    createConnections();
}

EditorManager::~EditorManager()
{
    clear();
}

bool EditorManager::isEmpty() const
{
    return numEditors() == 0;
}

//! Retrieve the number of created editors
int EditorManager::numEditors() const
{
    return mEditors.size();
}

//! Remove all the editors
void EditorManager::clear()
{
    mpEditorsList->clear();
    int numItems = mEditors.size();
    for (int i = 0; i != numItems; ++i)
        mEditors[i]->deleteLater();
    mEditors.clear();
    mpCurrentEditor = nullptr;
}

//! Create a specific editor based on element type
void EditorManager::createEditor(KCL::Model& model, Core::Selection const& selection)
{
    // Slice elements by selection
    KCL::ElasticSurface& surface = selection.iSurface >= 0 ? model.surfaces[selection.iSurface] : model.specialSurface;
    KCL::AbstractElement* pElement = surface.element(selection.type, selection.iElement);
    if (!pElement)
        return;

    // Create the edtior
    Editor* pEditor = nullptr;
    auto type = pElement->type();
    QString name = Utility::getLabel(selection);
    if (Utility::beamTypes().contains(type))
        pEditor = new BeamEditor(surface, pElement, name);
    else if (Utility::panelTypes().contains(type))
        pEditor = new PanelEditor(surface, pElement, name);
    else if (Utility::massTypes().contains(type))
        pEditor = new MassEditor(surface, pElement, name);
    else if (Utility::aeroTrapeziumTypes().contains(type) && pElement->subType() != KCL::AE1)
        pEditor = new AeroTrapeziumEditor(surface, pElement, name);
    else if (type == KCL::OD)
        pEditor = new GeneralDataEditor(surface, (KCL::GeneralData*) pElement, name);
    else if (type == KCL::CO)
        pEditor = new ConstantsEditor((KCL::Constants*) pElement, name);
    else if (type == KCL::WP)
        pEditor = new AnalysisParametersEditor((KCL::AnalysisParameters*) pElement, name);
    else if (type == KCL::PK && surface.containsElement(KCL::QK))
        pEditor = new PolyExponentsEditor((KCL::PolyExponentsX*) pElement, (KCL::PolyExponentsZ*) surface.element(KCL::QK), name);
    else if (type == KCL::QK && surface.containsElement(KCL::PK))
        pEditor = new PolyExponentsEditor((KCL::PolyExponentsX*) surface.element(KCL::PK), (KCL::PolyExponentsZ*) pElement, name);
    else if (type == KCL::PR)
        pEditor = new SpringDamperEditor(model.surfaces, (KCL::SpringDamper*) pElement, name);
    else
        pEditor = new RawDataEditor(pElement, name);

    // Add the editor to the manager
    addEditor(pEditor);

    // Set the connection
    auto setEdited = [this, &model]() { emit modelEdited(model); };
    connectEditCommand(pEditor, setEdited);
}

//! Create a model editor
void EditorManager::createEditor(KCL::Model& model)
{
    Editor* pEditor = new ModelEditor(model, tr("Model"));
    addEditor(pEditor);
}

//! Create editor of modal options
void EditorManager::createEditor(Backend::Core::ModalOptions& options)
{
    Editor* pEditor = new ModalOptionsEditor(options, tr("Modal options"));
    addEditor(pEditor);
    auto setEdited = [this, &options]() { emit modalOptionsEdited(options); };
    connectEditCommand(pEditor, setEdited);
}

//! Create editor of flutter options
void EditorManager::createEditor(Backend::Core::FlutterOptions& options)
{
    Editor* pEditor = new FlutterOptionsEditor(options, tr("Flutter options"));
    addEditor(pEditor);
    auto setEdited = [this, &options]() { emit flutterOptionsEdited(options); };
    connectEditCommand(pEditor, setEdited);
}

//! Create editor of optimization options
void EditorManager::createEditor(Backend::Core::OptimOptions& options)
{
    Editor* pEditor = new OptimOptionsEditor(options, tr("Optimization options"));
    addEditor(pEditor);
    auto setEdited = [this, &options]() { emit optimOptionsEdited(options); };
    connectEditCommand(pEditor, setEdited);
}

//! Create editor of optimization constraints
void EditorManager::createEditor(Backend::Core::Constraints& constraints)
{
    Editor* pEditor = new ConstraintsEditor(constraints, tr("Optimization constraints"));
    addEditor(pEditor);
    auto setEdited = [this, &constraints]() { emit constraintsEdited(constraints); };
    connectEditCommand(pEditor, setEdited);
}

//! Create editor of optimization targets
void EditorManager::createEditor(Core::OptimTarget& target)
{
    Editor* pEditor = new TargetEditor(target, tr("Optimization target"));
    addEditor(pEditor);
    auto setEdited = [this, &target]() { emit optimTargetEdited(target); };
    connectEditCommand(pEditor, setEdited);
}

//! Set the current editor to work with
void EditorManager::setCurrentEditor(int index)
{
    if (mpCurrentEditor)
    {
        layout()->removeWidget(mpCurrentEditor);
        mpCurrentEditor->hide();
    }
    if (index >= 0 && index < numEditors())
    {
        QSignalBlocker blocker(mpEditorsList);
        mpEditorsList->setCurrentIndex(index);
        Editor* pEditor = mEditors[index];
        layout()->addWidget(pEditor);
        mpCurrentEditor = pEditor;
        mpCurrentEditor->refresh();
        mpCurrentEditor->show();
    }
}

//! Update the current editor state from the source
void EditorManager::refreshCurrentEditor()
{
    if (mpCurrentEditor)
        mpCurrentEditor->refresh();
}

//! Create all the widgets which are common for editors
void EditorManager::createContent()
{
    mpCurrentEditor = nullptr;

    // Create the undo stack
    mpUndoStack = new QUndoStack(this);

    // Create the layout to select edtors
    QHBoxLayout* pSelectLayout = new QHBoxLayout;
    mpEditorsList = new QComboBox;
    mpEditorsList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    pSelectLayout->addWidget(new QLabel(tr("Editors: ")));
    pSelectLayout->addWidget(mpEditorsList);
    pSelectLayout->addStretch();

    // Create undo and redo actions
    QAction* pUndoAction = mpUndoStack->createUndoAction(this, tr("&Undo"));
    QAction* pRedoAction = mpUndoStack->createRedoAction(this, tr("&Redo"));

    // Set the icons of the actions
    pUndoAction->setIcon(QIcon(":/icons/edit-undo.svg"));
    pRedoAction->setIcon(QIcon(":/icons/edit-redo.svg"));

    // Set the shortcuts
    pUndoAction->setShortcuts(QKeySequence::Undo);
    pRedoAction->setShortcuts(QKeySequence::Redo);

    // Create the connecions
    connect(pUndoAction, &QAction::triggered, this, &EditorManager::refreshCurrentEditor);
    connect(pRedoAction, &QAction::triggered, this, &EditorManager::refreshCurrentEditor);

    // Create the toolbar
    QToolBar* pToolBar = new QToolBar;
    pToolBar->addAction(pUndoAction);
    pToolBar->addAction(pRedoAction);
    pSelectLayout->addWidget(pToolBar);
    Utility::setShortcutHints(pToolBar);

    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pSelectLayout);
    setLayout(pMainLayout);
}

//! Specify the connections
void EditorManager::createConnections()
{
    connect(mpEditorsList, &QComboBox::currentIndexChanged, this, [this](int index) { setCurrentEditor(index); });
}

//! Register the editor
void EditorManager::addEditor(Editor* pEditor)
{
    if (!pEditor)
        return;
    mEditors.push_back(pEditor);
    mpEditorsList->addItem(pEditor->icon(), pEditor->name());
}

//! Set the connections for editing command
void EditorManager::connectEditCommand(Editor* pEditor, std::function<void()> setEdited)
{
    if (!pEditor)
        return;
    connect(pEditor, &Editor::commandExecuted, this,
            [this, setEdited](EditCommand* pCommand)
            {
                mpUndoStack->push(pCommand);
                connect(pCommand->pHandler, &EditHandler::edited, this, setEdited);
                setEdited();
            });
}

EditHandler::EditHandler(QObject* pParent)
    : QObject(pParent)
{
}

EditCommand::EditCommand()
{
    pHandler = new EditHandler;
}

EditCommand::~EditCommand()
{
}

void EditCommand::setEdited()
{
    emit pHandler->edited();
}

EditElements::EditElements(QList<KCL::AbstractElement*> elements, QList<KCL::VecN> const& dataSet, QString const& name)
    : mElements(elements)
{
    int numElements = mElements.size();
    mOldDataSet.resize(numElements);
    for (int i = 0; i != numElements; ++i)
        mOldDataSet[i] = mElements[i]->get();
    mNewDataSet = dataSet;
    setText(QObject::tr("Multiple edits %1").arg(name));
}

EditElements::EditElements(KCL::AbstractElement* pElement, KCL::VecN const& data, QString const& name)
{
    mElements.push_back(pElement);
    mOldDataSet.push_back(pElement->get());
    mNewDataSet.push_back(data);
    setText(QObject::tr("Edit %1").arg(name));
}

//! Revert the changes
void EditElements::undo()
{
    try
    {
        int numElements = mElements.size();
        for (int i = 0; i != numElements; ++i)
            mElements[i]->set(mOldDataSet[i]);
        setEdited();
    }
    catch (...)
    {
    }
}

//! Apply the changes
void EditElements::redo()
{
    try
    {
        int numElements = mElements.size();
        for (int i = 0; i != numElements; ++i)
            mElements[i]->set(mNewDataSet[i]);
        setEdited();
    }
    catch (...)
    {
    }
}

template<typename T>
EditProperty<T>::EditProperty(T& object, QString const& name, QVariant const& value)
    : mObject(object)
{
    QMetaObject const& metaObject = mObject.staticMetaObject;
    int iProperty = metaObject.indexOfProperty(name.toStdString().c_str());
    if (iProperty > -1)
    {
        mProperty = metaObject.property(iProperty);
        mOldValue = mProperty.readOnGadget(&mObject);
    }
    mNewValue = value;
    setText(QObject::tr("Edit property %1").arg(name));
}

//! Revert the changes
template<typename T>
void EditProperty<T>::undo()
{
    try
    {
        if (mProperty.isValid())
        {
            mProperty.writeOnGadget(&mObject, mOldValue);
            setEdited();
        }
    }
    catch (...)
    {
    }
}

//! Apply the changes
template<typename T>
void EditProperty<T>::redo()
{
    try
    {
        if (mProperty.isValid())
        {
            mProperty.writeOnGadget(&mObject, mNewValue);
            setEdited();
        }
    }
    catch (...)
    {
    }
}

template<typename T>
EditObject<T>::EditObject(T& object, QString const& name, T const& value)
    : mObject(object)
    , mOldValue(object)
    , mNewValue(value)
{
    setText(QObject::tr("Edit %1").arg(name));
}

//! Revert the changes
template<typename T>
void EditObject<T>::undo()
{
    try
    {
        mObject = mOldValue;
        setEdited();
    }
    catch (...)
    {
    }
}

//! Apply the changes
template<typename T>
void EditObject<T>::redo()
{
    try
    {
        mObject = mNewValue;
        setEdited();
    }
    catch (...)
    {
    }
}

// Explicit template instantiation
template class Frontend::EditProperty<Backend::Core::ModalOptions>;
template class Frontend::EditProperty<Backend::Core::FlutterOptions>;
template class Frontend::EditProperty<Backend::Core::OptimOptions>;
template class Frontend::EditObject<Backend::Core::OptimTarget>;
template class Frontend::EditObject<Backend::Core::Constraints>;
template class Frontend::EditObject<Eigen::VectorXi>;
template class Frontend::EditObject<Eigen::VectorXd>;
