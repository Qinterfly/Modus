#ifndef SPRINGDAMPEREDITOR_H
#define SPRINGDAMPEREDITOR_H

#include "editormanager.h"

namespace KCL
{
struct SpringDamper;
}

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

private:
    std::vector<KCL::ElasticSurface> const& mSurfaces;
    KCL::AbstractElement* mpElement;
};

}

#endif // SPRINGDAMPEREDITOR_H
