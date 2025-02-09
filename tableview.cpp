#include "tableview.h"

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
    connect(copyShortcut, &QShortcut::activated, this, &TableView::copyAction_triggered_handler);
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
