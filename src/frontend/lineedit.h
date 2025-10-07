#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>

QT_FORWARD_DECLARE_CLASS(QDoubleValidator)

namespace Frontend
{
class DoubleLineEdit : public QLineEdit
{
public:
    DoubleLineEdit(QWidget* pParent = nullptr);
    DoubleLineEdit(double minimum, double maximum, int decimals, QWidget* pParent = nullptr);
    ~DoubleLineEdit() = default;

    double value() const;
    double minimum() const;
    double maximum() const;
    int decimals() const;

    void setValue(double value);
    void setRange(double minimum, double maximum);
    void setDecimals(int number);

private:
    QDoubleValidator* mpValidator;
};

}

#endif // LINEEDIT_H
