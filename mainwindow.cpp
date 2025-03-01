#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QMessageBox>
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
#ifdef Q_OS_WIN
    setWindowIcon(QIcon(":/icons/images/endless.ico"));
#elif defined(Q_OS_LINUX)
    setWindowIcon(QIcon(":/icons/images/endless.png"));
    if (templatesPath.isEmpty()) {
        templatesPath = env.value("HOME") + "/.sp";
    }
#endif
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
    // 连续多组解析
    multiGroup = false;
    // 连接信号与槽
    connect(tmpMgmtWin, &TmpMgmtWin::tempSelected, structViewWin, &StructViewWin::tempMgmt_tempSelected_handler);
    connect(ui->resultTable, &TableView::rowSelected, structViewWin, &StructViewWin::result_rowSelected_handler);
    connect(tmpMgmtWin, &TmpMgmtWin::tempSelected, this, &MainWindow::tempMgmt_tempSelected_handler);
    connect(dataInputWin, &DataInputWin::submitClicked, this, &MainWindow::dataInput_submitClicked_handler);
    connect(dataInputWin, &DataInputWin::multiGroupChecked, this, [this](bool checked) {
        multiGroup = checked;
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

uint32_t MainWindow::extractSubfield(uint32_t number, int n, int m)
{
    // 确保 n 和 m 在有效范围内（0 <= n <= m <= 31）
    if ((n < 0) || (m > 31) || (n > m)) {
        return 0;  // 如果输入无效，返回 0
    }

    // 计算掩码：从第 n 位到第 m 位的掩码
    uint32_t mask;
    if ((n == 0) && (m == 31)) {
        return number;
    } else {
        mask = (1UL << (m - n + 1)) - 1;  // 生成 (m - n + 1) 个 1
    }
    mask <<= n;  // 将掩码左移 n 位，对齐到第 n 位

    // 提取字段值
    uint32_t result = (number & mask) >> n;

    return result;
}

void MainWindow::dataInput_submitClicked_handler(QStringList &lines)
{
    if (curDesc.empty()) {
        QMessageBox::warning(this, tr("Error"), tr("No valid template selected"));
        return;
    }
    size_t curDescSize = curDesc.size();
    size_t linesSize = size_t(lines.size());
    if (linesSize < curDescSize) {
        QMessageBox::warning(this, tr("Error"), tr("Not enough lines applied to the selected template"));
        return;
    }

    size_t desc_cnt = multiGroup ? (linesSize / curDescSize) : 1;

    int row = 0;
    model->clear();
    ui->resultTable->setUpdatesEnabled(false); // 禁用更新
    for (size_t k = 0; k < desc_cnt; k++) {
        for (size_t i = 0; i < curDesc.size(); i++) {
            const DescDWordObj &dwordObj = curDesc.at(i);
            const QString &line = lines.at(int(i + curDesc.size() * k));
            uint32_t dwordValue = line.toUInt(nullptr, 16);

            for (size_t j = 0; j < dwordObj.size(); j++) {
                const DescFieldObj &fieldObj = dwordObj.at(j);
                int lsb = fieldObj["LSB"].toInt();
                int msb = fieldObj["MSB"].toInt();
                QString fieldName(fieldObj["field"].toString());
                uint32_t fieldValue = extractSubfield(dwordValue, lsb, msb);

                model->setItem(row, 0, new QStandardItem(fieldName));
                model->setItem(row, 1, new QStandardItem(QString::asprintf("0x%x", fieldValue)));
                model->setItem(row, 2, new QStandardItem(QString::number(fieldValue)));
                qDebug() << fieldName << ":" << fieldValue;
                ++row;
            }
        }
    }
    ui->resultTable->resizeColumnsToContents();
    ui->resultTable->setUpdatesEnabled(true); // 启用更新
}

void MainWindow::tempMgmt_tempSelected_handler(const DescObj &desc)
{
    curDesc = desc;
    qDebug() << __func__ << "isEmpty:" << curDesc.empty();
}
