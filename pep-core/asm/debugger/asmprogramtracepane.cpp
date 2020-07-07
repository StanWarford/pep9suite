// File: asmprogramtracepane.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2019  J. Stanley Warford & Matthew McRaven, Pepperdine University

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

#include "asmprogramtracepane.h"
#include "ui_asmprogramtracepane.h"

#include <QPainter>

#include "assembler/asmcode.h"
#include "assembler/asmprogram.h"
#include "assembler/asmprogrammanager.h"
#include "cpu/acpumodel.h"
#include "highlight/pepasmhighlighter.h"
#include "pep/apepversion.h"
#include "style/fonts.h"

AsmProgramTracePane::AsmProgramTracePane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AsmProgramTracePane), inDarkMode(false)
{
    ui->setupUi(this);
    ui->label->setFont(QFont(PepCore::labelFont, PepCore::labelFontSize));
    pepHighlighter = new PepASMHighlighter(PepColors::lightMode, ui->tracePaneTextEdit->document());
    ui->tracePaneTextEdit->setFont(QFont(PepCore::codeFont, PepCore::codeFontSize));
    connect(((AsmProgramTraceTextEdit*)ui->tracePaneTextEdit), &AsmProgramTraceTextEdit::breakpointAdded, this, &AsmProgramTracePane::onBreakpointAddedProp);
    connect(((AsmProgramTraceTextEdit*)ui->tracePaneTextEdit), &AsmProgramTraceTextEdit::breakpointRemoved, this, &AsmProgramTracePane::onBreakpointRemovedProp);
}

void AsmProgramTracePane::init(QSharedPointer<const APepVersion> pep_version,
                               QSharedPointer<const ACPUModel> controlSection,
                               AsmProgramManager* programManager)
{
    this->pep_version = pep_version;
    this->cpu = controlSection;
    this->programManager = programManager;
}

AsmProgramTracePane::~AsmProgramTracePane()
{
    delete ui;
}

void AsmProgramTracePane::showTitleLabel(bool showLabel)
{
    ui->label->setVisible(showLabel);
}

void AsmProgramTracePane::clearSourceCode()
{
    ui->tracePaneTextEdit->clear();
    activeProgram.clear();
}

void AsmProgramTracePane::highlightOnFocus()
{
    if (ui->tracePaneTextEdit->hasFocus()) {
        ui->label->setAutoFillBackground(true);
    }
    else {
        ui->label->setAutoFillBackground(false);
    }
}

bool AsmProgramTracePane::hasFocus()
{
    return ui->tracePaneTextEdit->hasFocus();
}

void AsmProgramTracePane::rebuildHighlightingRules()
{
    if(inDarkMode) pepHighlighter->rebuildHighlightingRules(PepColors::darkMode);
    else pepHighlighter->rebuildHighlightingRules(PepColors::lightMode);
    pepHighlighter->rehighlight();
}

void AsmProgramTracePane::setProgram(QSharedPointer<AsmProgram> program)
{
    this->activeProgram = program;
    ui->tracePaneTextEdit->setTextFromCode(program);
}

void AsmProgramTracePane::setBreakpoints(QSet<quint16> memAddresses)
{
    ui->tracePaneTextEdit->setBreakpoints(memAddresses);
}

const QSet<quint16> AsmProgramTracePane::getBreakpoints() const
{
    return ui->tracePaneTextEdit->getBreakpoints();
}

void AsmProgramTracePane::writeSettings(QSettings &)
{

}

void AsmProgramTracePane::readSettings(QSettings &)
{

}

void AsmProgramTracePane::startSimulationView()
{
    ui->tracePaneTextEdit->startSimulationView();
}

void AsmProgramTracePane::updateSimulationView()
{
    auto pc_reg = pep_version->get_global_register_number(APepVersion::global_registers::PC);
    quint16 pc = cpu->getCPURegWordStart(pc_reg);
    if(activeProgram.data() != programManager->getProgramAt(pc)) {

        if(programManager->getOperatingSystem()->getProgramBounds().first <= pc &&
                programManager->getOperatingSystem()->getProgramBounds().second >= pc) {
            setProgram(programManager->getOperatingSystem());
        }
        else {
            setProgram(programManager->getUserProgram());
        }
    }
    ui->tracePaneTextEdit->setActiveAddress(pc);
    ui->tracePaneTextEdit->highlightActiveLine();
    ui->tracePaneTextEdit->centerCursor();
}

void AsmProgramTracePane::clearSimulationView()
{
    ui->tracePaneTextEdit->clearSimulationView();
}

void AsmProgramTracePane::onFontChanged(QFont font)
{
    ui->tracePaneTextEdit->setFont(font);
}

void AsmProgramTracePane::onDarkModeChanged(bool darkMode)
{
    inDarkMode = darkMode;
    if(darkMode) pepHighlighter->rebuildHighlightingRules(PepColors::darkMode);
    else pepHighlighter->rebuildHighlightingRules(PepColors::lightMode);
    ((AsmProgramTraceTextEdit*)ui->tracePaneTextEdit)->onDarkModeChanged(darkMode);
    pepHighlighter->rehighlight();
}

void AsmProgramTracePane::onRemoveAllBreakpoints()
{
    ((AsmProgramTraceTextEdit*)ui->tracePaneTextEdit)->onRemoveAllBreakpoints();
}

void AsmProgramTracePane::onBreakpointAdded(quint16 address)
{
    ((AsmProgramTraceTextEdit*)ui->tracePaneTextEdit)->onBreakpointAdded(address);
}

void AsmProgramTracePane::onBreakpointRemoved(quint16 address)
{
    ((AsmProgramTraceTextEdit*)ui->tracePaneTextEdit)->onBreakpointRemoved(address);
}

void AsmProgramTracePane::mouseReleaseEvent(QMouseEvent *)
{

}

void AsmProgramTracePane::mouseDoubleClickEvent(QMouseEvent *)
{

}

void AsmProgramTracePane::onBreakpointAddedProp(quint16 address)
{
    emit breakpointAdded(address);
}

void AsmProgramTracePane::onBreakpointRemovedProp(quint16 address)
{
    emit breakpointRemoved(address);
}

AsmProgramTraceTextEdit::AsmProgramTraceTextEdit(QWidget *parent): QPlainTextEdit(parent), colors(PepColors::lightMode),
    activeProgram(), updateHighlight(false)
{
    breakpointArea = new AsmTraceBreakpointArea(this);
    connect(this, &QPlainTextEdit::blockCountChanged, this, &AsmProgramTraceTextEdit::updateBreakpointAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &AsmProgramTraceTextEdit::updateBreakpointArea);
}

AsmProgramTraceTextEdit::~AsmProgramTraceTextEdit()
{
    delete breakpointArea;
}

void AsmProgramTraceTextEdit::breakpointAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(breakpointArea);
    painter.fillRect(event->rect(), colors.lineAreaBackground); // light grey
    QTextBlock block;
    int blockNumber, top, bottom;
    block = firstVisibleBlock();
    blockNumber = block.blockNumber();
    top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    bottom = top + (int) blockBoundingRect(block).height();
    // Store painter's previous Antialiasing hint so it can be restored at the end
    bool antialias = painter.renderHints() & QPainter::Antialiasing;
    painter.setRenderHint(QPainter::Antialiasing, true);
    while (block.isValid() && top < event->rect().bottom()) {
        // If the current block is in the repaint zone
        if (block.isVisible() && bottom >= event->rect().top()) {
            // And if it has a breakpoint
            if(lineToAddr.contains(blockNumber) && breakpoints.contains(blockNumber)) {
                painter.setPen(PepColors::transparent);
                painter.setBrush(colors.combCircuitRed);
                painter.drawEllipse(QPoint(fontMetrics().height()/2, top+fontMetrics().height()/2),
                                    fontMetrics().height()/2 -1, fontMetrics().height()/2 -1);
            }
        }
        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
    painter.setRenderHint(QPainter::Antialiasing, antialias);
}

int AsmProgramTraceTextEdit::breakpointAreaWidth()
{
    return 5 + fontMetrics().height();
}

void AsmProgramTraceTextEdit::breakpointAreaMousePress(QMouseEvent *event)
{
    QTextBlock block;
    int blockNumber, top, bottom, breakpointAddress;
    block = firstVisibleBlock();
    blockNumber = block.blockNumber();
    top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    bottom = top + (int) blockBoundingRect(block).height();
    // For each visible block (usually a line), iterate until the current block contains the location of the mouse event.
    while (block.isValid() && top <= event->pos().y()) {
        if (event->pos().y()>=top && event->pos().y()<=bottom) {
            // Check if the clicked line is a code line
            if(lineToAddr.contains(blockNumber)) {
                breakpointAddress = lineToAddr[blockNumber];
                // If the clicked code line has a breakpoint, remove it.
                if(breakpoints.contains(blockNumber)) {
                    // Breakpoint remove will be propogated back via events
                    emit breakpointRemoved(breakpointAddress);
                }
                // Otherwise add a breakpoint.
                else {
                    // Breakpoint addition will be propogated back via events
                    breakpoints.insert(blockNumber);
                    emit breakpointAdded(breakpointAddress);
                }
            }
            break;
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
    update();
}

const QSet<quint16> AsmProgramTraceTextEdit::getBreakpoints() const
{
    return breakpoints;
}

void AsmProgramTraceTextEdit::setTextFromCode(QSharedPointer<AsmProgram> code)
{
    activeProgram = code;
    QStringList finList, traceList;
    int visualIt = 0;
    const AsmCode* codePtr;
    breakpoints.clear();
    for(int it = 0; it < code->numberOfLines(); it++)
    {
        codePtr = code->getCodeAtIndex(it);
        traceList = codePtr->getAssemblerListing().split("\n");
        if(dynamic_cast<const UnaryInstruction*>(codePtr) != nullptr ||
                dynamic_cast<const NonUnaryInstruction*>(codePtr) != nullptr)
        {
            lineToAddr[visualIt] = codePtr->getMemoryAddress();
            addrToLine[codePtr->getMemoryAddress()] = visualIt;
            lineToIndex[visualIt] = it;
            if(codePtr->hasBreakpoint()) {
                onBreakpointAdded(codePtr->getMemoryAddress());

            }
        }
        finList.append(traceList);
        visualIt += traceList.length();
    }
    setPlainText(finList.join("\n"));
    update();
}

void AsmProgramTraceTextEdit::setBreakpoints(QSet<quint16> memAddresses)
{
    breakpoints = memAddresses;
}

void AsmProgramTraceTextEdit::highlightActiveLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if(updateHighlight && addrToLine.contains(activeAddress)) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor(56, 117, 215)); // dark blue
        selection.format.setForeground(Qt::white);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        QTextCursor cursor = QTextCursor(document());
        cursor.setPosition(0);
        // Iterate over blocks, because lines do not work correctly with line wrap
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, addrToLine[activeAddress]);
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
        this->setTextCursor(cursor);
        // Since the cursor (potentially) spans multiple lines, center it, since
        // ensureCursorVisible() might only show the bottom
        // line of a multi-row selection.
        selection.cursor = cursor;
        extraSelections.append(selection);
    }
    centerCursor();
    setExtraSelections(extraSelections);
}

void AsmProgramTraceTextEdit::startSimulationView()
{
    updateHighlight = true;
}

void AsmProgramTraceTextEdit::setActiveAddress(quint16 address)
{
    activeAddress = address;
}

void AsmProgramTraceTextEdit::clearSimulationView()
{
    updateHighlight = false;
    QList<QTextEdit::ExtraSelection> extraSelections;
    setExtraSelections(extraSelections);
}

void AsmProgramTraceTextEdit::onRemoveAllBreakpoints()
{
    breakpoints.clear();
    update();
}

void AsmProgramTraceTextEdit::onBreakpointAdded(quint16 address)
{
    if(addrToLine.contains(address)) {
        breakpoints.insert(addrToLine[address]);
        activeProgram->getCodeAtIndex(lineToIndex[addrToLine[address]])->setBreakpoint(true);
    }
}

void AsmProgramTraceTextEdit::onBreakpointRemoved(quint16 address)
{
    if(addrToLine.contains(address)) {
        breakpoints.remove(addrToLine[address]);
        activeProgram->getCodeAtIndex(lineToIndex[addrToLine[address]])->setBreakpoint(false);
    }
}

void AsmProgramTraceTextEdit::onDarkModeChanged(bool darkMode)
{
    if(darkMode) colors = PepColors::darkMode;
    else colors = PepColors::lightMode;
}

void AsmProgramTraceTextEdit::updateBreakpointAreaWidth(int )
{
    setViewportMargins(breakpointAreaWidth(), 0, 0, 0);
}

void AsmProgramTraceTextEdit::updateBreakpointArea(const QRect &rect, int dy)
{
    if (dy)
        breakpointArea->scroll(0, dy);
    else
        breakpointArea->update(0, rect.y(), breakpointArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateBreakpointAreaWidth(0);
}

void AsmProgramTraceTextEdit::resizeEvent(QResizeEvent *evt)
{
    QPlainTextEdit::resizeEvent(evt);

    QRect cr = contentsRect();
    breakpointArea->setGeometry(QRect(cr.left(), cr.top(), breakpointAreaWidth(), cr.height()));
}

AsmTraceBreakpointArea::~AsmTraceBreakpointArea()
{

}
