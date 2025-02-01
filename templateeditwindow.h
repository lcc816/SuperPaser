#ifndef TMPEDITWIN_H
#define TMPEDITWIN_H

#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMenu>
#include "descobj.h"

QT_BEGIN_NAMESPACE

class UiTmpEditWin
{
public:
    QVBoxLayout *rootLayout;
    QTableWidget *editTable;
    QHBoxLayout *btnLaylout;
    QPushButton *addButton;
    QPushButton *okButton;
    QPushButton *cancelButton;

    QMenu *contextMenu;

    void setupUi(QWidget *parent)
    {
        rootLayout = new QVBoxLayout(parent);

        editTable = new QTableWidget(parent);
        editTable->setObjectName(QString::fromUtf8("editTable"));
        editTable->setColumnCount(4);
        editTable->setHorizontalHeaderLabels({"DW", "Field", "LSB", "MSB"});
        rootLayout->addWidget(editTable);

        btnLaylout = new QHBoxLayout();
        rootLayout->addLayout(btnLaylout);

        btnLaylout->addStretch();

        okButton = new QPushButton(QObject::tr("OK"), parent);
        okButton->setFixedSize(75, 23);
        btnLaylout->addWidget(okButton);

        cancelButton = new QPushButton(QObject::tr("Cancel"), parent);
        cancelButton->setFixedSize(75, 23);
        btnLaylout->addWidget(cancelButton);

        contextMenu = new QMenu(parent);
    }
};

namespace Ui {
    class TmpEditWin: public UiTmpEditWin {};
} // namespace Ui

QT_END_NAMESPACE

class TmpEditWin : public QDialog
{
    Q_OBJECT
public:
    TmpEditWin(QWidget *parent, DescObj &obj);
    ~TmpEditWin();

    int rowCount();

    void convertRow(int row, DescFieldObj &fieldObj, int &dwIdx);
    void showTemplate();
    void saveTemplate();

    void addRow();
    void insertRowBelow(int row);
    void insertRowAt(int row);
    void deleteRowAt(int row);

    bool changed() { return mChanged; }
    static DescObj &getEditedDesc(QWidget *parent, DescObj &inputDesc, bool *ok);

private slots:
    void insertBelowAction_triggered_handler();
    void insertAboveAction_triggered_handler();
    void deleteRowAction_triggered_handler();

    void saveAndExit();

private:
    void checkDWCellValid(int row);
    void checkLSBCellValid(int row);
    void checkMSBCellValid(int row);
    void checkCellValid(int row, int column);

    Ui::TmpEditWin *ui;
    DescObj &mDescObj;
    bool mChanged;
    QBrush defaultBrush;
    QAction *insertBelowAction;
    QAction *insertAboveAction;
    QAction *deleteRowAction;
};

#endif // TMPEDITWIN_H
