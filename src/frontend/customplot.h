
#ifndef CUSTOMPLOT_H
#define CUSTOMPLOT_H

#include <qcustomplot.h>

#include "custompropertyeditor.h"

class QCPAbstractPlottable;

namespace Frontend
{

class DataTip;

class CustomPlot : public QCustomPlot
{
    Q_OBJECT

public:
    CustomPlot(QWidget* pParent = nullptr);
    virtual ~CustomPlot();

    void clear();
    void refresh();
    void fit();

    QString title() const;
    void setTitle(QString const& title);
    QColor getAvailableColor() const;

signals:
    void dataDropped(QList<QStandardItem*> items);

protected:
    void keyPressEvent(QKeyEvent* pEvent) override;

private:
    // Create content and connections
    void initializePlot();
    void createActions();
    void specifyConnections();

    // State
    QList<QCPAbstractPlottable*> selectedPlottables();

    // Modifying entities
    void removeSelectedPlottables();
    void removeAllDataTips();
    void removeDataTip(uint index);
    void changeSelectedPlottable();
    void renameAxis(QCPAxis* pAxis, QCPAxis::SelectablePart part);
    void renamePlottable(QCPLegend* pLegend, QCPAbstractLegendItem* pItem);
    void renameTitle(QMouseEvent* pEvent);

    // Slots handling
    void processSelection();
    void processMousePress(QMouseEvent* pEvent);
    void processMouseWheel(QWheelEvent* pEvent);
    void processMouseMove(QMouseEvent* pEvent);
    void processMouseRelease(QMouseEvent* pEvent);
    void processBeforeReplot();
    void showContextMenu(QPoint position);
    void showDataTip(double xData, double yData);
    void copyPixmapToClipboard();
    void savePixmap();
    void viewPlotData();
    QPointF mapToLocal(QPointF const& position) const;

    // Legend
    bool mIsDragLegend;
    QPointF mDragLegendOrigin;

    // Items
    QList<DataTip*> mDataTips;

    // Elements
    QCPTextElement* mpTitle;
};

class DataTip : public QCPItemText
{
public:
    DataTip(double xData, double yData, CustomPlot* pPlot);
    virtual ~DataTip();

    void startDrag(QPointF const& origin);
    void processDrag(QPointF const& point);
    void finishDrag();
    bool isDrag() const;
    bool isSelect(QPointF const& position, int tolerance);

private:
    double mXData;
    double mYData;
    bool mIsDrag;
    QPointF mDragOrigin;
    CustomPlot* mpPlot;
    QCPCurve* mpCurve;
};

class PlottablePropertyEditor : public QWidget
{
    Q_OBJECT

public:
    enum Type
    {
        kLineStyle,
        kLineColor,
        kMarkerColor,
        kMarkerSize,
        kMarkerShape
    };

    PlottablePropertyEditor(QCPAbstractPlottable* pPlottable, QWidget* pParent = nullptr);
    virtual ~PlottablePropertyEditor() = default;

protected:
    QSize sizeHint() const override;

private:
    void createContent();
    void createProperties();
    void createConnections();

    void setIntValue(QtProperty* pProperty, int value);
    void setDoubleValue(QtProperty* pProperty, double value);
    void setColorValue(QtProperty* pProperty, QColor const& value);

private:
    QCPAbstractPlottable* mpPlottable;
    CustomPropertyEditor* mpEditor;
};
}

#endif // CUSTOMPLOT_H
