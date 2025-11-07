#ifndef CUSTOMTABWIDGET_H
#define CUSTOMTABWIDGET_H

#include <QTabWidget>

namespace Frontend
{

class CustomTabWidget : public QTabWidget
{
public:
    CustomTabWidget(QWidget* pParent = nullptr);
    virtual ~CustomTabWidget() = default;

protected:
    bool eventFilter(QObject* pObject, QEvent* pEvent) override;

private:
    void renameTabDialog(int iTab);
};

}

#endif // CUSTOMTABWIDGET_H
