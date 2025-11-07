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
    Q_OBJECT

public:
    AeroTrapeziumEditor(KCL::ElasticSurface const& surface, KCL::AbstractElement* pElement, QString const& name, QWidget* pParent = nullptr);
    virtual ~AeroTrapeziumEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createConnections();
    void setGlobalByLocal();
    void setLocalByGlobal();
    void setElementData();
    QLayout* createAileronLayout();
    QGroupBox* createLocalGroupBox();
    QGroupBox* createGlobalGroupBox();
    QGroupBox* createMeshGroupBox();
    QGroupBox* createFactorsGroupBox();

private:
    Transformation mTransform;
    KCL::AbstractElement* mpElement;
    // Aileron index
    Edit1i* mpAileronIndexEdit;
    // Local edits
    Edits2d mLocal0Edits;
    Edits2d mLocal1Edits;
    Edits2d mLocal2Edits;
    // Global edits
    Edits3d mGlobal0Edits;
    Edits3d mGlobal1Edits;
    Edits2d mGlobal2Edits;
    // Mesh edits
    Edit1i* mpNumStripsEdit;
    Edit1i* mpNumPanelsEdit;
    // Factor edits
    Edit1d* mpStiffnessFactorEdit;
    Edit1d* mpDampingFactorEdit;
};

}

#endif // AEROTRAPEZIUMEDITOR_H
