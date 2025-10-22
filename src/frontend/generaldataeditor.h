#ifndef GENERALDATAEDITOR_H
#define GENERALDATAEDITOR_H

#include "editormanager.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QGroupBox)
QT_FORWARD_DECLARE_CLASS(QCheckBox)

namespace KCL
{
struct GeneralData;
}

namespace Frontend
{

//! Class to edit general data
class GeneralDataEditor : public Editor
{
    Q_OBJECT

public:
    GeneralDataEditor(KCL::ElasticSurface const& surface, KCL::GeneralData* pElement, QString const& name, QWidget* pParent = nullptr);
    ~GeneralDataEditor() = default;

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
    QGroupBox* createAnglesGroupBox();
    QGroupBox* createParametersGroupBox();

private:
    Transformation mTransform;
    KCL::GeneralData* mpElement;
    // Coordinates
    Edits2d mLocalEdits;
    Edits3d mGlobalEdits;
    // Angles
    Edit1d* mpDihedralEdit;
    Edit1d* mpSweepEdit;
    Edit1d* mpAttackEdit;
    // Parameters
    QCheckBox* mpSymmetryCheckBox;
    Edit1i* mpLiftSurfacesEdit;
    Edit1i* mpGroupEdit;
    Edit1d* mpTorsionalEdit;
    Edit1d* mpBendingEdit;
};
}

#endif // GENERALDATAEDITOR_H
