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
#include "microcodeeditor.h"

#include "pep.h"

namespace Ui {
    class HelpDialog;
}
class CppHighlighter;
class PepASMHighlighter;
class PepMicroHighlighter;
class MicroHelpDialog : public QDialog {
    Q_OBJECT
public:
    MicroHelpDialog(QWidget *parent = 0);
    ~MicroHelpDialog();

    QString getCode(Enu::EPane &destPane, Enu::EPane &inputDest, QString &input);

    bool hasFocus();
    // Post: returns true if either of the text edits have focus

    void copy();
    // Post: the text edit that has focus has the copy() operation performed

    void setCopyButtonDisabled(bool b);
    // Post: the enabled state of the copy to source/object code button is set to b

    void selectItem(QString string);
protected:
    void changeEvent(QEvent *e);

private:
    Ui::HelpDialog *ui;

    PepMicroHighlighter *leftMicroHighlighter;
    PepASMHighlighter *leftPepHighlighter;
    CppHighlighter *rightCppHighlighter;
    PepASMHighlighter *rightPepHighlighter;
    static const int defaultHelpTreeWidth;
    enum Row {
        // Top level menu
        eWRITINGASM = 0,
        eDEBUGGINGASM,
        eTRAPS,
        eUSINGPEP9CPU,
        ePEP9REFERENCE,
        eMICROEXAMPLES,
        eASMEXAMPLES,
        eOS,
        eMICROIMPL,

        //Writing Assembler options
        eMACHINELANG = 0,
        EASMLANG,

        //Using Pep/9 CPU options
        eCPU = 0,
        eMICROCODE,
        eDEBUGGINGMICRO,

        // Microcode Examples,
        eMFig101 = 0,
        eMFig102,
        eMFig103,
        eMFig201,
        eMFig202,
        eMFig203,
        eMFig204,
        eMFig301,
        eMFig302,
        eMFig303,
        // Assembler Examples
        eFIG433 = 0,
        eFIG435,
        eFIG436,
        eFIG437,
        eFIG503,
        eFIG506,
        eFIG507,
        eFIG510,
        eFIG511,
        eFIG512,
        eFIG513,
        eFIG514a,
        eFIG514b,
        eFIG515,
        eFIG516,
        eFIG519,
        eFIG522,
        eFIG527,
        eFIG601,
        eFIG604,
        eFIG606,
        eFIG608,
        eFIG610,
        eFIG612,
        eFIG614,
        eFIG616,
        eFIG618,
        eFIG621,
        eFIG623,
        eFIG625,
        eFIG627, // Interactive input
        eFIG629, // Interactive input
        eFIG632,
        eFIG634,
        eFIG636,
        eFIG638,
        eFIG640, // Interactive input
        eFIG642,
        eFIG644,
        eFIG646,
        eFIG648,
    };
	
public slots:
    void onFontChanged(QFont font);
    void onDarkModeChanged(bool);
private slots:
    void onCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*);

signals:
    void copyToSourceClicked();

};

#endif // HELPDIALOG_H
