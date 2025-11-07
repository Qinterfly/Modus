#ifndef PANELEDITOR_H
#define PANELEDITOR_H

#include "editormanager.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QGroupBox)

namespace Frontend
{

using PanelLocalEdits = std::array<Edits2d, 4>;
using PanelGlobalEdits = std::array<Edits3d, 4>;

//! Class to edit properties of panel elements
class PanelEditor : public Editor
{
    Q_OBJECT

public:
    PanelEditor(KCL::ElasticSurface const& surface, KCL::AbstractElement* pElement, QString const& name, QWidget* pParent = nullptr);
    virtual ~PanelEditor() = default;

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
    QGroupBox* createDepthGroupBox();
    QGroupBox* createMaterialGroupBox();

private:
    Transformation mTransform;
    KCL::AbstractElement* mpElement;
    // Thickness
    Edit1d* mpThicknessEdit;
    // Coordinates
    PanelLocalEdits mLocalEdits;
    PanelGlobalEdits mGlobalEdits;
    // Depths
    EditsXd mDepthEdits;
    // Material
    Edit1d* mpYoungsModulus1Edit;
    Edit1d* mpYoungsModulus2Edit;
    Edit1d* mpShearModulusEdit;
    Edit1d* mpPoissonRatioEdit;
    Edit1d* mpAngleE1ZEdit;
    Edit1d* mpDensityEdit;
};

}

#endif // PANELEDITOR_H
