#include "structviewwindow.h"

StructViewWin::StructViewWin(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::StructViewWin)
{
    ui->setupUi(this);
}

StructViewWin::~StructViewWin()
{
    delete ui;
}

void StructViewWin::tempMgmt_tempSelected_handler(const DescObj &desc)
{
    ui->displayTable->setRowCount(0); // 删除所有行
    int row = 0;
    for (size_t i = 0; i < desc.size(); i++) {
        const DescDWordObj &dword = desc.at(i);
        for (size_t j = 0; j < dword.size(); j++) {
            const DescFieldObj &field = dword.at(j);
            ui->displayTable->insertRow(row);
            ui->displayTable->setItem(row, 0, new QTableWidgetItem(QString::number(i)));
            ui->displayTable->setItem(row, 1, new QTableWidgetItem(field["field"].toString()));
            ui->displayTable->setItem(row, 2, new QTableWidgetItem(QString::number(field["LSB"].toInt())));
            ui->displayTable->setItem(row, 3, new QTableWidgetItem(QString::number(field["MSB"].toInt())));

            ++row;
        }
    }
    ui->displayTable->resizeColumnsToContents();
}

void StructViewWin::result_rowSelected_handler(int row)
{
    if (row >= 0 && row < ui->displayTable->model()->rowCount()) {
        ui->displayTable->selectionModel()->clear();
        // 选中指定行
        ui->displayTable->selectionModel()->select(ui->displayTable->model()->index(row, 0),
                                                   QItemSelectionModel::Select | QItemSelectionModel::Rows);
        // 滚动到指定行
        ui->displayTable->scrollTo(ui->displayTable->model()->index(row, 0));
    } else {
        qWarning() << "Invalid row:" << row;
    }
}
