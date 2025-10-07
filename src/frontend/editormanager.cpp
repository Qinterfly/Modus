#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>

#include <kcl/model.h>

#include "beameditor.h"
#include "editormanager.h"
#include "selectionset.h"
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

EditorManager::EditorManager(QWidget* pParent)
    : QDialog(pParent)
    , mpCurrentWidget(nullptr)
{
    setWindowTitle(tr("Editor Manager"));
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
}

//! Create a specific editor based on element type
void EditorManager::createEditor(KCL::Model& model, Core::Selection const& selection)
{
    KCL::ElasticSurface& surface = selection.iSurface >= 0 ? model.surfaces[selection.iSurface] : model.specialSurface;
    KCL::AbstractElement* pElement = surface.element(selection.type, selection.iElement);
    if (!pElement)
        return;
    Editor* pEditor = nullptr;
    auto type = pElement->type();
    QString name = Utility::getLabel(selection);
    if (Utility::beamTypes().contains(type))
        pEditor = new BeamEditor(surface, pElement, name);
    if (pEditor)
    {
        mEditors.push_back(pEditor);
        mpEditorsList->addItem(pEditor->icon(), pEditor->name());
    }
}

//! Set the current editor to work with
void EditorManager::setCurrentEditor(int index)
{
    if (mpCurrentWidget)
    {
        layout()->removeWidget(mpCurrentWidget);
        mpCurrentWidget->hide();
    }
    if (index >= 0 && index < numEditors())
    {
        QSignalBlocker blocker(mpEditorsList);
        mpEditorsList->setCurrentIndex(index);
        Editor* pEditor = mEditors[index];
        layout()->addWidget(pEditor);
        mpCurrentWidget = pEditor;
        mpCurrentWidget->show();
    }
}

//! Create all the widgets which are common for editors
void EditorManager::createContent()
{
    // Create the layout to select edtors
    QHBoxLayout* pSelectLayout = new QHBoxLayout;
    mpEditorsList = new QComboBox;
    pSelectLayout->addWidget(new QLabel(tr("Editors: ")));
    pSelectLayout->addWidget(mpEditorsList);
    pSelectLayout->addStretch();

    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pSelectLayout);
    pMainLayout->addWidget(mpCurrentWidget);
    setLayout(pMainLayout);
}

//! Specify the connections
void EditorManager::createConnections()
{
    connect(mpEditorsList, &QComboBox::currentIndexChanged, this, [this](int index) { setCurrentEditor(index); });
}
