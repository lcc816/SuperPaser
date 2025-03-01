#include <QDebug>
#include <QStandardItemModel>
#include "datainputwindow.h"

DataInputWin::DataInputWin(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::DataInputWin)
    , curDesc()
{
    ui->setupUi(this);
    connect(ui->submitButton, &QPushButton::clicked, this, &DataInputWin::submitButton_clicked_handler);
    connect(ui->clearButton, &QPushButton::clicked, this, [this]() {
        qDebug() << "{DataInputWin} clear text";
        this->ui->inputWidget->clear();
    });
    connect(ui->multiCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        qDebug() << "{DataInputWin} multi check state:" << checked;
        emit this->multiGroupChecked(checked);
    });
}

DataInputWin::~DataInputWin()
{
    delete ui;
}

void DataInputWin::submitButton_clicked_handler()
{
    bool ok;
    QStringList lines = ui->inputWidget->stripLines(&ok);
    if (!ok || lines.isEmpty()) {
        qWarning("%s[%d]: Invalid input", __func__, __LINE__);
        return;
    }

    emit submitClicked(lines);
}
