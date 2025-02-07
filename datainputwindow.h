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
    DataInputEdit(QWidget *parent = nullptr)
        : QPlainTextEdit(parent)
        , hexDwordRegex(R"(0x[0-9a-fA-F]{8})")
    {
        // 设置等宽字体
        QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        setFont(fixedFont);

        // 初始化 stripAction
        stripAction = new QAction(tr("Strip"), this);
        stripAction->setShortcut(QKeySequence("Ctrl+S"));
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

        // 初始化 reverseAction
        reverseAction = new QAction(tr("Reverse Bytes"), this);
        reverseAction->setShortcut(QKeySequence("Ctrl+R"));
        connect(reverseAction, &QAction::triggered, this, [this]() {
            qDebug() << __func__ << "Reverse Bytes";
            reverseLines();
        });

        // 将 Action 添加到窗口的 Action 列表中
        this->addAction(stripAction);
        this->addAction(reverseAction);
    }

    QStringList stripLines(bool *ok)
    {
        QString inputText = this->toPlainText();
        QStringList lines = inputText.split('\n', Qt::SkipEmptyParts);
        QStringList processedLines;
        if (ok)
            *ok = true;

        for (QString &line : lines) {
            // 去除行首和行尾的空白字符
            line = line.trimmed();

            // 查找所有匹配的十六进制 DWORD 字符串
            QRegularExpressionMatchIterator matchIterator = hexDwordRegex.globalMatch(line);
            QString lastMatch;

            // 遍历所有匹配项，保留最后一个匹配项
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                lastMatch = match.captured(0); // 获取匹配的字符串
            }

            if (!lastMatch.isEmpty()) {
                // 如果找到有效的 DWORD 字符串，则添加到结果列表
                processedLines.append(lastMatch);
            } else {
                // 如果没有找到有效的 DWORD 字符串，则报错
                QString message = QString(tr("No valid hexadecimal DWORD found in line: %1")).arg(line);
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
        QString inputText = this->toPlainText();
        QStringList lines = inputText.split('\n', Qt::SkipEmptyParts);
        bool hasValidDword = false; // 标记是否存在合法的 DWORD 字符串
        int totalLines = lines.size(); // 总行数
        int convertedLines = 0; // 成功转换的行数

        for (int i = 0; i < lines.size(); ++i) {
            QString &line = lines[i];
            line = line.trimmed(); // 去除行首和行尾的空白字符

            // 检查当前行是否是一个合法的 DWORD 字符串
            QRegularExpressionMatch match = hexDwordRegex.match(line);
            if (match.hasMatch()) {
                QString dwordStr = match.captured(0); // 获取匹配的 DWORD 字符串
                QString reversedDwordStr = reverseByteOrder(dwordStr); // 调换字节序
                line.replace(dwordStr, reversedDwordStr); // 替换原字符串
                hasValidDword = true; // 标记存在合法的 DWORD 字符串
                convertedLines++; // 增加成功转换的行数
            }
        }

        if (hasValidDword) {
            // 如果有合法的 DWORD 字符串被调换，则更新文本内容
            this->setPlainText(lines.join('\n'));

            // 弹出消息框，显示总行数和成功转换的行数
            QString message = QString(tr("Converted %1 line(s) of total %2 line(s)"))
                                .arg(convertedLines).arg(totalLines);
            QMessageBox::information(this, tr("Conversion Result"), message);

            return true;
        } else {
            // 如果没有合法的 DWORD 字符串，弹出消息框提示
            QMessageBox::information(this, tr("Conversion Result"), tr("No valid DWORD strings found."));
            return false;
        }
    }

protected:
    void contextMenuEvent(QContextMenuEvent *event) override {
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
private:
    // 辅助函数：调换 DWORD 字符串的字节序
    QString reverseByteOrder(const QString &dwordStr)
    {
        // 去掉 "0x" 前缀
        QString hexStr = dwordStr.mid(2);

        // 将字符串分为 8 个字符（4 个字节），并调换字节序
        QString reversedHexStr;
        for (int i = hexStr.length() - 2; i >= 0; i -= 2) {
            reversedHexStr.append(hexStr.midRef(i, 2));
        }

        // 添加 "0x" 前缀并返回
        return "0x" + reversedHexStr;
    }

    QAction *stripAction;  // 用于 Strip 操作的 Action
    QAction *reverseAction; // 用于 Reverse Bytes 操作的 Action
    QRegularExpression hexDwordRegex; // 将正则表达式定义为成员变量
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
