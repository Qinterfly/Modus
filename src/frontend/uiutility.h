
#ifndef UIUTILITY_H
#define UIUTILITY_H

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QWidget);
QT_FORWARD_DECLARE_CLASS(QToolBar);

namespace Frontend::Utility
{

// Ui
int showSaveDialog(QWidget* pWidget, QString const& title, QString const& message);
void fullScreenResize(QWidget* pWidget);
void setShortcutHints(QToolBar* pToolBar);
}

#endif // UIUTILITY_H
