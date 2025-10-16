#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QScreen>
#include <QToolBar>
#include <QWidget>

#include <kcl/model.h>

#include <Eigen/Geometry>
#include <magicenum/magic_enum.hpp>
#include <vtkColor.h>
#include <vtkCylinderSource.h>
#include <vtkDataSetMapper.h>
#include <vtkGlyph3DMapper.h>
#include <vtkHexahedron.h>
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
#include "uiutility.h"

using namespace Eigen;
using namespace Backend;
using namespace Frontend;

namespace Frontend::Utility
{

void setEdits(DoubleLineEdit** edits, Eigen::Vector3d const& values, QList<int> const& indices = {0, 1, 2});

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
    int type = items.first()->type();
    uint numItems = items.size();
    bool isEqual;
    for (uint i = 1; i != numItems; ++i)
    {
        isEqual = items[i]->type() == type;
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
    result.append(aeroPanelsTypes());
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

//! Retrieve the KCL types which are associated with aerodynamic panel elements
QList<KCL::ElementType> aeroPanelsTypes()
{
    return {KCL::AE, KCL::DA};
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

//! Build up the transformation for the elastic surface
Transformation computeTransformation(KCL::ElasticSurface const& surface)
{
    Transformation result = Transformation::Identity();
    if (!surface.containsElement(KCL::OD))
        return result;
    auto pData = (KCL::GeneralData const*) surface.element(KCL::OD);
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

//! Set global coordinates by the two dimensional local ones
void setGlobalByLocalEdits(Transformation const& transform, Edits2d const& localEdits, Edits3d& globalEdits)
{
    Vector3d position = transform * Vector3d({localEdits[0]->value(), 0.0, localEdits[1]->value()});
    setEdits(globalEdits.data(), position);
}

//! Set global coordinates by the three dimensional local ones
void setGlobalByLocalEdits(Transformation const& transform, Edits3d const& localEdits, Edits3d& globalEdits)
{
    Vector3d position = transform * Vector3d({localEdits[0]->value(), localEdits[1]->value(), localEdits[2]->value()});
    setEdits(globalEdits.data(), position);
}

//! Set two dimensional local coordinates by the global ones
void setLocalByGlobalEdits(Transformation const& transform, Edits2d& localEdits, Edits3d const& globalEdits)
{
    QList<int> const kMapIndices = {0, 2};
    auto invTransform = transform.inverse();
    auto position = invTransform * Vector3d({globalEdits[0]->value(), globalEdits[1]->value(), globalEdits[2]->value()});
    setEdits(localEdits.data(), position, kMapIndices);
}

//! Set three dimensional local coordinates by the global ones
void setLocalByGlobalEdits(Transformation const& transform, Edits3d& localEdits, Edits3d const& globalEdits)
{
    auto invTransform = transform.inverse();
    auto position = invTransform * Vector3d({globalEdits[0]->value(), globalEdits[1]->value(), globalEdits[2]->value()});
    setEdits(localEdits.data(), position);
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
void setEdits(DoubleLineEdit** edits, Eigen::Vector3d const& values, QList<int> const& indices)
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
