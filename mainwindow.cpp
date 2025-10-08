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
    , descList()
    , groupStartRows()
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
    // 调整水平停靠窗口宽度
    resizeDocks({dataInputWin, tmpMgmtWin}, {440, 220}, Qt::Horizontal);
    // 结构展示窗口
    structViewWin = new StructViewWin(this);
    structViewWin->setObjectName(QString::fromUtf8("structViewWin"));
    addDockWidget(Qt::BottomDockWidgetArea, structViewWin);
    // 设置数据模型
    ui->resultTable->setModel(model);
    ui->resultTable->setItemDelegate(new CustomStyleDelegate(this));
    // 设置UI更新定时器
    updateResultTimer.setSingleShot(true);
    updateResultTimer.setInterval(50);  // 50ms更新一次UI
    isUpdating = false;
    // 连续多组解析
    multiGroup = false;
    // 连接信号与槽
    connect(tmpMgmtWin, &TmpMgmtWin::tempSelected, structViewWin, &StructViewWin::tempMgmt_tempSelected_handler);
    connect(ui->resultTable, &TableView::rowSelected, this, &MainWindow::result_rowSelected_handler);
    connect(tmpMgmtWin, &TmpMgmtWin::tempSelected, dataInputWin, &DataInputWin::tempMgmt_tempSelected_handler);
    connect(&updateResultTimer, &QTimer::timeout, this, &MainWindow::batchUpdateResult);
    connect(dataInputWin, &DataInputWin::multiGroupChecked, this, [this](bool checked) {
        multiGroup = checked;
    });
    connect(dataInputWin, &DataInputWin::requestToClear, this, &MainWindow::common_clearDisplay_handler);
    connect(dataInputWin, &DataInputWin::appendOneGroup, this, &MainWindow::dataInput_appendOneGroup_handler);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dataInput_appendOneGroup_handler(DescFieldList fields)
{
    // 加入到解析结果数组
    descList.push_back(std::move(fields));

    if (!updateResultTimer.isActive() && !isUpdating) {
        updateResultTimer.start();
    }
}

void MainWindow::common_clearDisplay_handler()
{
    model->clear();
    groupStartRows.clear();
    descList.clear();
}

void MainWindow::batchUpdateResult()
{
    if (isUpdating || descList.isEmpty()) return;

    isUpdating = true;

    int processCnt = 0;

    // 禁用 UI 自动更新
    ui->resultTable->setUpdatesEnabled(false);

    // 限制单次刷新行数
    while ((groupStartRows.size() < descList.size()) && (processCnt < 5000)) {
        int curGroupId = groupStartRows.size();
        auto &fields = descList[curGroupId];

        int startRow = model->rowCount();
        int groupSize = fields.size();
        qDebug() << "Append group" << curGroupId << "start" << startRow << "size" << groupSize;


        for (auto &field : fields) {
            QStandardItem *nameItem = new QStandardItem(field.name);

            QStandardItem *valueItem = new QStandardItem();
            valueItem->setData(field.value, Qt::DisplayRole);

            QStandardItem *hexValueItem = new QStandardItem(QString::asprintf("0x%x", field.value));

            QList<QStandardItem *> newRow({nameItem, valueItem, hexValueItem});

            if (multiGroup) {
                QStandardItem *groupIdItem = new QStandardItem(QString::asprintf("Group %d", curGroupId));
                if (curGroupId % 2 == 0) {
                    groupIdItem->setBackground(QBrush(Qt::white)); // 白色
                } else {
                    groupIdItem->setBackground(QBrush(Qt::lightGray)); // 浅灰色
                }
                newRow << groupIdItem;
            }

            model->appendRow(newRow);
            processCnt++;
        }

        // 存储每组的开始行号
        groupStartRows.push_back(startRow);
    }

    // 启用更新并刷新
    if (multiGroup)
        ui->resultTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->resultTable->setUpdatesEnabled(true);
    ui->resultTable->resizeColumnsToContents();

    isUpdating = false;

    if (groupStartRows.size() < descList.size()) {
        updateResultTimer.start();
    }
}

void MainWindow::result_rowSelected_handler(int row, int col)
{
    QStandardItem *groupIdItem = model->item(row, 3);
    if (groupIdItem == nullptr) return;

    int groupId = groupIdItem->text().midRef(6).toInt();

    if ((groupId < 0) || (groupId >= groupStartRows.size())) return;
    // 计算组内行索引
    int localRowId = row - groupStartRows[groupId];

    if ((localRowId < 0) || (localRowId >= descList[groupId].size())) return;
    // 获取解析条目
    DescFieldItem &field = descList[groupId][localRowId];

    structViewWin->fieldSelected_handler(field.dwIdx, field.lsb);
}
