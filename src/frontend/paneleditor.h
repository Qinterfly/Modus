#ifndef PANELEDITOR_H
#define PANELEDITOR_H

#include "editormanager.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QGroupBox)

namespace Frontend
{

using PanelLocalEdits = std::array<LocalEdits, 4>;
using PanelGlobalEdits = std::array<GlobalEdits, 4>;

//! Class to edit properties of panel elements
class PanelEditor : public Editor
{
    Q_OBJECT

public:
    PanelEditor(KCL::ElasticSurface const& surface, KCL::AbstractElement* pElement, QString const& name, QWidget* pParent = nullptr);
    ~PanelEditor() = default;

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
    DoubleLineEdit* mpThicknessEdit;
    // Coordinates
    PanelLocalEdits mLocalEdits;
    PanelGlobalEdits mGlobalEdits;
    // Depths
    QList<DoubleLineEdit*> mDepthEdits;
    // Material
    DoubleLineEdit* mpYoungsModulus1Edit;
    DoubleLineEdit* mpYoungsModulus2Edit;
    DoubleLineEdit* mpShearModulusEdit;
    DoubleLineEdit* mpPoissonRatioEdit;
    DoubleLineEdit* mpAngleE1ZEdit;
    DoubleLineEdit* mpDensityEdit;
};

}

#endif // PANELEDITOR_H
