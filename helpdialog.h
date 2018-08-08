// File: helpdialog.h
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
#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "pepmicrohighlighter.h"
#include "microcodeeditor.h"

#include "pep.h"

namespace Ui {
    class HelpDialog;
}

class HelpDialog : public QDialog {
    Q_OBJECT
public:
    HelpDialog(QWidget *parent = 0);
    ~HelpDialog();

    QString getExampleText();
    Enu::CPUType getExamplesModel();

    void copy();
    // Post: the text edit that has focus has the copy() operation performed

    bool hasFocus();
    // Returns if the webview or the textedit has focus

    void selectItem(QString string);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::HelpDialog *ui;

    MicrocodeEditor *microcodeEditor;
    PepMicroHighlighter *leftHighlighter;

    enum Row {
        eUSINGPEP9CPU = 0,
        ePEP9REFERENCE = 1,
        eTWOBYTEBUSEXAMPLES = 2,
        eTWOBYTEBUSPROBLEMS = 3,

        eCPU = 0,
        eMICROCODE = 1,
        eDEBUGGING = 2,

        eFIG1220 = 0,
        eFIG1221 = 1,
        eFIG1223 = 2,

        ePR1234A = 0,
        ePR1234B = 1,
        ePR1235A = 2,
        ePR1235B = 3,
        ePR1235C = 4,
        ePR1235D = 5,
        ePR1235E = 6,
        ePR1235F = 7,
        ePR1235G = 8,
        ePR1235H = 9,
        ePR1235I = 10,
        ePR1235J = 11,
        ePR1235K = 12,
        ePR1235L = 13,
        ePR1236A = 14,
        ePR1236B = 15,
        ePR1236C = 16,
        ePR1236D = 17,
        ePR1236E = 18,
        ePR1236F = 19,
    };

public slots:
    void onFontChanged(QFont font);
    void onDarkModeChanged(bool);
private slots:
    void onCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*);

signals:
    void copyToMicrocodeClicked();

};

#endif // HELPDIALOG_H
