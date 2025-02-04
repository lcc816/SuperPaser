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
    deleteFolderAction = new QAction(tr("Delete Group"), this);
    deleteTemplateAction = new QAction(tr("Delete Template"), this);
    editAction = new QAction(tr("Edit"), this);

    // 连接动作到槽函数
    connect(addNewFolderAction, &QAction::triggered, this, &TmpMgmtWin::addNewFolderAction_triggered_handler);
    connect(addNewTemplateAction, &QAction::triggered, this, &TmpMgmtWin::addNewTemplateAction_triggered_handler);
    connect(deleteFolderAction, &QAction::triggered, this, &TmpMgmtWin::deleteFolderAction_triggered_handler);
    connect(deleteTemplateAction, &QAction::triggered, this, &TmpMgmtWin::deleteTemplateAction_triggered_handler);
    connect(editAction, &QAction::triggered, this, &TmpMgmtWin::editAction_triggered_handler);

    // 将动作添加到右键菜单
    ui->contextMenu->addAction(addNewFolderAction);
    ui->contextMenu->addAction(addNewTemplateAction);
    ui->contextMenu->addAction(deleteFolderAction);
    ui->contextMenu->addAction(editAction);
    ui->contextMenu->addAction(deleteTemplateAction);

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

void TmpMgmtWin::editTemplate(QString &filePath)
{
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

void TmpMgmtWin::tempDirView_customContextMenuRequested_handler(const QPoint &pos)
{
    QModelIndex index = getIndexUnderMouse(pos);
    bool isFile = (mModel->isDir(index) == false);

    // 根据点击的是文件还是文件夹来调整右键菜单
    if (isFile) {
        addNewFolderAction->setVisible(false);
        addNewTemplateAction->setVisible(false);
        deleteFolderAction->setVisible(false);
        editAction->setVisible(true);
        deleteTemplateAction->setVisible(true);
    } else {
        addNewFolderAction->setVisible(true);
        addNewTemplateAction->setVisible(true);
        deleteFolderAction->setVisible(true);
        deleteTemplateAction->setVisible(false);
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
                QMessageBox::warning(this, "Error", "Failed to create a new foler.");
            }
        }
    }
}

void TmpMgmtWin::addNewTemplateAction_triggered_handler()
{
    QModelIndex currentIndex = ui->tempDirView->currentIndex();
    QString dirPath = mModel->fileInfo(currentIndex).absoluteFilePath();
    qDebug() << "Add a new template to: " << dirPath;
    QDir dir(dirPath);
    if (dir.exists()) {
        // 弹出输入对话框让用户输入新模板文件名
        bool ok;
        QString newTemplateName = QInputDialog::getText(this, "New Template", "Enter the template name:", QLineEdit::Normal, "", &ok);

        // 检查用户是否输入了有效的文件名并点击了OK
        if (ok && !newTemplateName.isEmpty()) {
            // 确保文件名以 ".json" 结尾（假设模板文件以 .template 为扩展名）
            if (!newTemplateName.endsWith(".json")) {
                newTemplateName += ".json";
            }

            // 构建新文件的完整路径
            QString newFilePath = dir.absoluteFilePath(newTemplateName);

            // 尝试创建新文件
            QFile newFile(newFilePath);
            if (newFile.open(QIODevice::WriteOnly)) {
                newFile.write(QByteArray("[]"));
                newFile.close();
                qDebug() << "Added new template" << newTemplateName;

                editTemplate(newFilePath);
            } else {
                QMessageBox::warning(this, "Error", "Failed to create the template file.");
            }
        }
    }
}

void TmpMgmtWin::deleteFolderAction_triggered_handler()
{
    // 获取当前选中的目录索引
    QModelIndex currentIndex = ui->tempDirView->currentIndex();
    QString dirPath = mModel->fileInfo(currentIndex).absoluteFilePath();
    qDebug() << "Delete folder: " << dirPath;

    // 检查当前选中项是否为目录
    QDir dir(dirPath);
    if (dir.exists() && dir.isReadable()) {
        // 弹出确认对话框，确保用户确认删除
        QMessageBox::StandardButton confirm;
        confirm = QMessageBox::question(this, "Confirm Deletion",
                                        "Are you sure you want to delete this gourp?",
                                        QMessageBox::Yes | QMessageBox::No);

        if (confirm == QMessageBox::Yes) {
            // 尝试删除目录
            if (dir.removeRecursively()) {
                qDebug() << "Deleted folder: " << dirPath;

                // 强制刷新模型：重新设置根路径
                mModel->setRootPath(""); // 先设置为空路径
                mModel->setRootPath(mTempPath); // 重新设置回原来的路径
            } else {
                // 删除失败，可能是由于权限问题或目录非空
                QMessageBox::warning(this, "Error", "Failed to delete the folder.");
            }
        }
    } else {
        // 当前选中项不是目录或不可读
        QMessageBox::warning(this, "Error", "The selected item is not a valid group.");
    }
}

void TmpMgmtWin::deleteTemplateAction_triggered_handler()
{
    // 获取当前选中的文件索引
    QModelIndex currentIndex = ui->tempDirView->currentIndex();
    QString filePath = mModel->fileInfo(currentIndex).absoluteFilePath();
    qDebug() << "Delete template file: " << filePath;

    // 检查当前选中项是否为文件
    QFileInfo fileInfo(filePath);
    if (fileInfo.isFile() && fileInfo.isWritable()) {
        // 弹出确认对话框，确保用户确认删除
        QMessageBox::StandardButton confirm;
        confirm = QMessageBox::question(this, "Confirm Deletion",
                                        "Are you sure you want to delete this template file?",
                                        QMessageBox::Yes | QMessageBox::No);

        if (confirm == QMessageBox::Yes) {
            // 尝试删除文件
            QFile file(filePath);
            if (file.remove()) {
                qDebug() << "Deleted template file: " << filePath;
            } else {
                // 删除失败，可能是由于权限问题或文件被占用
                QMessageBox::warning(this, "Error", "Failed to delete the template file.");
            }
        }
    } else {
        // 当前选中项不是文件或不可写
        QMessageBox::warning(this, "Error", "The selected item is not a valid template file.");
    }
}

void TmpMgmtWin::editAction_triggered_handler()
{
    QModelIndex currentIndex = ui->tempDirView->currentIndex();
    QString filePath = mModel->fileInfo(currentIndex).absoluteFilePath();
    qDebug() << "To edit template" << filePath;

    editTemplate(filePath);
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
