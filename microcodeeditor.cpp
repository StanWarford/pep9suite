// File: microcodeeditor.cpp
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2010  J. Stanley Warford, Pepperdine University

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtGui>

#include "microcodeeditor.h"
#include "pep.h"
#include "cpucontrolsection.h"
#include "microcodeprogram.h"
#include <QDebug>

MicrocodeEditor::MicrocodeEditor(QWidget *parent, bool highlightCurrentLine, bool isReadOnly) : QPlainTextEdit(parent), colors(&PepColors::lightMode)
{

    highlightCurLine = highlightCurrentLine;

    lineNumberArea = new LineNumberArea(this);

    setReadOnly(isReadOnly);

    connect(this, &QPlainTextEdit::blockCountChanged, this, &MicrocodeEditor::updateLineNumberAreaWidth);
    connect(this, SIGNAL(textChanged()), this, SLOT(repaint()));
    connect(this, &QPlainTextEdit::updateRequest, this, &MicrocodeEditor::updateLineNumberArea);
    //    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    connect(this, SIGNAL(cursorPositionChanged()), lineNumberArea, SLOT(update()));

    updateLineNumberAreaWidth(0);
}

int MicrocodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 4 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void MicrocodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void MicrocodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void MicrocodeEditor::highlightSimulatedLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        selection.format.setBackground(QColor(56, 117, 215)); // dark blue
        selection.format.setForeground(Qt::white);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        QTextCursor cursor = QTextCursor(document());
        cursor.setPosition(0);
        CPUControlSection* inst = CPUControlSection::getInstance();
        for (int i = 0; i < inst->getProgram()->codeLineToProgramLine(inst->getLineNumber()); i++) {
            cursor.movePosition(QTextCursor::NextBlock);
        }

        // this chunk moves the cursor down and scrolls the text edit to it:
        cursor.clearSelection();
        this->setTextCursor(cursor);
        ensureCursorVisible();

        // move the cursor to the end of the line and select it so we can add it to the extra selections
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor); // select to end of line

        selection.cursor = cursor;
        extraSelections.append(selection);

    }

    setExtraSelections(extraSelections);
}

void MicrocodeEditor::clearSimulationView()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    setExtraSelections(extraSelections);
}

void MicrocodeEditor::unCommentSelection()
{
    QTextCursor cursor = textCursor();
    QTextDocument *doc = cursor.document();
    cursor.beginEditBlock();

    int pos = cursor.position();
    int anchor = cursor.anchor();
    int start = qMin(anchor, pos);
    int end = qMax(anchor, pos);
//    bool anchorIsStart = (anchor == start);

    QTextBlock startBlock = doc->findBlock(start);
    QTextBlock endBlock = doc->findBlock(end);

    if (end > start && endBlock.position() == end) {
        --end;
        endBlock = endBlock.previous();
    }

    bool doCStyleUncomment = false;
    //bool doCStyleComment = false;
    bool doCppStyleUncomment = false;

    bool hasSelection = cursor.hasSelection();

    if (hasSelection) {
        QString startText = startBlock.text();
        int startPos = start - startBlock.position();
        //bool hasLeadingCharacters = !startText.left(startPos).trimmed().isEmpty();
        if ((startPos >= 2
            && startText.at(startPos-2) == QLatin1Char('/')
             && startText.at(startPos-1) == QLatin1Char('*'))) {
            startPos -= 2;
            start -= 2;
        }

        bool hasSelStart = (startPos < startText.length() - 2
                            && startText.at(startPos) == QLatin1Char('/')
                            && startText.at(startPos+1) == QLatin1Char('*'));


        QString endText = endBlock.text();
        int endPos = end - endBlock.position();
        //bool hasTrailingCharacters = !endText.left(endPos).remove(QLatin1String("//")).trimmed().isEmpty()
        //                             && !endText.mid(endPos).trimmed().isEmpty();
        if ((endPos <= endText.length() - 2
            && endText.at(endPos) == QLatin1Char('*')
             && endText.at(endPos+1) == QLatin1Char('/'))) {
            endPos += 2;
            end += 2;
        }

        bool hasSelEnd = (endPos >= 2
                          && endText.at(endPos-2) == QLatin1Char('*')
                          && endText.at(endPos-1) == QLatin1Char('/'));

        doCStyleUncomment = hasSelStart && hasSelEnd;
        // doCStyleComment = !doCStyleUncomment && (hasLeadingCharacters || hasTrailingCharacters);
    }

    if (doCStyleUncomment) {
        cursor.setPosition(end);
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, 2);
        cursor.removeSelectedText();
        cursor.setPosition(start);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2);
        cursor.removeSelectedText();
// commented because we don't want to use block commenting:
//    } else if (doCStyleComment) {
//        cursor.setPosition(end);
//        cursor.insertText(QLatin1String("*/"));
//        cursor.setPosition(start);
//        cursor.insertText(QLatin1String("/*"));
    } else {
        endBlock = endBlock.next();
        doCppStyleUncomment = true;
        for (QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            QString text = block.text();
            if (!text.trimmed().startsWith(QLatin1String("//"))) {
                doCppStyleUncomment = false;
                break;
            }
        }
        for (QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            QString text = block.text();
            if ((text.trimmed() == "")) {
                doCppStyleUncomment = true;
            }
            else if (!text.trimmed().startsWith(QLatin1String("//"))) {
                doCppStyleUncomment = false;
            }
            else {
                doCppStyleUncomment = true;
            }
            if (doCppStyleUncomment) {
                QString text = block.text();
                int i = 0;
                while (i < text.size() - 1) {
                    if (text.at(i) == QLatin1Char('/')
                        && text.at(i + 1) == QLatin1Char('/')) {
                        cursor.setPosition(block.position() + i);
                        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2);
                        cursor.removeSelectedText();
                        break;
                    }
                    if (!text.at(i).isSpace())
                        break;
                    ++i;
                }
            } else {
                cursor.setPosition(block.position());
                cursor.insertText(QLatin1String("//"));
            }
        }
    }

    // adjust selection when commenting out
//    if (hasSelection && !doCStyleUncomment && !doCppStyleUncomment) {
//        cursor = textCursor();
//        if (!doCStyleComment)
//            start = startBlock.position(); // move the double slashes into the selection
//        int lastSelPos = anchorIsStart ? cursor.position() : cursor.anchor();
//        if (anchorIsStart) {
////            cursor.setPosition(start);
//            cursor.setPosition(lastSelPos, QTextCursor::MoveAnchor);
//        } else {
//            cursor.setPosition(lastSelPos);
////            cursor.setPosition(start, QTextCursor::MoveAnchor);
//        }
//        setTextCursor(cursor);
//    }
    cursor.clearSelection();
    setTextCursor(cursor);

    cursor.endEditBlock();
}

void MicrocodeEditor::readSettings(QSettings &settings)
{
    settings.beginGroup("MicrocodeEditor");
    settings.endGroup();
}

void MicrocodeEditor::writeSettings(QSettings &settings)
{
    settings.beginGroup("MicrocodeEditor");
    settings.endGroup();
}

void MicrocodeEditor::onDarkModeChanged(bool darkMode)
{
    if(darkMode)colors = &PepColors::darkMode;
    else colors = &PepColors::lightMode;
}

void MicrocodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void MicrocodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(),colors->backgroundFill); // light grey
    QTextBlock block;
    int blockNumber;
    int top;
    int bottom;

    // Highlight the current line containing the cursor
    if (highlightCurLine && textCursor().block().isVisible()) {
        block = firstVisibleBlock();
        blockNumber = block.blockNumber();
        top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
        bottom = top + (int) blockBoundingRect(block).height();
        while (blockNumber != textCursor().block().blockNumber() && block.isValid()) {
            block = block.next();
            top = bottom;
            bottom = top + (int) blockBoundingRect(block).height();
            ++blockNumber;
        }
        if (block.isValid()) {
            painter.setPen(QColor(232, 232, 232));
            painter.setBrush(colors->lineAreaHighlight);
            painter.drawRect(-1, top, lineNumberArea->width(), fontMetrics().height());
        }
    }

    // Display the cycle numbers
    block = firstVisibleBlock();
    blockNumber = block.blockNumber();
    top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    bottom = top + (int) blockBoundingRect(block).height();
    int cycleNumber = 1;
    QList<int> blockToCycleNumber;
    QStringList sourceCodeList = toPlainText().split('\n');
    for (int i = 0; i < sourceCodeList.size(); i++) {
        if (QRegExp("^\\s*//|^\\s*$|^\\s*unitpre|^\\s*unitpost", Qt::CaseInsensitive).indexIn(sourceCodeList.at(i)) == 0) {
            blockToCycleNumber << -1;
        }
        else {
            blockToCycleNumber << cycleNumber++;
        }
    }
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = blockToCycleNumber.at(blockNumber) == -1 ? QString("") : QString::number(blockToCycleNumber.at(blockNumber));
            painter.setPen(colors->lineAreaText); // grey
            painter.setFont(QFont(Pep::codeFont,Pep::codeFontSize));
            painter.drawText(-1, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
//    lineNumberArea->update();
}
