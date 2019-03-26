// File: helpdialog.cpp
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

#include "pep.h"
#include <QClipboard>
#include <QDebug>
#include "colors.h"
HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpDialog)
{
    ui->setupUi(this);

    connect(ui->helpTreeWidget, &QTreeWidget::currentItemChanged, this,
            &HelpDialog::onCurrentItemChanged);
    // Forward the helpCopyToMicrocodeButton_clicked() signal from this to the main window
    connect(ui->helpCopyToMicrocodeButton, &QAbstractButton::clicked, this, &HelpDialog::copyToMicrocodeClicked);

    ui->helpSplitter->widget(1)->hide();
    ui->helpTreeWidget->expandAll();
    ui->helpTreeWidget->itemAt(0,0)->setSelected(true);

    delete ui->helpTextEdit;

    microcodeEditor = new MicrocodeEditor(false, true, this);
    ui->verticalLayout->insertWidget(0, microcodeEditor);

    leftHighlighter = new PepMicroHighlighter(Enu::CPUType::OneByteDataBus, false, PepColors::lightMode, microcodeEditor->document());
    leftHighlighter->forceAllFeatures(true);
    ui->helpTreeWidget->setFont(QFont(Pep::labelFont, Pep::labelFontSize));

    microcodeEditor->setFont(QFont(Pep::codeFont, Pep::codeFontSize));
}

HelpDialog::~HelpDialog()
{
    delete ui;
}

void HelpDialog::selectItem(QString string) {
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

QString HelpDialog::getExampleText()
{
    return microcodeEditor->toPlainText();
}

Enu::CPUType HelpDialog::getExamplesModel()
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

void HelpDialog::changeEvent(QEvent *e)
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

void HelpDialog::onFontChanged(QFont font)
{
    microcodeEditor->setFont(font);
}

void HelpDialog::onDarkModeChanged(bool darkMode)
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

void HelpDialog::onCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*) {
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
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/fig1205.pepcpu"));
                ui->helpFigureLabel->setText("<b>Figure 12.5</b> The control signals to fetch the instruction specifier and increment PC by 1.");
            }
            else if (row == eFIG1207) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/fig1207.pepcpu"));
                ui->helpFigureLabel->setText("<b>Figure 12.7</b> Combining cycles of Figure 12.5.");
            }
            else if (row == eFIG1209) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/fig1209.pepcpu"));
                ui->helpFigureLabel->setText("<b>Figure 12.9</b> The control signals to implement the store byte instruction with direct addressing.");
            }
            else if (row == eFIG1210) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/fig1210.pepcpu"));
                ui->helpFigureLabel->setText("<b>Figure 12.10</b> The control signals to implement the store word instruction with direct addressing.");
            }
            else if (row == eFIG1211) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/fig1211.pepcpu"));
                ui->helpFigureLabel->setText("<b>Figure 12.11</b> The control signals to implement the add instruction with immediate addressing.");
            }
            else if (row == eFIG1212) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/fig1212.pepcpu"));
                ui->helpFigureLabel->setText("<b>Figure 12.12</b> The control signals to implement the load instruction with indirect addressing.");
            }
            else if (row == eFIG1214) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/fig1214.pepcpu"));
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
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/fig1220.pepcpu"));
                ui->helpFigureLabel->setText("<b>Figure 12.20</b> The fetch and increment part of the von Neumann cycle with the two-byte data bus.");
            }
            else if (row == eFIG1221) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/fig1221.pepcpu"));
                ui->helpFigureLabel->setText("<b>Figure 12.21</b> The fetch and increment part of the von Neumann cycle with pre-fetched instruction specifier.");
            }
            else if (row == eFIG1223) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/fig1223.pepcpu"));
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
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1228.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.28</b> Specification to fetch the operand specifier and increment PC.");
            }
            else if (row == ePR1229A) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1229a.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.29 (a)</b> Specification for <code>MOVSPA</code>.");
            }
            else if (row == ePR1229B) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1229b.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.29 (b)</b> Specification for <code>MOVFLGA</code>.");
            }
            else if (row == ePR1229C) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1229c.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.29 (c)</b> Specification for <code>MOVAFLG</code>.");
            }
            else if (row == ePR1229D) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1229d.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.29 (d)</b> Specification for <code>NOTA</code>.");
            }
            else if (row == ePR1229E) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1229e.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.29 (e)</b> Specification for <code>NEGA</code>.");
            }
            else if (row == ePR1229F) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1229f.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.29 (f)</b> Specification for <code>ROLA</code>.");
            }
            else if (row == ePR1229G) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1229g.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.29 (g)</b> Specification for <code>RORA</code>.");
            }
            else if (row == ePR1230) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1230.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.30</b> Specification for <code>ASLA</code>.");
            }
            else if (row == ePR1231A) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1231a.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.31 (a)</b> Specification for <code>SUBA this,i</code>.");
            }
            else if (row == ePR1231B) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1231b.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.31 (b)</b> Specification for <code>ANDA this,i</code>.");
            }
            else if (row == ePR1231C) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1231c.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.31 (c)</b> Specification for <code>ORA this,i</code>.");
            }
            else if (row == ePR1231D) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1231d.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.31 (d)</b> Specification for <code>CPWA this,i</code>.");
            }
            else if (row == ePR1231E) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1231e.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.31 (e)</b> Specification for <code>CPBA this,i</code>.");
            }
            else if (row == ePR1231F) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1231f.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.31 (f)</b> Specification for <code>LDWA this,i</code>.");
            }
            else if (row == ePR1231G) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1231g.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.31 (g)</b> Specification for <code>LDBA this,i</code>.");
            }
            else if (row == ePR1232A) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232a.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (a)</b> Specification for <code>LDWA here,d</code>.");
            }
            else if (row == ePR1232B) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232b.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (b)</b> Specification for <code>LDWA here,s</code>.");
            }
            else if (row == ePR1232C) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232c.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (c)</b> Specification for <code>LDWA here,sf</code>.");
            }
            else if (row == ePR1232D) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232d.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (d)</b> Specification for <code>LDWA here,x</code>.");
            }
            else if (row == ePR1232E) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232e.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (e)</b> Specification for <code>LDWA here,sx</code>.");
            }
            else if (row == ePR1232F) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232f.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (f)</b> Specification for <code>LDWA here,sfx</code>.");
            }
            else if (row == ePR1232G) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232g.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (g)</b> Specification for <code>STWA there,n</code>.");
            }
            else if (row == ePR1232H) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232h.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (h)</b> Specification for <code>STWA there,s</code>.");
            }
            else if (row == ePR1232I) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232i.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (i)</b> Specification for <code>STWA there,sf</code>.");
            }
            else if (row == ePR1232J) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232j.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (j)</b> Specification for <code>STWA there,x</code>.");
            }
            else if (row == ePR1232K) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232k.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (k)</b> Specification for <code>STWA there,sx</code>.");
            }
            else if (row == ePR1232L) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1232l.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.32 (l)</b> Specification for <code>STWA there,sfx</code>.");
            }
            else if (row == ePR1233A) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1233a.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.33 (a)</b> Specification for <code>BR main</code>.");
            }
            else if (row == ePR1233B) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1233b.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.33 (b)</b> Specification for <code>BR guessJT,x</code>.");
            }
            else if (row == ePR1233C) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1233c.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.33 (c)</b> Specification for <code>CALL alpha</code>.");
            }
            else if (row == ePR1233D) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1233d.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.33 (d)</b> Specification for <code>RET</code>.");
            }
            else if (row == ePR1233E) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1233e.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.33 (e)</b> Specification for <code>DECO num,i</code>.");
            }
            else if (row == ePR1233F) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1233f.pepcpu"));
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
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1234a.pepcpu"));
                ui->helpFigureLabel->setText("<b>Exercise 12.34 (a)</b> Specification to fetch OprndSpec assuming no previous pre-fetch.");
            }
            else if (row == ePR1234B) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1234b.pepcpu"));
                ui->helpFigureLabel->setText("<b>Exercise 12.34 (b)</b> Specification to fetch OprndSpec assuming previous pre-fetch.");
            }
            else if (row == ePR1235A) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235a.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (a)</b> Specification for <code>LDWA here,d</code>.");
            }
            else if (row == ePR1235B) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235b.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (b)</b> Specification for <code>LDWA here,s</code>.");
            }
            else if (row == ePR1235C) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235c.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (c)</b> Specification for <code>LDWA here,sf</code>.");
            }
            else if (row == ePR1235D) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235d.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (d)</b> Specification for <code>LDWA here,x</code>.");
            }
            else if (row == ePR1235E) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235e.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (e)</b> Specification for <code>LDWA here,sx</code>.");
            }
            else if (row == ePR1235F) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235f.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (f)</b> Specification for <code>LDWA here,sfx</code>.");
            }
            else if (row == ePR1235G) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235g.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (g)</b> Specification for <code>STWA there,n</code>.");
            }
            else if (row == ePR1235H) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235h.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (h)</b> Specification for <code>STWA there,s</code>.");
            }
            else if (row == ePR1235I) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235i.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (i)</b> Specification for <code>STWA there,sf</code>.");
            }
            else if (row == ePR1235J) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235j.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (j)</b> Specification for <code>STWA there,x</code>.");
            }
            else if (row == ePR1235K) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235k.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (k)</b> Specification for <code>STWA there,sx</code>.");
            }
            else if (row == ePR1235L) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1235l.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.35 (l)</b> Specification for <code>STWA there,sfx</code>.");
            }
            else if (row == ePR1236A) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1236a.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.36 (a)</b> Specification for <code>BR main</code>.");
            }
            else if (row == ePR1236B) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1236b.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.36 (b)</b> Specification for <code>BR guessJT,x</code>.");
            }
            else if (row == ePR1236C) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1236c.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.36 (c)</b> Specification for <code>CALL alpha</code>.");
            }
            else if (row == ePR1236D) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1236d.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.36 (d)</b> Specification for <code>RET</code>.");
            }
            else if (row == ePR1236E) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1236e.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.36 (e)</b> Specification for <code>DECO num,i</code>.");
            }
            else if (row == ePR1236F) {
                microcodeEditor->setPlainText(Pep::resToString(":/help-cpu/figures/prob1236f.pepcpu"));
                ui->helpFigureLabel->setText("<b>Problem 12.36 (f)</b> Specification for <code>RETTR</code>.");
            }
        }
    }
}

void HelpDialog::copy()
{
    if (microcodeEditor->hasFocus()) {
        microcodeEditor->copy();
    } else if (ui->helpTopWebView->hasFocus()) {
        QApplication::clipboard()->setText(ui->helpTopWebView->selectedText());
    }
}

bool HelpDialog::hasFocus()
{
    return microcodeEditor->hasFocus() || ui->helpTopWebView->hasFocus();
}

