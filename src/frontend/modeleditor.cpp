#include <QApplication>
#include <QTextEdit>
#include <QVBoxLayout>

#include <magicenum/magic_enum.hpp>

#include "modeleditor.h"
#include "uiutility.h"

using namespace Frontend;

ModelHighlighter::ModelHighlighter(QTextDocument* pParent)
    : QSyntaxHighlighter(pParent)
{
    // Set element rules
    QTextCharFormat elementFormat;
    elementFormat.setForeground(Qt::darkBlue);
    elementFormat.setFontWeight(QFont::Bold);
    QStringList elementPatterns = {"CCCC", "KIND OF ELEMENT", "ES NUMBER", "TOTAL PARAMETERS AND SPRINGS", "SPRING NUMBER", "POLYNOMIAL LENGTH"};
    for (QString& pattern : elementPatterns)
        pattern = QString("^.*%1.*$").arg(pattern);
    addRules(elementPatterns, elementFormat);
}

void ModelHighlighter::addRule(QString const& pattern, QTextCharFormat const& format)
{
    HighlightingRule rule;
    rule.pattern = QRegularExpression(pattern);
    rule.format = format;
    mRules.append(rule);
}

void ModelHighlighter::addRules(QStringList const& patterns, QTextCharFormat const& format)
{
    for (QString const& pattern : patterns)
        addRule(pattern, format);
}

void ModelHighlighter::highlightBlock(QString const& text)
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

ModelEditor::ModelEditor(KCL::Model& model, QString const& name, QWidget* pParent)
    : Editor(kModel, name, QIcon(":/icons/model.svg"), pParent)
    , mModel(model)
{
    createContent();
    ModelEditor::refresh();
}

QSize ModelEditor::sizeHint() const
{
    return QSize(1024, 768);
}

//! Update the widgets from the model
void ModelEditor::refresh()
{
    QString text = Utility::toString(mModel);
    mpEdit->setText(text);
}

//! Create all the widgets
void ModelEditor::createContent()
{
    // Create the widget to edit
    mpEdit = new QTextEdit;
    mpEdit->setReadOnly(true);
    QFont fnt = Utility::getMonospaceFont();
    fnt.setPointSize(fnt.pointSize() - 1);
    mpEdit->setFont(fnt);

    // Create the highlighter
    mpHighlighter = new ModelHighlighter(mpEdit->document());

    // Create the layout
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(mpEdit);

    // Set the layout
    setLayout(pLayout);
}
