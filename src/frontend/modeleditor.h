#ifndef MODELEDITOR_H
#define MODELEDITOR_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>

#include "editormanager.h"

QT_FORWARD_DECLARE_CLASS(QTextEdit);

namespace Frontend
{

class ModelHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    ModelHighlighter(QTextDocument* pParent = 0);
    virtual ~ModelHighlighter() = default;

protected:
    void highlightBlock(QString const& text) override;

private:
    void addRule(QString const& pattern, QTextCharFormat const& format);
    void addRules(QStringList const& patterns, QTextCharFormat const& format);

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> mRules;
};

//! Class to edit properties of entire model
class ModelEditor : public Editor
{
    Q_OBJECT

public:
    ModelEditor(KCL::Model& model, QString const& name, QWidget* pParent = nullptr);
    virtual ~ModelEditor() = default;

    QSize sizeHint() const override;
    void refresh() override;

private:
    void createContent();

private:
    KCL::Model& mModel;
    QTextEdit* mpEdit;
    ModelHighlighter* mpHighlighter;
};

}

#endif // MODELEDITOR_H
