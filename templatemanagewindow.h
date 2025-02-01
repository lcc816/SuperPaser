#ifndef TEMPLATEMANAGEWINDOW_H
#define TEMPLATEMANAGEWINDOW_H

#include <QtCore/QVariant>
#include <QDockWidget>
#include <QFileSystemModel>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QTreeView>
#include <QMenu>
#include "descobj.h"

QT_BEGIN_NAMESPACE

class UiTmpMgmtWin
{
public:
    QVBoxLayout *contentLayout;
    QWidget *contentWidget;
    QTreeView *tempDirView;
    QMenu *contextMenu;

    void setupUi(QDockWidget *dockWin)
    {
        dockWin->setWindowTitle(QObject::tr("Templates Manager"));
        dockWin->setMinimumWidth(220);

        contentWidget = new QWidget(dockWin);
        contentWidget->setObjectName(QString::fromUtf8("contentWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(contentWidget->sizePolicy().hasHeightForWidth());
        contentWidget->setSizePolicy(sizePolicy);

        dockWin->setWidget(contentWidget);

        contentLayout = new QVBoxLayout(contentWidget);

        tempDirView = new QTreeView(contentWidget);
        tempDirView->setObjectName(QString::fromUtf8("dirView"));
        sizePolicy.setHeightForWidth(tempDirView->sizePolicy().hasHeightForWidth());
        contentLayout->addWidget(tempDirView);

        contextMenu = new QMenu(dockWin);
    }
};

namespace Ui {
    class TmpMgmtWin: public UiTmpMgmtWin {};
} // namespace Ui

QT_END_NAMESPACE

class TmpMgmtWin : public QDockWidget
{
    Q_OBJECT

public:
    TmpMgmtWin(QWidget *parent = nullptr, QString tempPath = "");
    ~TmpMgmtWin();

signals:
    void tempSelected(const DescObj &rootObj);

private:
    QString             mRootPath;
    QString             mTempPath;
    QFileSystemModel    *mModel;
    QAction             *addNewFolderAction;
    QAction             *addNewTemplateAction;
    QAction             *editAction;

    Ui::TmpMgmtWin *ui;

private slots:
    void tempDirView_customContextMenuRequested_handler(const QPoint &pos);
    void tempDirView_tempSelected_handler(const QModelIndex &index);
    void addNewFolderAction_triggered_handler();
    void addNewTemplateAction_triggered_handler();
    void editAction_triggered_handler();

private:
    QModelIndex getIndexUnderMouse(const QPoint &pos);
};

#endif // TEMPLATEMANAGEWINDOW_H
