#ifndef BEAMEDITOR_H
#define BEAMEDITOR_H

#include "editormanager.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QGroupBox)

namespace Frontend
{

class DoubleLineEdit;

using LocalEdits = std::array<DoubleLineEdit*, 2>;
using GlobalEdits = std::array<DoubleLineEdit*, 3>;

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
    LocalEdits mStartLocalEdits;
    LocalEdits mEndLocalEdits;
    GlobalEdits mStartGlobalEdits;
    GlobalEdits mEndGlobalEdits;
    QList<DoubleLineEdit*> mStiffnessEdits;
    QList<DoubleLineEdit*> mInertiaEdits;
};
}

#endif // BEAMEDITOR_H
