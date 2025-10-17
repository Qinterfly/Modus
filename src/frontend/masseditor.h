#ifndef MASSEDITOR_H
#define MASSEDITOR_H

#include "editormanager.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QGroupBox)

namespace Frontend
{

//! Class to edit mass properties
class MassEditor : public Editor
{
public:
    MassEditor(KCL::ElasticSurface const& surface, KCL::AbstractElement* pElement, QString const& name, QWidget* pParent = nullptr);
    ~MassEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createConnections();
    void setGlobalByLocal();
    void setLocalByGlobal();
    void setElementData();
    QGroupBox* createLocalGroupBox();
    QGroupBox* createGlobalGroupBox();

private:
    Transformation mTransform;
    KCL::AbstractElement* mpElement;
    Edit1d* mpMassEdit;
    Edits2d mLocalEdits2D;
    Edits3d mLocalEdits3D;
    Edits3d mGlobalEdits;
    Edit1d* mpInertiaEdit;
    Edits3d mInertiaEdits;
    Edit1d* mpLengthRodEdit;
    Edit1d* mpAngleRodZEdit;
};

}

#endif // MASSEDITOR_H
