#include "templateeditwindow.h"
#include <QMessageBox>

TmpEditWin::TmpEditWin(QWidget *parent, DescObj &obj)
    : QDialog(parent)
    , ui(new Ui::TmpEditWin)
    , mDescObj(obj)
    , mChanged(false)
{
    setWindowTitle(QObject::tr("Edit template"));
    resize(720, 405);
    ui->setupUi(this);
    qDebug() << "hello, TmpEditWin";

    showTemplate();

    // 获取默认前景色
    QPalette palette = ui->editTable->palette();
    QColor defaultColor = palette.color(QPalette::Text);
    defaultBrush = QBrush(defaultColor);

    insertBelowAction = new QAction(tr("Insert a row below"), this);
    insertAboveAction = new QAction(tr("Insert a row above"), this);
    deleteRowAction = new QAction(tr("Delete this row"), this);

    connect(insertBelowAction, &QAction::triggered, this, &TmpEditWin::insertBelowAction_triggered_handler);
    connect(insertAboveAction, &QAction::triggered, this, &TmpEditWin::insertAboveAction_triggered_handler);
    connect(deleteRowAction, &QAction::triggered, this, &TmpEditWin::deleteRowAction_triggered_handler);

    ui->contextMenu->addAction(insertAboveAction);
    ui->contextMenu->addAction(insertBelowAction);
    ui->contextMenu->addAction(deleteRowAction);

    ui->editTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->editTable, &QTableWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        this->ui->contextMenu->exec(this->ui->editTable->mapToGlobal(pos));
    });

    connect(ui->editTable, &QTableWidget::cellChanged, this, &TmpEditWin::checkCellValid);
    connect(ui->okButton, &QPushButton::clicked, this, &TmpEditWin::saveAndExit);
    connect(ui->cancelButton, &QPushButton::clicked, this, &TmpEditWin::close);

    exec(); // 阻塞住
}

TmpEditWin::~TmpEditWin()
{
    qDebug() << "bye, TmpEditWin";
    delete ui;
}

int TmpEditWin::rowCount()
{
    return ui->editTable->rowCount();
}

void TmpEditWin::convertRow(int row, DescFieldObj &fieldObj, int &dwIdx)
{
    auto item = ui->editTable->item(row, 0);
    if (item) {
        dwIdx = item->text().toInt();
    }
    item = ui->editTable->item(row, 1);
    if (item) {
        fieldObj["field"] = QJsonValue(item->text());
    }
    item = ui->editTable->item(row, 2);
    if (item) {
        fieldObj["LSB"] = QJsonValue(item->text().toInt());
    }
    item = ui->editTable->item(row, 3);
    if (item) {
        fieldObj["MSB"] = QJsonValue(item->text().toInt());
    }
    qDebug() << "Convert row" << row << "to DW" << dwIdx;
}

void TmpEditWin::showTemplate()
{
    ui->editTable->setRowCount(0); // 删除所有行
    int row = 0;
    for (size_t i = 0; i < mDescObj.size(); i++) {
        DescDWordObj dword = mDescObj.at(i);
        for (size_t j = 0; j < dword.size(); j++) {
            DescFieldObj field = dword.at(j);
            ui->editTable->insertRow(row);
            ui->editTable->setItem(row, 0, new QTableWidgetItem(QString::number(i)));
            ui->editTable->setItem(row, 1, new QTableWidgetItem(field["field"].toString()));
            ui->editTable->setItem(row, 2, new QTableWidgetItem(QString::number(field["LSB"].toInt())));
            ui->editTable->setItem(row, 3, new QTableWidgetItem(QString::number(field["MSB"].toInt())));

            ++row;
        }
    }
}

void TmpEditWin::addRow()
{
    ui->editTable->insertRow(rowCount());
}

void TmpEditWin::insertRowBelow(int row)
{
    ui->editTable->insertRow(row + 1);
}

void TmpEditWin::insertRowAt(int row)
{
    ui->editTable->insertRow(row);
}

void TmpEditWin::deleteRowAt(int row)
{
    ui->editTable->removeRow(row);
}

DescObj &TmpEditWin::getEditedDesc(QWidget *parent, DescObj &inputDesc, bool *changed)
{
    TmpEditWin editWin(parent, inputDesc);
    if (changed != nullptr) {
        *changed = editWin.changed();
    }
    return inputDesc;
}

void TmpEditWin::insertBelowAction_triggered_handler()
{
    auto currentRow = ui->editTable->currentRow();
    qDebug() << "insert row below" << currentRow;
    if (currentRow < 0) {
        addRow();
    } else {
        insertRowBelow(currentRow);
    }
}

void TmpEditWin::insertAboveAction_triggered_handler()
{
    auto currentRow = ui->editTable->currentRow();
    qDebug() << "insert row at" << currentRow;
    if (currentRow < 0) {
        addRow();
    } else {
        insertRowAt(currentRow);
    }
}

void TmpEditWin::deleteRowAction_triggered_handler()
{
    auto currentRow = ui->editTable->currentRow();
    qDebug() << "delete row at" << currentRow;
    if (currentRow >= 0) {
        deleteRowAt(currentRow);
    }
}

void TmpEditWin::saveAndExit()
{
    DescObj desc;
    int preDWIdx = 0;
    DescDWordObj dwObj;
    for (int row = 0; row < ui->editTable->rowCount(); ++row) {
        DescFieldObj fieldObj;
        int curDWIdx = -1;
        convertRow(row, fieldObj, curDWIdx);
        if (curDWIdx == preDWIdx) {
            dwObj.push_back(fieldObj);
        } else if (curDWIdx == (preDWIdx + 1)) {
            desc.push_back(dwObj);
            dwObj.clear();
            dwObj.push_back(fieldObj);
        } else {
            // error
            QMessageBox::warning(this, tr("Error"), tr("The template format is invalid"));
            return;
        }
        preDWIdx = curDWIdx;
    }
    if (!dwObj.empty())
        desc.push_back(dwObj);
    mDescObj = desc;
    mChanged = true;
    close();
}

void TmpEditWin::checkDWCellValid(int row)
{
    auto curItem = ui->editTable->item(row, 0);
    if (nullptr == curItem)
        return;

    // 相邻的 DW 必须相等或递增
    do {
        bool ok;
        int curDW = curItem->text().toInt(&ok);
        if (!ok || (curDW < 0)) {
            qDebug() << "Invalid DW index" << curItem->text();
            break;
        }

        auto preItem = ui->editTable->item(row - 1, 0);
        if (nullptr != preItem) {
            int preDW = preItem->text().toInt();
            if (preDW > curDW) {
                qDebug() << "Previous DW" << preDW << "> current" << curDW;
                break;
            }
        }

        auto nxtItem = ui->editTable->item(row + 1, 0);
        if (nullptr != nxtItem) {
            int nxtDW = nxtItem->text().toInt();
            if (nxtDW < curDW) {
                qDebug() << "Next DW" << nxtDW << "< current" << curDW;
                break;
            }
        }

        curItem->setForeground(defaultBrush);
        return;
    } while(0);

    curItem->setForeground(QBrush(Qt::red));
}

void TmpEditWin::checkLSBCellValid(int row)
{
    auto curItem = ui->editTable->item(row, 2);
    if (nullptr == curItem)
        return;

    // LSB 数值大于等于 0，小于等于 31
    // LSB 数值必须小于等于同行 MSB
    // LSB 数值非 0，则必须大于前一行 MSB
    do {
        bool ok;
        int curLSB = curItem->text().toInt(&ok);
        if (!ok || (curLSB < 0) || (curLSB > 31)) {
            qDebug() << "Invalid LSB" << curItem->text();
            break;
        }

        auto nxtMSBTtem = ui->editTable->item(row, 3);
        if (nullptr != nxtMSBTtem) {
            int nxtMSB = nxtMSBTtem->text().toInt();
            if (nxtMSB < curLSB) {
                qDebug() << "Next MSB" << nxtMSB << "< current LSB" << curLSB;
                break;
            }
        }

        if (0 != curLSB) {
            auto preMSBItem = ui->editTable->item(row - 1, 3);
            if (nullptr != preMSBItem) {
                int preMSB = preMSBItem->text().toInt();
                if (preMSB >= curLSB) {
                    qDebug() << "Previous MSB" << preMSB << ">= current LSB" << curLSB;
                    break;
                }
            }
        }

        curItem->setForeground(defaultBrush);
        return;
    } while (0);

    curItem->setForeground(QBrush(Qt::red));
}

void TmpEditWin::checkMSBCellValid(int row)
{
    auto curItem = ui->editTable->item(row, 3);
    if (nullptr == curItem)
        return;

    // MSB 数值大于等于 0，小于等于 31
    // MSB 数值必须大于等于同行 LSB
    // MSB 数值不是 31，则必须小于后一行 LSB
    do {
        bool ok;
        int curMSB = curItem->text().toInt(&ok);
        if (!ok || (curMSB < 0) || (curMSB > 31)) {
            qDebug() << "Invalid MSB" << curItem->text();
            break;
        }

        auto preLSBTtem = ui->editTable->item(row, 2);
        if (nullptr != preLSBTtem) {
            int preLSB = preLSBTtem->text().toInt();
            if (preLSB > curMSB) {
                qDebug() << "Previous LSB" << preLSB << "> current MSB" << curMSB;
                break;
            }
        }

        if (31 != curMSB) {
            auto nxtLSBItem = ui->editTable->item(row + 1, 3);
            if (nullptr != nxtLSBItem) {
                int nxtLSB = nxtLSBItem->text().toInt();
                if (nxtLSB <= curMSB) {
                    qDebug() << "Next LSB" << nxtLSB << "<= current MSB" << curMSB;
                    break;
                }
            }
        }

        curItem->setForeground(defaultBrush);
        return;
    } while (0);

    curItem->setForeground(QBrush(Qt::red));
}

void TmpEditWin::checkCellValid(int row, int column)
{
    qDebug() << "("<< row << "," << column << ") changed";
    switch (column) {
    case 0:
        checkDWCellValid(row);
        break;
    case 1:
        // do nothing
        break;
    case 2:
        checkLSBCellValid(row);
        break;
    case 3:
        checkMSBCellValid(row);
        break;
    default:
        break;
    }
}
