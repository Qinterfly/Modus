#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <Eigen/Core>

#include "iview.h"

namespace Backend::Core
{
struct ModalSolution;
struct FlutterSolution;
}

namespace Frontend
{

class CustomTable;

class TableView : public IView
{
    Q_OBJECT

public:
    TableView();
    TableView(Eigen::VectorXd const& data);
    TableView(Backend::Core::ModalSolution const& solution);
    TableView(Backend::Core::FlutterSolution const& solution);
    virtual ~TableView() = default;

    void clear() override;
    void plot() override;
    void refresh() override;
    IView::Type type() const override;

private:
    void createContent();
    void setData(Eigen::VectorXd const& data);
    void setData(Backend::Core::ModalSolution const& solution);
    void setData(Backend::Core::FlutterSolution const& solution);

private:
    Eigen::MatrixXd mData;
    QStringList mHorizontalLabels;
    QStringList mVerticalLabels;
    CustomTable* mpTable;
};

}

#endif // TABLEVIEW_H
