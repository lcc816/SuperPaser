#include "structviewwindow.h"

StructViewWin::StructViewWin(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::StructViewWin)
    , model(new QStandardItemModel(0, 32, this))
{
    ui->setupUi(this);
    // 设置列标题
    for (int i = 0; i < 32; i++) {
        model->setHorizontalHeaderItem(i, new QStandardItem(QString::number(i)));
    }
    // 设置表格各列等宽并随窗口伸缩
    ui->displayTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // 移除最小列宽限制，允许列宽无限缩小
    ui->displayTable->horizontalHeader()->setMinimumSectionSize(0);
    // 显示垂直表头，即行数
    ui->displayTable->verticalHeader()->setVisible(true);
    ui->displayTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    border: 1px solid #ccc;"  // 设置1像素宽的灰色实线边框
        "}"
        );
    ui->displayTable->verticalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    border: 1px solid #ccc;"  // 设置1像素宽的灰色实线边框
        "}"
        );
    // 设置数据模型
    ui->displayTable->setModel(model);
}

StructViewWin::~StructViewWin()
{
    delete ui;
}

void StructViewWin::tempMgmt_tempSelected_handler(const DescObj &desc)
{
    curDesc = desc;

    model->setRowCount(0); // 删除所有行
    int row = 0;
    for (size_t i = 0; i < desc.size(); i++) {
        const DescDWordObj &dword = desc.at(i);
        model->insertRow(row);
        for (size_t j = 0; j < dword.size(); j++) {
            const DescFieldObj &field = dword.at(j);
            int lsb = field["LSB"].toInt();
            int msb = field["MSB"].toInt();
            int filedWdith = msb - lsb + 1;
            QStandardItem *item = new QStandardItem(field["field"].toString());
            item->setTextAlignment(Qt::AlignCenter);
            model->setItem(row, lsb, item);
            if (filedWdith > 1)
                ui->displayTable->setSpan(row, lsb, 1, msb - lsb + 1);
        }
        model->setVerticalHeaderItem(row, new QStandardItem(QString::number(row)));
        // DWord换行
        ++row;
    }
}

void StructViewWin::result_rowSelected_handler(int row, int col)
{
    if (row < 0) {
        qWarning() << "Invalid row:" << row;
    }

    int fieldCnt = 0;
    for (size_t i = 0; i < curDesc.size(); i++) {
        const DescDWordObj &dword = curDesc.at(i);
        fieldCnt += dword.size();
        if (fieldCnt > row) {
            ui->displayTable->rowSelected_handler(int(i), 32 - (fieldCnt - row));
            break;
        }
    }
}
