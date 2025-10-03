#ifndef STRUCTVIEWWINDOW_H
#define STRUCTVIEWWINDOW_H

#include <QDockWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QStandardItemModel>
#include "descobj.h"
#include "tableview.h"

using std::size_t;

QT_BEGIN_NAMESPACE

class UiStructViewWin
{
public:
    QVBoxLayout *contentLayout;
    QWidget *contentWidget;
    TableView *displayTable;

    void setupUi(QDockWidget *dockWin)
    {
        dockWin->setWindowTitle(QObject::tr("Structure View"));
        dockWin->setMinimumHeight(200);

        contentWidget = new QWidget(dockWin);
        contentWidget->setObjectName(QString::fromUtf8("contentWidget"));

        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        contentWidget->setSizePolicy(sizePolicy);

        dockWin->setWidget(contentWidget);
        contentLayout = new QVBoxLayout(contentWidget);

        displayTable = new TableView(contentWidget);
        displayTable->setObjectName(QString::fromUtf8("displayTable"));
        // 隐藏垂直表头（序号列）
        displayTable->verticalHeader()->setVisible(false);
        // 设置内容不可编辑
        displayTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        // 设置最小列宽
        // displayTable->horizontalHeader()->setMinimumSectionSize(40);
        // 设置表格视图的布局方向为从右向左
        displayTable->setLayoutDirection(Qt::RightToLeft);
        contentLayout->addWidget(displayTable);
    }
};

namespace Ui {
    class StructViewWin: public UiStructViewWin {};
} // namespace Ui

QT_END_NAMESPACE

class StructViewWin : public QDockWidget
{
    Q_OBJECT
public:
    StructViewWin(QWidget *parent);
    ~StructViewWin();

public slots:
    void tempMgmt_tempSelected_handler(const DescObj &rootObj);
    void result_rowSelected_handler(int row);

private:
    Ui::StructViewWin *ui;
    QStandardItemModel *model;
};

#endif // STRUCTVIEWWINDOW_H
