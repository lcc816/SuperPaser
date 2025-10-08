#include <QDebug>
#include <QMessageBox>
#include <QStandardItemModel>
#include "datainputwindow.h"

DataInputWin::DataInputWin(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::DataInputWin)
    , curDesc()
    , isParsering(false)
    , multiGroup(false)
{
    ui->setupUi(this);
    connect(ui->submitButton, &QPushButton::clicked, this, &DataInputWin::submitButton_clicked_handler);
    connect(ui->clearButton, &QPushButton::clicked, this, [this]() {
        qDebug() << "{DataInputWin} clear text";
        this->ui->inputWidget->clear();
    });
    connect(ui->multiCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        qDebug() << "{DataInputWin} multi check state:" << checked;
        multiGroup = checked;
        emit this->multiGroupChecked(checked);
    });
}

DataInputWin::~DataInputWin()
{
    delete ui;
}

void DataInputWin::tempMgmt_tempSelected_handler(const DescObj &desc)
{
    if (!isParsering)
        curDesc = desc;
    else
        qWarning() << __func__ << "Parsing process in progress!";
}

void DataInputWin::submitButton_clicked_handler()
{
    if (curDesc.empty()) {
        QMessageBox::warning(this, tr("Error"), tr("No valid template selected"));
        return;
    }

    if (isParsering) {
        QMessageBox::warning(this, tr("Error"), tr("Parsing process in progress!"));
        return;
    }

    isParsering = true;

    bool ok;
    QStringList lines = ui->inputWidget->stripLines(&ok);
    if (!ok || lines.isEmpty()) {
        qWarning("%s[%d]: Invalid input", __func__, __LINE__);
        isParsering = false;
        return;
    }

    // 清空显示
    emit requestToClear();

    int minDescSize = curDesc.size();
    int linesSize = lines.size();
    if (linesSize < minDescSize) {
        QMessageBox::warning(this, tr("Error"), tr("Not enough lines applied to the selected template"));
        isParsering = false;
        return;
    }

    DescFieldList fields;

    auto it = lines.constBegin();
    while (it != lines.constEnd()) {
        fields.clear();
        int curDescSize = curDesc.size();
        int dwIdx = 0;
        // 将来单个描述符行数可能根据数据内容变化
        // 即解析到某一行, curDescSize 可能调整
        while ((dwIdx < curDescSize) && (it != lines.constEnd())) {
            const DescDWordObj &dwordObj = curDesc.at(dwIdx);
            uint32_t dwordValue = it->toUInt(nullptr, 16);
            for (int j = 0; j < dwordObj.size(); j++) {
                DescFieldItem field = {};
                const DescFieldObj &fieldObj = dwordObj.at(j);

                int lsb = fieldObj["LSB"].toInt();
                int msb = fieldObj["MSB"].toInt();
                QString fieldName(fieldObj["field"].toString());
                uint32_t fieldValue = DescObj::extractSubfield(dwordValue, lsb, msb);
                field.name = fieldName;
                field.lsb = lsb;
                field.msb = msb;
                field.dwIdx = dwIdx;
                field.value = fieldValue;

                // qDebug() << fieldName << ":" << fieldValue;
                fields.push_back(std::move(field));
            }
            dwIdx++;
            it++;
        }

        // 发送一组数据给主窗口显示
        emit appendOneGroup(std::move(fields));

        if (!multiGroup)
            break;
    }

    // emit submitClicked(lines);

    isParsering = false;
}
