#ifndef DATAINPUTWINDOW_H
#define DATAINPUTWINDOW_H

#include <QDockWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenu>
#include "descobj.h"
#include "texteditor.h"

QT_BEGIN_NAMESPACE

class UiDataInputWin
{
public:
    QWidget *contentWidget;
    QVBoxLayout *contentLayout;
    DataInputEdit *inputWidget;
    QHBoxLayout *btnLaylout;
    QPushButton *submitButton;
    QPushButton *clearButton;

    QMenu *contextMenu;

    void setupUi(QDockWidget *dockWin)
    {
        dockWin->setWindowTitle(QObject::tr("Data Input"));
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

        inputWidget = new DataInputEdit(dockWin);
        inputWidget->setObjectName(QString::fromUtf8("editTable"));
        contentLayout->addWidget(inputWidget);

        btnLaylout = new QHBoxLayout();
        contentLayout->addLayout(btnLaylout);

        btnLaylout->addStretch();

        submitButton = new QPushButton(QObject::tr("Submit"), dockWin);
        submitButton->setFixedSize(70, 23);
        btnLaylout->addWidget(submitButton);

        clearButton = new QPushButton(QObject::tr("Clear"), dockWin);
        clearButton->setFixedSize(70, 23);
        btnLaylout->addWidget(clearButton);

        contextMenu = new QMenu(dockWin);
    }
};

namespace Ui {
    class DataInputWin: public UiDataInputWin {};
}

QT_END_NAMESPACE

class DataInputWin : public QDockWidget
{
    Q_OBJECT
public:
    DataInputWin(QWidget *parent);
    ~DataInputWin();

signals:
    void submitClicked(QStringList &lines);

private slots:
    void submitButton_clicked_handler();

private:
    Ui::DataInputWin *ui;
    DescObj curDesc;
};

#endif // DATAINPUTWINDOW_H
