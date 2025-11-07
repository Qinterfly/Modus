#ifndef CONSTANTSEDITOR_H
#define CONSTANTSEDITOR_H

#include "editormanager.h"
#include "uialiasdata.h"

namespace KCL
{
struct Constants;
}

namespace Frontend
{

//! Class to edit constants
class ConstantsEditor : public Editor
{
    Q_OBJECT

public:
    ConstantsEditor(KCL::Constants* pElement, QString const& name, QWidget* pParent = nullptr);
    virtual ~ConstantsEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createConnections();
    void setElementData();

private:
    KCL::Constants* mpElement;
    Edit1d* mpGravityAccelerationEdit;
    Edit1d* mpReferenceLengthEdit;
    Edit1d* mpAirDensityEdit;
    Edit1d* mpSoundSpeedEdit;
    Edit1d* mpMachNumberEdit;
    Edit1d* mpStrouhalNumberEdit;
    Edit1d* mpReferenceChordEdit;
};

}

#endif // CONSTANTSEDITOR_H
