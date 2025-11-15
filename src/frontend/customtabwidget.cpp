#include <QEvent>
#include <QInputDialog>
#include <QMenu>
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
    connect(tabBar(), &QTabBar::tabCloseRequested, this, &CustomTabWidget::removePage);
}

//! Remove tab as well as widget associated with it
void CustomTabWidget::removePage(int index)
{
    QWidget* pWidget = widget(index);
    QTabWidget::removeTab(index);
    pWidget->deleteLater();
}

void CustomTabWidget::removeAllPages()
{
    while (count() > 0)
        removePage(0);
}

//! Reimplemented filter of events
bool CustomTabWidget::eventFilter(QObject* pObject, QEvent* pEvent)
{
    if (pObject == tabBar())
    {
        if (pEvent->type() == QEvent::MouseButtonDblClick)
        {
            QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(pEvent);
            if (pMouseEvent->button() == Qt::LeftButton)
            {
                int iTab = tabBar()->tabAt(pMouseEvent->pos());
                if (iTab != -1)
                {
                    renameTabDialog(iTab);
                    return true;
                }
            }
        }
        else if (pEvent->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(pEvent);
            if (pMouseEvent->button() == Qt::RightButton)
            {
                int iTab = tabBar()->tabAt(pMouseEvent->pos());
                if (iTab != -1)
                {
                    // Create the menu
                    QMenu* pMenu = new QMenu(this);

                    // Create the actions
                    QAction* pCloseAction = new QAction(tr("&Close tab"), this);
                    QAction* pRenameAction = new QAction(tr("Rename tab"), this);
                    QAction* pCloseAllAction = new QAction(tr("&Close all tabs"), this);

                    // Set the icons
                    pCloseAction->setIcon(QIcon(":/icons/edit-remove.svg"));
                    pRenameAction->setIcon(QIcon(":/icons/edit-edit.svg"));

                    // Set the connections
                    connect(pCloseAction, &QAction::triggered, this, [this, iTab]() { removeTab(iTab); });
                    connect(pRenameAction, &QAction::triggered, this, [this, iTab]() { renameTabDialog(iTab); });
                    connect(pCloseAllAction, &QAction::triggered, this, &CustomTabWidget::removeAllPages);

                    // Add the actions
                    pMenu->addAction(pCloseAction);
                    pMenu->addAction(pRenameAction);
                    pMenu->addSeparator();
                    pMenu->addAction(pCloseAllAction);

                    // Display the menu
                    pMenu->exec(QCursor::pos());
                }
            }
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
