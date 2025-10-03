#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QDebug>
#include <QMenu>
#include <QTableView>
#include <QClipboard>
#include <QShortcut>
#include <QCheckBox>
#include <QPushButton>
#include <QApplication>

class TableView : public QTableView
{
    Q_OBJECT

public:
    TableView(QWidget *parent = nullptr);
    void rowSelected_handler(int row, int col);

signals:
    void rowSelected(int row, int col);

private slots:
    void copyAction_triggered_handler();
    void findShortcut_triggered_handler();

private:
    QMenu *menu;
    QAction *copyAction;
    QShortcut *copyShortcut;
    QShortcut *findShortcut;

    int lastFindRow;
    int lastFindCol;

    QDialog *findDialog;
    QLineEdit *searchEdit;
    QCheckBox *matchCaseCheckBox;
    QCheckBox *wholeWordCheckBox;
    QPushButton *findNextButton;
    QPushButton *findPreviousButton;
    QPushButton *closeButton;

    void setupFindDialog();
    void findNext();
    void findPrevious();
    void findAndSelectCell(const QString &keyword, bool searchForward, bool matchCase, bool wholeWord);
};

#endif // TABLEVIEW_H
