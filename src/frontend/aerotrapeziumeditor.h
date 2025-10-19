#ifndef AEROTRAPEZIUMEDITOR_H
#define AEROTRAPEZIUMEDITOR_H

#include "editormanager.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QGroupBox)

namespace Frontend
{

//! Class to edit aerodynamic trapeziums
class AeroTrapeziumEditor : public Editor
{
public:
    AeroTrapeziumEditor(KCL::ElasticSurface const& surface, KCL::AbstractElement* pElement, QString const& name, QWidget* pParent = nullptr);
    ~AeroTrapeziumEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createConnections();
    void setElementData();
    QGroupBox* createLocalGroupBox();
    QGroupBox* createGlobalGroupBox();

private:
    Transformation mTransform;
    KCL::AbstractElement* mpElement;
    // Local edits
    Edits2d mLocal0Edits;
    Edits2d mLocal1Edits;
    Edit1d* mpLocal2Edit;
    Edit1d* mpLocal3Edit;
    // Global edits
    Edits3d mGlobal0Edits;
    Edits3d mGlobal1Edits;
    Edit1d* mpGlobal2Edit;
    Edit1d* mpGlobal3Edit;
};

}

#endif // AEROTRAPEZIUMEDITOR_H
