#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QDebug>
#include <QMenu>
#include <QTableView>
#include <QClipboard>
#include <QShortcut>
#include <QApplication>

class TableView : public QTableView
{
    Q_OBJECT

public:
    TableView(QWidget *parent = nullptr);

signals:
    void rowSelected(int row);

public slots:
    void rowSelected_handler(int row);

private slots:
    void copyAction_triggered_handler();

private:
    QMenu *menu;
    QAction *copyAction;
    QShortcut *copyShortcut;
};

#endif // TABLEVIEW_H
