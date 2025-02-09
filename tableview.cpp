#include "tableview.h"
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QKeySequence>
#include <QPushButton>

TableView::TableView(QWidget *parent)
    : QTableView(parent)
{
    copyAction = new QAction(tr("Copy Selected"), this);
    connect(copyAction, &QAction::triggered, this, &TableView::copyAction_triggered_handler);
    // 创建菜单
    menu = new QMenu(this);
    menu->addAction(copyAction);
    // 添加快捷键 Ctrl+C
    copyShortcut = new QShortcut(QKeySequence::Copy, this);
    // 关键修改：将焦点设置为当前部件，否则 Ctrl+C 快捷键冲突
    copyShortcut->setContext(Qt::WidgetShortcut);
    connect(copyShortcut, &QShortcut::activated, this, &TableView::copyAction_triggered_handler);
    // 绑定 Ctrl+F 快捷键
    findShortcut = new QShortcut(QKeySequence::Find, this);
    findShortcut->setContext(Qt::WidgetShortcut); // 仅在 TableView 获得焦点时生效
    connect(findShortcut, &QShortcut::activated, this, &TableView::findShortcut_triggered_handler);
    // 显示自定义右键菜单
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTableView::customContextMenuRequested, this, [this](const QPoint &pos) {
        this->menu->exec(viewport()->mapToGlobal(pos));
    });
    // 连接单元格点击信号到槽函数
    connect(this, &TableView::clicked, this, [this](const QModelIndex &index) {
        // 获取点击单元格的行号
        int row = index.row();
        qDebug() << "Cell clicked at row:" << row;
        emit rowSelected(row);
    });
}

void TableView::findAndSelectCell(const QString &keyword)
{
    QAbstractItemModel *model = this->model();
    if (!model || keyword.isEmpty()) return; // 检查模型和关键词有效性

    // 获取当前选中位置作为搜索起点
    int startRow = lastFindRow > 0 ? lastFindRow + 1 : 0;
    int startCol = lastFindCol > 0 ? lastFindCol + 1 : 0;

    int totalRows = model->rowCount();
    int totalCols = model->columnCount();

    // 从起点开始逐行逐列搜索
    for (int row = startRow; row < totalRows; ++row) {
        for (int col = (row == startRow ? startCol : 0); col < totalCols; ++col) {
            QModelIndex index = model->index(row, col);
            QString cellText = model->data(index, Qt::DisplayRole).toString();

            // 不区分大小写匹配（可根据需要改为 Qt::CaseSensitive）
            if (cellText.contains(keyword, Qt::CaseInsensitive)) {
                setCurrentIndex(index); // 选中单元格
                scrollTo(index, QAbstractItemView::PositionAtCenter); // 滚动到视图中心
                lastFindRow = row;
                lastFindCol = col;
                return; // 找到第一个匹配项后退出
            }
        }
    }

    // 未找到时提示用户
    QMessageBox::information(this, tr("Search"), tr("The end of the table has been reached"));
    lastFindRow = -1;
    lastFindCol = -1;
}

void TableView::rowSelected_handler(int row)
{
    if (row >= 0 && row < model()->rowCount()) {
        selectionModel()->clear();
        // 选中指定行
        selectionModel()->select(model()->index(row, 0),
                                 QItemSelectionModel::Select | QItemSelectionModel::Rows);
        // 滚动到指定行
        scrollTo(model()->index(row, 0));
    } else {
        qWarning() << "Invalid row:" << row;
    }
}

void TableView::copyAction_triggered_handler()
{
    // 获取选中的范围
    const QModelIndexList &selectedIndexes = selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        return; // 如果没有选中任何内容，直接返回
    }

    // 确定选中范围的行列边界
    int minRow = selectedIndexes.first().row();
    int maxRow = selectedIndexes.first().row();
    int minCol = selectedIndexes.first().column();
    int maxCol = selectedIndexes.first().column();

    for (const QModelIndex &index : selectedIndexes) {
        minRow = qMin(minRow, index.row());
        maxRow = qMax(maxRow, index.row());
        minCol = qMin(minCol, index.column());
        maxCol = qMax(maxCol, index.column());
    }

    // 将选中的内容格式化为文本
    QString text;
    for (int row = minRow; row <= maxRow; ++row) {
        for (int col = minCol; col <= maxCol; ++col) {
            QModelIndex index = model()->index(row, col);
            text += index.data().toString();

            if (col < maxCol) {
                text += "\t"; // 列之间用制表符分隔
            }
        }
        if (row < maxRow) {
            text += "\n"; // 行之间用换行符分隔
        }
    }

    // 将文本复制到剪贴板
    QApplication::clipboard()->setText(text);
    qDebug() << "Copied to clipboard:\n" << text;
}

void TableView::findShortcut_triggered_handler()
{
    QDialog findDialog;
    QVBoxLayout *findLayout;
    QLineEdit *searchEdit;
    QPushButton *findButton;

    lastFindCol = currentIndex().isValid() ? currentIndex().row() - 1 : -1;
    lastFindRow = currentIndex().isValid() ? currentIndex().column() - 1 : -1;

    findLayout = new QVBoxLayout(&findDialog);
    searchEdit = new QLineEdit(&findDialog);
    findLayout->addWidget(searchEdit);
    findButton = new QPushButton("Find Next", &findDialog);
    findLayout->addWidget(findButton);
    connect(findButton, &QPushButton::clicked, this, [this, searchEdit]() {
        QString keyword = searchEdit->text();
        this->findAndSelectCell(keyword);
    });

    findDialog.exec();
}
