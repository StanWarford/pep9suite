// File: helpdialog.h
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

#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>

#include "highlight/cpphighlighter.h"
#include "pep/constants.h"

#include "pep9asmhighlighter.h"
namespace Ui {
    class HelpDialog;
}

class AsmHelpDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(AsmHelpDialog)
public:
    explicit AsmHelpDialog(QWidget *parent = nullptr);
    virtual ~AsmHelpDialog();

    void selectItem(QString string);

    QString getCode(PepCore::EPane &destPane, PepCore::EPane &inputDest, QString &input);

    bool hasFocus();
    // Post: returns true if either of the text edits have focus

    void copy();
    // Post: the text edit that has focus has the copy() operation performed

    void setCopyButtonDisabled(bool b);
    // Post: the enabled state of the copy to source/object code button is set to b

private:
    Ui::HelpDialog *ui;

    Pep9ASMHighlighter *leftHighlighter;
    CppHighlighter *rightCppHighlighter;
    Pep9ASMHighlighter *rightPepHighlighter;
    static const int defaultHelpTreeWidth;
    enum Row {
        eWRITING = 0,
        eDEBUGGING = 1,
        eTRAP =2,
        //eCACHE = 3,
        eREFERENCE = 3,
        eEXAMPLES = 4,
        ePROBLEMS = 5,
        eOS = 6,

        eMACHINE = 0,
        eASSEMBLY = 1,

        eFIG433 = 0,
        eFIG435 = 1,
        eFIG436 = 2,
        eFIG437 = 3,
        eFIG503  = 4,
        eFIG506  = 5,
        eFIG507  = 6,
        eFIG510 = 7,
        eFIG511 = 8,
        eFIG512 = 9,
        eFIG513 = 10,
        eFIG514a= 11,
        eFIG514b= 12,
        eFIG515 = 13,
        eFIG516 = 14,
        eFIG519 = 15,
        eFIG522 = 16,
        eFIG527 = 17,
        eFIG601 = 18,
        eFIG604 = 19,
        eFIG606 = 20,
        eFIG608 = 21,
        eFIG610 = 22,
        eFIG612 = 23,
        eFIG614 = 24,
        eFIG616 = 25,
        eFIG618 = 26,
        eFIG621 = 27,
        eFIG623 = 28,
        eFIG625 = 29,
        eFIG627 = 30, // Interactive input
        eFIG629 = 31, // Interactive input
        eFIG632 = 32,
        eFIG634 = 33,
        eFIG636 = 34,
        eFIG638 = 35,
        eFIG640 = 36, // Interactive input
        eFIG642 = 37,
        eFIG644 = 38,
        eFIG646 = 39,
        eFIG648 = 40,

        eEXER804 = 0,
        ePROB826 = 1,
        ePROB827 = 2,
        ePROB828 = 3,
        ePROB829 = 4,
        ePROB830 = 5,
        ePROB831 = 6,
        ePROB832 = 7
    };

private slots:
    void onCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*);
public slots:
    void onFontChanged(QFont font);
    void onDarkModeChanged(bool);
signals:
    void copyToSourceClicked();
};

#endif // HELPDIALOG_H
