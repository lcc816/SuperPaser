#include "templatemanagewindow.h"
#include <QCoreApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include "templateeditwindow.h"

TmpMgmtWin::TmpMgmtWin(QWidget *parent, QString tempPath)
    : QDockWidget(parent)
    , mTempPath(tempPath)
    , ui(new Ui::TmpMgmtWin)
{
    ui->setupUi(this);

    // 获取可执行文件所在根目录
    if (mTempPath.isEmpty()) {
        mTempPath = QCoreApplication::applicationDirPath() + "/descriptortemplates";
    }

    // 规范化模板目录名
    mTempPath = QDir::cleanPath(mTempPath);
    qDebug() << "模板目录： " << mTempPath;

    // 检查子目录是否存在
    QDir dir(mTempPath);
    if (!dir.exists()) {
        // 如果不存在，则创建子目录
        if (dir.mkpath(".")) {
            qDebug() << "子目录'descriptortemplates'已创建。";
        } else {
            qDebug() << "无法创建子目录'descriptortemplates'。";
        }
    } else {
        qDebug() << "子目录'descriptortemplates'已存在。";
    }

    mModel = new QFileSystemModel(this);
    mModel->setRootPath(mTempPath);

    const QModelIndex rootIndex = mModel->index(mTempPath);
    if (rootIndex.isValid()) {
        ui->tempDirView->setModel(mModel);
        ui->tempDirView->setRootIndex(rootIndex);
        ui->tempDirView->setHeaderHidden(true);
        ui->tempDirView->setColumnHidden(1, true);  // 隐藏大小列
        ui->tempDirView->setColumnHidden(2, true);  // 隐藏类型列
        ui->tempDirView->setColumnHidden(3, true);  // 隐藏最后修改时间列
    }

    addNewFolderAction = new QAction(tr("Add New Group"), this);
    addNewTemplateAction = new QAction(tr("Add New Template"), this);
    editAction = new QAction(tr("Edit"), this);

    // 连接动作到槽函数
    connect(addNewFolderAction, &QAction::triggered, this, &TmpMgmtWin::addNewFolderAction_triggered_handler);
    connect(addNewTemplateAction, &QAction::triggered, this, &TmpMgmtWin::addNewTemplateAction_triggered_handler);
    connect(editAction, &QAction::triggered, this, &TmpMgmtWin::editAction_triggered_handler);

    // 将动作添加到右键菜单
    ui->contextMenu->addAction(addNewFolderAction);
    ui->contextMenu->addAction(addNewTemplateAction);
    ui->contextMenu->addSeparator();
    ui->contextMenu->addAction(editAction);

    // 连接模板管理视图的自定义右键菜单请求信号到槽函数
    ui->tempDirView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tempDirView, &QTreeView::customContextMenuRequested, this, &TmpMgmtWin::tempDirView_customContextMenuRequested_handler);

    // 连接模板被选中信号到槽函数
    connect(ui->tempDirView, &QTreeView::clicked, this, &TmpMgmtWin::tempDirView_tempSelected_handler);
}

TmpMgmtWin::~TmpMgmtWin()
{
    delete ui;
}

void TmpMgmtWin::tempDirView_customContextMenuRequested_handler(const QPoint &pos)
{
    QModelIndex index = getIndexUnderMouse(pos);
    bool isFile = (mModel->isDir(index) == false);

    // 根据点击的是文件还是文件夹来调整右键菜单
    if (isFile) {
        addNewFolderAction->setVisible(false);
        addNewTemplateAction->setVisible(false);
        editAction->setVisible(true);
    } else {
        addNewFolderAction->setVisible(true);
        addNewTemplateAction->setVisible(true);
        editAction->setVisible(false);
    }

    // 选中条目
    ui->tempDirView->setCurrentIndex(index);
    // 在鼠标点击位置显示右键菜单
    ui->contextMenu->exec(ui->tempDirView->mapToGlobal(pos));
}

void TmpMgmtWin::tempDirView_tempSelected_handler(const QModelIndex &index)
{
    if (mModel->isDir(index)) {
        // Not a template, do nothing
        return;
    }

    QString filePath = mModel->fileInfo(index).absoluteFilePath();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Couldn't open the file.");
        return;
    }
    QByteArray fileData = file.readAll();
    bool ok = false;
    DescObj rootObj = DescObj::fromJson(fileData, &ok);

    if (!ok || !rootObj.checkFormat()) {
        qWarning("%s[%d]: Not a valid template", __func__, __LINE__);
        // 确保发送空的描述符模板
        rootObj.clear();
    }

    emit this->tempSelected(rootObj);
}

void TmpMgmtWin::addNewFolderAction_triggered_handler()
{
    QModelIndex currentIndex = ui->tempDirView->currentIndex();
    QString dirPath = mModel->fileInfo(currentIndex).absoluteFilePath();
    qDebug() << "Add a new group to: " << dirPath;
    QDir dir(dirPath);
    if (dir.exists()) {
        // 弹出输入对话框让用户输入新目录名
        bool ok;
        QString newFolderName = QInputDialog::getText(this, "New Group", "Enter the name:", QLineEdit::Normal, "", &ok);

        // 检查用户是否输入了有效的目录名并点击了OK
        if (ok && !newFolderName.isEmpty()) {
            QDir newDir(dir.absoluteFilePath(newFolderName));
            // 尝试创建新目录
            if (newDir.mkdir(".")) {
                // 目录创建成功，刷新文件系统模型
                //m_model->refresh();
                qDebug() << "Added new group" << newFolderName;
            } else {
                // 目录创建失败，可能是由于权限问题或目录名已存在等原因
                // 可以在这里添加错误处理代码，比如显示一个错误消息框
            }
        }
    }
}

void TmpMgmtWin::addNewTemplateAction_triggered_handler()
{
}

void TmpMgmtWin::editAction_triggered_handler()
{
    QModelIndex currentIndex = ui->tempDirView->currentIndex();
    QString filePath = mModel->fileInfo(currentIndex).absoluteFilePath();
    qDebug() << "To edit template" << filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("%s[%d]: Cannot open the template.", __func__, __LINE__);
        return;
    }
    QByteArray fileData = file.readAll();
    file.close();

    bool ok = false;
    DescObj rootObj = DescObj::fromJson(fileData, &ok);

    if (!ok || !rootObj.checkFormat()) {
        QMessageBox::warning(this, tr("Error"), tr("The template format is invalid"));
        return;
    }
    bool changed = false;
    rootObj = TmpEditWin::getEditedDesc(this, rootObj, &changed);
    if (changed) {
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            qWarning("%s[%d]: Cannot open the template.", __func__, __LINE__);
        }
        file.write(rootObj.toBtyeArray());
        file.close();
    }
}

QModelIndex TmpMgmtWin::getIndexUnderMouse(const QPoint &pos)
{
    QModelIndex index = ui->tempDirView->indexAt(pos);
    if (!index.isValid()) {
        // 右键点击的是空白处
        qDebug() << "Blank area, set index to" << mTempPath;
        return ui->tempDirView->rootIndex();
    }
    return index;
}
