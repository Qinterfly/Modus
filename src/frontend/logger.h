
#ifndef LOGGER_H
#define LOGGER_H

#include <QTextEdit>

namespace Frontend
{

//! Log all the messages sent
class Logger : public QTextEdit
{
    Q_OBJECT

public:
    Logger(QWidget* pParent = nullptr);
    Logger(Logger const& another) = delete;
    Logger& operator=(Logger const& another) = delete;
    virtual ~Logger();

    QSize sizeHint() const override;
    void log(QtMsgType messageType, QString const& message);
};

}

#endif // LOGGER_H
