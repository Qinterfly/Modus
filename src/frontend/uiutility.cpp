#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QScreen>
#include <QSettings>
#include <QToolBar>
#include <QWidget>

#include <kcl/model.h>

#include <Eigen/Geometry>
#include <magicenum/magic_enum.hpp>
#include <vtkColor.h>
#include <vtkColorTransferFunction.h>
#include <vtkCylinderSource.h>
#include <vtkDataSetMapper.h>
#include <vtkGlyph3DMapper.h>
#include <vtkHexahedron.h>
#include <vtkLookupTable.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkUnstructuredGrid.h>

#include "isolver.h"
#include "lineedit.h"
#include "selectionset.h"
#include "uiconstants.h"
#include "uiutility.h"

using namespace Eigen;
using namespace Backend;
using namespace Frontend;

namespace Frontend::Utility
{

void setEdits(DoubleLineEdit** edits, Eigen::Vector3d const& values, VectorXi const& indices = Vector3i(0, 1, 2));

//! Retrieve the active text color from the palette
QColor textColor(const QPalette& palette)
{
    return palette.color(QPalette::Active, QPalette::Text);
}

//! Change the text color of the widget
void setTextColor(QWidget* pWidget, const QColor& color)
{
    auto palette = pWidget->palette();
    if (textColor(palette) != color)
    {
        palette.setColor(QPalette::Active, QPalette::Text, color);
        pWidget->setPalette(palette);
    }
}

//! Convert a VTK color to Qt one
QColor getColor(vtkColor3d color)
{
    return QColor::fromRgbF(color[0], color[1], color[2]);
}

//! Convert a Qt color to Vtk one
vtkColor3d getColor(QColor color)
{
    return vtkColor3d(color.redF(), color.greenF(), color.blueF());
}

//! Show save dialog when closing a widget and process its output
int showSaveDialog(QWidget* pWidget, QString const& title, QString const& message)
{
    QMessageBox* pMessageBox = new QMessageBox(QMessageBox::Question, title, message,
                                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    pMessageBox->setFont(pWidget->font());
    auto result = pMessageBox->exec();
    switch (result)
    {
    case QMessageBox::Save:
        return 1;
    case QMessageBox::Discard:
        return 0;
    default:
        return -1;
    }
}

//! Resize widget to maximize its width and height
void fullScreenResize(QWidget* pWidget)
{
    QRect screenGeometry = QApplication::QGuiApplication::primaryScreen()->availableGeometry();
    pWidget->resize(screenGeometry.width(), screenGeometry.height());
}

//! Get primary screen size
QSize getScreenSize()
{
    QRect screenGeometry = QApplication::QGuiApplication::primaryScreen()->availableGeometry();
    return screenGeometry.size();
}

//! Add shortcurt hints to all items contained in a tool bar
void setShortcutHints(QToolBar* pToolBar)
{
    QList<QAction*> actions = pToolBar->actions();
    int numActions = actions.size();
    for (int i = 0; i != numActions; ++i)
    {
        QAction* pAction = actions[i];
        QKeySequence shortcut = pAction->shortcut();
        if (shortcut.isEmpty())
            continue;
        pAction->setToolTip(QString("%1 (%2)").arg(pAction->toolTip(), shortcut.toString()));
    }
}

//! Get color name associated with error value
QString errorColorName(double value, double acceptThreshold, double criticalThreshold)
{
    QString result = "yellow";
    if (qAbs(value) < acceptThreshold)
        result = "green";
    else if (qAbs(value) > criticalThreshold)
        result = "red";
    return result;
}

//! Retrieve a selection label to display
QString getLabel(Core::Selection selection)
{
    // The first element is informational, so we do not enumerate it
    if (selection.type == KCL::AE)
        --selection.iElement;
    QString typeName = magic_enum::enum_name(selection.type).data();
    QString result = QString("%1:%2 %3").arg(typeName).arg(selection.iElement + 1).arg(getLabel(selection.iSurface));
    return result;
}

//! Retrieve a surface label to display
QString getLabel(int iSurface)
{
    if (iSurface >= 0)
        return QString("%1:%2").arg("ES").arg(iSurface + 1);
    else
        return "ES51";
}

//! Search for hierarchy items of the specified type
QList<HierarchyItem*> findItems(HierarchyItem* pRootItem, HierarchyItem::Type type)
{
    QList<HierarchyItem*> result;
    if (!pRootItem->hasChildren())
        return result;
    int numChildren = pRootItem->rowCount();
    for (int i = 0; i != numChildren; ++i)
    {
        HierarchyItem* pItem = (HierarchyItem*) pRootItem->child(i);
        if (pItem->type() == type)
            result.push_back(pItem);
    }
    return result;
}

//! Substitute a file suffix to the expected one, if necessary
void modifyFileSuffix(QString& pathFile, QString const& expectedSuffix)
{
    QFileInfo info(pathFile);
    QString currentSuffix = info.suffix();
    if (currentSuffix.isEmpty())
        pathFile.append(QString(".%1").arg(expectedSuffix));
    else if (currentSuffix != expectedSuffix)
        pathFile.replace(currentSuffix, expectedSuffix);
}

//! Retrieve last used directory
QDir getLastDirectory(QSettings const& settings)
{
    return QFileInfo(getLastPathFile(settings)).dir();
}

//! Retrieve last used path file
QString getLastPathFile(QSettings const& settings)
{
    return settings.value(Constants::Settings::skLastPathFile, QString()).toString();
}

//! Set last used path file
void setLastPathFile(QSettings& settings, QString const& pathFile)
{
    settings.setValue(Constants::Settings::skLastPathFile, pathFile);
}

//! Cast the container consisted of pointers to hierarchy items
template<typename Item>
QList<Item*> castHierarchyItems(QList<HierarchyItem*> const& items)
{
    QList<Item*> result;
    result.reserve(items.size());
    for (auto ptr : items)
        result.emplace_back(reinterpret_cast<Item*>(ptr));
    return result;
}

//! Retrieve children of a hierarchy item
QList<HierarchyItem*> childItems(HierarchyItem* pItem)
{
    int numItems = pItem->rowCount();
    QList<HierarchyItem*> result(numItems);
    for (int k = 0; k != numItems; ++k)
        result[k] = (HierarchyItem*) pItem->child(k);
    return result;
}

//! Check if the items have the same type
bool isSameType(QList<HierarchyItem*> const& items)
{
    if (items.isEmpty())
        return false;
    auto type = (HierarchyItem::Type) items.first()->type();
    return isSameType(items, type);
}

//! Check if the have the same specified type
bool isSameType(QList<HierarchyItem*> const& items, HierarchyItem::Type type)
{
    if (items.isEmpty())
        return false;
    uint numItems = items.size();
    for (uint i = 1; i != numItems; ++i)
    {
        bool isEqual = items[i]->type() == type;
        if (!isEqual)
            return false;
    }
    return true;
}

//! Find parent of the specified type
HierarchyItem* findParentByType(HierarchyItem* pItem, HierarchyItem::Type type)
{
    QStandardItem* pParent = pItem->parent();
    while (pParent)
    {
        if (pParent->type() == type)
            return (HierarchyItem*) pParent;
        pParent = pParent->parent();
    }
    return nullptr;
}

//! Retrieve the KCL types which can be rendered
QList<KCL::ElementType> drawableTypes()
{
    QList<KCL::ElementType> result;
    result.append(beamTypes());
    result.append(panelTypes());
    result.append(aeroTrapeziumTypes());
    result.append(massTypes());
    result.append(springTypes());
    return result;
}

//! Retrieve the KCL types which are associated with beam elements
QList<KCL::ElementType> beamTypes()
{
    return {KCL::BI, KCL::BK, KCL::DB, KCL::ST, KCL::BP};
}

//! Retrieve the KCL types which are associated with panel elements
QList<KCL::ElementType> panelTypes()
{
    return {KCL::PN, KCL::OP, KCL::P4};
}

//! Retrieve the KCL types which are associated with aerodynamic trapezium elements
QList<KCL::ElementType> aeroTrapeziumTypes()
{
    return {KCL::AE, KCL::DA, KCL::DE, KCL::GS};
}

//! Retrieve the KCL types which are associated with mass elements
QList<KCL::ElementType> massTypes()
{
    return {KCL::M3, KCL::SM};
}

//! Retrieve the KCL types which are associated with spring elements
QList<KCL::ElementType> springTypes()
{
    return {KCL::PR};
}

//! Retrieve the KCL types which are associated with polynoms
QList<KCL::ElementType> polyTypes()
{
    return {KCL::PK, KCL::QK, KCL::DQ};
}

//! Check if an aerodynamic trapezium is perpendiculart to an elastic surface
bool isAeroVertical(KCL::ElementType type)
{
    return type == KCL::ElementType::DA;
}

//! Check if an aerodynamic trapezium is an aileron
bool isAeroAileron(KCL::ElementType type)
{
    return type == KCL::DE;
}

//! Check if an aerodynamic trapezium is meshable
bool isAeroMeshable(KCL::ElementType type)
{
    return type == KCL::AE || type == KCL::DA;
}

//! Build up the transformation for the elastic surface
Transformation computeTransformation(KCL::ElasticSurface const& surface, bool isAero)
{
    Transformation result = Transformation::Identity();
    if (!surface.containsElement(KCL::OD))
        return result;
    auto pData = (KCL::GeneralData const*) surface.element(KCL::OD);
    if (isAero)
        result = computeTransformation(pData->coords, pData->dihedralAngle, 0.0, pData->zAngle);
    else
        result = computeTransformation(pData->coords, pData->dihedralAngle, pData->sweepAngle, pData->zAngle);
    return result;
}

//! Build up the transformation for the elastic surface using its local coordinate
Transformation computeTransformation(KCL::Vec3 const& coords, double dihedralAngle, double sweepAngle, double zAngle)
{
    Transformation result = Transformation::Identity();
    result.translate(Vector3d(coords[0], coords[1], coords[2]));
    result.rotate(AngleAxisd(-qDegreesToRadians(dihedralAngle), Vector3d::UnitX()));
    result.rotate(AngleAxisd(qDegreesToRadians(sweepAngle), Vector3d::UnitY()));
    result.rotate(AngleAxisd(qDegreesToRadians(zAngle), Vector3d::UnitZ()));
    return result;
}

//! Reflect the current transformation about the XOY plane
Transformation reflectTransformation(Transformation const& transform)
{
    Matrix4d matrix = Matrix4d::Identity();
    matrix(2, 2) = -1.0;
    return Transformation(matrix * transform.matrix());
}

//! Distribute model properties
void setupModel(KCL::Model& model)
{
    int numSurfaces = model.surfaces.size();

    // Copy data to the first aerodynamic trapezium
    for (int i = 0; i != numSurfaces; ++i)
    {
        KCL::ElasticSurface& surface = model.surfaces[i];

        // Obtain elements
        if (!surface.containsElement(KCL::OD) || !surface.containsElement(KCL::CO) || !surface.containsElement(KCL::AE))
            continue;
        if (surface.element(KCL::AE)->subType() != KCL::AE1)
            continue;
        KCL::GeneralData* pData = (KCL::GeneralData*) surface.element(KCL::OD);
        KCL::Constants* pConstants = (KCL::Constants*) surface.element(KCL::CO);
        KCL::AerodynamicTrapezium1* pTrapezium = (KCL::AerodynamicTrapezium1*) surface.element(KCL::AE);

        // Set the data
        pTrapezium->machNumber = pConstants->machNumber;
        pTrapezium->soundSpeed = pConstants->soundSpeed;
        pTrapezium->airDensity = pConstants->airDensity;
        pTrapezium->referenceLength = pConstants->referenceLength;
        pTrapezium->strouhalNumber = pConstants->strouhalNumber;
        pTrapezium->locationSymmetryAxis = 0.0;
        pTrapezium->iSymmetry = pData->iSymmetry;
        pTrapezium->sweepAngle = pData->sweepAngle;
    }
}

//! Read a model from a file
KCL::Model readModel(QString const& pathFile)
{
    KCL::Model model;
    try
    {
        model.read(pathFile.toStdString());
    }
    catch (...)
    {
        qWarning() << QObject::tr("Unexpected error occurred while reading the file: %1").arg(pathFile);
    }
    qInfo() << QObject::tr("Model was read from the file %1").arg(pathFile);
    return model;
}

//! Write a model to a file
bool writeModel(QString const& pathFile, KCL::Model const& model)
{
    try
    {
        model.write(pathFile.toStdString());
    }
    catch (...)
    {
        qWarning() << QObject::tr("Unexpected error occurred while writing the file: %1").arg(pathFile);
        return false;
    }
    qInfo() << QObject::tr("Model was written to the file %1").arg(pathFile);
    return true;
}

//! Check whether three points are located clockwise or counterclockwise
int orientation(Point const& p, Point const& q, Point const& r)
{
    int product = (q.x - p.x) * (r.y - p.y) - (r.x - p.x) * (q.y - p.y);
    if (product > 0)
        return 1; // Counterclockwise
    else if (product < 0)
        return -1; // Clockwise
    return 0;      // Collinear
}

//! Create a convex hull
QList<int> jarvisMarch(QList<Point> const& points)
{
    // Check if the number of points is enough
    int n = points.size();
    if (n < 3)
    {
        qWarning() << QObject::tr("There must be at least three points to build a convex hull");
        return {};
    }

    // Find the leftmost point
    int l = 0;
    for (int i = 1; i != n; ++i)
        if (points[i].y < points[l].y)
            l = i;

    // Search for points in clockwise orientation
    QList<int> order;
    int p = l;
    int q;
    do
    {
        order.push_back(p);
        q = (p + 1) % n;
        for (int i = 0; i < n; i++)
            if (orientation(points[p], points[i], points[q]) == -1)
                q = i;
        p = q;
    } while (p != l);

    return order;
}

//! Evalute the depth of the last point
void setLastDepth(Matrix42d const& coords, Vector4d& depths)
{
    int iLast = depths.size() - 1;

    // Build up the depth equation
    Matrix3d A;
    Vector3d b;
    for (int i = 0; i != iLast; ++i)
    {
        A(i, 0) = coords(i, 0);
        A(i, 1) = coords(i, 1);
        A(i, 2) = 1.0;
        b(i) = depths[i];
    }

    // Solve the linear system to find depth coefficients
    ColPivHouseholderQR<Matrix3d> dec(A);
    Vector3d c = dec.solve(b);

    // Set the last depth
    depths[iLast] = coords(iLast, 0) * c[0] + coords(iLast, 1) * c[1] + c[2];
}

//! Construct a helix of the given radius between two points
vtkSmartPointer<vtkActor> createHelixActor(Eigen::Vector3d const& startPosition, Eigen::Vector3d const& endPosition, double radius, int numTurns,
                                           int resolution)
{
    int kNumCellPoints = 2;
    double kRunoutFactor = 0.1;

    // Compute the direction vector
    Vector3d direction = endPosition - startPosition;
    double length = direction.norm();
    direction.normalize();

    // Build up the transformation matrix
    auto transform = Eigen::Transform<double, 3, Eigen::Affine>::Identity();
    double rotationAngle = acos(Vector3d::UnitZ().dot(direction));
    if (std::abs(rotationAngle) > std::numeric_limits<double>::epsilon())
    {
        auto rotationAxis = Vector3d::UnitZ().cross(direction);
        rotationAxis.normalize();
        transform.rotate(AngleAxisd(rotationAngle, rotationAxis));
    }

    // Compute the runout positions
    double multiplier = 0.5 * kRunoutFactor * length;
    Vector3d startRunoutPosition = startPosition + multiplier * direction;
    Vector3d endRunoutPosition = endPosition - multiplier * direction;

    // Construct the list of points
    vtkNew<vtkPoints> points;
    int numPoints = resolution * numTurns;
    double h = 2.0 * M_PI * numTurns / (numPoints - 1);
    double t = 0.0;
    points->InsertNextPoint(startPosition[0], startPosition[1], startPosition[2]);
    if (length > std::numeric_limits<double>::epsilon())
    {
        points->InsertNextPoint(startRunoutPosition[0], startRunoutPosition[1], startRunoutPosition[2]);
        for (int k = 0; k != numPoints; ++k)
        {
            double z = (1.0 - kRunoutFactor) * length * k / numPoints;
            Vector3d positon = startRunoutPosition + transform * Vector3d(radius * cos(t), radius * sin(t), z);
            points->InsertNextPoint(positon[0], positon[1], positon[2]);
            t += h;
        }
        points->InsertNextPoint(endRunoutPosition[0], endRunoutPosition[1], endRunoutPosition[2]);
    }
    points->InsertNextPoint(endPosition[0], endPosition[1], endPosition[2]);

    // Set the connectivity list
    vtkNew<vtkCellArray> indices;
    int numFullPoints = points->GetNumberOfPoints();
    for (int k = 0; k != numFullPoints - 1; ++k)
    {
        indices->InsertNextCell(kNumCellPoints);
        indices->InsertCellPoint(k);
        indices->InsertCellPoint(k + 1);
    }

    // Create the polygons
    vtkNew<vtkPolyData> data;
    data->SetPoints(points);
    data->SetLines(indices);

    // Map the geometrical data
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(data);

    // Create the actor
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);

    return actor;
}

//! Construct an actor for a given set of points
vtkSmartPointer<vtkActor> createPointsActor(QList<Eigen::Vector3d> const& positions, double radius)
{
    // Construct the source to be rendered at each location
    vtkNew<vtkSphereSource> sphereSource;
    sphereSource->SetRadius(radius);

    // Add the points
    vtkNew<vtkPoints> points;
    for (Vector3d const& position : positions)
        points->InsertNextPoint(position[0], position[1], position[2]);

    // Construct the polygons
    vtkNew<vtkPolyData> data;
    data->SetPoints(points);

    // Build up the mapper
    vtkNew<vtkGlyph3DMapper> mapper;
    mapper->SetInputData(data);
    mapper->SetSourceConnection(sphereSource->GetOutputPort());
    mapper->ScalarVisibilityOff();
    mapper->ScalingOff();

    // Create the actor
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);

    return actor;
}

//! Construct an oriented cylinder which connects two points
vtkSmartPointer<vtkActor> createCylinderActor(Eigen::Vector3d const& startPosition, Eigen::Vector3d const& endPosition, double radius,
                                              int resolution)
{
    // Constants
    Vector3d kBaseAxis = Vector3d::UnitY();

    // Compute the direction vector
    Vector3d direction = endPosition - startPosition;
    double length = direction.norm();
    direction.normalize();

    // Create the cylinder
    vtkNew<vtkCylinderSource> source;
    source->SetResolution(resolution);
    source->SetRadius(radius);
    source->SetHeight(length);
    source->SetCenter(0, 0.5 * length, 0);

    // Compute the transformation in order to align the cylinder
    vtkNew<vtkTransform> sourceTransform;
    double rotationAngle = acos(kBaseAxis.dot(direction));
    if (std::abs(rotationAngle) > std::numeric_limits<double>::epsilon())
    {
        Vector3d rotationAxis = kBaseAxis.cross(direction);
        rotationAxis.normalize();
        sourceTransform->Translate(startPosition[0], startPosition[1], startPosition[2]);
        sourceTransform->RotateWXYZ(qRadiansToDegrees(rotationAngle), rotationAxis.data());
    }

    // Set the transformation filter
    vtkNew<vtkTransformPolyDataFilter> filter;
    filter->SetTransform(sourceTransform);
    filter->SetInputConnection(source->GetOutputPort());

    // Create the mapper
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(filter->GetOutputPort());

    // Construct the actor
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);

    return actor;
}

//! Construct a shell actor using coordinates of middle surface, thickness and depths
vtkSmartPointer<vtkActor> createShellActor(Transformation const& transform, Matrix42d const& coords, Eigen::Vector4d const& depths,
                                           double thickness)
{
    int kNumVertices = 8;

    // Reorder the points
    int numCoords = coords.rows();
    QList<Point> planePoints(numCoords);
    for (int i = 0; i != numCoords; ++i)
        planePoints[i] = {coords(i, 0), coords(i, 1)};
    QList<int> order = jarvisMarch(planePoints);

    // Create the hexahedrons
    vtkNew<vtkPoints> points;
    vtkNew<vtkUnstructuredGrid> grid;
    if (thickness != 0.0)
    {
        // Add the points
        for (int i = 0; i != numCoords; ++i)
        {
            int iOrder = order[i];
            double x = coords(iOrder, 0);
            double z = coords(iOrder, 1);
            double d = depths[iOrder];
            double db = 0.5 * (d - thickness);
            double dt = 0.5 * (d + thickness);

            // Top surface
            Vector3d bottomPosition = transform * Vector3d(x, db, z);
            Vector3d topPosition = transform * Vector3d(x, dt, z);
            points->InsertPoint(i, bottomPosition[0], bottomPosition[1], bottomPosition[2]);
            points->InsertPoint(i + numCoords, topPosition[0], topPosition[1], topPosition[2]);

            // Bottom surface
            bottomPosition = transform * Vector3d(x, -db, z);
            topPosition = transform * Vector3d(x, -dt, z);
            points->InsertPoint(i + kNumVertices, bottomPosition[0], bottomPosition[1], bottomPosition[2]);
            points->InsertPoint(i + kNumVertices + numCoords, topPosition[0], topPosition[1], topPosition[2]);
        }

        // Create a hexahedron from the points
        vtkNew<vtkHexahedron> bottomHex;
        vtkNew<vtkHexahedron> topHex;
        for (int i = 0; i != kNumVertices; ++i)
        {
            bottomHex->GetPointIds()->SetId(i, i);
            topHex->GetPointIds()->SetId(i, i + kNumVertices);
        }

        // Add the points and hexahedron to an unstructured grid
        grid->SetPoints(points);
        grid->InsertNextCell(bottomHex->GetCellType(), bottomHex->GetPointIds());
        grid->InsertNextCell(topHex->GetCellType(), topHex->GetPointIds());
    }
    else
    {
        // Add the points
        for (int i = 0; i != numCoords; ++i)
        {
            int iOrder = order[i];
            double x = coords(iOrder, 0);
            double z = coords(iOrder, 1);
            double d = 0.5 * depths[iOrder];
            Vector3d bottomPosition = transform * Vector3d(x, -d, z);
            Vector3d topPosition = transform * Vector3d(x, d, z);
            points->InsertPoint(i, bottomPosition[0], bottomPosition[1], bottomPosition[2]);
            points->InsertPoint(i + numCoords, topPosition[0], topPosition[1], topPosition[2]);
        }

        // Create a hexahedron from the points
        vtkNew<vtkHexahedron> hex;
        for (int i = 0; i != kNumVertices; ++i)
            hex->GetPointIds()->SetId(i, i);

        // Add the points and hexahedron to an unstructured grid
        grid->SetPoints(points);
        grid->InsertNextCell(hex->GetCellType(), hex->GetPointIds());
    }

    // Create a mapper and actor
    vtkNew<vtkDataSetMapper> mapper;
    mapper->SetInputData(grid);
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);

    return actor;
}

//! Create the diverging color map from blue to red colors
vtkSmartPointer<vtkLookupTable> createBlueToRedColorMap()
{
    // Constants
    int const kTableSize = 256;

    // Set the colors
    vtkNew<vtkColorTransferFunction> ctf;
    ctf->SetColorSpaceToDiverging();
    ctf->AddRGBPoint(0.0, 0.230, 0.299, 0.754);
    ctf->AddRGBPoint(0.5, 0.865, 0.865, 0.865);
    ctf->AddRGBPoint(1.0, 0.706, 0.016, 0.150);

    // Create the lookup table
    vtkNew<vtkLookupTable> lut;
    lut->SetNumberOfTableValues(kTableSize);
    lut->Build();

    // Set the table values
    int numColors = lut->GetNumberOfColors();
    for (auto i = 0; i != numColors; ++i)
    {
        std::array<double, 3> rgb;
        ctf->GetColor(static_cast<double>(i) / lut->GetNumberOfColors(), rgb.data());
        std::array<double, 4> rgba{0.0, 0.0, 0.0, 1.0};
        std::copy(std::begin(rgb), std::end(rgb), std::begin(rgba));
        lut->SetTableValue(static_cast<vtkIdType>(i), rgba.data());
    }

    return lut;
}

//! Get maximum view dimension based on already rendered objects
double getMaximumDimension(vtkSmartPointer<vtkRenderer> renderer)
{
    double result = 0.0;
    double* dimensions = renderer->ComputeVisiblePropBounds();
    result = std::max(result, std::abs(dimensions[1] - dimensions[0]));
    result = std::max(result, std::abs(dimensions[3] - dimensions[2]));
    result = std::max(result, std::abs(dimensions[5] - dimensions[4]));
    return result;
}

//! Set global coordinate by the local one
void setGlobalByLocalEdits(Transformation const& transform, Edit1d* pLocalEdit, Edit1d* pGlobalEdit)
{
    VectorXi indices(1);
    indices << 0;
    Vector3d position = transform * Vector3d({pLocalEdit->value(), 0.0, 0.0});
    setEdits(&pGlobalEdit, position, indices);
}

//! Set global coordinates by the two dimensional local ones
void setGlobalByLocalEdits(Transformation const& transform, Edits2d const& localEdits, Edits3d& globalEdits, Vector2i const& indices)
{
    Vector3d position;
    position.fill(0.0);
    int numIndices = indices.size();
    for (int i = 0; i != numIndices; ++i)
        position[indices[i]] = localEdits[i]->value();
    position = transform * position;
    setEdits(globalEdits.data(), position);
}

//! Set global coordinates by the three dimensional local ones
void setGlobalByLocalEdits(Transformation const& transform, Edits3d const& localEdits, Edits3d& globalEdits)
{
    Vector3d position = transform * Vector3d({localEdits[0]->value(), localEdits[1]->value(), localEdits[2]->value()});
    setEdits(globalEdits.data(), position);
}

//! Set local coordinate by the global one
void setLocalByGlobalEdits(Transformation const& transform, Edit1d* pLocalEdit, Edit1d* pGlobalEdit)
{
    VectorXi indices(1);
    indices << 0;
    auto invTransform = transform.inverse();
    auto position = invTransform * Vector3d({pGlobalEdit->value(), 0.0, 0.0});
    setEdits(&pLocalEdit, position, indices);
}

//! Set two dimensional local coordinates by the global ones
void setLocalByGlobalEdits(Transformation const& transform, Edits2d& localEdits, Edits3d const& globalEdits, Vector2i const& indices)
{
    auto invTransform = transform.inverse();
    auto position = invTransform * Vector3d({globalEdits[0]->value(), globalEdits[1]->value(), globalEdits[2]->value()});
    setEdits(localEdits.data(), position, indices);
}

//! Set three dimensional local coordinates by the global ones
void setLocalByGlobalEdits(Transformation const& transform, Edits3d& localEdits, Edits3d const& globalEdits)
{
    auto invTransform = transform.inverse();
    auto position = invTransform * Vector3d({globalEdits[0]->value(), globalEdits[1]->value(), globalEdits[2]->value()});
    setEdits(localEdits.data(), position);
}

//! Set combobox current index by item key
void setIndexByKey(QComboBox* pComboBox, int key)
{
    int numItems = pComboBox->count();
    pComboBox->setCurrentIndex(-1);
    for (int i = 0; i != numItems; ++i)
    {
        if (pComboBox->itemData(i).toInt() == key)
        {
            pComboBox->setCurrentIndex(i);
            break;
        }
    }
}

//! Retrieve an icon associated with an element by pointer
QIcon getIcon(KCL::AbstractElement const* pElement)
{
    if (!pElement)
        return QIcon();
    return getIcon(pElement->type());
}

//! Retrieve an icon associated with an element by type
QIcon getIcon(KCL::ElementType type)
{
    switch (type)
    {
    case KCL::OD:
        return QIcon(":/icons/configuration.png");
    case KCL::SM:
        return QIcon(":/icons/mass.png");
    case KCL::BI:
        return QIcon(":/icons/beam-bending.png");
    case KCL::PN:
        return QIcon(":/icons/panel.png");
    case KCL::EL:
        return QIcon(":/icons/aileron.png");
    case KCL::DE:
        return QIcon(":/icons/aileron.png");
    case KCL::M3:
        return QIcon(":/icons/mass.png");
    case KCL::OP:
        return QIcon(":/icons/layer.png");
    case KCL::BK:
        return QIcon(":/icons/beam-torsion.png");
    case KCL::GS:
        return QIcon(":/icons/trapezium.png");
    case KCL::AE:
        return QIcon(":/icons/trapezium.png");
    case KCL::DQ:
        return QIcon(":/icons/function.png");
    case KCL::DA:
        return QIcon(":/icons/trapezium.png");
    case KCL::DB:
        return QIcon(":/icons/beam-bending.png");
    case KCL::P4:
        return QIcon(":/icons/layer.png");
    case KCL::PK:
        return QIcon(":/icons/function.png");
    case KCL::QK:
        return QIcon(":/icons/function.png");
    case KCL::WP:
        return QIcon(":/icons/setup.png");
    case KCL::PR:
        return QIcon(":/icons/spring.png");
    case KCL::TE:
        return QIcon(":/icons/damper.png");
    case KCL::CO:
        return QIcon(":/icons/constants.png");
    default:
        break;
    }
    return QIcon();
}

//! Retrieve an icon associated with a solver
QIcon getIcon(Core::ISolver const* pSolver)
{
    if (!pSolver)
        return QIcon();
    switch (pSolver->type())
    {
    case Core::ISolver::kModal:
        return QIcon(":/icons/spectrum.png");
    case Core::ISolver::kFlutter:
        return QIcon(":/icons/flutter.png");
    case Core::ISolver::kOptim:
        return QIcon(":/icons/optimization.png");
    }
    return QIcon();
}

//! Helper function to set values of edits
void setEdits(DoubleLineEdit** edits, Eigen::Vector3d const& values, VectorXi const& indices)
{
    int numIndices = indices.size();
    for (int i = 0; i != numIndices; ++i)
    {
        QSignalBlocker blocker(edits[i]);
        edits[i]->setValue(values[indices[i]]);
    }
}

// Explicit template instantiation
template QList<ElementHierarchyItem*> castHierarchyItems(QList<HierarchyItem*> const&);
}
