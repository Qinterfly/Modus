#include <QDoubleValidator>

#include "lineedit.h"

using namespace Frontend;

DoubleLineEdit::DoubleLineEdit(QWidget* pParent)
{
    int const kNumDecimals = 6;
    double const kRangeValue = 1e9;
    mpValidator = new QDoubleValidator(this);
    mpValidator->setNotation(QDoubleValidator::StandardNotation);
    mpValidator->setRange(-kRangeValue, kRangeValue, kNumDecimals);
    setValidator(mpValidator);
}

DoubleLineEdit::DoubleLineEdit(double minimum, double maximum, int decimals, QWidget* pParent)
    : DoubleLineEdit(pParent)
{
    setRange(minimum, maximum);
    setDecimals(decimals);
}

double DoubleLineEdit::value() const
{
    bool isOk = false;
    double result = text().toDouble(&isOk);
    return isOk ? result : (mpValidator->bottom() + mpValidator->top()) / 2.0;
}

double DoubleLineEdit::minimum() const
{
    return mpValidator->bottom();
}

double DoubleLineEdit::maximum() const
{
    return mpValidator->top();
}

int DoubleLineEdit::decimals() const
{
    return mpValidator->decimals();
}

void DoubleLineEdit::setValue(double value)
{
    QString newText = QString::number(value, 'g', mpValidator->decimals());
    int position = 0;
    if (mpValidator->validate(newText, position) == QValidator::Acceptable)
        setText(newText);
}

void DoubleLineEdit::setRange(double minimum, double maximum)
{
    mpValidator->setRange(minimum, maximum);
}

void DoubleLineEdit::setDecimals(int number)
{
    mpValidator->setDecimals(number);
}
