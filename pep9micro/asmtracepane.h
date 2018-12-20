/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2018  Matthew McRaven, Pepperdine University

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

#ifndef ASMTRACEPANE_H
#define ASMTRACEPANE_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QSet>

#include "colors.h"
#include "enu.h"
namespace Ui {
class AsmTracePane;
}

class AsmTraceBreakpointArea;
class AsmCode;
class PepASMHighlighter;
class AsmProgram;
class AsmProgramManager;
/*
 * The breakpointable text editor must be a sublass of QPlainTextEdit because it must subclass resizeEvent to function properly.
 * So, this functionality cannot be implemented in the AsmSourceCodePane.
 */
class AsmTraceTextEdit : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit AsmTraceTextEdit(QWidget *parent = nullptr);
    virtual ~AsmTraceTextEdit() override;
    void breakpointAreaPaintEvent(QPaintEvent *event);
    int breakpointAreaWidth();
    void breakpointAreaMousePress(QMouseEvent* event);
    const QSet<quint16> getBreakpoints() const;
    void setTextFromCode(QSharedPointer<AsmProgram> program);
    void setBreakpoints(QSet<quint16> memAddresses);

    void highlightActiveLine();
    void startSimulationView();
    void setActiveAddress(quint16 address);
    void clearSimulationView();
public slots:
    void onRemoveAllBreakpoints();
    void onBreakpointAdded(quint16 address);
    void onBreakpointRemoved(quint16 line);
    void onDarkModeChanged(bool darkMode);

private slots:
    void updateBreakpointAreaWidth(int newBlockCount);
    void updateBreakpointArea(const QRect &, int);
    void resizeEvent(QResizeEvent *evt) override;

signals:
    void breakpointAdded(quint16 line);
    void breakpointRemoved(quint16 line);

private:
    PepColors::Colors colors;
    AsmTraceBreakpointArea* breakpointArea;
    // Breakpoints are stored as line numbers, not memory addresses.
    QSet<quint16> breakpoints;
    QMap<quint16, quint16> lineToAddr, addrToLine;
    QMap<quint16, quint16> lineToIndex;
    QSharedPointer<AsmProgram> activeProgram;
    int activeAddress;
    bool updateHighlight;
};
class CPUControlSection;
class AsmTracePane : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(AsmTracePane)
public:
    explicit AsmTracePane(QWidget *parent = nullptr);
    void init(const CPUControlSection* controlSection, AsmProgramManager* programManager);
    virtual ~AsmTracePane();

    void clearSourceCode();
    // Post: Clears the source code pane

    void highlightOnFocus();
    // Post: Highlights the label based on the label window color saved in the UI file

    bool hasFocus();
    // Post: returns if the pane has focus


    void setProgram(QSharedPointer<AsmProgram> program);
    void setBreakpoints(QSet<quint16> memAddresses);
    const QSet<quint16> getBreakpoints() const;
    void writeSettings(QSettings& settings);
    void readSettings(QSettings& settings);

    void startSimulationView();
    void updateSimulationView();
    void clearSimulationView();

public slots:
    void onFontChanged(QFont font);
    void onDarkModeChanged(bool darkMode);
    // Forwards event to AsmSourceTextEdit
    void onRemoveAllBreakpoints();
    // Forwards event to AsmSourceTextEdit
    void onBreakpointAdded(quint16 line);
    // Forwards event to AsmSourceTextEdit
    void onBreakpointRemoved(quint16 line);

private:
    Ui::AsmTracePane *ui;
    QSharedPointer<AsmProgram> activeProgram;
    AsmProgramManager* programManager;
    const CPUControlSection* controlSection;
    PepASMHighlighter *pepHighlighter;
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);

private slots:
    void onBreakpointAddedProp(quint16 address); //Propogate breakpointAdded(quint16) from AsmSourceTextEdit
    void onBreakpointRemovedProp(quint16 address); //Propogate breakpointRemoved(quint16) from AsmSourceTextEdit

signals:
    // Propogates event from AsmSourceTextEdit
    void breakpointAdded(quint16 line);
    // Propogates event from AsmSourceTextEdit
    void breakpointRemoved(quint16 line);
};

class AsmTraceBreakpointArea : public QWidget
{
public:
    AsmTraceBreakpointArea(AsmTraceTextEdit *editor) : QWidget(editor) {
        this->editor = editor;
    }

    virtual ~AsmTraceBreakpointArea() override {}

    QSize sizeHint() const override {
        return QSize(editor->breakpointAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        editor->breakpointAreaPaintEvent(event);
    }

    void mousePressEvent(QMouseEvent *event) override{
        editor->breakpointAreaMousePress(event);
    }

private:
    AsmTraceTextEdit *editor;
};


#endif // ASMTRACEPANE_H
