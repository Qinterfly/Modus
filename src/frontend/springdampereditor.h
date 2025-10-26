#ifndef SPRINGDAMPEREDITOR_H
#define SPRINGDAMPEREDITOR_H

#include "editormanager.h"
#include "uialiasdata.h"

namespace KCL
{
struct SpringDamper;
}

QT_FORWARD_DECLARE_CLASS(QGroupBox)

namespace Frontend
{

class SpringDamperEditor : public Editor
{
    Q_OBJECT

public:
    SpringDamperEditor(std::vector<KCL::ElasticSurface> const& surfaces, KCL::SpringDamper* pElement, QString const& name,
                       QWidget* pParent = nullptr);
    ~SpringDamperEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createConnections();
    void setGlobalByLocal();
    void setLocalByGlobal();
    void setElementData();
    QGroupBox* createPairGroupBox();
    QGroupBox* createSurfaceGroupBox(bool isFirst);
    QGroupBox* createOrientationGroupBox();
    QGroupBox* createMatrixDataGroupBox();

private:
    std::vector<KCL::ElasticSurface> const& mSurfaces;
    KCL::AbstractElement* mpElement;
    // Pairing
    QComboBox* mpIFirstSurfaceEdit;
    QComboBox* mpISecondSurfaceEdit;
    // First surface
    Edits2d mFirstLocalEdits;
    Edits3d mFirstGlobalEdits;
    Edit1d* mpFirstLengthEdit;
    Edits2d mFirstAngleEdits;
    // Second surface
    Edits2d mSecondLocalEdits;
    Edits3d mSecondGlobalEdits;
    Edit1d* mpSecondLengthEdit;
    Edits2d mSecondAngleEdits;
    // Spring
    Edits3d mOrientationEdits;
    // Type
    QComboBox* mpTypeComboBox;
};

}

#endif // SPRINGDAMPEREDITOR_H
