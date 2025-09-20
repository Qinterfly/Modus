
#ifndef UIUTILITY_H
#define UIUTILITY_H

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QWidget);
QT_FORWARD_DECLARE_CLASS(QToolBar);
QT_FORWARD_DECLARE_CLASS(QColor);
QT_FORWARD_DECLARE_CLASS(QPalette);

namespace Frontend::Utility
{

// Text
QColor textColor(const QPalette& palette);
void setTextColor(QWidget* pWidget, const QColor& color);

// Ui
int showSaveDialog(QWidget* pWidget, QString const& title, QString const& message);
void fullScreenResize(QWidget* pWidget);
void setShortcutHints(QToolBar* pToolBar);
QString errorColorName(double value, double acceptThreshold, double criticalThreshold);

// File
void modifyFileSuffix(QString& pathFile, QString const& expectedSuffix);
}

#endif // UIUTILITY_H
