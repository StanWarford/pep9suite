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
#include "microcodeprogram.h"
#include <limits.h>
#include "interfacemccpu.h"
#include <QScrollBar>

MicrocodeEditor::MicrocodeEditor(bool highlightCurrentLine, bool isReadOnly, QWidget *parent): QPlainTextEdit(parent), cpu(nullptr), colors(&PepColors::lightMode)
{
    highlightCurLine = highlightCurrentLine;

    lineNumberArea = new LineNumberArea(this);

    setReadOnly(isReadOnly);

    connect(this, &QPlainTextEdit::blockCountChanged, this, &MicrocodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &MicrocodeEditor::updateLineNumberArea);
    connect(this, &MicrocodeEditor::textChanged, this, &MicrocodeEditor::onTextChanged);
    //    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    connect(this, SIGNAL(cursorPositionChanged()), lineNumberArea, SLOT(update()));

    updateLineNumberAreaWidth(0);

}

void MicrocodeEditor::init(QSharedPointer<InterfaceMCCPU> cpu)
{
    this->cpu = cpu;
}

int MicrocodeEditor::lineNumberAreaWidth()
{
    int max = qMax(1, blockCount());
    // Calculate the number of digits in the row number.
    // Floor(Log10) returns the number of digits in number (minus one)
    int digits = 1 + static_cast<int>(log10(max));

    int space = 4 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space + fontMetrics().height();
}

void MicrocodeEditor::lineAreaMousePress(QMouseEvent *event)
{
    QTextBlock block;
    int blockNumber, top, bottom;
    quint16 lineNumber;
    block = firstVisibleBlock();
    blockNumber = block.blockNumber();
    top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    bottom = top + static_cast<int>(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->pos().y()) {
        if (event->pos().y()>=top && event->pos().y()<=bottom) {
            if(blockToCycle.contains(blockNumber)) {
                lineNumber = blockToCycle[blockNumber];
                if(breakpoints.contains(lineNumber)) {
                    breakpoints.remove(lineNumber);
                    emit breakpointRemoved(lineNumber);
                }
                else {
                    breakpoints.insert(lineNumber);
                    emit breakpointAdded(lineNumber);
                }
            }
            break;
        }
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
    update();
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

void MicrocodeEditor::onTextChanged()
{
    QMap<int, quint16> oldToNew, old = blockToCycle;
    blockToCycle.clear();
    quint16 cycleNumber = 1;
    QStringList sourceCodeList = toPlainText().split('\n');

    for (int i = 0; i < sourceCodeList.size(); i++) {
        // The following regular expression will find a match in any non-microcode line
        if (QRegExp("^\\s*//|^\\s*$|^\\s*unitpre|^\\s*unitpost", Qt::CaseInsensitive).indexIn(sourceCodeList.at(i)) == 0) {
        }
        else {
            blockToCycle.insert(i, cycleNumber++);
        }
    }
    // A deletion of text could may make high line number disappear,
    // so any line numbers that have a breakpoint but no longer exist have their
    // breakpoint removed.
    QSet<quint16> toRemove;
    for(auto x : breakpoints) {
        if (blockToCycle.key(x, std::numeric_limits<quint16>::max()) == std::numeric_limits<quint16>::max()) toRemove.insert(x);
    }
    breakpoints.subtract(toRemove);
    update();
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

        // Fetches the micro-address of the currently executing instruction.
        // Address are 0-indexed, but block numbers are 1 indexed.
        int activeLineNum = cpu->getMicrocodeLineNumber() + 1;
        // Convert a micro-address to a logical text block
        int activeBlockNum = blockToCycle.key(activeLineNum, std::numeric_limits<quint16>::max());
        // Iterate over blocks, because lines do not work correctly with line wrap
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, activeBlockNum);
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
        this->setTextCursor(cursor);
        centerCursor();
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

    QTextBlock startBlock = doc->findBlock(start);
    QTextBlock endBlock = doc->findBlock(end);

    if (end > start && endBlock.position() == end) {
        --end;
        endBlock = endBlock.previous();
    }

    bool doCStyleUncomment = false;
    bool doCppStyleUncomment = false;

    bool hasSelection = cursor.hasSelection();

    if (hasSelection) {
        QString startText = startBlock.text();
        int startPos = start - startBlock.position();
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

const QSet<quint16> MicrocodeEditor::getBreakpoints() const
{
    return breakpoints;
}

void MicrocodeEditor::onDarkModeChanged(bool darkMode)
{
    if(darkMode) colors = &PepColors::darkMode;
    else colors = &PepColors::lightMode;
}

void MicrocodeEditor::onRemoveAllBreakpoints()
{
    breakpoints.clear();
    update();
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
    // Draw the background rectangle for the line area
    painter.fillRect(event->rect(), colors->lineAreaBackground); // light grey
    // Since the text cursor may span multiple lines, use multiple cursors to determine
    // the span of the selected text.
    QTextCursor start = textCursor(), end = textCursor();
    start.setPosition(textCursor().selectionStart(), QTextCursor::MoveAnchor);
    end.setPosition(textCursor().selectionEnd(), QTextCursor::MoveAnchor);
    QTextBlock block;
    int blockNumber, top, bottom;
    // Display the cycle numbers
    block = firstVisibleBlock();
    blockNumber = block.blockNumber();
    top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    bottom = top + static_cast<int>(blockBoundingRect(block).height());
    bool antialias = painter.renderHints() & QPainter::Antialiasing;
    while (block.isValid() && top < event->rect().bottom()) {
        // If the selected block is on the screen, and it is within the repaint area
        if (block.isVisible() && bottom >= event->rect().top()) {
            // Handle the line highlighting
            // If the cursor is highlight a visible line, and the current block number is the target block number.
            if(highlightCurLine && start.block().isVisible() && start.blockNumber() == block.blockNumber()) {
                painter.setPen(PepColors::transparent);
                painter.setBrush(colors->lineAreaHighlight);
                // Querry the target block for how many lines it spans.
                painter.drawRect(-1, top, lineNumberArea->width(), start.block().lineCount() * fontMetrics().height());
            }
            // If the current block is a valid μ-operation, and is an μ-op with a breakpoint => draw a red circle
            if(blockToCycle.contains(blockNumber) && breakpoints.contains(blockToCycle[blockNumber])) {
                painter.setPen(PepColors::transparent);
                painter.setBrush(colors->combCircuitRed);
                painter.setRenderHint(QPainter::Antialiasing, true);
                // Draw a circular breakpoint that is scaled to the height of the text
                painter.drawEllipse(QPoint(fontMetrics().height()/2, top+fontMetrics().height()/2),
                                    fontMetrics().height()/2 - 1, fontMetrics().height()/2 - 1);
                // Undo antialias mode, so as not to accidentally antialias text
                painter.setRenderHint(QPainter::Antialiasing, antialias);
            }
            // Determine the line number text, or the empty string otherwise
            QString number = !blockToCycle.contains(blockNumber) ? QString("") : QString::number(blockToCycle[blockNumber]);
            painter.setPen(colors->lineAreaText); // grey
            painter.setFont(QFont(Pep::codeFont, Pep::codeFontSize));
            // Render the line number
            painter.drawText(-1, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight | Qt::AlignVCenter, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void LineNumberArea::mousePressEvent(QMouseEvent *event)
{
    microcodeEditor->lineAreaMousePress(event);
}
