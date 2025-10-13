#ifndef GENERALDATAEDITOR_H
#define GENERALDATAEDITOR_H

#include "editormanager.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QGroupBox)
QT_FORWARD_DECLARE_CLASS(QCheckBox)

namespace KCL
{
class GeneralData;
}

namespace Frontend
{
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
    QGroupBox* createLocalGroupBox();
    QGroupBox* createGlobalGroupBox();
    QGroupBox* createAnglesGroupBox();
    QGroupBox* createParametersGroupBox();

private:
    Transformation mTransform;
    KCL::GeneralData* mpElement;
    // Coordinates
    LocalEdits mLocalEdits;
    GlobalEdits mGlobalEdits;
    // Angles
    DoubleLineEdit* mpDihedralEdit;
    DoubleLineEdit* mpSweepEdit;
    DoubleLineEdit* mpAttackEdit;
    // Parameters
    QCheckBox* mpSymmetryCheckBox;
    IntLineEdit* mpLiftSurfaceEdit;
    IntLineEdit* mpGroupEdit;
    DoubleLineEdit* mpTorsionalEdit;
    DoubleLineEdit* mpBendingEdit;
};
}

#endif // GENERALDATAEDITOR_H
