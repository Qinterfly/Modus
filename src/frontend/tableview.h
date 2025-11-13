#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <Eigen/Core>

#include "iview.h"

namespace Backend::Core
{
struct FlutterSolution;
}

namespace Frontend
{

class CustomTable;

class TableView : public IView
{
    Q_OBJECT

public:
    TableView(Backend::Core::FlutterSolution const& solution);
    virtual ~TableView() = default;

    void clear() override;
    void plot() override;
    void refresh() override;
    IView::Type type() const override;

private:
    void createContent();
    void setData(Backend::Core::FlutterSolution const& solution);

private:
    Eigen::MatrixXd mData;
    QStringList mHorizontalLabels;
    QStringList mVerticalLabels;
    CustomTable* mpTable;
};

}

#endif // TABLEVIEW_H
