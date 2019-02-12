// File: assemblerlistingpane.cpp
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

#include <QScrollBar>
#include <QFontDialog>
#include "asmlistingpane.h"
#include "ui_asmlistingpane.h"
#include "pep.h"
#include "pepasmhighlighter.h"
#include <QMouseEvent>
#include "colors.h"
#include "symbolentry.h"
#include "symboltable.h"
#include "symbolvalue.h"
AsmListingPane::AsmListingPane(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::AsmListingPane), inDarkMode(false), currentFile()
{
    ui->setupUi(this);

    pepHighlighter = new PepASMHighlighter(PepColors::lightMode, ui->textEdit->document());

    ui->label->setFont(QFont(Pep::labelFont, Pep::labelFontSize));
    ui->textEdit->setFont(QFont(Pep::codeFont, Pep::codeFontSize));
}

AsmListingPane::~AsmListingPane()
{
    delete ui;
}

void AsmListingPane::setAssemblerListing(QStringList assemblerListingList, QSharedPointer<SymbolTable> symTable) {
    clearAssemblerListing();
    ui->textEdit->append("-------------------------------------------------------------------------------");
    ui->textEdit->append("      Object");
    ui->textEdit->append("Addr  code   Symbol   Mnemon  Operand     Comment");
    ui->textEdit->append("-------------------------------------------------------------------------------");
    ui->textEdit->append(assemblerListingList.join("\n"));
    ui->textEdit->append("-------------------------------------------------------------------------------");
    QList<QSharedPointer<SymbolEntry>> list = symTable->getSymbolEntries();
    if (list.size() > 0) {
        ui->textEdit->append("");
        ui->textEdit->append("");
        ui->textEdit->append("Symbol table");
        ui->textEdit->append("--------------------------------------");
        ui->textEdit->append("Symbol    Value        Symbol    Value");
        ui->textEdit->append("--------------------------------------");
        QString symbolTableLine = "";
        QString hexString;
        for(QSharedPointer<SymbolEntry> item : list) {
            hexString = QString("%1").arg(item->getValue(), 4, 16, QLatin1Char('0')).toUpper();
            if (symbolTableLine.length() == 0) {
                symbolTableLine = QString("%1%2").arg(item->getName(), -10).arg(hexString, -13);
            }
            else {
                symbolTableLine.append(QString("%1%2").arg(item->getName(), -10).arg(hexString, -4));
                ui->textEdit->append(symbolTableLine);
                symbolTableLine = "";
            }
        }
        if (symbolTableLine.length() > 0) {
            ui->textEdit->append(symbolTableLine);
        }
        ui->textEdit->append("--------------------------------------");
    }
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->minimum());
}

void AsmListingPane::clearAssemblerListing()
{
    ui->textEdit->clear();
}

bool AsmListingPane::isModified()
{
    return ui->textEdit->document()->isModified();
}

QString AsmListingPane::toPlainText()
{
    return ui->textEdit->toPlainText();
}

void AsmListingPane::setCurrentFile(QString string)
{
    if (!string.isEmpty()) {
        currentFile.setFileName(string);
        ui->label->setText("Assembler Listing - " + QFileInfo(currentFile).fileName());
    }
    else {
        currentFile.setFileName("");
        ui->label->setText("Assembler Listing - untitled.pepl");
    }
}

const QFile &AsmListingPane::getCurrentFile() const
{
    return currentFile;
}

void AsmListingPane::highlightOnFocus()
{
    if (ui->textEdit->hasFocus()) {
        ui->label->setAutoFillBackground(true);
    }
    else {
        ui->label->setAutoFillBackground(false);
    }
}

bool AsmListingPane::hasFocus()
{
    return ui->textEdit->hasFocus();
}

void AsmListingPane::copy()
{
    ui->textEdit->copy();
}

void AsmListingPane::setFocus()
{
    ui->textEdit->setFocus();
}

bool AsmListingPane::isEmpty()
{
    return ui->textEdit->toPlainText() == "";
}

void AsmListingPane::onFontChanged(QFont font)
{
    ui->textEdit->setFont(font);
}

void AsmListingPane::onDarkModeChanged(bool darkMode)
{
    inDarkMode = darkMode;
    if(darkMode) pepHighlighter->rebuildHighlightingRules(PepColors::darkMode);
    else pepHighlighter->rebuildHighlightingRules(PepColors::lightMode);
    pepHighlighter->rehighlight();
}

void AsmListingPane::mouseReleaseEvent(QMouseEvent *)
{
    ui->textEdit->setFocus();
}

void AsmListingPane::mouseDoubleClickEvent(QMouseEvent *)
{
    emit labelDoubleClicked(Enu::EPane::EListing);
}
