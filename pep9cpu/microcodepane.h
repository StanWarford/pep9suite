// File: microcodepane.h
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
#ifndef MICROCODEPANE_H
#define MICROCODEPANE_H

#include <QWidget>
#include "pepmicrohighlighter.h"
#include "microcodeeditor.h"
#include "microasm.h" // For Code in QList<Code> codeList;

#include <QPlainTextEdit>
#include <QObject>

namespace Ui {
    class MicrocodePane;
}
class SymbolTable;
class MicrocodeProgram;
class NewCPUDataSection;
class MicrocodePane : public QWidget {
    Q_OBJECT
public:
    MicrocodePane(QWidget *parent = 0);
    ~MicrocodePane();

    void init(QSharedPointer<InterfaceMCCPU> cpu, QSharedPointer<NewCPUDataSection> dataSection,  QSharedPointer<AMemoryDevice> memDevice, bool fullCtrlSection);
    void initCPUModelState();

    bool microAssemble();
    // Pre: The source code pane contains a Pep/9 microcode program.
    // Post: If the program assembles correctly, true is returned, and codeList is populated
    // with the code structs. Otherwise false is returned and codeList is partially populated.

    QSharedPointer<MicrocodeProgram> getMicrocodeProgram();

    void removeErrorMessages();
    // Post: Searces for the string "// ERROR: " on each line and removes the end of the line.

    void appendMessageInSourceCodePaneAt(int lineNumber, QString message);
    // Post: Appends message to the end of line lineNumber.

    void setMicrocode(QString microcode);
    // Post: Set

    QString getMicrocodeText();
    // Post: returns the text of the editor

    void highlightOnFocus();
    // Highlights the label based on the label window color saved in the UI file

    bool hasFocus();
    // Returns if the text edit has focus

    void setCurrentFile(QString string);
    // Post: Title bar of the window is set to "string"

    const QFile& getCurrentFile() const;
    // Post: Returns the file (if any) associated with the window

    void cut();
    void copy();
    void paste();
    void undo();
    void redo();

    bool isUndoable();
    bool isRedoable();

    void setReadOnly(bool ro);

    bool isModified();
    void setModifiedFalse();

    void updateSimulationView();
    void clearSimulationView();

    void unCommentSelection();

    void readSettings(QSettings &settings);
    void writeSettings(QSettings &settings);
    MicrocodeEditor* getEditor();
    void asHTML(QString& html) const;

    void useFullCtrlSection(bool fullCtrlSection);

public slots:
    void onFontChanged(QFont font);
    void onDarkModeChanged(bool darkMode);
    void onRemoveAllBreakpoints();
    void onCPUTypeChanged(Enu::CPUType type);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MicrocodePane *ui;
    QSharedPointer<NewCPUDataSection> dataSection;
    MicroAsm *microASM;
    bool inDarkMode, fullCtrlSection;
    QSharedPointer<SymbolTable> symbolTable;
    PepMicroHighlighter *highlighter;
    MicrocodeEditor *editor;
    QSharedPointer<MicrocodeProgram> program;
    QFile currentFile;
private slots:
    void setLabelToModified(bool modified);
    void onBreakpointAdded(quint16 line);
    void onBreakpointRemoved(quint16 line);

signals:
    void undoAvailable(bool);
    void redoAvailable(bool);

};

#endif // MICROCODEPANE_H
