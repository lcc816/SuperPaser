#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include "mainwindow.h"

// 自定义表格样式委托
class CustomStyleDelegate : public QStyledItemDelegate
{
public:
    explicit CustomStyleDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};

void CustomStyleDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);

    // 设置交替显示颜色
    if (index.row() % 2 == 0) {
        option->backgroundBrush = QBrush(Qt::white); // 偶数行白色
    } else {
        option->backgroundBrush = QBrush(Qt::lightGray); // 奇数行灰色
    }

    // 读取组颜色，不改变组颜色策略
    QVariant bgColor = index.data(Qt::BackgroundRole);
    if (bgColor.isValid()) {
        option->backgroundBrush = bgColor.value<QBrush>();
    }

    // 设置第二列和第三列右对齐
    if (index.column() == 1 || index.column() == 2) {
        option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
    }
}

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
    ui->resultTable->setItemDelegate(new CustomStyleDelegate(this));
    // 连续多组解析
    multiGroup = false;
    // 解析进程正在执行
    isParsering = false;
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
    // 设置标记，阻止描述符对象被更新
    isParsering = true;
    for (size_t k = 0; k < desc_cnt; k++) {
        ui->resultTable->setUpdatesEnabled(false); // 禁用更新
        int startRow = row;
        int groupSize = 0;
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
                if (multiGroup) {
                    QStandardItem *groupItem = new QStandardItem(QString::asprintf("Group %zu", k));
                    if (k % 2 == 0) {
                        groupItem->setBackground(QBrush(Qt::white)); // 白色
                    } else {
                        groupItem->setBackground(QBrush(Qt::lightGray)); // 浅灰色
                    }
                    model->setItem(row, 3, groupItem); // 组号列
                }
                qDebug() << fieldName << ":" << fieldValue;
                ++row;
                ++groupSize;
            }
        }
        // 合并组号列
        if (multiGroup && (groupSize > 1)) {
            ui->resultTable->setSpan(startRow, 3, groupSize, 1); // 合并 groupSize 行
            ui->resultTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
        }
        ui->resultTable->resizeColumnsToContents();
        ui->resultTable->setUpdatesEnabled(true); // 启用更新
        QApplication::processEvents();
    }

    isParsering = false;
}

void MainWindow::tempMgmt_tempSelected_handler(const DescObj &desc)
{
    if (!isParsering)
        curDesc = desc;
    else
        qDebug() << __func__ << "Parsing process in progress!";
}
