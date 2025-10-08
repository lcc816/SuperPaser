#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QVariant>
#include <QMainWindow>
#include <QtGui/QIcon>
#include <QStatusBar>
#include <QVBoxLayout>
#include <datainputwindow.h>
#include <structviewwindow.h>
#include <templatemanagewindow.h>
#include <QStandardItemModel>
#include <QCoreApplication>
#include <QTimer>
#include "tableview.h"

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
        MainWindow->resize(1280, 768);
        MainWindow->setMinimumSize(QSize(1022, 630));
        MainWindow->setContentsMargins(2, 2, 2, 2);

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

private slots:
    void dataInput_appendOneGroup_handler(DescFieldList fields);
    void common_clearDisplay_handler();
    void batchUpdateResult();
    void result_rowSelected_handler(const QModelIndex &index);

private:
    QString templatesPath;
    DataInputWin *dataInputWin;
    TmpMgmtWin *tmpMgmtWin;
    StructViewWin *structViewWin;
    Ui::MainWindow *ui;
    QList<DescFieldList> descList;
    QVector<int> groupStartRows;
    QTimer updateResultTimer;
    bool isUpdating;
    QStandardItemModel *model;
    bool multiGroup;
};
#endif // MAINWINDOW_H
