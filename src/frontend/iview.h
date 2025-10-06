#ifndef IVIEW_H
#define IVIEW_H

#include <QWidget>

class IView : public QWidget
{
public:
    enum Type
    {
        kModel
    };
    virtual ~IView() = default;
    virtual void clear() = 0;
    virtual void plot() = 0;
    virtual void refresh() = 0;
    virtual Type type() const = 0;
};

#endif // IVIEW_H
