#include "structviewwindow.h"

StructViewWin::StructViewWin(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::StructViewWin)
    , model(new QStandardItemModel(0, 4, this))
{
    ui->setupUi(this);
    // 设置数据模型
    model->setHorizontalHeaderLabels({"DW", "Field", "LSB", "MSB"});
    ui->displayTable->setModel(model);
}

StructViewWin::~StructViewWin()
{
    delete ui;
}

void StructViewWin::tempMgmt_tempSelected_handler(const DescObj &desc)
{
    model->setRowCount(0); // 删除所有行
    int row = 0;
    for (size_t i = 0; i < desc.size(); i++) {
        const DescDWordObj &dword = desc.at(i);
        for (size_t j = 0; j < dword.size(); j++) {
            const DescFieldObj &field = dword.at(j);
            model->insertRow(row);
            model->setItem(row, 0, new QStandardItem(QString::number(i)));
            model->setItem(row, 1, new QStandardItem(field["field"].toString()));
            model->setItem(row, 2, new QStandardItem(QString::number(field["LSB"].toInt())));
            model->setItem(row, 3, new QStandardItem(QString::number(field["MSB"].toInt())));

            ++row;
        }
    }
    ui->displayTable->resizeColumnsToContents();
}

void StructViewWin::result_rowSelected_handler(int row)
{
    ui->displayTable->rowSelected_handler(row);
}
