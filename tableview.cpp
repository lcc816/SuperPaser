#include "tableview.h"
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QKeySequence>
#include <QLabel>
#include "tableview.h"

TableView::TableView(QWidget *parent)
    : QTableView(parent)
{
    // 设置被选中单元格背景为绿色
    setStyleSheet("QTableView::item:selected { background-color: lightgreen; }");

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

    // 初始化查找对话框相关成员
    findDialog = nullptr;
    searchEdit = nullptr;
    matchCaseCheckBox = nullptr;
    wholeWordCheckBox = nullptr;
    findNextButton = nullptr;
    findPreviousButton = nullptr;
    closeButton = nullptr;
}

void TableView::rowSelected_handler(int row)
{
    if (row >= 0) {
        row = row % model()->rowCount();
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

void TableView::findAndSelectCell(const QString &keyword, bool searchForward,
                                 bool matchCase, bool wholeWord)
{
    QAbstractItemModel *model = this->model();
    if (!model || keyword.isEmpty()) return;

    int totalRows = model->rowCount();
    int totalCols = model->columnCount();

    if (totalRows == 0 || totalCols == 0) return;

    // 设置搜索起点
    int startRow, startCol;
    if (searchForward) {
        // 向前搜索：从当前位置的下一个单元格开始
        startRow = currentIndex().isValid() ? currentIndex().row() : 0;
        startCol = currentIndex().isValid() ? currentIndex().column() + 1 : 0;
    } else {
        // 向后搜索：从当前位置的前一个单元格开始
        startRow = currentIndex().isValid() ? currentIndex().row() : totalRows - 1;
        startCol = currentIndex().isValid() ? currentIndex().column() - 1 : totalCols - 1;
    }

    // 调整起点位置
    if (startCol >= totalCols) {
        startCol = 0;
        startRow++;
    } else if (startCol < 0) {
        startCol = totalCols - 1;
        startRow--;
    }

    // 设置匹配选项
    Qt::CaseSensitivity caseSensitivity = matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive;

    // 搜索逻辑
    bool found = false;
    int row = startRow;
    int col = startCol;
    int iterations = 0;
    int maxIterations = totalRows * totalCols; // 防止无限循环

    while (iterations < maxIterations) {
        QModelIndex index = model->index(row, col);
        if (index.isValid()) {
            QString cellText = model->data(index, Qt::DisplayRole).toString();
            bool matches = false;

            if (wholeWord) {
                // 全字匹配：文本必须完全等于关键词（考虑大小写）
                matches = (matchCase ? (cellText == keyword) :
                                      (cellText.toLower() == keyword.toLower()));
            } else {
                // 部分匹配：文本包含关键词
                matches = cellText.contains(keyword, caseSensitivity);
            }

            if (matches) {
                setCurrentIndex(index);
                scrollTo(index, QAbstractItemView::PositionAtCenter);
                found = true;
                break;
            }
        }

        // 移动到下一个/上一个单元格
        if (searchForward) {
            col++;
            if (col >= totalCols) {
                col = 0;
                row++;
                if (row >= totalRows) {
                    row = 0; // 循环到开头
                }
            }
        } else {
            col--;
            if (col < 0) {
                col = totalCols - 1;
                row--;
                if (row < 0) {
                    row = totalRows - 1; // 循环到末尾
                }
            }
        }

        iterations++;

        // 如果回到起点，说明搜索了整个表格
        if (row == startRow && col == startCol) {
            break;
        }
    }

    if (!found) {
        QString message = searchForward ? tr("The end of the table has been reached. Start from the beginning?")
                                       : tr("The beginning of the table has been reached. Continue from the end?");

        int result = QMessageBox::question(this, tr("Search"), message,
                                          QMessageBox::Yes | QMessageBox::No);

        if (result == QMessageBox::Yes) {
            // 重置搜索位置并重新搜索
            if (searchForward) {
                findAndSelectCell(keyword, true, matchCase, wholeWord);
            } else {
                findAndSelectCell(keyword, false, matchCase, wholeWord);
            }
        }
    }
}

void TableView::findNext()
{
    if (!searchEdit || searchEdit->text().isEmpty()) {
        return;
    }

    QString keyword = searchEdit->text();
    bool matchCase = matchCaseCheckBox ? matchCaseCheckBox->isChecked() : false;
    bool wholeWord = wholeWordCheckBox ? wholeWordCheckBox->isChecked() : false;

    findAndSelectCell(keyword, true, matchCase, wholeWord);
}

void TableView::findPrevious()
{
    if (!searchEdit || searchEdit->text().isEmpty()) {
        return;
    }

    QString keyword = searchEdit->text();
    bool matchCase = matchCaseCheckBox ? matchCaseCheckBox->isChecked() : false;
    bool wholeWord = wholeWordCheckBox ? wholeWordCheckBox->isChecked() : false;

    findAndSelectCell(keyword, false, matchCase, wholeWord);
}

void TableView::setupFindDialog()
{
    if (findDialog) {
        findDialog->show();
        findDialog->activateWindow();
        searchEdit->setFocus();
        return;
    }

    findDialog = new QDialog(this);
    findDialog->setWindowTitle(tr("Find"));
    findDialog->setFixedSize(300, 150);

    QVBoxLayout *mainLayout = new QVBoxLayout(findDialog);

    // 搜索输入框
    QHBoxLayout *findLayout = new QHBoxLayout();
    QLabel *findLabel = new QLabel(tr("Find what:"), findDialog);
    searchEdit = new QLineEdit(findDialog);
    findLayout->addWidget(findLabel);
    findLayout->addWidget(searchEdit);
    mainLayout->addLayout(findLayout);

    // 选项复选框
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    matchCaseCheckBox = new QCheckBox(tr("Match case"), findDialog);
    wholeWordCheckBox = new QCheckBox(tr("Match whole word only"), findDialog);
    optionsLayout->addWidget(matchCaseCheckBox);
    optionsLayout->addWidget(wholeWordCheckBox);
    mainLayout->addLayout(optionsLayout);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    findNextButton = new QPushButton(tr("Find Next"), findDialog);
    findPreviousButton = new QPushButton(tr("Find Previous"), findDialog);
    closeButton = new QPushButton(tr("Close"), findDialog);
    buttonLayout->addWidget(findNextButton);
    buttonLayout->addWidget(findPreviousButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // 连接信号槽
    connect(findNextButton, &QPushButton::clicked, this, &TableView::findNext);
    connect(findPreviousButton, &QPushButton::clicked, this, &TableView::findPrevious);
    connect(closeButton, &QPushButton::clicked, findDialog, &QDialog::close);
    connect(searchEdit, &QLineEdit::returnPressed, this, &TableView::findNext);

    // 设置快捷键
    findNextButton->setShortcut(QKeySequence(Qt::Key_Return));
    findPreviousButton->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Return));
    closeButton->setShortcut(QKeySequence(Qt::Key_Escape));

    // 对话框关闭时清理
    connect(findDialog, &QDialog::finished, this, [this]() {
        // 不删除对话框，只是隐藏，以便下次快速打开
        findDialog->hide();
    });
}

void TableView::findShortcut_triggered_handler()
{
    setupFindDialog();
    findDialog->show();
    searchEdit->setFocus();
    searchEdit->selectAll();
}
