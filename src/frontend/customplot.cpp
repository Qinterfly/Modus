#include <QSizePolicy>
#include <set>

#include "customplot.h"
#include "uiconstants.h"
#include "uiutility.h"
#include "customtable.h"

using namespace Frontend;

static const char* skLineStyle = "lineStyle";
static const char* skScatterStyle = "scatterStyle";

QPair<QList<double>, QList<double>> getPlottableData(QCPAbstractPlottable* pPlottable);

CustomPlot::CustomPlot(QWidget* pParent)
    : QCustomPlot(pParent)
    , mIsDragLegend(false)
{
    initializePlot();
    createActions();
    specifyConnections();
}

CustomPlot::~CustomPlot()
{
    CustomPlot::clear();
}

void CustomPlot::clear()
{
    QString nullString;
    clearItems();
    mDataTips.clear();
    clearPlottables();
    legend->setVisible(false);
    setTitle(nullString);
    xAxis->setLabel(nullString);
    yAxis->setLabel(nullString);
    replot();
}

void CustomPlot::refresh()
{
    replot(QCustomPlot::rpRefreshHint);
}

void CustomPlot::fit()
{
    rescaleAxes();
    replot();
}

QString CustomPlot::title() const
{
    return mpTitle->text();
}

void CustomPlot::setTitle(QString const& title)
{
    mpTitle->setText(title);
}

//! Retrieve the next color which is available for plotting
QColor CustomPlot::getAvailableColor() const
{
    uint numPlottables = plottableCount();
    uint numColors = Constants::Colors::skStandardColors.size();
    uint iColor = numPlottables;
    while (iColor >= numColors)
        iColor -= numColors;
    if (iColor < 0)
        iColor = 0;
    return Constants::Colors::skStandardColors[iColor];
}

//! Set the plot configuration
void CustomPlot::initializePlot()
{
    const QColor kTranslucentWhite(255, 255, 255, 150);
    const uint kNumTicks = 10;

    // Specify user interactions
    setAcceptDrops(true);
    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables | QCP::iMultiSelect);
    setContextMenuPolicy(Qt::CustomContextMenu);

    // Set rendering options
    setAntialiasedElement(QCP::AntialiasedElement::aeAll, true);

    // Set ticker strategy
    xAxis->ticker()->setTickStepStrategy(QCPAxisTicker::tssReadability);
    yAxis->ticker()->setTickStepStrategy(QCPAxisTicker::tssReadability);

    // Set the number of ticks
    xAxis->ticker()->setTickCount(kNumTicks);
    yAxis->ticker()->setTickCount(kNumTicks);

    // Configure the legend
    legend->setSelectableParts(QCPLegend::spItems);
    legend->setBrush(QBrush(kTranslucentWhite));
    axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight | Qt::AlignTop);
    axisRect()->insetLayout()->setInsetPlacement(0, QCPLayoutInset::ipBorderAligned);

    // Create the title
    plotLayout()->insertRow(0);
    QFont font = qApp->font();
    font.setBold(true);
    mpTitle = new QCPTextElement(this, QString(), font);
    plotLayout()->addElement(0, 0, mpTitle);
}

void CustomPlot::createActions()
{
    // Copy action
    QAction* pCopyAction = new QAction(tr("&Copy image"), this);
    pCopyAction->setShortcut(Qt::Modifier::CTRL | Qt::ALT | Qt::Key_C);
    connect(pCopyAction, &QAction::triggered, this, &CustomPlot::copyPixmapToClipboard);

    // Save action
    QAction* pSaveAction = new QAction(tr("&Save image"), this);
    pSaveAction->setShortcut(Qt::Modifier::CTRL | Qt::ALT | Qt::Key_S);
    connect(pSaveAction, &QAction::triggered, this, &CustomPlot::savePixmap);

    // View action
    QAction* pViewAction = new QAction(tr("&View data"), this);
    pViewAction->setShortcut(Qt::Modifier::CTRL | Qt::ALT | Qt::Key_V);
    connect(pViewAction, &QAction::triggered, this, &CustomPlot::viewPlotData);

    // Add the actions to the widget
    addActions({pCopyAction, pSaveAction, pViewAction});
}

//! Specify how to handle signals
void CustomPlot::specifyConnections()
{
    // Rename text elements
    connect(this, &CustomPlot::axisDoubleClick, this, &CustomPlot::renameAxis);
    connect(this, &CustomPlot::legendDoubleClick, this, &CustomPlot::renamePlottable);
    connect(mpTitle, &QCPTextElement::doubleClicked, this, &CustomPlot::renameTitle);

    // Drag and zoom axes
    connect(this, &CustomPlot::mousePress, this, &CustomPlot::processMousePress);
    connect(this, &CustomPlot::mouseWheel, this, &CustomPlot::processMouseWheel);
    connect(this, &CustomPlot::mouseMove, this, &CustomPlot::processMouseMove);
    connect(this, &CustomPlot::mouseRelease, this, &CustomPlot::processMouseRelease);
    connect(this, &CustomPlot::beforeReplot, this, &CustomPlot::processBeforeReplot);
    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), xAxis2, SLOT(setRange(QCPRange)));
    connect(yAxis, SIGNAL(rangeChanged(QCPRange)), yAxis2, SLOT(setRange(QCPRange)));

    // Interact with the legend
    connect(this, &CustomPlot::customContextMenuRequested, this, &CustomPlot::showContextMenu);
    connect(this, &CustomPlot::selectionChangedByUser, this, &CustomPlot::processSelection);
}

void CustomPlot::keyPressEvent(QKeyEvent* pEvent)
{
    int iKey = pEvent->key();
    if (iKey == Qt::Key_Escape)
    {
        // Drop selected plottables
        QList<QCPAbstractLegendItem*> selectedItems = legend->selectedItems();
        int numItems = selectedItems.size();
        for (int i = 0; i != numItems; ++i)
        {
            QCPAbstractLegendItem* pItem = selectedItems[i];
            if (pItem)
                pItem->setSelected(false);
        }

        // Remove data tips
        removeAllDataTips();
    }
}

//! Retrieve selected plottables
QList<QCPAbstractPlottable*> CustomPlot::selectedPlottables()
{
    std::set<uint> indices;

    // Collect indices of selected plottables from the legend
    uint numItems = legend->itemCount();
    for (uint i = 0; i != numItems; ++i)
    {
        QCPAbstractLegendItem* pItem = legend->item(i);
        if (pItem && pItem->selected())
            indices.insert(i);
    }

    // Collect indices of selected plottables from the plot
    uint numPlottables = plottableCount();
    for (uint i = 0; i != numPlottables; ++i)
    {
        QCPAbstractPlottable* pPlottable = plottable(i);
        if (pPlottable && pPlottable->selected())
            indices.insert(i);
    }

    // Slice plottables by indices
    QList<QCPAbstractPlottable*> result;
    for (auto index : indices)
        result.push_back(plottable(index));

    return result;
}

void CustomPlot::removeSelectedPlottables()
{
    QList<QCPAbstractPlottable*> plottables = selectedPlottables();
    int numPlottables = plottables.size();
    for (int i = 0; i != numPlottables; ++i)
    {
        QCPAbstractPlottable* pItem = plottables[i];
        removePlottable(pItem);
    }

    if (legend->itemCount() == 0)
        clear();
    else
        replot();
}

void CustomPlot::removeAllDataTips()
{
    int numDataTips = mDataTips.size();
    for (int i = 0; i != numDataTips; ++i)
    {
        DataTip* pDataTip = mDataTips[i];
        removeItem(pDataTip);
    }
    mDataTips.clear();
    replot();
}

void CustomPlot::removeDataTip(uint index)
{
    removeItem(mDataTips[index]);
    mDataTips.removeAt(index);
    replot();
}

void CustomPlot::changeSelectedPlottable()
{
    QList<QCPAbstractPlottable*> plottables = selectedPlottables();
    QDialog* pDialog = new QDialog(this);
    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->addWidget(new PlottablePropertyEditor(plottables.first()));
    pDialog->setWindowTitle(tr("Plottable property editor"));
    pDialog->setLayout(pLayout);
    pDialog->exec();
    replot();
}

//! Rename an axis by double clicking on its label
void CustomPlot::renameAxis(QCPAxis* pAxis, QCPAxis::SelectablePart part)
{
    if (part == QCPAxis::spAxisLabel)
    {
        bool isOk;
        QString newLabel = QInputDialog::getText(this, tr("Change label dialog"), tr("New axis label:"), QLineEdit::Normal, pAxis->label(), &isOk);
        if (isOk)
        {
            pAxis->setLabel(newLabel);
            replot();
        }
    }
}

//! Rename a plottable by double clicking on its legend item
void CustomPlot::renamePlottable(QCPLegend* pLegend, QCPAbstractLegendItem* pItem)
{
    Q_UNUSED(pLegend);
    if (pItem)
    {
        QCPPlottableLegendItem* pLegendItem = qobject_cast<QCPPlottableLegendItem*>(pItem);
        bool isOk;
        QString newName = QInputDialog::getText(this, tr("Rename plottable dialog"), tr("New plottable name:"), QLineEdit::Normal, pLegendItem->plottable()->name(), &isOk);
        if (isOk)
        {
            pLegendItem->plottable()->setName(newName);
            replot();
        }
    }
}

//! Rename a title by double clicking on it
void CustomPlot::renameTitle(QMouseEvent* pEvent)
{
    Q_UNUSED(pEvent)
    if (QCPTextElement* pTitle = qobject_cast<QCPTextElement*>(sender()))
    {
        // Set the plot title by double clicking on it
        bool isOk;
        QString newTitle = QInputDialog::getText(this, tr("Change title dialog"), tr("New title:"), QLineEdit::Normal, pTitle->text(), &isOk);
        if (isOk)
        {
            pTitle->setText(newTitle);
            replot();
        }
    }
}

//! Process selection of axes and and plottables
void CustomPlot::processSelection()
{
    // Make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object
    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis) || xAxis->selectedParts().testFlag(QCPAxis::spTickLabels)
            || xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        xAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
        xAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
    }

    // Make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object
    if (yAxis->selectedParts().testFlag(QCPAxis::spAxis) || yAxis->selectedParts().testFlag(QCPAxis::spTickLabels)
            || yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        yAxis2->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
        yAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
    }

    // Synchronize selection of plottables with selection of corresponding legend items
    uint numPlottables = plottableCount();
    for (int i = 0; i != numPlottables; ++i)
    {
        QCPAbstractPlottable* pPlottable = plottable(i);

        // Select plottables in the legend
        QCPPlottableLegendItem* pItem = legend->itemWithPlottable(pPlottable);
        if (pItem && (pItem->selected() || pPlottable->selected()))
            pItem->setSelected(true);

        // Select the plottable points
        if (pPlottable->selected())
        {
            QCPDataSelection selection = pPlottable->selection();
            QCPDataRange range = selection.dataRange();
            uint iPoint = range.end() - 1;
            if (QCPCurve* pCurve = qobject_cast<QCPCurve*>(pPlottable))
                showDataTip(pCurve->dataMainKey(iPoint), pCurve->dataMainValue(iPoint));
            else if (QCPGraph* pGraph = qobject_cast<QCPGraph*>(pPlottable))
                showDataTip(pGraph->dataMainKey(iPoint), pGraph->dataMainValue(iPoint));
        }
    }
}

void CustomPlot::processMousePress(QMouseEvent* pEvent)
{
    // If an axis is selected, only allow the direction of that axis to be dragged
    // If no axis is selected, both directions may be dragged
    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        axisRect()->setRangeDrag(xAxis->orientation());
    else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        axisRect()->setRangeDrag(yAxis->orientation());
    else
        axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);

    // Process the start of dragging
    if (pEvent->button() & Qt::LeftButton)
    {
        QPointF position = pEvent->position();

        // Check if the legend was clicked
        if (legend->selectTest(position, false) > 0)
        {
            mIsDragLegend = true;
            setInteraction(QCP::iRangeDrag, false);
            mDragLegendOrigin = mapToLocal(position) - axisRect()->insetLayout()->insetRect(0).topLeft();
        }

        // Check if the data tip was clicked
        int numDataTips = mDataTips.size();
        for (int i = 0; i != numDataTips; ++i)
        {
            DataTip* pDataTip = mDataTips[i];
            if (pDataTip->isSelect(position, selectionTolerance()))
            {
                setInteraction(QCP::iRangeDrag, false);
                QPointF origin = mapToLocal(position);
                pDataTip->startDrag(origin);
            }
        }
    }
}

void CustomPlot::processMouseWheel(QWheelEvent* pEvent)
{
    // If an axis is selected, only allow the direction of that axis to be zoomed
    // If no axis is selected, both directions may be zoomed
    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        axisRect()->setRangeZoom(xAxis->orientation());
    else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        axisRect()->setRangeZoom(yAxis->orientation());
    else
        axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
}

void CustomPlot::processMouseMove(QMouseEvent* pEvent)
{
    QPointF position = pEvent->position();

    bool isReplot = false;

    // Process dragging legend
    if (mIsDragLegend)
    {
        QRectF rect = axisRect()->insetLayout()->insetRect(0);
        rect.moveTopLeft(mapToLocal(position) - mDragLegendOrigin);
        axisRect()->insetLayout()->setInsetRect(0, rect);
        isReplot = true;
    }

    // Process dragging data tips
    int numDataTips = mDataTips.size();
    for (int i = 0; i != numDataTips; ++i)
    {
        DataTip* pDataTip = mDataTips[i];
        if (pDataTip->isDrag())
        {
            pDataTip->processDrag(mapToLocal(position));
            isReplot = true;
        }
    }

    // Update the plot
    if (isReplot)
        replot();
}

void CustomPlot::processMouseRelease(QMouseEvent* pEvent)
{
    Q_UNUSED(pEvent);

    // Finish legend dragging
    if (mIsDragLegend)
    {
        mIsDragLegend = false;
        setInteraction(QCP::iRangeDrag, true);
    }

    // Finish data tips dragging
    int numDataTips = mDataTips.size();
    for (int i = 0; i != numDataTips; ++i)
    {
        DataTip* pDataTip = mDataTips[i];
        if (pDataTip->isDrag())
        {
            pDataTip->finishDrag();
            setInteraction(QCP::iRangeDrag, true);
        }
    }
}

void CustomPlot::processBeforeReplot()
{
    legend->setMaximumSize(legend->minimumOuterSizeHint());
}

void CustomPlot::showContextMenu(QPoint position)
{
    QMenu* pMenu = new QMenu(this);
    pMenu->setAttribute(Qt::WA_DeleteOnClose);

    // Context menu on data tips requested
    uint numDataTips = mDataTips.size();
    for (uint i = 0; i != numDataTips; ++i)
    {
        if (mDataTips[i]->isSelect(position, selectionTolerance()))
        {
            pMenu->addAction(tr("Remove current data tip"), this, [this, i]() { removeDataTip(i); });
            pMenu->addAction(tr("Remove all data tips"), this, &CustomPlot::removeAllDataTips);
            pMenu->popup(mapToGlobal(position));
            return;
        }
    }

    // Context menu on plottables requested
    if (selectedPlottables().size() > 0)
    {
        pMenu->addAction(tr("Remove selected plottables"), this, &CustomPlot::removeSelectedPlottables);
        pMenu->addAction(tr("Change selected plottable"), this, &CustomPlot::changeSelectedPlottable);
    }
    pMenu->addAction(tr("Fit"), this, &CustomPlot::fit);
    if (plottableCount() > 0)
        pMenu->addAction(tr("Clear all"), this, &CustomPlot::clear);
    pMenu->addSeparator();
    pMenu->addActions(actions());
    pMenu->popup(mapToGlobal(position));
}

//! Represent a data tip at the specified location
void CustomPlot::showDataTip(double xData, double yData)
{
    mDataTips.emplaceBack(new DataTip(xData, yData, this));
}

//! Copy an image of plottables to the clipboard
void CustomPlot::copyPixmapToClipboard()
{
    QApplication::clipboard()->setPixmap(toPixmap());
}

//! Save an image of plottables to a file
void CustomPlot::savePixmap()
{
    QString const kSuffix = ".png";

    QString defaultPathFile = QDir::currentPath() + QDir::separator() + title() + kSuffix;
    QString pathFile = QFileDialog::getSaveFileName(this, tr("Save Plot"), defaultPathFile,
                       tr("Image format (*%1)").arg(kSuffix));
    if (pathFile.isEmpty())
        return;
    if (QFileInfo(pathFile).suffix().isEmpty())
        pathFile += kSuffix;
    toPixmap().save(pathFile);
}

//! View data of all plottables
void CustomPlot::viewPlotData()
{
    const QSize kDefaultSize(1024, 800);
    const uint kNumHeader = 2;
    const uint kNumSubColumns = 2;
    auto const kFormatter = [](double value) { return QString::number(value, 'g', 6); };

    uint numPlottables = plottableCount();
    int maxNumData = 0;

    // Determine the maximum length of the plottable
    for (uint i = 0; i != numPlottables; ++i)
    {
        int numData = 0;
        QCPAbstractPlottable* pPlottable = plottable(i);
        if (QCPGraph* pGraph = qobject_cast<QCPGraph*>(pPlottable))
            numData = pGraph->dataCount();
        else if (QCPCurve* pCurve = qobject_cast<QCPCurve*>(pPlottable))
            numData = pCurve->dataCount();
        maxNumData = std::max(maxNumData, numData);
    }

    // Create the table to view
    CustomTable* pTable = new CustomTable(this);
    pTable->setWindowTitle(tr("Plot data"));
    pTable->setWindowFlag(Qt::Window, true);
    pTable->setAttribute(Qt::WA_DeleteOnClose, true);
    pTable->horizontalHeader()->hide();
    pTable->verticalHeader()->hide();
    pTable->resize(kDefaultSize);

    // Change the number of rows and columns
    pTable->setRowCount(kNumHeader + maxNumData);
    pTable->setColumnCount(kNumSubColumns * numPlottables);

    // Add all the plottables
    for (uint iPlottable = 0; iPlottable != numPlottables; ++iPlottable)
    {
        // Retrieve the plottable data
        QCPAbstractPlottable* pPlottable = plottable(iPlottable);
        auto data = getPlottableData(pPlottable);
        QList<double> const& xData = data.first;
        QList<double> const& yData = data.second;

        // Assign the header
        uint iInsert = iPlottable * kNumSubColumns;
        pTable->setItem(0, iInsert, new QTableWidgetItem(pPlottable->name()));
        pTable->setSpan(0, iInsert, 1, kNumSubColumns);
        pTable->setItem(1, iInsert, new QTableWidgetItem("X"));
        pTable->setItem(1, iInsert + 1, new QTableWidgetItem("Y"));

        // Assign the data
        uint numData = xData.size();
        QString xText, yText;
        for (uint iData = 0; iData != numData; ++iData)
        {
            xText = kFormatter(xData[iData]);
            yText = kFormatter(yData[iData]);
            pTable->setItem(kNumHeader + iData, iInsert, new QTableWidgetItem(xText));
            pTable->setItem(kNumHeader + iData, iInsert + 1, new QTableWidgetItem(yText));
        }
    }

    // Resize the table
    pTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    // Align the data
    pTable->setDataAlignment(Qt::AlignCenter);

    // Show the table
    pTable->show();
}

//! Convert a global point position to a local one
QPointF CustomPlot::mapToLocal(QPointF const& position) const
{
    return QPointF((position.x() - axisRect()->left()) / (double)axisRect()->width(),
                   (position.y() - axisRect()->top()) / (double)axisRect()->height());
}

QString format(double value)
{
    return QString::number(value, 'g', 3);
}

DataTip::DataTip(double xData, double yData, CustomPlot* pPlot)
    : QCPItemText(pPlot)
    , mXData(xData)
    , mYData(yData)
    , mIsDrag(false)
    , mpPlot(pPlot)
{
    // Constants
    QColor const kBrushColor = QColor(255, 255, 255, 200);
    QColor const kPenColor = Qt::black;
    QColor const kMarkerColor = QColor(0, 115, 200);
    uint const kMarkerSize = 8;
    uint const kFontSize = 14;
    QMargins const kPadding(4, 0, 4, 0);

    // Set position
    position->setType(QCPItemPosition::ptPlotCoords);
    position->setCoords(xData, yData);
    setPositionAlignment(Qt::AlignLeft | Qt::AlignBottom);

    // Set style
    setPadding(kPadding);
    setBrush(QBrush(kBrushColor));
    QPen pen(kPenColor);
    pen.setJoinStyle(Qt::RoundJoin);
    setPen(pen);

    // Set text
    setTextAlignment(Qt::AlignLeft);
    setFont(QFont(font().family(), kFontSize));
    QString text = QString("X: %1\nY: %2").arg(format(xData), format(yData));
    setText(text);

    // Add the tracer point
    mpCurve = new QCPCurve(mpPlot->xAxis, mpPlot->yAxis);
    mpCurve->addData(xData, yData);
    mpCurve->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, kMarkerColor, kMarkerSize));
    mpCurve->removeFromLegend();
}

DataTip::~DataTip()
{
    mpPlot->removePlottable(mpCurve);
}

bool DataTip::isDrag() const
{
    return mIsDrag;
}

bool DataTip::isSelect(QPointF const& position, int tolerance)
{
    double distance = selectTest(position, false);
    return distance > 0 && distance <= 0.99 * tolerance;
}

void DataTip::startDrag(QPointF const& origin)
{
    mDragOrigin = origin;
    mIsDrag = true;
}

void DataTip::processDrag(QPointF const& point)
{
    if (!mIsDrag)
        return;
    float shiftX = point.x() - mDragOrigin.x();
    float shiftY = point.y() - mDragOrigin.y();
    Qt::AlignmentFlag horizontalFlag = positionAlignment().testFlag(Qt::AlignLeft) ? Qt::AlignLeft : Qt::AlignRight;
    Qt::AlignmentFlag verticalFlag = positionAlignment().testFlag(Qt::AlignTop) ? Qt::AlignTop : Qt::AlignBottom;
    if (std::abs(shiftX) > std::abs(shiftY))
        horizontalFlag = shiftX > 0 ? Qt::AlignLeft : Qt::AlignRight;
    else
        verticalFlag = shiftY > 0 ? Qt::AlignTop : Qt::AlignBottom;
    setPositionAlignment(horizontalFlag | verticalFlag);
}

void DataTip::finishDrag()
{
    mIsDrag = false;
}

PlottablePropertyEditor::PlottablePropertyEditor(QCPAbstractPlottable* pPlottable, QWidget* pParent)
    : mpPlottable(pPlottable)
{
    createContent();
    createProperties();
    createConnections();
}

QSize PlottablePropertyEditor::sizeHint() const
{
    return QSize(400, 300);
}

//! Create all the widgets
void PlottablePropertyEditor::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;
    mpEditor = new CustomPropertyEditor;
    pLayout->addWidget(mpEditor);
    setLayout(pLayout);
}

void PlottablePropertyEditor::createProperties()
{
    QStringList kMarkerShapeNames = {QString(),
                                     tr("Dot"),
                                     tr("Cross"),
                                     tr("Plus"),
                                     tr("Circle"),
                                     tr("Disc"),
                                     tr("Square"),
                                     tr("Diamond"),
                                     tr("Star"),
                                     tr("Triangle"),
                                     tr("Inverted Triangle"),
                                     tr("Cross Square"),
                                     tr("Plus Square"),
                                     tr("Cross Circle"),
                                     tr("Plus Circle"),
                                     tr("Peace"),
                                     tr("Pixmap"),
                                     tr("Custom")};

    QStringList kLineStyleNames = {QString(), tr("Line"), tr("Left step"), tr("Right step"), tr("Center step"), tr("Impulse")};

    // Slice the plottable data
    int lineStyle = mpPlottable->property(skLineStyle).toInt();
    QCPScatterStyle scatterStyle = mpPlottable->property(skScatterStyle).value<QCPScatterStyle>();

    // Create the properties
    mpEditor->createEnumProperty(kLineStyle, tr("Style"), lineStyle, kLineStyleNames);
    mpEditor->createColorProperty(kLineColor, tr("Line color"), mpPlottable->pen().color());
    mpEditor->createEnumProperty(kMarkerShape, tr("Marker shape"), scatterStyle.shape(), kMarkerShapeNames);
    mpEditor->createIntProperty(kMarkerSize, tr("Marker size"), scatterStyle.size());
    mpEditor->createColorProperty(kLineColor, tr("Marker color"), scatterStyle.pen().color());
}

//! Specify connections
void PlottablePropertyEditor::createConnections()
{
    connect(mpEditor, &CustomPropertyEditor::intValueChanged, this, &PlottablePropertyEditor::setIntValue);
    connect(mpEditor, &CustomPropertyEditor::doubleValueChanged, this, &PlottablePropertyEditor::setDoubleValue);
    connect(mpEditor, &CustomPropertyEditor::colorValueChanged, this, &PlottablePropertyEditor::setColorValue);
}

//! Process changing of an integer value
void PlottablePropertyEditor::setIntValue(QtProperty* pProperty, int value)
{
    if (!mpEditor->contains(pProperty))
        return;

    QCPScatterStyle scatterStyle = mpPlottable->property(skScatterStyle).value<QCPScatterStyle>();
    int id = mpEditor->id(pProperty);
    switch (id)
    {
    case kLineStyle:
        mpPlottable->setProperty(skLineStyle, value);
        break;
    case kMarkerShape:
        scatterStyle.setShape((QCPScatterStyle::ScatterShape) value);
        break;
    }
    mpPlottable->setProperty(skScatterStyle, QVariant::fromValue(scatterStyle));
}

//! Process changing of a color
void PlottablePropertyEditor::setColorValue(QtProperty* pProperty, QColor const& value)
{
    if (!mpEditor->contains(pProperty))
        return;

    QCPScatterStyle scatterStyle = mpPlottable->property(skScatterStyle).value<QCPScatterStyle>();
    int id = mpEditor->id(pProperty);
    switch (id)
    {
    case kLineColor:
        mpPlottable->setPen(QPen(value));
        break;
    case kMarkerColor:
        scatterStyle.setPen(QPen(value));
        break;
    }
    mpPlottable->setProperty(skScatterStyle, QVariant::fromValue(scatterStyle));
}

//! Process changing of a double value
void PlottablePropertyEditor::setDoubleValue(QtProperty* pProperty, double value)
{
    if (!mpEditor->contains(pProperty))
        return;

    QCPScatterStyle scatterStyle = mpPlottable->property(skScatterStyle).value<QCPScatterStyle>();
    int id = mpEditor->id(pProperty);
    if (id == kMarkerSize)
        scatterStyle.setSize(value);
    mpPlottable->setProperty(skScatterStyle, QVariant::fromValue(scatterStyle));
}

//! Helper function to retrieve plottable data
QPair<QList<double>, QList<double>> getPlottableData(QCPAbstractPlottable* pPlottable)
{
    uint numData = 0;
    uint iData = 0;
    QList<double> xData, yData;
    if (QCPGraph* pGraph = qobject_cast<QCPGraph*>(pPlottable))
    {
        numData = pGraph->dataCount();
        xData.resize(numData);
        yData.resize(numData);
        QCPGraphDataContainer* pData = pGraph->data().data();
        for (auto iter = pData->begin(); iter != pData->end(); ++iter, ++iData)
        {
            xData[iData] = iter->key;
            yData[iData] = iter->value;
        }
    }
    else if (QCPCurve* pCurve = qobject_cast<QCPCurve*>(pPlottable))
    {
        numData = pCurve->dataCount();
        xData.resize(numData);
        yData.resize(numData);
        QCPCurveDataContainer* pData = pCurve->data().data();
        for (auto iter = pData->begin(); iter != pData->end(); ++iter, ++iData)
        {
            xData[iData] = iter->key;
            yData[iData] = iter->value;
        }
    }
    return QPair<QList<double>, QList<double>>(xData, yData);
}
