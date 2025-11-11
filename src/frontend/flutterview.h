#ifndef FLUTTERVIEW_H
#define FLUTTERVIEW_H

#include <Eigen/Core>

#include "customplot.h"
#include "iview.h"

class CustomPlot;
class QListWidget;

namespace Backend::Core
{
struct FlutterSolution;
}

namespace Frontend
{

using Marker = QCPScatterStyle::ScatterShape;

//! Flutter displaying options
struct FlutterViewOptions
{
    FlutterViewOptions();
    ~FlutterViewOptions() = default;

    // Display
    QList<int> indicesModes;
    QList<QColor> modeColors;
    QList<Marker> modeMarkers;

    // Limits
    QPair<double, double> limitsFrequencies;
    QPair<double, double> limitsDecrements;

    // Flags
    bool showCircular;
    bool showLines;

    // Size
    int markerSize;
    double lineWidth;
};

//! Class to edit options interactively
class FlutterViewEditor : public QWidget
{
    Q_OBJECT

public:
    FlutterViewEditor(FlutterViewOptions& options);
    virtual ~FlutterViewEditor() = default;

    QSize sizeHint() const override;
    void refresh(QList<bool> const& maskModes);

signals:
    void edited();

private:
    void createContent();
    void createConnections();
    void processModeSelection();
    void processModeDoubleClick(QListWidgetItem* pItem);
    void invertModeSelection();

private:
    FlutterViewOptions& mOptions;
    QListWidget* mpModeList;
};

//! Class to display flutter solution
class FlutterView : public IView
{
    Q_OBJECT

public:
    FlutterView(Backend::Core::FlutterSolution const& solution, FlutterViewOptions const& options = FlutterViewOptions());
    virtual ~FlutterView();

    Backend::Core::FlutterSolution const& solution() const;

    void clear() override;
    void plot() override;
    void refresh() override;
    IView::Type type() const override;

private:
    // Content
    void createContent();
    void createConnections();

    // Process
    void setData();

    // Plot
    void plotVgDiagram();
    void plotHodograph();
    void addGraph(CustomPlot* pPlot, QList<double> const& xData, QList<double> const& yData, QColor const& color, Marker marker);
    void addCurve(CustomPlot* pPlot, QList<double> const& xData, QList<double> const& yData, QColor const& color, Marker marker);

private:
    Backend::Core::FlutterSolution const& mSolution;
    FlutterViewOptions mOptions;

    // Plots
    CustomPlot* mpFrequencyPlot;
    CustomPlot* mpDecrementPlot;
    CustomPlot* mpHodographPlot;

    // Editor
    FlutterViewEditor* mpEditor;

    // Data
    QList<bool> mMaskModes;
    Eigen::MatrixXd mFrequencies;
    Eigen::MatrixXd mDecrements;
};

}

#endif // FLUTTERVIEW_H
