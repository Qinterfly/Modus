#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>

QT_FORWARD_DECLARE_CLASS(QDoubleValidator)
QT_FORWARD_DECLARE_CLASS(QIntValidator)

namespace Frontend
{

//! Line editor of integer values
class IntLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    IntLineEdit(QWidget* pParent = nullptr);
    IntLineEdit(int minimum, int maximum, QWidget* pParent = nullptr);
    virtual ~IntLineEdit() = default;

    int value() const;
    int minimum() const;
    int maximum() const;

    void setValue(int value);
    void setMinimum(int value);
    void setMaximum(int value);
    void setRange(int minimum, int maximum);

    void hideBorders();

signals:
    void valueChanged();

private:
    void processEditingFinished();

private:
    QIntValidator* mpValidator;
    QString mPreviousText;
};

//! Line editor of double values
class DoubleLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    DoubleLineEdit(QWidget* pParent = nullptr);
    DoubleLineEdit(double minimum, double maximum, int decimals, QWidget* pParent = nullptr);
    virtual ~DoubleLineEdit() = default;

    double value() const;
    double minimum() const;
    double maximum() const;
    int decimals() const;

    void setValue(double value);
    void setMinimum(double value);
    void setMaximum(double value);
    void setRange(double minimum, double maximum);
    void setDecimals(int number);

    void hideBorders();

signals:
    void valueChanged();

private:
    void processTextEdited();
    void processEditingFinished();

private:
    QDoubleValidator* mpValidator;
    QString mPreviousText;
};

}

#endif // LINEEDIT_H
