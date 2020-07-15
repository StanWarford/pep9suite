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

#include <QObject>
#include <QPlainTextEdit>
#include <QWidget>


#include "editor/microcodeeditor.h"
#include "pep/highlight.h"

namespace Ui {
    class MicrocodePane;
}

class APepVersion;
class MicrocodeProgram;
class SymbolTable;

class MicrocodePane : public QWidget {
    Q_OBJECT
public:
    MicrocodePane(QWidget *parent = nullptr);
    ~MicrocodePane();

    void init(QSharedPointer<const APepVersion> pep_version, QSharedPointer<InterfaceMCCPU> cpu, bool fullCtrlSection);

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
    QString toPlainText();

    void useFullCtrlSection(bool fullCtrlSection);

    QSet<quint16> getBreakpoints() const;

public slots:
    void onFontChanged(QFont font);
    void onDarkModeChanged(bool darkMode);
    void onRemoveAllBreakpoints();
    void onCPUTypeChanged(PepCore::CPUType type);
signals:
    void breakpointAdded(quint16 line);
    void breakpointRemoved(quint16 line);
protected:
    void changeEvent(QEvent *e);

private:
    Ui::MicrocodePane *ui;
    QSharedPointer<const APepVersion> pep_version;
    bool inDarkMode, fullCtrlSection;
    MicroHighlighter *highlighter;
    MicrocodeEditor *editor;
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
