#ifndef DATAINPUTWINDOW_H
#define DATAINPUTWINDOW_H

#include <QDockWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenu>
#include <QDebug>
#include <QMessageBox>
#include "descobj.h"

class DataInputEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    DataInputEdit(QWidget *parent = nullptr) : QPlainTextEdit(parent)
    {
        // 设置等宽字体
        QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        setFont(fixedFont);
    }

    QStringList stripLines(bool *ok)
    {
        QString inputText = this->toPlainText();
        QStringList lines = inputText.split('\n', Qt::SkipEmptyParts);
        QStringList processedLines;
        if (ok)
            *ok = true;

        for (QString &line : lines) {
            // Strip leading and trailing whitespace, including '\r' and '\n'
            line = line.trimmed();

            // Check if the line is a valid hexadecimal string
            bool ook;
            line.toUInt(&ook, 16); // Assume hexadecimal input
            if (ook) {
                // Add the valid hexadecimal string to the processed list
                processedLines.append(line);
            } else {
                QString message = QString(tr("Invalid hexadecimal input: %1")).arg(line);
                QMessageBox::warning(this, tr("Error"), message);
                if (ok)
                    *ok = false;
                break;
            }
        }

        return processedLines;
    }

    bool reverseLines()
    {
        return true;
    }

protected:
    void contextMenuEvent(QContextMenuEvent *event) override {
        // 创建自定义的菜单项
        QAction *stripAction = new QAction(tr("Strip"), this);
        connect(stripAction, &QAction::triggered, this, [this]() {
            bool ok = false;
            QStringList processedLines = stripLines(&ok);
            if (!ok)
                return;

            // Join the processed lines back into a single string with newlines
            QString processedText = processedLines.join('\n');

            // Write the processed text back to the QPlainTextEdit
            this->setPlainText(processedText);
        });

        QAction *reverseAction = new QAction(tr("Reverse Bytes"), this);
        connect(reverseAction, &QAction::triggered, this, [this]() {
            qDebug() << __func__ << "Reverse Bytes";
            reverseLines();
        });

        // 获取当前的上下文菜单
        QMenu *menu = this->createStandardContextMenu();

        // 如果菜单为空，则创建一个新的菜单
        if (!menu) {
            menu = new QMenu(this);
        } else {
            menu->addSeparator();
        }

        // 添加自定义的菜单项
        menu->addAction(stripAction);
        menu->addAction(reverseAction);

        // 在鼠标位置显示菜单
        menu->exec(event->globalPos());
        delete menu;
    }
};

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
