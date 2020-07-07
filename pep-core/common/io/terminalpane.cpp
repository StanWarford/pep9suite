//File: terminalpane.cpp
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

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
#include "terminalpane.h"
#include "ui_terminalpane.h"

#include <QFontDialog>
#include <QScrollBar>

#include "style/fonts.h"

TerminalPane::TerminalPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TerminalPane)
{
    ui->setupUi(this);

    waiting = false;

    connect(ui->plainTextEdit, &QPlainTextEdit::undoAvailable, this, &TerminalPane::undoAvailable);
    connect(ui->plainTextEdit, &QPlainTextEdit::redoAvailable, this, &TerminalPane::redoAvailable);

    ui->label->setFont(QFont(PepCore::labelFont, PepCore::labelFontSize));
    ui->plainTextEdit->setFont(QFont(PepCore::codeFont, PepCore::ioFontSize));

    installEventFilter(this);
    ui->plainTextEdit->installEventFilter(this);
}

TerminalPane::~TerminalPane()
{
    delete ui;
}

void TerminalPane::cancelWaiting()
{
    waiting = false;
    displayTerminal();
}

void TerminalPane::appendOutput(QString str)
{
    ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText().append(str));
    strokeString.append(str);
    ui->plainTextEdit->verticalScrollBar()->setValue(ui->plainTextEdit->verticalScrollBar()->maximum()); // Scroll to bottom
}

void TerminalPane::waitingForInput()
{
    waiting = true;
    displayTerminal();
    ui->plainTextEdit->setFocus();
    while(waiting) {
        QApplication::processEvents();
    }
}

void TerminalPane::clearTerminal()
{
    ui->plainTextEdit->clear();
    retString = "";
    strokeString = "";
}

void TerminalPane::highlightOnFocus()
{
    if (ui->plainTextEdit->hasFocus()) {
        ui->label->setAutoFillBackground(true);
    }
    else {
        ui->label->setAutoFillBackground(false);
    }
}

bool TerminalPane::hasFocus()
{
    return ui->plainTextEdit->hasFocus();
}

bool TerminalPane::isUndoable() const
{
    return false;
}

bool TerminalPane::isRedoable() const
{
    return false;
}

void TerminalPane::onFontChanged(QFont font)
{
    ui->plainTextEdit->setFont(font);
}

void TerminalPane::copy() const
{
    ui->plainTextEdit->copy();
}

void TerminalPane::cut()
{
    ui->plainTextEdit->cut();
}

void TerminalPane::paste()
{
    ui->plainTextEdit->paste();
}

void TerminalPane::undo()
{
    // No undo.
}

void TerminalPane::redo()
{
    // No redo.
}

void TerminalPane::displayTerminal()
{
    if(waiting) {
        ui->plainTextEdit->setPlainText(strokeString + retString + QString("_"));
    }
    else {
        ui->plainTextEdit->setPlainText(strokeString + retString);
    }
    ui->plainTextEdit->verticalScrollBar()->setValue(ui->plainTextEdit->verticalScrollBar()->maximum()); // Scroll to bottom
}

bool TerminalPane::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::KeyPress && ui->plainTextEdit->hasFocus() && waiting) {
        QKeyEvent *e = static_cast<QKeyEvent *>(event);
        if (e->key() == Qt::Key_Shift || e->key() == Qt::Key_Control ||
            e->key() == Qt::Key_Meta || e->key() == Qt::Key_Alt ||
            e->key() == Qt::Key_CapsLock || e->key() == Qt::Key_NumLock ||
            e->key() == Qt::Key_ScrollLock || e->key() == Qt::Key_Tab) {
            // skip
        }
        else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            retString.append('\n');
            strokeString.append(retString);
            waiting = false;
            emit inputReady(retString);
            retString = "";
            displayTerminal();
            emit inputReceived();
            return true;
        }
        else if (e->key() == Qt::Key_Backspace && !retString.isEmpty()) {
            retString.truncate(retString.length() - 1);
        }
        else {
            retString.append(e->text());
        }
        displayTerminal();
        return true;
    }
    return false;
}

void TerminalPane::mouseReleaseEvent(QMouseEvent *)
{
    ui->plainTextEdit->setFocus();
}

