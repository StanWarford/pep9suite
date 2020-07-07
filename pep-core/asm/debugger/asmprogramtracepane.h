// File: asmprogramtracepane.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2018 J. Stanley Warford & Matthew McRaven, Pepperdine University

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
#include <QSettings>

#include "style/colors.h"

namespace Ui {
class AsmProgramTracePane;
}

class AsmTraceBreakpointArea;
class AsmCode;
class PepASMHighlighter;
class AsmProgram;
class AsmProgramManager;

class ACPUModel;
class APepVersion;
class AsmProgramTraceTextEdit;
class AsmProgramTracePane : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(AsmProgramTracePane)
public:
    explicit AsmProgramTracePane(QWidget *parent = nullptr);
    void init(QSharedPointer<const APepVersion> pep_version,
              QSharedPointer<const ACPUModel> controlSection, AsmProgramManager* programManager);
    virtual ~AsmProgramTracePane();

    // Optionally, add a label to the top of the memory dump pane displaying the
    // widget's name.
    // By default, this title is visible.
    void showTitleLabel(bool showLabel = true);

    void clearSourceCode();
    // Post: Clears the source code pane

    void highlightOnFocus();
    // Post: Highlights the label based on the label window color saved in the UI file

    bool hasFocus();
    // Post: returns if the pane has focus

    void rebuildHighlightingRules();
    // Post: highlighting rules for highlighter will be recreated from mnemonic maps

    void setProgram(QSharedPointer<AsmProgram> program);
    void setBreakpoints(QSet<quint16> memAddresses);
    const QSet<quint16> getBreakpoints() const;
    void writeSettings(QSettings& settings);
    void readSettings(QSettings& settings);

    void startSimulationView();
    void updateSimulationView();
    void clearSimulationView();

signals:
    // Propogates event from AsmSourceTextEdit
    void breakpointAdded(quint16 line);
    // Propogates event from AsmSourceTextEdit
    void breakpointRemoved(quint16 line);

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
    Ui::AsmProgramTracePane *ui;
    QSharedPointer<AsmProgram> activeProgram;
    AsmProgramManager* programManager;
    QSharedPointer<const APepVersion> pep_version;
    QSharedPointer<const ACPUModel> cpu;
    PepASMHighlighter *pepHighlighter;
    bool inDarkMode;
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);

private slots:
    void onBreakpointAddedProp(quint16 address); // Propogate breakpointAdded(quint16) from AsmSourceTextEdit
    void onBreakpointRemovedProp(quint16 address); // Propogate breakpointRemoved(quint16) from AsmSourceTextEdit
};

/*
 * The breakpointable text editor must be a sublass of QPlainTextEdit because it must subclass resizeEvent to function properly.
 * So, this functionality cannot be implemented in the AsmSourceCodePane.
 */
class AsmProgramTraceTextEdit : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit AsmProgramTraceTextEdit(QWidget *parent = nullptr);
    virtual ~AsmProgramTraceTextEdit() override;
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

class AsmTraceBreakpointArea : public QWidget
{
private:
    AsmProgramTraceTextEdit *editor;
public:
    AsmTraceBreakpointArea(AsmProgramTraceTextEdit *editor) : QWidget(editor) {
        this->editor = editor;
    }
    virtual ~AsmTraceBreakpointArea() override;

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
};
#endif // ASMTRACEPANE_H
