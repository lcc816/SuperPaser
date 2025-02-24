#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QRegularExpression>

class LineNumberArea;

class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);

    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth();

    void updateLineNumberArea(const QRect &rect, int dy);

    void highlightCurrentLine();

private:
    LineNumberArea *lineNumberArea;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextEditor *editor) : QWidget(editor), editor(editor) {}

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    TextEditor *editor;
};

class DataInputEdit : public TextEditor {
    Q_OBJECT

public:
    DataInputEdit(QWidget *parent = nullptr);

    QStringList stripLines(bool *ok);

    bool reverseLines();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
private:
    // 辅助函数：调换 DWORD 字符串的字节序
    QString reverseByteOrder(const QString &dwordStr);

    QAction *stripAction;  // 用于 Strip 操作的 Action
    QAction *reverseAction; // 用于 Reverse Bytes 操作的 Action
    QRegularExpression hexDwordRegex; // 将正则表达式定义为成员变量
};

#endif // TEXTEDITOR_H
