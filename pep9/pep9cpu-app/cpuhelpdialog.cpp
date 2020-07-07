// File: cpuhelpdialog.cpp
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

#include "cpuhelpdialog.h"
#include "ui_cpuhelpdialog.h"

#include <QClipboard>
#include <QDebug>

#include "pep/pep.h"
#include "style/colors.h"
#include "style/fonts.h"

CPUHelpDialog::CPUHelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpDialog)
{
    ui->setupUi(this);

    connect(ui->helpTreeWidget, &QTreeWidget::currentItemChanged, this,
            &CPUHelpDialog::onCurrentItemChanged);
    // Forward the helpCopyToMicrocodeButton_clicked() signal from this to the main window
    connect(ui->helpCopyToMicrocodeButton, &QAbstractButton::clicked, this, &CPUHelpDialog::copyToMicrocodeClicked);

    ui->helpSplitter->widget(1)->hide();
    ui->helpTreeWidget->expandAll();
    ui->helpTreeWidget->itemAt(0,0)->setSelected(true);

    //delete ui->helpTextEdit;

    //ui->helpTextEdit = new ui->helpTextEdit(false, true, this);
    //ui->verticalLayout->insertWidget(0, ui->helpTextEdit);

    leftHighlighter = new PepMicroHighlighter(Enu::CPUType::OneByteDataBus, false, PepColors::lightMode, ui->helpTextEdit->document());
    leftHighlighter->forceAllFeatures(true);
    ui->helpTreeWidget->setFont(QFont(PepCore::labelFont, PepCore::labelFontSize));

    ui->helpTextEdit->setFont(QFont(PepCore::codeFont, PepCore::codeFontSize));
}

CPUHelpDialog::~CPUHelpDialog()
{
    delete ui;
}

void CPUHelpDialog::selectItem(QString string) {
    QTreeWidgetItemIterator it(ui->helpTreeWidget);
    while (*it) {
        if ((*it)->text(0) == string) {
            (*it)->setSelected(true);
            ui->helpTreeWidget->setCurrentItem((*it));
        } else {
            (*it)->setSelected(false);
        }
        ++it;
    }
}

QString CPUHelpDialog::getExampleText()
{
    return ui->helpTextEdit->toPlainText();
}

Enu::CPUType CPUHelpDialog::getExamplesModel()
{
    // Is this a subcategory?
    bool isHelpSubCat = ui->helpTreeWidget->currentIndex().parent().isValid();
    // Parent row (if it has a parent, -1 else)
    int parentRow = ui->helpTreeWidget->currentIndex().parent().row();
    // Row (if it has a parent, this is the child row)
    int row = ui->helpTreeWidget->currentIndex().row();

    if ((!isHelpSubCat && row == eONEBYTEBUSEXAMPLES) || parentRow == eONEBYTEBUSEXAMPLES) {
        return Enu::OneByteDataBus;
    }
    else if ((!isHelpSubCat && row == eTWOBYTEBUSEXAMPLES) || parentRow == eTWOBYTEBUSEXAMPLES) {
        return Enu::TwoByteDataBus;
    }
    else if ((!isHelpSubCat && row == eONEBYTEBUSPROBLEMS) || parentRow == eONEBYTEBUSPROBLEMS) {
        return Enu::OneByteDataBus;
    }
    else if ((!isHelpSubCat && row == eTWOBYTEBUSPROBLEMS) || parentRow == eTWOBYTEBUSPROBLEMS) {
        return Enu::TwoByteDataBus;
    }
    else{ //This case should never be reached, but is needed to silence a compiler warning
        return Enu::OneByteDataBus; //Default to one byte data bus.
    }
}

void CPUHelpDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void CPUHelpDialog::onFontChanged(QFont font)
{
    ui->helpTextEdit->setFont(font);
}

void CPUHelpDialog::onDarkModeChanged(bool darkMode)
{
    if(darkMode)
    {
        leftHighlighter->rebuildHighlightingRules(PepColors::darkMode);
    }
    else
    {
        leftHighlighter->rebuildHighlightingRules(PepColors::lightMode);
    }
    leftHighlighter->rehighlight();
}

void CPUHelpDialog::onCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*) {
    // Is this a subcategory?
    bool isHelpSubCat = ui->helpTreeWidget->currentIndex().parent().isValid();
    // Parent row (if it has a parent, -1 else)
    int parentRow = ui->helpTreeWidget->currentIndex().parent().row();
    // Row (if it has a parent, this is the child row)
    int row = ui->helpTreeWidget->currentIndex().row();

    //    qDebug() << "Selected: " << ui->helpTreeWidget->currentIndex();

    if (!isHelpSubCat && row == eUSINGPEP9CPU) { // Using Pep/9 CPU
        ui->helpSplitter->widget(1)->hide();
        ui->helpTopWebView->show();
        ui->helpTopWebView->load(QUrl("qrc:/help-cpu/usingpep9cpu.html"));
    }
    else if (isHelpSubCat && parentRow == eUSINGPEP9CPU && row == eCPU) { // Interactive use
        ui->helpSplitter->widget(1)->hide();
        ui->helpTopWebView->show();
        ui->helpTopWebView->load(QUrl("qrc:/help-cpu/cpu.html"));
    }
    else if (isHelpSubCat && parentRow == eUSINGPEP9CPU && row == eMICROCODE) { // Microcode use
        ui->helpSplitter->widget(1)->hide();
        ui->helpTopWebView->show();
        ui->helpTopWebView->load(QUrl("qrc:/help-cpu/microcode.html"));
    }
    else if (isHelpSubCat && parentRow == eUSINGPEP9CPU && row == eDEBUGGING) { // Debugging use
        ui->helpSplitter->widget(1)->hide();
        ui->helpTopWebView->show();
        ui->helpTopWebView->load(QUrl("qrc:/help-cpu/debugging.html"));
    }
    else if (!isHelpSubCat && row == ePEP9REFERENCE) { // Pep/9 Reference
        ui->helpSplitter->widget(1)->hide();
        ui->helpTopWebView->show();
        ui->helpTopWebView->load(QUrl("qrc:/help/pep9reference.html"));
    }
    else if ((!isHelpSubCat && row == eONEBYTEBUSEXAMPLES) || parentRow == eONEBYTEBUSEXAMPLES) {
        if (!isHelpSubCat) {
            ui->helpSplitter->widget(1)->hide();
            ui->helpTopWebView->show();
            ui->helpTopWebView->load(QUrl("qrc:/help-cpu/onebytebusexamples.html"));
        }
        else {
            ui->helpSplitter->widget(0)->hide();
            ui->helpSplitter->widget(1)->show();
            if (row == eFIG1205) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/fig1205.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Figure 12.5</b> The control signals to fetch the instruction specifier and increment PC by 1.");
            }
            else if (row == eFIG1207) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/fig1207.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Figure 12.7</b> Combining cycles of Figure 12.5.");
            }
            else if (row == eFIG1209) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/fig1209.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Figure 12.9</b> The control signals to implement the store byte instruction with direct addressing.");
            }
            else if (row == eFIG1210) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/fig1210.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Figure 12.10</b> The control signals to implement the store word instruction with direct addressing.");
            }
            else if (row == eFIG1211) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/fig1211.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Figure 12.11</b> The control signals to implement the add instruction with immediate addressing.");
            }
            else if (row == eFIG1212) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/fig1212.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Figure 12.12</b> The control signals to implement the load instruction with indirect addressing.");
            }
            else if (row == eFIG1214) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/fig1214.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Figure 12.14</b> The control signals to implement the unary ASRA instruction.");
            }
        }
    }
    else if ((!isHelpSubCat && row == eTWOBYTEBUSEXAMPLES) || parentRow == eTWOBYTEBUSEXAMPLES) {
        if (!isHelpSubCat) {
            ui->helpSplitter->widget(1)->hide();
            ui->helpTopWebView->show();
            ui->helpTopWebView->load(QUrl("qrc:/help-cpu/twobytebusexamples.html"));
        }
        else {
            ui->helpSplitter->widget(0)->hide();
            ui->helpSplitter->widget(1)->show();
            if (row == eFIG1220) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/fig1220.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Figure 12.20</b> The fetch and increment part of the von Neumann cycle with the two-byte data bus.");
            }
            else if (row == eFIG1221) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/fig1221.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Figure 12.21</b> The fetch and increment part of the von Neumann cycle with pre-fetched instruction specifier.");
            }
            else if (row == eFIG1223) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/fig1223.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Figure 12.23</b> The two-byte bus implementation of the load word instruction with indirect addressing.");
            }
        }
    }
    else if ((!isHelpSubCat && row == eONEBYTEBUSPROBLEMS) || parentRow == eONEBYTEBUSPROBLEMS) {
        if (!isHelpSubCat) {
            ui->helpSplitter->widget(1)->hide();
            ui->helpTopWebView->show();
            ui->helpTopWebView->load(QUrl("qrc:/help-cpu/onebytebusproblems.html"));
        }
        else {
            ui->helpSplitter->widget(0)->hide();
            ui->helpSplitter->widget(1)->show();
            if (row == ePR1228) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1228.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.28</b> Specification to fetch the operand specifier and increment PC.");
            }
            else if (row == ePR1229A) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1229a.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.29 (a)</b> Specification for <code>MOVSPA</code>.");
            }
            else if (row == ePR1229B) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1229b.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.29 (b)</b> Specification for <code>MOVFLGA</code>.");
            }
            else if (row == ePR1229C) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1229c.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.29 (c)</b> Specification for <code>MOVAFLG</code>.");
            }
            else if (row == ePR1229D) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1229d.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.29 (d)</b> Specification for <code>NOTA</code>.");
            }
            else if (row == ePR1229E) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1229e.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.29 (e)</b> Specification for <code>NEGA</code>.");
            }
            else if (row == ePR1229F) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1229f.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.29 (f)</b> Specification for <code>ROLA</code>.");
            }
            else if (row == ePR1229G) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1229g.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.29 (g)</b> Specification for <code>RORA</code>.");
            }
            else if (row == ePR1230) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1230.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.30</b> Specification for <code>ASLA</code>.");
            }
            else if (row == ePR1231A) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1231a.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.31 (a)</b> Specification for <code>SUBA this,i</code>.");
            }
            else if (row == ePR1231B) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1231b.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.31 (b)</b> Specification for <code>ANDA this,i</code>.");
            }
            else if (row == ePR1231C) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1231c.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.31 (c)</b> Specification for <code>ORA this,i</code>.");
            }
            else if (row == ePR1231D) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1231d.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.31 (d)</b> Specification for <code>CPWA this,i</code>.");
            }
            else if (row == ePR1231E) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1231e.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.31 (e)</b> Specification for <code>CPBA this,i</code>.");
            }
            else if (row == ePR1231F) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1231f.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.31 (f)</b> Specification for <code>LDWA this,i</code>.");
            }
            else if (row == ePR1231G) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1231g.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.31 (g)</b> Specification for <code>LDBA this,i</code>.");
            }
            else if (row == ePR1232A) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232a.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (a)</b> Specification for <code>LDWA here,d</code>.");
            }
            else if (row == ePR1232B) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232b.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (b)</b> Specification for <code>LDWA here,s</code>.");
            }
            else if (row == ePR1232C) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232c.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (c)</b> Specification for <code>LDWA here,sf</code>.");
            }
            else if (row == ePR1232D) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232d.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (d)</b> Specification for <code>LDWA here,x</code>.");
            }
            else if (row == ePR1232E) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232e.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (e)</b> Specification for <code>LDWA here,sx</code>.");
            }
            else if (row == ePR1232F) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232f.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (f)</b> Specification for <code>LDWA here,sfx</code>.");
            }
            else if (row == ePR1232G) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232g.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (g)</b> Specification for <code>STWA there,n</code>.");
            }
            else if (row == ePR1232H) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232h.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (h)</b> Specification for <code>STWA there,s</code>.");
            }
            else if (row == ePR1232I) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232i.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (i)</b> Specification for <code>STWA there,sf</code>.");
            }
            else if (row == ePR1232J) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232j.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (j)</b> Specification for <code>STWA there,x</code>.");
            }
            else if (row == ePR1232K) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232k.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (k)</b> Specification for <code>STWA there,sx</code>.");
            }
            else if (row == ePR1232L) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232l.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (l)</b> Specification for <code>STWA there,sfx</code>.");
            }
            else if (row == ePR1233A) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1233a.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.33 (a)</b> Specification for <code>BR main</code>.");
            }
            else if (row == ePR1233B) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1233b.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.33 (b)</b> Specification for <code>BR guessJT,x</code>.");
            }
            else if (row == ePR1233C) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1233c.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.33 (c)</b> Specification for <code>CALL alpha</code>.");
            }
            else if (row == ePR1233D) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1233d.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.33 (d)</b> Specification for <code>RET</code>.");
            }
            else if (row == ePR1233E) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1233e.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.33 (e)</b> Specification for <code>DECO num,i</code>.");
            }
            else if (row == ePR1233F) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1233f.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.33 (f)</b> Specification for <code>RETTR</code>.");
            }
        }
    }
    else if ((!isHelpSubCat && row == eTWOBYTEBUSPROBLEMS) || parentRow == eTWOBYTEBUSPROBLEMS) {
        if (!isHelpSubCat) {
            ui->helpSplitter->widget(1)->hide();
            ui->helpTopWebView->show();
            ui->helpTopWebView->load(QUrl("qrc:/help-cpu/twobytebusproblems.html"));
        }
        else {
            ui->helpSplitter->widget(0)->hide();
            ui->helpSplitter->widget(1)->show();
            if (row == ePR1234A) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1234a.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Exercise 12.34 (a)</b> Specification to fetch OprndSpec assuming no previous pre-fetch.");
            }
            else if (row == ePR1234B) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1234b.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Exercise 12.34 (b)</b> Specification to fetch OprndSpec assuming previous pre-fetch.");
            }
            else if (row == ePR1235A) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235a.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (a)</b> Specification for <code>LDWA here,d</code>.");
            }
            else if (row == ePR1235B) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235b.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (b)</b> Specification for <code>LDWA here,s</code>.");
            }
            else if (row == ePR1235C) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235c.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (c)</b> Specification for <code>LDWA here,sf</code>.");
            }
            else if (row == ePR1235D) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235d.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (d)</b> Specification for <code>LDWA here,x</code>.");
            }
            else if (row == ePR1235E) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235e.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (e)</b> Specification for <code>LDWA here,sx</code>.");
            }
            else if (row == ePR1235F) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235f.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (f)</b> Specification for <code>LDWA here,sfx</code>.");
            }
            else if (row == ePR1235G) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235g.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (g)</b> Specification for <code>STWA there,n</code>.");
            }
            else if (row == ePR1235H) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235h.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (h)</b> Specification for <code>STWA there,s</code>.");
            }
            else if (row == ePR1235I) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235i.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (i)</b> Specification for <code>STWA there,sf</code>.");
            }
            else if (row == ePR1235J) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235j.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (j)</b> Specification for <code>STWA there,x</code>.");
            }
            else if (row == ePR1235K) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235k.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (k)</b> Specification for <code>STWA there,sx</code>.");
            }
            else if (row == ePR1235L) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235l.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (l)</b> Specification for <code>STWA there,sfx</code>.");
            }
            else if (row == ePR1236A) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1236a.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.36 (a)</b> Specification for <code>BR main</code>.");
            }
            else if (row == ePR1236B) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1236b.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.36 (b)</b> Specification for <code>BR guessJT,x</code>.");
            }
            else if (row == ePR1236C) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1236c.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.36 (c)</b> Specification for <code>CALL alpha</code>.");
            }
            else if (row == ePR1236D) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1236d.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.36 (d)</b> Specification for <code>RET</code>.");
            }
            else if (row == ePR1236E) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1236e.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.36 (e)</b> Specification for <code>DECO num,i</code>.");
            }
            else if (row == ePR1236F) {
                ui->helpTextEdit->setPlainText(Pep::resToString(":/help-cpu/figures/prob1236f.pepcpu", true));
                ui->helpFigureLabel->setText("<b>Problem 12.36 (f)</b> Specification for <code>RETTR</code>.");
            }
        }
    }
}

void CPUHelpDialog::copy()
{
    if (ui->helpTextEdit->hasFocus()) {
        ui->helpTextEdit->copy();
    } else if (ui->helpTopWebView->hasFocus()) {
        QApplication::clipboard()->setText(ui->helpTopWebView->selectedText());
    }
}

bool CPUHelpDialog::hasFocus()
{
    return ui->helpTextEdit->hasFocus() || ui->helpTopWebView->hasFocus();
}

