#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QScreen>
#include <QToolBar>
#include <QWidget>

#include "uiutility.h"

namespace Frontend::Utility
{

//! Retrieve the active text color from the palette
QColor textColor(const QPalette& palette)
{
    return palette.color(QPalette::Active, QPalette::Text);
}

//! Change the text color of the widget
void setTextColor(QWidget* pWidget, const QColor& color)
{
    auto palette = pWidget->palette();
    if (textColor(palette) != color)
    {
        palette.setColor(QPalette::Active, QPalette::Text, color);
        pWidget->setPalette(palette);
    }
}

//! Show save dialog when closing a widget and process its output
int showSaveDialog(QWidget* pWidget, QString const& title, QString const& message)
{
    QMessageBox* pMessageBox = new QMessageBox(QMessageBox::Question, title, message,
                                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    pMessageBox->setFont(pWidget->font());
    auto result = pMessageBox->exec();
    switch (result)
    {
    case QMessageBox::Save:
        return 1;
    case QMessageBox::Discard:
        return 0;
    default:
        return -1;
    }
}

//! Resize widget to maximize its width and height
void fullScreenResize(QWidget* pWidget)
{
    QRect screenGeometry = QApplication::QGuiApplication::primaryScreen()->availableGeometry();
    pWidget->resize(screenGeometry.width(), screenGeometry.height());
}

//! Add shortcurt hints to all items contained in a tool bar
void setShortcutHints(QToolBar* pToolBar)
{
    QList<QAction*> actions = pToolBar->actions();
    int numActions = actions.size();
    for (int i = 0; i != numActions; ++i)
    {
        QAction* pAction = actions[i];
        QKeySequence shortcut = pAction->shortcut();
        if (shortcut.isEmpty())
            continue;
        pAction->setToolTip(QString("%1 (%2)").arg(pAction->toolTip(), shortcut.toString()));
    }
}

//! Get color name associated with error value
QString errorColorName(double value, double acceptThreshold, double criticalThreshold)
{
    QString result = "yellow";
    if (qAbs(value) < acceptThreshold)
        result = "green";
    else if (qAbs(value) > criticalThreshold)
        result = "red";
    return result;
}

//! Substitute a file suffix to the expected one, if necessary
void modifyFileSuffix(QString& pathFile, QString const& expectedSuffix)
{
    QFileInfo info(pathFile);
    QString currentSuffix = info.suffix();
    if (currentSuffix.isEmpty())
        pathFile.append(QString(".%1").arg(expectedSuffix));
    else if (currentSuffix != expectedSuffix)
        pathFile.replace(currentSuffix, expectedSuffix);
}

}
