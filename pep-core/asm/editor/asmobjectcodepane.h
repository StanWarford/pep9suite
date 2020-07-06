// File: asmobjectcodepane.h
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

#ifndef ASMOBJECTCODEPANE_H
#define ASMOBJECTCODEPANE_H

#include <QFile>
#include <QWidget>

#include "pep/enu.h"

namespace Ui {
    class ObjectCodePane;
}

class AsmObjectCodePane : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(AsmObjectCodePane)
public:
    explicit AsmObjectCodePane(QWidget *parent = nullptr);
    virtual ~AsmObjectCodePane();

    void setObjectCode(QList<int> objectCode);
    // Post: Sets text in source code pane.

    void setObjectCode(QVector<quint8> objectCode);
    // Post: Sets text in source code pane.

    void setObjectCodePaneText(QString string);
    // Post: Sets text in source code pane to string.

    bool getObjectCode(QList<int> &objectCodeList);
    // Pre: The object code pane contains object code
    // Post: If the object code is syntactically correct, true is returned, and
    // &objectCodeList contains the object code, one byte per integer.
    // Otherwise, false is returned.

    void clearObjectCode();
    // Post: Clears the source code pane

    bool isModified();
    // Post: Returns true if the source code pane has been modified

    void setModified(bool modified);
    // Post: Sets isModified of the source code pane to modified

    QString toPlainText();
    // Post: Contents of the source code pane are returned

    void setCurrentFile(QString string);
    // Post: Title bar of the window is set to "Object Code - 'string'"

    const QFile& getCurrentFile() const;
    // Post: Returns the file (if any) associated with the window

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

public slots:
    void onFontChanged(QFont);
private:
    Ui::ObjectCodePane *ui;
    QFile currentFile;

    void mouseReleaseEvent(QMouseEvent *);

    void mouseDoubleClickEvent(QMouseEvent *);

private slots:
    void setLabelToModified(bool modified);
signals:
    void undoAvailable(bool);
    void redoAvailable(bool);

    void labelDoubleClicked(Enu::EPane pane);
};

#endif // OBJECTCODEPANE_H
