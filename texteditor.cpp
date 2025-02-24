#include <QPainter>
#include <QTextBlock>
#include <QDebug>
#include <QMessageBox>
#include <QMenu>
#include "texteditor.h"

TextEditor::TextEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &TextEditor::blockCountChanged, this, &TextEditor::updateLineNumberAreaWidth);
    connect(this, &TextEditor::updateRequest, this, &TextEditor::updateLineNumberArea);
    connect(this, &TextEditor::cursorPositionChanged, this, &TextEditor::highlightCurrentLine);

    updateLineNumberAreaWidth();
    highlightCurrentLine();
}

void TextEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

int TextEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void TextEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEditor::updateLineNumberAreaWidth()
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth();
}

void TextEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(160);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    editor->lineNumberAreaPaintEvent(event);
}

DataInputEdit::DataInputEdit(QWidget *parent)
    : TextEditor(parent)
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

QStringList DataInputEdit::stripLines(bool *ok)
{
    QString inputText = this->toPlainText();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList lines = inputText.split('\n', Qt::SkipEmptyParts);
#else
    QStringList lines = inputText.split('\n', QString::SkipEmptyParts);
#endif
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

bool DataInputEdit::reverseLines()
{
    QString inputText = this->toPlainText();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList lines = inputText.split('\n', Qt::SkipEmptyParts);
#else
    QStringList lines = inputText.split('\n', QString::SkipEmptyParts);
#endif
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

void DataInputEdit::contextMenuEvent(QContextMenuEvent *event)
{
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

QString DataInputEdit::reverseByteOrder(const QString &dwordStr)
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
