#include <qcustomplot.h>
#include <QVBoxLayout>

#include "customplot.h"
#include "customtabwidget.h"
#include "fluttersolver.h"
#include "flutterview.h"
#include "lineedit.h"
#include "uiconstants.h"
#include "uiutility.h"

using namespace Backend;
using namespace Frontend;

QString getModeName(int iMode);
QIcon getIcon(QCPScatterStyle const& style, QSize const& size, bool isLine);
void updateLimits(Edit1d* pMinEdit, Edit1d* pMaxEdit);

FlutterViewOptions::FlutterViewOptions()
    : indicesModes(128)
{
    // Display
    for (int i = 0; i != indicesModes.size(); ++i)
        indicesModes[i] = i;
    modeColors = Constants::Colors::skStandardColors;
    modeMarkers = {QCPScatterStyle::ssCross,  QCPScatterStyle::ssPlus,    QCPScatterStyle::ssCircle,   QCPScatterStyle::ssDisc,
                   QCPScatterStyle::ssSquare, QCPScatterStyle::ssDiamond, QCPScatterStyle::ssTriangle, QCPScatterStyle::ssTriangleInverted};

    // Limits
    limitsFrequencies = {0, 200};
    limitsDecrements = {-10, 5};

    // Grid
    numFrequency = 5;
    numDecrement = 5;

    // Flags
    showCircular = true;
    showLines = true;

    // Size
    markerSize = 6;
    lineWidth = 1.5;
}

FlutterViewEditor::FlutterViewEditor(FlutterViewOptions& options)
    : mOptions(options)
{
    createContent();
    createConnections();
}

QSize FlutterViewEditor::sizeHint() const
{
    return QSize(40, 800);
}

void FlutterViewEditor::refresh(QList<bool> const& maskModes)
{
    // Constants
    int const kPanWidth = 5;
    int const kShapeSize = 20;
    QSize const kIconSize(64, 64);

    // Clear the list of modes
    QSignalBlocker blockerModeList(mpModeList);
    mpModeList->clear();

    // Add the mode legend items
    int numModes = maskModes.size();
    for (int iMode = 0; iMode != numModes; ++iMode)
    {
        // Get rendering properties
        int iColor = Utility::getRepeatedIndex(iMode, mOptions.modeColors.size());
        int iMarker = Utility::getRepeatedIndex(iMode, mOptions.modeMarkers.size());
        QColor color = mOptions.modeColors[iColor];
        auto marker = mOptions.modeMarkers[iMarker];

        // Get the icon
        QCPScatterStyle style(marker, QPen(color, kPanWidth), QBrush(), kShapeSize);
        QIcon icon = getIcon(style, kIconSize, mOptions.showLines);

        // Add the item
        QListWidgetItem* pItem = new QListWidgetItem(icon, getModeName(iMode));
        mpModeList->addItem(pItem);

        // Select the item
        if (maskModes[iMode])
            mpModeList->setCurrentItem(pItem, QItemSelectionModel::Select);
    }

    // Set the limits
    QSignalBlocker blockerMinFrequency(mpMinFrequencyEdit);
    QSignalBlocker blockerMaxFrequency(mpMaxFrequencyEdit);
    QSignalBlocker blockerMinDecrement(mpMinDecrementEdit);
    QSignalBlocker blockerMaxDecrement(mpMaxDecrementEdit);
    mpMinFrequencyEdit->setValue(mOptions.limitsFrequencies.first);
    mpMaxFrequencyEdit->setValue(mOptions.limitsFrequencies.second);
    mpMinDecrementEdit->setValue(mOptions.limitsDecrements.first);
    mpMaxDecrementEdit->setValue(mOptions.limitsDecrements.second);

    // Set the grid
    QSignalBlocker blockerNumFrequency(mpNumFrequencyEdit);
    QSignalBlocker blockerNumDecrement(mpNumDecrementEdit);
    mpNumFrequencyEdit->setValue(mOptions.numFrequency);
    mpNumDecrementEdit->setValue(mOptions.numDecrement);

    // Set the flags
    QSignalBlocker blockerCircular(mpCircularCheckBox);
    QSignalBlocker blockerLines(mpLinesCheckBox);
    mpCircularCheckBox->setChecked(mOptions.showCircular);
    mpLinesCheckBox->setChecked(mOptions.showLines);
}

//! Create all the widget
void FlutterViewEditor::createContent()
{
    // Construct the main layout
    QHBoxLayout* pMainLayout = new QHBoxLayout;
    pMainLayout->setContentsMargins(0, 0, 0, 0);

    // Set the widgets to edit mode selection
    QVBoxLayout* pModeLayout = new QVBoxLayout;
    pModeLayout->addWidget(createModeGroupBox());
    pModeLayout->addStretch();
    pMainLayout->addLayout(pModeLayout);

    // Set the widgets to edit options
    QVBoxLayout* pOptionsLayout = new QVBoxLayout;
    pOptionsLayout->addWidget(createLimitsGroupBox());
    pOptionsLayout->addWidget(createGridGroupBox());
    pOptionsLayout->addWidget(createFlagsGroupBox());
    pOptionsLayout->addStretch();
    pMainLayout->addLayout(pOptionsLayout);

    // Set the layout
    setLayout(pMainLayout);
}

//! Specify widget connections
void FlutterViewEditor::createConnections()
{
    // Mode
    connect(mpModeList, &QListWidget::itemSelectionChanged, this, &FlutterViewEditor::setOptions);
    connect(mpModeList, &QListWidget::itemDoubleClicked, this, &FlutterViewEditor::processModeDoubleClick);

    // Limits
    connect(mpMinFrequencyEdit, &Edit1d::valueChanged, this, &FlutterViewEditor::setOptions);
    connect(mpMaxFrequencyEdit, &Edit1d::valueChanged, this, &FlutterViewEditor::setOptions);
    connect(mpMinDecrementEdit, &Edit1d::valueChanged, this, &FlutterViewEditor::setOptions);
    connect(mpMaxDecrementEdit, &Edit1d::valueChanged, this, &FlutterViewEditor::setOptions);

    // Grid
    connect(mpNumFrequencyEdit, &Edit1i::valueChanged, this, &FlutterViewEditor::setOptions);
    connect(mpNumDecrementEdit, &Edit1i::valueChanged, this, &FlutterViewEditor::setOptions);
}

//! Create widgets to handle mode selection
QGroupBox* FlutterViewEditor::createModeGroupBox()
{
    // Create the list to select modes
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    mpModeList = new QListWidget;
    mpModeList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mpModeList->setContentsMargins(0, 0, 0, 0);
    mpModeList->setResizeMode(QListWidget::Adjust);
    mpModeList->setSizeAdjustPolicy(QListWidget::AdjustToContents);
    pMainLayout->addWidget(mpModeList);

    // Create the selection controls
    QPushButton* pInvertButton = new QPushButton(tr("Invert selection"));
    connect(pInvertButton, &QPushButton::clicked, this, &FlutterViewEditor::invertModeSelection);
    QHBoxLayout* pControlLayout = new QHBoxLayout;
    pControlLayout->addWidget(pInvertButton);
    pControlLayout->addStretch();
    pMainLayout->addLayout(pControlLayout);
    pMainLayout->addStretch();

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(tr("Mode selection"));
    pGroupBox->setContentsMargins(0, 0, 0, 0);
    pGroupBox->setLayout(pMainLayout);

    return pGroupBox;
}

//! Create widgets to handle limits
QGroupBox* FlutterViewEditor::createLimitsGroupBox()
{
    // Create the layout
    QGridLayout* pLayout = new QGridLayout;
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->setAlignment(Qt::AlignTop);

    // Create the widgets
    mpMinFrequencyEdit = new Edit1d;
    mpMaxFrequencyEdit = new Edit1d;
    mpMinDecrementEdit = new Edit1d;
    mpMaxDecrementEdit = new Edit1d;

    // Set the frequency limits
    pLayout->addWidget(new QLabel(tr("f<sub>min</sub>: ")), 0, 0);
    pLayout->addWidget(new QLabel(tr("f<sub>max</sub>: ")), 1, 0);
    pLayout->addWidget(mpMinFrequencyEdit, 0, 1);
    pLayout->addWidget(mpMaxFrequencyEdit, 1, 1);

    // Set the decrement limits
    pLayout->addWidget(new QLabel(tr("d<sub>min</sub>: ")), 2, 0);
    pLayout->addWidget(new QLabel(tr("d<sub>max</sub>: ")), 3, 0);
    pLayout->addWidget(mpMinDecrementEdit, 2, 1);
    pLayout->addWidget(mpMaxDecrementEdit, 3, 1);

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(tr("Limits"));
    pGroupBox->setContentsMargins(0, 0, 0, 0);
    pGroupBox->setLayout(pLayout);

    return pGroupBox;
}

//! Create widgets to handle grid
QGroupBox* FlutterViewEditor::createGridGroupBox()
{
    // Create the layout
    QGridLayout* pLayout = new QGridLayout;
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->setAlignment(Qt::AlignTop);

    // Create the widgets
    mpNumFrequencyEdit = new Edit1i;
    mpNumDecrementEdit = new Edit1i;
    mpNumFrequencyEdit->setMinimum(1);
    mpNumDecrementEdit->setMinimum(1);

    // Set the frequency limits
    pLayout->addWidget(new QLabel(tr("N<sub>f</sub>: ")), 0, 0);
    pLayout->addWidget(new QLabel(tr("N<sub>d</sub>: ")), 1, 0);
    pLayout->addWidget(mpNumFrequencyEdit, 0, 1);
    pLayout->addWidget(mpNumDecrementEdit, 1, 1);

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(tr("Grid"));
    pGroupBox->setContentsMargins(0, 0, 0, 0);
    pGroupBox->setLayout(pLayout);

    return pGroupBox;
}

//! Create widgets to handle flags
QGroupBox* FlutterViewEditor::createFlagsGroupBox()
{
    // Create the layout
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->setAlignment(Qt::AlignTop);

    // Set the auxilary function
    auto createFlagWidget = [this, pLayout](QString const& text)
    {
        QCheckBox* pCheckBox = new QCheckBox(text);
        pLayout->addWidget(pCheckBox);
        connect(pCheckBox, &QCheckBox::clicked, this, &FlutterViewEditor::setOptions);
        return pCheckBox;
    };

    // Create the widgets
    mpCircularCheckBox = createFlagWidget(tr("Circular"));
    mpLinesCheckBox = createFlagWidget(tr("Lines"));

    // Create the group box
    QGroupBox* pGroupBox = new QGroupBox(tr("Flags"));
    pGroupBox->setContentsMargins(0, 0, 0, 0);
    pGroupBox->setLayout(pLayout);

    return pGroupBox;
}

//! Process double click event on item
void FlutterViewEditor::processModeDoubleClick(QListWidgetItem* pItem)
{
    // Retrieve the current color
    int iMode = mpModeList->row(pItem);
    int iColor = Utility::getRepeatedIndex(iMode, mOptions.modeColors.size());
    QColor color = mOptions.modeColors[iColor];

    // Change the color via dialog
    mOptions.modeColors[iColor] = QColorDialog::getColor(color, this, tr("Change mode color"));

    // Mark the finish of editing
    emit edited();
}

//! Invert the selection of modes
void FlutterViewEditor::invertModeSelection()
{
    QSignalBlocker blocker(mpModeList);

    // Slice selected items
    QList<QListWidgetItem*> selectedItems = mpModeList->selectedItems();

    // Slice dimensions
    int numModes = mpModeList->count();
    int numSelected = selectedItems.size();

    // Set the mask of modes
    QList<bool> maskModes(numModes, true);
    for (int i = 0; i != numSelected; ++i)
    {
        int iMode = mpModeList->row(selectedItems[i]);
        maskModes[iMode] = false;
    }

    // Apply the selection
    for (int i = 0; i != numModes; ++i)
    {
        if (maskModes[i])
            mpModeList->setCurrentRow(i, QItemSelectionModel::Select);
        else
            mpModeList->setCurrentRow(i, QItemSelectionModel::Deselect);
    }

    // Set the options
    setOptions();
}

//! Set the new values of options based on widget state
void FlutterViewEditor::setOptions()
{
    // Update the limits
    updateLimits(mpMinFrequencyEdit, mpMaxFrequencyEdit);
    updateLimits(mpMinDecrementEdit, mpMaxDecrementEdit);

    // Set the mode selection
    QList<QListWidgetItem*> selectedItems = mpModeList->selectedItems();
    int numSelected = selectedItems.size();
    mOptions.indicesModes.resize(numSelected);
    for (int i = 0; i != numSelected; ++i)
        mOptions.indicesModes[i] = mpModeList->row(selectedItems[i]);

    // Set the limits
    mOptions.limitsFrequencies.first = mpMinFrequencyEdit->value();
    mOptions.limitsFrequencies.second = mpMaxFrequencyEdit->value();
    mOptions.limitsDecrements.first = mpMinDecrementEdit->value();
    mOptions.limitsDecrements.second = mpMaxDecrementEdit->value();

    // Set the grid
    mOptions.numFrequency = mpNumFrequencyEdit->value();
    mOptions.numDecrement = mpNumDecrementEdit->value();

    // Set the grid
    mOptions.showCircular = mpCircularCheckBox->isChecked();
    mOptions.showLines = mpLinesCheckBox->isChecked();

    // Finish up the editing
    emit edited();
}

FlutterView::FlutterView(Core::FlutterSolution const& solution, FlutterViewOptions const& options)
    : mSolution(solution)
    , mOptions(options)
{
    createContent();
    createConnections();
}

FlutterView::~FlutterView()
{
}

Backend::Core::FlutterSolution const& FlutterView::solution() const
{
    return mSolution;
}

//! Clear all the items from the scene
void FlutterView::clear()
{
    mpFrequencyPlot->clearPlottables();
    mpDecrementPlot->clearPlottables();
    mpHodographPlot->clearPlottables();
    mMaskModes.clear();
    mDecrements = Eigen::MatrixXd();
    mFrequencies = Eigen::MatrixXd();
}

//! Draw the scene
void FlutterView::plot()
{
    clear();
    setData();
    plotVgDiagram();
    plotHodograph();
    mpEditor->refresh(mMaskModes);
}

//! Update the scene
void FlutterView::refresh()
{
    plot();
}

//! Get the view type
IView::Type FlutterView::type() const
{
    return IView::kFlutter;
}

//! Create all the widgets
void FlutterView::createContent()
{
    // Constants
    int const kHandleWidth = 10;

    // Create the main layout
    QHBoxLayout* pMainLayout = new QHBoxLayout;

    // Create the plots
    mpFrequencyPlot = new CustomPlot;
    mpDecrementPlot = new CustomPlot;
    mpHodographPlot = new CustomPlot;

    // Create the editor
    mpEditor = new FlutterViewEditor(mOptions);

    // Add the graphs
    QSplitter* pVSplitter = new QSplitter(Qt::Vertical);
    pVSplitter->setHandleWidth(kHandleWidth);
    pVSplitter->addWidget(mpDecrementPlot);
    pVSplitter->addWidget(mpFrequencyPlot);

    // Create the graph layout
    CustomTabWidget* pTabWidget = new CustomTabWidget;
    pTabWidget->setTabsClosable(false);
    pTabWidget->setTabPosition(CustomTabWidget::West);
    pTabWidget->addTab(pVSplitter, tr("Vg Diagram"));
    pTabWidget->addTab(mpHodographPlot, tr("Hodograph"));

    QSplitter* pHSplitter = new QSplitter(Qt::Horizontal);
    pHSplitter->setHandleWidth(kHandleWidth);
    pHSplitter->addWidget(pTabWidget);
    pHSplitter->addWidget(mpEditor);

    // Create the layout to manipulate graphs
    pMainLayout->addWidget(pHSplitter);

    // Set the layout
    setLayout(pMainLayout);
}

//! Specify connections between widgets
void FlutterView::createConnections()
{
    connect(mpEditor, &FlutterViewEditor::edited, this, &FlutterView::plot);
}

//! Compute the data for plotting
void FlutterView::setData()
{
    // Constants
    double const kTwoPi = 2.0 * M_PI;
    double const kThreshold = 1e-6;

    // Slice dimensions
    int numSteps = mSolution.flow.size();
    int numModes = mSolution.roots.rows();

    // Set the matrices of frequencies and decrements
    mFrequencies.resize(numModes, numSteps);
    mDecrements.resize(numModes, numSteps);
    for (int iMode = 0; iMode != numModes; ++iMode)
    {
        for (int iStep = 0; iStep != numSteps; ++iStep)
        {
            double realValue = mSolution.roots(iMode, iStep).real();
            double imagValue = mSolution.roots(iMode, iStep).imag();
            double frequency, decrement;
            if (mOptions.showCircular)
            {
                frequency = imagValue;
                decrement = realValue;
            }
            else
            {
                frequency = imagValue / kTwoPi;
                if (std::abs(imagValue) > kThreshold)
                    decrement = kTwoPi * realValue / imagValue;
                else
                    decrement = std::copysign(std::numeric_limits<double>::infinity(), realValue);
            }
            frequency = std::clamp(frequency, mOptions.limitsFrequencies.first, mOptions.limitsFrequencies.second);
            decrement = std::clamp(decrement, mOptions.limitsDecrements.first, mOptions.limitsDecrements.second);
            mFrequencies(iMode, iStep) = frequency;
            mDecrements(iMode, iStep) = decrement;
        }
    }

    // Set the mask of modes for plotting
    mMaskModes.resize(numModes);
    int numRequestedModes = mOptions.indicesModes.size();
    for (int i = 0; i != numRequestedModes; ++i)
    {
        int iMode = mOptions.indicesModes[i];
        if (iMode >= 0 && iMode < numModes)
            mMaskModes[iMode] = true;
    }
}

//! Display aerodynamic Vg diagram
void FlutterView::plotVgDiagram()
{
    // Slice dimensions
    int numSteps = mSolution.flow.size();
    int numModes = mMaskModes.size();

    // Get the flow values
    QList<double> flow(numSteps);
    for (int i = 0; i != numSteps; ++i)
        flow[i] = mSolution.flow[i];

    // Process all the modes
    for (int iMode = 0; iMode != numModes; ++iMode)
    {
        if (!mMaskModes[iMode])
            continue;

        // Set the data
        QList<double> frequency(numSteps);
        QList<double> decrement(numSteps);
        for (int iStep = 0; iStep != numSteps; ++iStep)
        {
            frequency[iStep] = mFrequencies(iMode, iStep);
            decrement[iStep] = mDecrements(iMode, iStep);
        }

        // Get rendering properties
        int iColor = Utility::getRepeatedIndex(iMode, mOptions.modeColors.size());
        int iMarker = Utility::getRepeatedIndex(iMode, mOptions.modeMarkers.size());
        QColor color = mOptions.modeColors[iColor];
        auto marker = mOptions.modeMarkers[iMarker];

        // Add the graphs
        addGraph(mpFrequencyPlot, flow, frequency, color, marker);
        addGraph(mpDecrementPlot, flow, decrement, color, marker);
    }

    // Set the ranges
    mpFrequencyPlot->yAxis->setRange(mOptions.limitsFrequencies.first, mOptions.limitsFrequencies.second);
    mpDecrementPlot->yAxis->setRange(mOptions.limitsDecrements.first, mOptions.limitsDecrements.second);

    // Set the ticks
    mpFrequencyPlot->yAxis->ticker()->setTickCount(mOptions.numFrequency);
    mpDecrementPlot->yAxis->ticker()->setTickCount(mOptions.numDecrement);

    // Set the labels
    QString suffixFrequency = mOptions.showCircular ? tr("rad/s") : tr("Hz");
    mpFrequencyPlot->xAxis->setLabel(tr("Flow"));
    mpDecrementPlot->xAxis->setLabel(tr("Flow"));
    mpFrequencyPlot->yAxis->setLabel(tr("Frequency, %1").arg(suffixFrequency));
    mpDecrementPlot->yAxis->setLabel(tr("Decrement"));

    // Update the plots
    mpFrequencyPlot->replot();
    mpDecrementPlot->replot();
}

//! Display hodograph based on real and imaginary values of roots
void FlutterView::plotHodograph()
{
    // Slice dimensions
    int numSteps = mSolution.flow.size();
    int numModes = mMaskModes.size();

    // Process all the modes
    for (int iMode = 0; iMode != numModes; ++iMode)
    {
        if (!mMaskModes[iMode])
            continue;

        // Set the data
        QList<double> frequency(numSteps);
        QList<double> decrement(numSteps);
        for (int iStep = 0; iStep != numSteps; ++iStep)
        {
            frequency[iStep] = mFrequencies(iMode, iStep);
            decrement[iStep] = mDecrements(iMode, iStep);
        }

        // Get rendering properties
        int iColor = Utility::getRepeatedIndex(iMode, mOptions.modeColors.size());
        int iMarker = Utility::getRepeatedIndex(iMode, mOptions.modeMarkers.size());
        QColor color = mOptions.modeColors[iColor];
        auto marker = mOptions.modeMarkers[iMarker];

        // Add the curves
        addCurve(mpHodographPlot, decrement, frequency, color, marker);
    }

    // Set the ranges
    mpHodographPlot->xAxis->setRange(mOptions.limitsDecrements.first, mOptions.limitsDecrements.second);
    mpHodographPlot->yAxis->setRange(mOptions.limitsFrequencies.first, mOptions.limitsFrequencies.second);

    // Set the ticks
    mpHodographPlot->xAxis->ticker()->setTickCount(mOptions.numDecrement);
    mpHodographPlot->yAxis->ticker()->setTickCount(mOptions.numFrequency);

    // Set the labels
    QString suffixFrequency = mOptions.showCircular ? tr("rad/s") : tr("Hz");
    mpHodographPlot->xAxis->setLabel(tr("Decrement"));
    mpHodographPlot->yAxis->setLabel(tr("Frequency, %1").arg(suffixFrequency));

    // Update the plot
    mpHodographPlot->replot();
}

//! Add the graph to the specified plot
void FlutterView::addGraph(CustomPlot* pPlot, QList<double> const& xData, QList<double> const& yData, QColor const& color, Marker marker)
{
    // Create a new graph
    QCPGraph* pGraph = pPlot->addGraph();
    pGraph->setData(xData, yData, false);
    pGraph->setSelectable(QCP::SelectionType::stSingleData);
    pGraph->setAdaptiveSampling(true);
    pGraph->setScatterStyle(QCPScatterStyle(marker, mOptions.markerSize));
    if (mOptions.showLines)
        pGraph->setLineStyle(QCPGraph::lsLine);
    else
        pGraph->setLineStyle(QCPGraph::lsNone);

    // Set the graph style
    pGraph->setPen(QPen(color, mOptions.lineWidth));

    // Rescale the axes, so that all the graphs fit
    bool isEnlarge = pPlot->graphCount() > 1;
    pGraph->rescaleAxes(isEnlarge);
}

//! Add the curve to the specified plot
void FlutterView::addCurve(CustomPlot* pPlot, QList<double> const& xData, QList<double> const& yData, QColor const& color, Marker marker)
{
    // Create a new curve
    QCPCurve* pCurve = new QCPCurve(pPlot->xAxis, pPlot->yAxis);
    pCurve->setData(xData, yData);
    pCurve->setSelectable(QCP::SelectionType::stSingleData);
    pCurve->setScatterStyle(QCPScatterStyle(marker, mOptions.markerSize));
    if (mOptions.showLines)
        pCurve->setLineStyle(QCPCurve::lsLine);
    else
        pCurve->setLineStyle(QCPCurve::lsNone);

    // Set the curve style
    pCurve->setPen(QPen(color, mOptions.lineWidth));

    // Rescale the axes, so that all the graphs fit
    bool isEnlarge = pPlot->graphCount() > 1;
    pCurve->rescaleAxes(isEnlarge);
}

//! Helper function to get mode name
QString getModeName(int iMode)
{
    return QObject::tr("Mode %1").arg(1 + iMode);
}

//! Helper function to get icons for legend
QIcon getIcon(QCPScatterStyle const& style, QSize const& size, bool isLine)
{
    // Construct the pixmap to be drawn at
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    // Create the painter
    QCPPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    style.applyTo(&painter, style.pen());

    // Draw the line style
    if (isLine)
    {
        QLineF line(0, size.height() / 2.0, size.width(), size.height() / 2);
        painter.drawLine(line);
    }

    // Draw the scatter style
    QRectF targetRect(0, 0, size.width(), size.height());
    style.drawShape(&painter, targetRect.center());

    return QIcon(pixmap);
}

//! Helper function to swap limits, if necessary
void updateLimits(Edit1d* pMinEdit, Edit1d* pMaxEdit)
{
    QSignalBlocker blockerMin(pMinEdit);
    QSignalBlocker blockerMax(pMaxEdit);
    double minValue = pMinEdit->value();
    double maxValue = pMaxEdit->value();
    if (maxValue < minValue)
        std::swap(minValue, maxValue);
    pMinEdit->setValue(minValue);
    pMaxEdit->setValue(maxValue);
}
