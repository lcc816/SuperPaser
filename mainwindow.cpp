#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , model(new QStandardItemModel(0, 3, this))
{
    // 获取环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    templatesPath = env.value("TEMPLATE_PATH");
    qDebug() << "APP root path: " << templatesPath;
    // 设置 UI
    ui->setupUi(this);
    // 数据输入子窗口
    dataInputWin = new DataInputWin(this);
    dataInputWin->setObjectName(QString::fromUtf8("dataInputWin"));
    addDockWidget(Qt::LeftDockWidgetArea, dataInputWin);
    // 模板管理子窗口
    tmpMgmtWin = new TmpMgmtWin(this, templatesPath);
    tmpMgmtWin->setObjectName(QString::fromUtf8("tmpMgmtWin"));
    addDockWidget(Qt::RightDockWidgetArea, tmpMgmtWin);
    // 结构展示窗口
    structViewWin = new StructViewWin(this);
    structViewWin->setObjectName(QString::fromUtf8("structViewWin"));
    addDockWidget(Qt::BottomDockWidgetArea, structViewWin);
    // 设置数据模型
    ui->resultTable->setModel(model);
    // 连接信号与槽
    connect(tmpMgmtWin, &TmpMgmtWin::tempSelected, structViewWin, &StructViewWin::tempMgmt_tempSelected_handler);
    connect(tmpMgmtWin, &TmpMgmtWin::tempSelected, this, &MainWindow::tempMgmt_tempSelected_handler);
    connect(dataInputWin, &DataInputWin::submitClicked, this, &MainWindow::dataInput_submitClicked_handler);
}

MainWindow::~MainWindow()
{
    delete ui;
}

uint32_t MainWindow::extractSubfield(uint32_t number, int n, int m)
{
    // 验证n和m的有效性
    if (m <= n || n < 0 || m > 31) {
        throw std::invalid_argument("Invalid n or m values");
    }

    // 构造掩码，其中n到m位为1，其他位为0
    uint32_t mask = (1U << (m + 1)) - 1; // 生成m位全为1的掩码
    mask = mask << n; // 将掩码左移至n位位置
    mask = mask & ~(mask >> (m - n + 1)); // 清除m位之后的1

    // 利用掩码提取子字段，并右移n位获得实际值
    return (number & mask) >> n;
}

void MainWindow::dataInput_submitClicked_handler(QStringList &lines)
{
    if (curDesc.empty()) {
        QMessageBox::warning(this, tr("Error"), tr("No valid template selected"));
        return;
    }

    if (size_t(lines.size()) < curDesc.size()) {
        QMessageBox::warning(this, tr("Error"), tr("Not enough lines applied to the selected template"));
        return;
    }

    int row = 0;
    model->clear();
    for (size_t i = 0; i < curDesc.size(); i++) {
        const DescDWordObj &dwordObj = curDesc.at(i);
        const QString &line = lines.at(int(i));
        uint32_t dwordValue = line.toUInt(nullptr, 16);
        for (size_t j = 0; j < dwordObj.size(); j++) {
            const DescFieldObj &fieldObj = dwordObj.at(j);
            int lsb = fieldObj["LSB"].toInt();
            int msb = fieldObj["MSB"].toInt();
            QString fieldName(fieldObj["field"].toString());
            uint32_t fieldValue = extractSubfield(dwordValue, lsb, msb);
            model->setItem(row, 0, new QStandardItem(fieldName));
            model->setItem(row, 1, new QStandardItem("0x" + QString::number(fieldValue, 16)));
            model->setItem(row, 2, new QStandardItem(QString::number(fieldValue)));
            qDebug() << fieldName << ":" << fieldValue;
            ++row;
        }
    }
    ui->resultTable->resizeColumnsToContents();
}

void MainWindow::tempMgmt_tempSelected_handler(const DescObj &desc)
{
    curDesc = desc;
    qDebug() << __func__ << "isEmpty:" << curDesc.empty();
}
