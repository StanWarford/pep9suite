// File: sourcecodepane.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.
    
    Copyright (C) 2009  J. Stanley Warford, Pepperdine University

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

#ifndef ASMSOURCECODEPANE_H
#define ASMSOURCECODEPANE_H

#include <QWidget>
#include <QString>
#include <QList>
#include <QSettings>
#include <QTextEdit>
#include <QPlainTextEdit>
#include "isaasm.h" // For Code in QList<Code *> codeList;
#include "pepasmhighlighter.h" // For syntax highlighting
#include "enu.h"
#include <QSharedPointer>

namespace Ui {
    class SourceCodePane;
}

class AsmSourceBreakpointArea;
class AsmProgram;
/*
 * The breakpointable text editor must be a sublass of QPlainTextEdit because it must subclass resizeEvent to function properly.
 * So, this functionality cannot be implemented in the AsmSourceCodePane.
 */
class AsmSourceTextEdit : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit AsmSourceTextEdit(QWidget *parent = nullptr);
    virtual ~AsmSourceTextEdit() override;
    void breakpointAreaPaintEvent(QPaintEvent *event);
    int breakpointAreaWidth();
    void breakpointAreaMousePress(QMouseEvent* event);
    const QSet<quint16> getBreakpoints() const;
    bool lineHasBreakpoint(int line) const;

public slots:
    void onRemoveAllBreakpoints();
    void onBreakpointAdded(quint16 line);
    void onBreakpointRemoved(quint16 line);
    void onDarkModeChanged(bool darkMode);

private slots:
    void updateBreakpointAreaWidth(int newBlockCount);
    void updateBreakpointArea(const QRect &, int);
    void onTextChanged();
    void resizeEvent(QResizeEvent *evt) override;

signals:
    void breakpointAdded(quint16 line);
    void breakpointRemoved(quint16 line);

private:
    PepColors::Colors colors;
    AsmSourceBreakpointArea* breakpointArea;
    QSet<quint16> breakpoints;
    QMap<quint16, quint16> blockToInstr;
};

class AsmProgram;
class AsmProgramManager;
class MemorySection;
class AsmSourceCodePane : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(AsmSourceCodePane)
public:
    explicit AsmSourceCodePane(QWidget *parent = nullptr);
    void init(MemorySection* memorySection, AsmProgramManager* manager);
    virtual ~AsmSourceCodePane();

    bool assemble();
    // Pre: The source code pane contains a Pep/9 source program.
    // Post: If the program assembles correctly, true is returned, and codeList is populated
    // with the code objects. Otherwise false is returned and codeList is partially populated.
    // Post: Pep::symbolTable is populated with values not adjusted for .BURN.
    // Post: Pep::byteCount is the byte count for the object code not adjusted for .BURN.
    // Post: Pep::burnCount is the number of .BURN instructions encountered in the source program.

    QList<int> getObjectCode();
    // Pre: codeList is populated with code from a complete correct Pep/9 source program.
    // Post: objectCode is populated with the object code, one byte per entry, and returned.

    QStringList getAssemblerListingList();
    // Pre: codeList is populated with code from a complete correct Pep/9 source program.
    // Post: assemlberListingList is populated with the assembler listing.
    // Post: listingTraceList is populated with the object code.
    // Post: hasCheckBox is populated with the checkBox list that specifies whether a trace line can have a break point.
    // Post: assemblerListingList is returned.

    void adjustCodeList(int addressDelta);
    // Pre: codeList is populated with code from a complete correct Pep/9 source program.
    // Post: The memAddress field of each code object is incremented by addressDelta.

    bool assembleDefaultOs();
    // Post: the pep/8 operating system is installed into memory, and true is returned
    // If assembly fails, false is returned
    // This function should only be called on program startup once

    bool assembleOS(QStringList fileName);

    void removeErrorMessages();
    // Post: Searces for the string ";ERROR: " on each line and removes the end of the line.
    // Post: Searces for the string ";WARNING: " on each line and removes the end of the line.

    void appendMessageInSourceCodePaneAt(int lineNumber, QString message);
    // Post: Appends message to the end of line lineNumber in color color.

    void setSourceCodePaneText(QString string);
    // Post: Sets text in source code pane to string.

    void clearSourceCode();
    // Post: Clears the source code pane

    bool isModified();
    // Post: Returns true if the source code pane has been modified

    void setModifiedFalse();
    // Post: Sets isModified of the source code pane to false

    QString toPlainText();
    // Post: Contents of the source code pane are returned

    void setCurrentFile(QString string);
    // Post: Title bar of the window is set to "Object Code - 'string'", and the currentFile is updated

    const QFile& getCurrentFile() const;
    // Post: Returns the file associated with the pane.

    void highlightOnFocus();
    // Post: Highlights the label based on the label window color saved in the UI file

    bool hasFocus();
    // Post: returns if the pane has focus

    void undo();
    // Post: the last action in the text edit is undone

    void redo();
    // Post: the last undo in the text edit is redone

    bool isUndoable();
    // Returns the undoability of the text edit

    bool isRedoable();
    // Returns the redoability of the text edit

    void cut();
    // Post: selected text in the text edit is cut to the clipboard

    void copy();
    // Post: selected text in the text edit is copied to the clipboard

    void paste();
    // Post: selected text in the clipboard is pasted to the text edit

    void setReadOnly(bool b);
    // Post: the text edit's read only attribute is set to b

    void tab();

    QSharedPointer<AsmProgram> getAsmProgram();
    const QSharedPointer<AsmProgram> getAsmProgram() const;

    void writeSettings(QSettings& settings);
    void readSettings(QSettings& settings);


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
    Ui::SourceCodePane *ui;
    MemorySection* memorySection;
    AsmProgramManager* programManager;
    QSharedPointer<AsmProgram> currentProgram;
    QList<int> objectCode;
    QStringList assemblerListingList;

    PepASMHighlighter *pepHighlighter;
    QFile currentFile;
    void mouseReleaseEvent(QMouseEvent *);

    void mouseDoubleClickEvent(QMouseEvent *);

private slots:
    void setLabelToModified(bool modified);
    void onBreakpointAddedProp(quint16 line); //Propogate breakpointAdded(quint16) from AsmSourceTextEdit
    void onBreakpointRemovedProp(quint16 line); //Propogate breakpointRemoved(quint16) from AsmSourceTextEdit

signals:
    void undoAvailable(bool);
    void redoAvailable(bool);

    // Propogates event from AsmSourceTextEdit
    void breakpointAdded(quint16 line);
    // Propogates event from AsmSourceTextEdit
    void breakpointRemoved(quint16 line);
    void labelDoubleClicked(Enu::EPane pane);
};

class AsmSourceBreakpointArea : public QWidget
{
public:
    AsmSourceBreakpointArea(AsmSourceTextEdit *editor) : QWidget(editor) {
        this->editor = editor;
    }

    QSize sizeHint() const override {
        return QSize(/*editor->breakpointAreaWidth()*/0, 0);
    }
protected:
    void paintEvent(QPaintEvent *event) override {
        editor->breakpointAreaPaintEvent(event);
    }
    void mousePressEvent(QMouseEvent *event) override{
        editor->breakpointAreaMousePress(event);
    }
private:
    AsmSourceTextEdit *editor;
};

#endif // SOURCECODEPANE_H

