#include <QApplication>
#include <QTime>

#include "logger.h"
#include "uiutility.h"

using namespace Frontend;

Logger::Logger(QWidget* pParent)
    : QTextEdit(pParent)
{
    setReadOnly(false);
    setFont(Utility::getMonospaceFont());
}

Logger::~Logger()
{
}

QSize Logger::sizeHint() const
{
    return QSize(800, 100);
}

//! Represent a message sent
void Logger::log(QtMsgType messageType, QString const& message)
{
    // Constants
    QChar kNewLine = '\n';
    QChar kComma = '\"';

    // Set the data to output
    QString time = QTime::currentTime().toString();
    QString filterMessage = message;
    if (filterMessage.endsWith(kComma))
        filterMessage.removeAt(filterMessage.size() - 1);
    if (filterMessage.startsWith(kComma))
        filterMessage.removeAt(0);

    // Determine the message color
    QColor color;
    switch (messageType)
    {
    case QtDebugMsg:
        color = Qt::gray;
        break;
    case QtInfoMsg:
        color = Qt::black;
        break;
    case QtWarningMsg:
        color = QColor("orange");
        break;
    case QtCriticalMsg:
        color = Qt::red;
        break;
    case QtFatalMsg:
        color = Qt::darkRed;
        break;
    }

    // Create the formats
    QTextCharFormat timeFormat;
    timeFormat.setUnderlineStyle(QTextCharFormat::UnderlineStyle::SingleUnderline);
    QTextCharFormat messageFormat;
    messageFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
    messageFormat.setForeground(QBrush(color));

    // Insert the text
    QTextCursor cursor = textCursor();
    if (document()->characterCount() > 0)
        cursor.insertText(kNewLine);
    cursor.insertText(time, timeFormat);
    cursor.insertText(kNewLine);
    cursor.insertText(filterMessage, messageFormat);

    // Insert a new line, if necessary
    if (message.back() != kNewLine)
        cursor.insertText(kNewLine);

    // Scroll to bottom
    moveCursor(QTextCursor::End);
}
