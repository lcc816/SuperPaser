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
    , descFields()
    , curGroupId(0)
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
    // 连续多组解析
    multiGroup = false;
    // 连接信号与槽
    connect(tmpMgmtWin, &TmpMgmtWin::tempSelected, structViewWin, &StructViewWin::tempMgmt_tempSelected_handler);
    connect(ui->resultTable, &TableView::rowSelected, structViewWin, &StructViewWin::result_rowSelected_handler);
    connect(tmpMgmtWin, &TmpMgmtWin::tempSelected, dataInputWin, &DataInputWin::tempMgmt_tempSelected_handler);
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
    }

    // 合并组号列
    if (multiGroup && (groupSize > 1)) {
        ui->resultTable->setSpan(startRow, 3, groupSize, 1); // 合并 groupSize 行
        ui->resultTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    }

    // 拼接到 field 数组末尾
    descFields += fields;
    curGroupId++;
}

void MainWindow::common_clearDisplay_handler()
{
    model->clear();
    curGroupId = 0;
    descFields.clear();
}
