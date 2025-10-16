#ifndef BEAMEDITOR_H
#define BEAMEDITOR_H

#include "editormanager.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QGroupBox)

namespace Frontend
{

//! Class to edit properties of beam elements
class BeamEditor : public Editor
{
    Q_OBJECT

public:
    BeamEditor(KCL::ElasticSurface const& surface, KCL::AbstractElement* pElement, QString const& name, QWidget* pParent = nullptr);
    ~BeamEditor() = default;

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
    QGroupBox* createStifnessGroupBox();
    QGroupBox* createInertiaGroupBox();

private:
    Transformation mTransform;
    KCL::AbstractElement* mpElement;
    Edits2d mStartLocalEdits;
    Edits2d mEndLocalEdits;
    Edits3d mStartGlobalEdits;
    Edits3d mEndGlobalEdits;
    EditsXd mStiffnessEdits;
    EditsXd mInertiaEdits;
};
}

#endif // BEAMEDITOR_H
