#include <QEvent>
#include <QInputDialog>
#include <QMouseEvent>
#include <QTabBar>

#include "customtabwidget.h"

using namespace Frontend;

CustomTabWidget::CustomTabWidget(QWidget* pParent)
    : QTabWidget(pParent)
{
    setContentsMargins(0, 0, 0, 0);
    setTabsClosable(true);
    tabBar()->installEventFilter(this);
    connect(tabBar(), &QTabBar::tabCloseRequested, this,
            [this](int index)
            {
                QWidget* pWidget = widget(index);
                removeTab(index);
                pWidget->deleteLater();
            });
}

//! Reimplemented filter of events
bool CustomTabWidget::eventFilter(QObject* pObject, QEvent* pEvent)
{
    if (pObject == tabBar() && pEvent->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(pEvent);
        int tabIndex = tabBar()->tabAt(mouseEvent->pos());
        if (tabIndex != -1)
        {
            renameTabDialog(tabIndex);
            return true;
        }
    }
    return QTabWidget::eventFilter(pObject, pEvent);
}

//! Create a dialog to edit tab text
void CustomTabWidget::renameTabDialog(int iTab)
{
    QString text = QInputDialog::getText(this, tr("Rename Tab"), tr("Tab name"), QLineEdit::Normal, tabText(iTab));
    if (!text.isEmpty())
        setTabText(iTab, text);
}
