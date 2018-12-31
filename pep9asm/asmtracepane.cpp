#include "asmtracepane.h"
#include "ui_asmtracepane.h"
#include "asmcode.h"
#include "pepasmhighlighter.h"
#include "asmprogram.h"
#include <QPainter>
#include "acpumodel.h"
#include "asmprogrammanager.h"
#include "asmprogram.h"
AsmTracePane::AsmTracePane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AsmTracePane)
{
    ui->setupUi(this);
    pepHighlighter = new PepASMHighlighter(PepColors::lightMode, ui->tracePaneTextEdit->document());
    ui->tracePaneTextEdit->setFont(QFont(Pep::codeFont, Pep::codeFontSize));
    connect(((AsmTraceTextEdit*)ui->tracePaneTextEdit), &AsmTraceTextEdit::breakpointAdded, this, &AsmTracePane::onBreakpointAddedProp);
    connect(((AsmTraceTextEdit*)ui->tracePaneTextEdit), &AsmTraceTextEdit::breakpointRemoved, this, &AsmTracePane::onBreakpointRemovedProp);
}

void AsmTracePane::init(QSharedPointer<const ACPUModel> controlSection, AsmProgramManager* programManager)
{
    this->cpu = controlSection;
    this->programManager = programManager;
}

AsmTracePane::~AsmTracePane()
{
    delete ui;
}

void AsmTracePane::clearSourceCode()
{
    ui->tracePaneTextEdit->clear();
    activeProgram.clear();
}

void AsmTracePane::highlightOnFocus()
{
    if (ui->tracePaneTextEdit->hasFocus()) {
        ui->tracePaneTextEdit->setAutoFillBackground(true);
    }
    else {
        ui->tracePaneTextEdit->setAutoFillBackground(false);
    }
}

bool AsmTracePane::hasFocus()
{
    return ui->tracePaneTextEdit->hasFocus();
}

void AsmTracePane::setProgram(QSharedPointer<AsmProgram> program)
{
    this->activeProgram = program;
    ui->tracePaneTextEdit->setTextFromCode(program);
}

void AsmTracePane::setBreakpoints(QSet<quint16> memAddresses)
{
    ui->tracePaneTextEdit->setBreakpoints(memAddresses);
}

const QSet<quint16> AsmTracePane::getBreakpoints() const
{
    return ui->tracePaneTextEdit->getBreakpoints();
}

void AsmTracePane::writeSettings(QSettings &)
{
}

void AsmTracePane::readSettings(QSettings &)
{

}

void AsmTracePane::startSimulationView()
{
    ui->tracePaneTextEdit->startSimulationView();
}

void AsmTracePane::updateSimulationView()
{
    quint16 pc = cpu->getCPURegWordStart(Enu::CPURegisters::PC);
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
}

void AsmTracePane::clearSimulationView()
{
    ui->tracePaneTextEdit->clearSimulationView();
}

void AsmTracePane::onFontChanged(QFont font)
{
    ui->tracePaneTextEdit->setFont(font);
}

void AsmTracePane::onDarkModeChanged(bool darkMode)
{
    if(darkMode) pepHighlighter->rebuildHighlightingRules(PepColors::darkMode);
    else pepHighlighter->rebuildHighlightingRules(PepColors::lightMode);
    ((AsmTraceTextEdit*)ui->tracePaneTextEdit)->onDarkModeChanged(darkMode);
    pepHighlighter->rehighlight();
}

void AsmTracePane::onRemoveAllBreakpoints()
{
    ((AsmTraceTextEdit*)ui->tracePaneTextEdit)->onRemoveAllBreakpoints();
}

void AsmTracePane::onBreakpointAdded(quint16 address)
{
    ((AsmTraceTextEdit*)ui->tracePaneTextEdit)->onBreakpointAdded(address);
}

void AsmTracePane::onBreakpointRemoved(quint16 address)
{
    ((AsmTraceTextEdit*)ui->tracePaneTextEdit)->onBreakpointRemoved(address);
}

void AsmTracePane::mouseReleaseEvent(QMouseEvent *)
{

}

void AsmTracePane::mouseDoubleClickEvent(QMouseEvent *)
{

}

void AsmTracePane::onBreakpointAddedProp(quint16 address)
{
    emit breakpointAdded(address);
}

void AsmTracePane::onBreakpointRemovedProp(quint16 address)
{
    emit breakpointRemoved(address);
}

AsmTraceTextEdit::AsmTraceTextEdit(QWidget *parent): QPlainTextEdit(parent), colors(PepColors::lightMode), updateHighlight(false), activeProgram()
{
    breakpointArea = new AsmTraceBreakpointArea(this);
    connect(this, &QPlainTextEdit::blockCountChanged, this, &AsmTraceTextEdit::updateBreakpointAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &AsmTraceTextEdit::updateBreakpointArea);
}

AsmTraceTextEdit::~AsmTraceTextEdit()
{
    delete breakpointArea;
}

void AsmTraceTextEdit::breakpointAreaPaintEvent(QPaintEvent *event)
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

int AsmTraceTextEdit::breakpointAreaWidth()
{
    return 5 + fontMetrics().height();
}

void AsmTraceTextEdit::breakpointAreaMousePress(QMouseEvent *event)
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

const QSet<quint16> AsmTraceTextEdit::getBreakpoints() const
{
    return breakpoints;
}

void AsmTraceTextEdit::setTextFromCode(QSharedPointer<AsmProgram> code)
{
    activeProgram = code;
    QStringList finList, traceList;
    int visualIt = 0;
    const AsmCode* codePtr;
    breakpoints.clear();
    for(int it = 0; it < code->numberOfLines(); it++)
    {
        codePtr = code->getCodeAtIndex(it);
        traceList = codePtr->getAssemblerTrace().split("\n");
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

void AsmTraceTextEdit::setBreakpoints(QSet<quint16> memAddresses)
{
    breakpoints = memAddresses;
}

void AsmTraceTextEdit::highlightActiveLine()
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
        this->setTextCursor(cursor);
        ensureCursorVisible();
        selection.cursor = cursor;
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}

void AsmTraceTextEdit::startSimulationView()
{
    updateHighlight = true;
}

void AsmTraceTextEdit::setActiveAddress(quint16 address)
{
    activeAddress = address;
}

void AsmTraceTextEdit::clearSimulationView()
{
    updateHighlight = false;
    QList<QTextEdit::ExtraSelection> extraSelections;
    setExtraSelections(extraSelections);
}

void AsmTraceTextEdit::onRemoveAllBreakpoints()
{
    breakpoints.clear();
    update();
}

void AsmTraceTextEdit::onBreakpointAdded(quint16 address)
{
    if(addrToLine.contains(address)) {
        breakpoints.insert(addrToLine[address]);
        activeProgram->getCodeAtIndex(lineToIndex[addrToLine[address]])->setBreakpoint(true);
    }
}

void AsmTraceTextEdit::onBreakpointRemoved(quint16 address)
{
    if(addrToLine.contains(address)) {
        breakpoints.remove(addrToLine[address]);
        activeProgram->getCodeAtIndex(lineToIndex[addrToLine[address]])->setBreakpoint(false);
    }
}

void AsmTraceTextEdit::onDarkModeChanged(bool darkMode)
{
    if(darkMode) colors = PepColors::darkMode;
    else colors = PepColors::lightMode;
}

void AsmTraceTextEdit::updateBreakpointAreaWidth(int )
{
    setViewportMargins(breakpointAreaWidth(), 0, 0, 0);
}

void AsmTraceTextEdit::updateBreakpointArea(const QRect &rect, int dy)
{
    if (dy)
        breakpointArea->scroll(0, dy);
    else
        breakpointArea->update(0, rect.y(), breakpointArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateBreakpointAreaWidth(0);
}

void AsmTraceTextEdit::resizeEvent(QResizeEvent *evt)
{
    QPlainTextEdit::resizeEvent(evt);

    QRect cr = contentsRect();
    breakpointArea->setGeometry(QRect(cr.left(), cr.top(), breakpointAreaWidth(), cr.height()));
}
