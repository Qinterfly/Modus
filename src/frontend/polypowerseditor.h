#ifndef POLYPOWERSEDITOR_H
#define POLYPOWERSEDITOR_H

#include "editormanager.h"

namespace KCL
{
class PolyPowersX;
class PolyPowersZ;
}

namespace Frontend
{

class PolyPowersEditor : public Editor
{
public:
    PolyPowersEditor(KCL::PolyPowersX* pElementX, KCL::PolyPowersZ* pElementZ, QString const& name, QWidget* pParent = nullptr);
    ~PolyPowersEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();
    void createConnections();
    void resizeElementData();
    void setElementData();

private:
    KCL::PolyPowersX* mpElementX;
    KCL::PolyPowersZ* mpElementZ;
};

}

#endif // POLYPOWERSEDITOR_H
