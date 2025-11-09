#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>

#include "iview.h"

QT_FORWARD_DECLARE_CLASS(QTextEdit)

namespace Frontend
{

class LogHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    LogHighlighter(QTextDocument* pParent = 0);
    virtual ~LogHighlighter() = default;

protected:
    void highlightBlock(QString const& text) override;

private:
    void addRule(QString const& pattern, QTextCharFormat const& format);

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> mRules;
};

//! Class to display solver log
class LogView : public IView
{
    Q_OBJECT

public:
    LogView(QString const& log);
    virtual ~LogView();

    void clear() override;
    void plot() override;
    void refresh() override;
    IView::Type type() const override;

    QString const& log() const;

private:
    void createContent();

private:
    QString const& mLog;
    QTextEdit* mpEdit;
    LogHighlighter* mpHighlighter;
};

}

#endif // LOGVIEW_H
