#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QVariant>
#include <QMainWindow>
#include <QtGui/QIcon>
#include <QStatusBar>
#include <QTableView>
#include <QVBoxLayout>
#include <datainputwindow.h>
#include <structviewwindow.h>
#include <templatemanagewindow.h>
#include <QStandardItemModel>
#include <QCoreApplication>
#include <QApplication>
#include <QClipboard>
#include <QShortcut>

class TableView : public QTableView
{
    Q_OBJECT

public:
    TableView(QWidget *parent = nullptr)
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
    }

private:
    void copyAction_triggered_handler()
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

    QMenu *menu;
    QAction *copyAction;
    QShortcut *copyShortcut;
};

QT_BEGIN_NAMESPACE

class UiMainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *centralLayout;
    QStatusBar *statusbar;

    TableView *resultTable;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1022, 630);
        MainWindow->setMinimumSize(QSize(1022, 630));
        MainWindow->setContentsMargins(5, 5, 5, 5);

        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy);

        MainWindow->setCentralWidget(centralWidget);

        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));

        MainWindow->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
        MainWindow->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

        resultTable = new TableView(centralWidget);
        resultTable->setObjectName(QString::fromUtf8("resultTable"));
        resultTable->verticalHeader()->setVisible(false);
        resultTable->horizontalHeader()->setVisible(false);
        // 设置内容不可编辑
        resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        // 设置最小列宽
        resultTable->horizontalHeader()->setMinimumSectionSize(75);
        centralLayout = new QVBoxLayout(centralWidget);
        centralLayout->addWidget(resultTable);
    }
};

namespace Ui {
    class MainWindow: public UiMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static uint32_t extractSubfield(uint32_t number, int n, int m);

private slots:
    void dataInput_submitClicked_handler(QStringList &lines);
    void tempMgmt_tempSelected_handler(const DescObj &desc);

private:
    QString templatesPath;
    DataInputWin *dataInputWin;
    TmpMgmtWin *tmpMgmtWin;
    StructViewWin *structViewWin;
    Ui::MainWindow *ui;
    QStandardItemModel *model;
    DescObj curDesc;
};
#endif // MAINWINDOW_H
