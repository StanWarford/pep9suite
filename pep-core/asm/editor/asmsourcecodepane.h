// File: asmsourcecodepane.h
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

#include <QList>
#include <QPlainTextEdit>
#include <QSettings>
#include <QSharedPointer>
#include <QString>
#include <QTextEdit>
#include <QWidget>

#include "assembler/asmprogram.h"
#include "highlight/pepasmhighlighter.h"
#include "pep/constants.h"

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
    QMap<quint16, quint16> blockToIndex;
};

class AsmProgram;
class AsmProgramManager;
class MainMemory;
class AsmSourceCodePane : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(AsmSourceCodePane)
public:
    explicit AsmSourceCodePane(QWidget *parent = nullptr);
    virtual ~AsmSourceCodePane() override;
    void init();

    void displayAssemblerOutput(AsmOutput output);

    void removeErrorMessages();
    // Post: Searces for the string ";ERROR: " on each line and removes the end of the line.
    // Post: Searces for the string ";WARNING: " on each line and removes the end of the line.

    void appendMessagesInSourceCodePane(QList<QPair<int, QString> > errList);
    // Post: For each <line #, message> pair, on the line => append error message.
    // Post: Returns immediately if given an empty error list.

    void setSourceCodePaneText(QString string);
    // Post: Sets text in source code pane to string.

    void clearSourceCode();
    // Post: Clears the source code pane

    bool isModified();
    // Post: Returns true if the source code pane has been modified

    void setModified(bool modified);
    // Post: Sets isModified of the source code pane to modified

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

    void rebuildHighlightingRules();
    // Post: highlighting rules for highlighter will be recreated from mnemonic maps

    void tab();
    void backTab();

    void writeSettings(QSettings& settings);
    void readSettings(QSettings& settings);
    bool eventFilter(QObject *, QEvent * event) override;

public slots:
    void onFontChanged(QFont font);
    void onDarkModeChanged(bool darkMode);
    // Forwards event to AsmSourceTextEdit
    void onRemoveAllBreakpoints();
    // Forwards event to AsmSourceTextEdit
    void onBreakpointAdded(quint16 address);
    // Forwards event to AsmSourceTextEdit
    void onBreakpointRemoved(quint16 address);

private:
    Ui::SourceCodePane *ui;
    bool inDarkMode;
    QSharedPointer<const AsmProgram> current_program;
    QMap<quint16, quint16> addressToIndex;
    PepASMHighlighter *pepHighlighter;
    QFile currentFile;
    void mouseReleaseEvent(QMouseEvent *) override;

    void mouseDoubleClickEvent(QMouseEvent *) override;

private slots:
    void setLabelToModified(bool modified);
    void onBreakpointAddedProp(quint16 line); //Propogate breakpointAdded(quint16) from AsmSourceTextEdit
    void onBreakpointRemovedProp(quint16 line); //Propogate breakpointRemoved(quint16) from AsmSourceTextEdit

signals:
    void undoAvailable(bool);
    void redoAvailable(bool);

    // Propogates event from AsmSourceTextEdit
    void breakpointAdded(quint16 address);
    // Propogates event from AsmSourceTextEdit
    void breakpointRemoved(quint16 address);
    void labelDoubleClicked(PepCore::EPane pane);
};

class AsmSourceBreakpointArea : public QWidget
{
public:
    AsmSourceBreakpointArea(AsmSourceTextEdit *editor) : QWidget(editor) {
        this->editor = editor;
    }
    virtual ~AsmSourceBreakpointArea() override;
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

