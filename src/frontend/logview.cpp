#include <QTextEdit>
#include <QVBoxLayout>

#include "logview.h"
#include "uiutility.h"

using namespace Frontend;

LogHighlighter::LogHighlighter(QTextDocument* pParent)
    : QSyntaxHighlighter(pParent)
{
    // Set step rules
    QTextCharFormat stepFormat;
    stepFormat.setFontWeight(QFont::Medium);
    QString stepPattern = "^\\*.*";
    addRule(stepPattern, stepFormat);

    // Set solver rules
    QTextCharFormat solverFormat;
    solverFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    QString solverPattern = "^.*Solver.*";
    addRule(solverPattern, solverFormat);

    // Set finish rules
    QTextCharFormat finishFormat;
    finishFormat.setForeground(Qt::darkGreen);
    finishFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    QString finishPattern = "^.*successfully.*";
    addRule(finishPattern, finishFormat);

    // Set error rules
    QTextCharFormat errorFormat;
    errorFormat.setForeground(Qt::red);
    QString errorPattern = "^.*Error.*";
    addRule(errorPattern, errorFormat);

    // Set Warning rules
    QTextCharFormat warningFormat;
    warningFormat.setForeground(Qt::yellow);
    QString warningPattern = "^.*Warning.*";
    addRule(warningPattern, warningFormat);

    // Set time rule
    QTextCharFormat timeFormat;
    timeFormat.setForeground(Qt::darkBlue);
    QString timePattern = "^\\[.*:.*:.*\\]";
    addRule(timePattern, timeFormat);
}

void LogHighlighter::addRule(QString const& pattern, QTextCharFormat const& format)
{
    HighlightingRule rule;
    rule.pattern = QRegularExpression(pattern);
    rule.format = format;
    mRules.append(rule);
}

void LogHighlighter::highlightBlock(QString const& text)
{
    setCurrentBlockState(0);
    for (HighlightingRule const& rule : std::as_const(mRules))
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

LogView::LogView(QString const& log)
    : mLog(log)
{
    createContent();
}

LogView::~LogView()
{
}

//! Clear all the text
void LogView::clear()
{
    mpEdit->clear();
}

//! Display the log text
void LogView::plot()
{
    clear();
    mpEdit->setText(mLog);
}

//! Update the log text
void LogView::refresh()
{
    plot();
}

//! Get the view type
IView::Type LogView::type() const
{
    return IView::kLog;
}

//! Retrieve the current log
QString const& LogView::log() const
{
    return mLog;
}

//! Create all the widgets
void LogView::createContent()
{
    // Create the text widget
    mpEdit = new QTextEdit;
    mpEdit->setReadOnly(true);
    mpEdit->setFont(Utility::getMonospaceFont());

    // Create the text highlighter
    mpHighlighter = new LogHighlighter(mpEdit->document());

    // Set the layout
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(mpEdit);
    setLayout(pLayout);
}
