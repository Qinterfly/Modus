#include <QDoubleValidator>

#include "lineedit.h"

using namespace Frontend;

static int const skNumDecimals = 4;
static double const skRangeValue = 1e6;

DoubleLineEdit::DoubleLineEdit(QWidget* pParent)
{
    mpValidator = new QDoubleValidator(this);
    mpValidator->setNotation(QDoubleValidator::StandardNotation);
    mpValidator->setRange(-skRangeValue, skRangeValue, skNumDecimals);
    mpValidator->setLocale(QLocale::C);
    setValidator(mpValidator);
    connect(this, &DoubleLineEdit::textEdited, this, &DoubleLineEdit::processTextChanged);
}

DoubleLineEdit::DoubleLineEdit(double minimum, double maximum, int decimals, QWidget* pParent)
    : DoubleLineEdit(pParent)
{
    setRange(minimum, maximum);
    setDecimals(decimals);
}

//! Get the current value
double DoubleLineEdit::value() const
{
    bool isOk = false;
    double result = text().toDouble(&isOk);
    return isOk ? result : (mpValidator->bottom() + mpValidator->top()) / 2.0;
}

//! Retrieve the minimum accepted value
double DoubleLineEdit::minimum() const
{
    return mpValidator->bottom();
}

//! Retrieve the maximum accepted value
double DoubleLineEdit::maximum() const
{
    return mpValidator->top();
}

//! Retrieve the number of decimals
int DoubleLineEdit::decimals() const
{
    return mpValidator->decimals();
}

//! Set the current value
void DoubleLineEdit::setValue(double value)
{
    QString newText = QString::number(value, 'g', mpValidator->decimals());
    mpValidator->fixup(newText);
    setText(newText);
}

//! Set value boundaries
void DoubleLineEdit::setRange(double minimum, double maximum)
{
    mpValidator->setRange(minimum, maximum);
}

//! Set number of decimals of text representation
void DoubleLineEdit::setDecimals(int number)
{
    mpValidator->setDecimals(number);
}

//! Slot function to handle text changes
void DoubleLineEdit::processTextChanged()
{
    int position = 0;
    QString newText = text();
    if (newText.contains(","))
    {
        newText.replace(",", ".");
        setText(newText);
    }
    if (mpValidator->validate(newText, position) == QValidator::Acceptable)
        emit valueChanged();
}
