// File: microhelpdialog.cpp
/*
    Pep9Micro is a complete CPU simulator for the Pep/9 instruction set,
    and is capable of assembling programs to object code, executing
    object code programs, and executing microcode fragments.

    Copyright (C) 2018  J. Stanley Warford & Matthew McRaven, Pepperdine University

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

#include "microhelpdialog.h"
#include "ui_microhelpdialog.h"

#include <QClipboard>
#include <QDebug>

#include "highlight/cpphighlighter.h"
#include "pep/pep.h"
#include "style/colors.h"
#include "style/fonts.h"

#include "pep9asmhighlighter.h"
#include "pep9microhighlighter.h"

const int MicroHelpDialog::defaultHelpTreeWidth = 225;

MicroHelpDialog::MicroHelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpDialog)
{
    ui->setupUi(this);

    connect(ui->helpTreeWidget, &QTreeWidget::currentItemChanged, this,
            &MicroHelpDialog::onCurrentItemChanged);
    // Forward the helpCopyToMicrocodeButton_clicked() signal from this to the main window
    connect(ui->copyToSourceButton, &QAbstractButton::clicked, this, &MicroHelpDialog::copyToSourceClicked);

    ui->helpSplitter->widget(1)->hide();
    ui->helpTreeWidget->expandAll();
    ui->helpTreeWidget->itemAt(0,0)->setSelected(true);

    // All features are forced, but a CPUType is required for construction.
    leftMicroHighlighter = new Pep9MicroHighlighter(PepCore::CPUType::OneByteDataBus, true, PepColors::lightMode, ui->microTextEdit->document());
    leftMicroHighlighter->forceAllFeatures(true);

    leftPepHighlighter = new Pep9ASMHighlighter(PepColors::lightMode, ui->leftPepTextEdit->document());

    rightPepHighlighter = new Pep9ASMHighlighter(PepColors::lightMode, ui->rightPepTextEdit->document());

    rightCppHighlighter = new CppHighlighter(PepColors::lightMode, ui->rightCppTextEdit->document());

    ui->helpTreeWidget->setFont(QFont(PepCore::labelFont, PepCore::labelFontSize));

    // Manual size allocation to prevent help web view from being hidden.
    QList<int> helpBalance;
    static const int desiredSize = size().width()-MicroHelpDialog::defaultHelpTreeWidth;
    helpBalance.append({defaultHelpTreeWidth, desiredSize});
    ui->splitter_3->setSizes(helpBalance);
}

MicroHelpDialog::~MicroHelpDialog()
{
    delete ui;
}

QString MicroHelpDialog::getCode(PepCore::EPane &destPane, PepCore::EPane &inputDest, QString &input)
{
    bool isHelpSubCat = ui->helpTreeWidget->currentIndex().parent().isValid();
    int row = ui->helpTreeWidget->currentIndex().row();
    int parentRow = ui->helpTreeWidget->currentIndex().parent().row();

    /*if (!isHelpSubCat && row == eOS) {         // Pep/9 Operating System
        destPane = Enu::ESource;
        return ui->leftTextEdit->toPlainText();
    }*/
    if(parentRow == eMICROEXAMPLES) {
        destPane = PepCore::EPane::EMicrocode;
        return ui->microTextEdit->toPlainText();
    }
    else if (parentRow == eASMEXAMPLES) {
        if (row == eFIG433) {
            destPane = PepCore::EPane::EObject;
            return Pep::resToString(":/help-micro/figures-asm/fig0433.pepo", false);
        }
        else if (row == eFIG435) {
            destPane = PepCore::EPane::EObject;
            input = "up";
            inputDest = PepCore::EPane::EBatchIO;
            return Pep::resToString(":/help-micro/figures-asm/fig0435.pepo", false);
        }
        else if (row == eFIG436) {
            destPane = PepCore::EPane::EObject;
            return Pep::resToString(":/help-micro/figures-asm/fig0436.pepo", false);
        }
        else if (row == eFIG437) {
            destPane = PepCore::EPane::EObject;
            return Pep::resToString(":/help-micro/figures-asm/fig0437.pepo", false);
        }
        else if (row == eFIG506) {
            input = "up";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG511) {
            input = "-479";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG512) {
            input = "-479";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG515) {
            input = "-479";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG522) {
            input = "M 419";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG527 || row == eFIG604) {
            input = "68 84";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG606) {
            input = "-25";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG608) {
            input = "75";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG610) {
            input = "Hello, world!*";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG616) {
            input = "3 -15 25";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG621 || row == eFIG623) {
            input = "12  3 13 17 34 27 23 25 29 16 10 0 2";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG632) {
            input = "25";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG634) {
            input = "60 70 80 90";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG636) {
            input = "2 26 -3 9";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG638) {
            input = "5  40 50 60 70 80";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG646) {
            input = "bj 32 m";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG648) {
            input = "10 20 30 40 -9999";
            inputDest = PepCore::EPane::EBatchIO;
        }
        else if (row == eFIG627 || row == eFIG629 || row == eFIG640) {
            inputDest = PepCore::EPane::ETerminal;
        }
    }
    else if(parentRow == eMICROIMPL || row == eMICROIMPL) {
        destPane = PepCore::EPane::EMicrocode;
        return ui->microTextEdit->toPlainText();
    }
    destPane = PepCore::EPane::ESource;
    return ui->leftPepTextEdit->toPlainText();
}

void MicroHelpDialog::selectItem(QString string)
{
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

void MicroHelpDialog::changeEvent(QEvent *e)
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

void MicroHelpDialog::onFontChanged(QFont font)
{
    ui->microTextEdit->setFont(font);
    ui->leftPepTextEdit->setFont(font);
    ui->rightPepTextEdit->setFont(font);
    ui->rightCppTextEdit->setFont(font);
}

void MicroHelpDialog::onDarkModeChanged(bool darkMode)
{
    if(darkMode) {
        leftMicroHighlighter->rebuildHighlightingRules(PepColors::darkMode);
        leftPepHighlighter->rebuildHighlightingRules(PepColors::darkMode);
        rightPepHighlighter->rebuildHighlightingRules(PepColors::darkMode);
        rightCppHighlighter->rebuildHighlightingRules(PepColors::darkMode);
    }
    else {
        leftMicroHighlighter->rebuildHighlightingRules(PepColors::lightMode);
        leftPepHighlighter->rebuildHighlightingRules(PepColors::lightMode);
        rightPepHighlighter->rebuildHighlightingRules(PepColors::lightMode);
        rightCppHighlighter->rebuildHighlightingRules(PepColors::lightMode);
    }
    leftMicroHighlighter->rehighlight();
    leftPepHighlighter->rehighlight();
    rightPepHighlighter->rehighlight();
    rightCppHighlighter->rehighlight();
}

void MicroHelpDialog::onCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*) {
    // Is this a subcategory?
    bool isHelpSubCat = ui->helpTreeWidget->currentIndex().parent().isValid();
    // Parent row (if it has a parent, -1 else)
    int parentRow = ui->helpTreeWidget->currentIndex().parent().row();
    // Row (if it has a parent, this is the child row)
    int row = ui->helpTreeWidget->currentIndex().row();

//    qDebug() << "Selected: " << ui->treeWidget->currentIndex();

    if ((!isHelpSubCat && row == eWRITINGASM) || parentRow == eWRITINGASM) {
        ui->helpSplitter->widget(1)->hide();
        ui->helpTopWebView->show();
        if (!isHelpSubCat) {                  // Writing Programs
            ui->helpTopWebView->load(QUrl("qrc:/help-micro/writingprograms.html"));
        }
        else if (row == eMACHINELANG) {           // Writing Programs > Machine Language
            ui->helpTopWebView->load(QUrl("qrc:/help-micro/machinelanguage.html"));
        }
        else if (row == EASMLANG) {          // Writing Programs > Assembly Language
            ui->helpTopWebView->load(QUrl("qrc:/help-micro/assemblylanguage.html"));
        }
    }
    else if (!isHelpSubCat && row == eDEBUGGINGASM) {
        ui->helpSplitter->widget(1)->hide();
        ui->helpTopWebView->show();
        ui->helpTopWebView->load(QUrl("qrc:/help-micro/debuggingprograms.html"));
    }
    if (!isHelpSubCat && row == eUSINGPEP9CPU) { // Using Pep/9 CPU
        ui->helpSplitter->widget(1)->hide();
        ui->helpTopWebView->show();
        ui->helpTopWebView->load(QUrl("qrc:/help-micro/usingpep9cpu.html"));
    }
    else if (isHelpSubCat && parentRow == eUSINGPEP9CPU && row == eCPU) { // Interactive use
            ui->helpSplitter->widget(1)->hide();
            ui->helpTopWebView->show();
            ui->helpTopWebView->load(QUrl("qrc:/help-micro/cpu.html"));
        }
    else if (isHelpSubCat && parentRow == eUSINGPEP9CPU && row == eMICROCODE) { // Microcode use
            ui->helpSplitter->widget(1)->hide();
            ui->helpTopWebView->show();
            ui->helpTopWebView->load(QUrl("qrc:/help-micro/microcode.html"));
        }
    else if (isHelpSubCat && parentRow == eUSINGPEP9CPU && row == eDEBUGGINGMICRO) { // Debugging use
            ui->helpSplitter->widget(1)->hide();
            ui->helpTopWebView->show();
            ui->helpTopWebView->load(QUrl("qrc:/help-micro/debugging.html"));
        }
    else if (!isHelpSubCat && row == eTRAPS) {  // Writing Trap Handlers
        ui->helpSplitter->widget(1)->hide();
        ui->helpTopWebView->show();
        ui->helpTopWebView->load(QUrl("qrc:/help-micro/writingtraphandlers.html"));
    }
    else if ((!isHelpSubCat && row == ePEP9REFERENCE)) {
        ui->helpSplitter->widget(1)->hide();
        ui->helpTopWebView->show();
        ui->helpTopWebView->load(QUrl("qrc:/help/pep9reference.html"));
    }
    else if ((!isHelpSubCat && row == eMICROEXAMPLES) || parentRow == eMICROEXAMPLES) {
        if (!isHelpSubCat) {
            ui->helpSplitter->widget(1)->hide();
            ui->helpTopWebView->show();
            #pragma message("Add microcode examples htmlfile")
            ui->helpTopWebView->load(QUrl("qrc:/help-micro/examples.html"));
        }
        else {
            ui->leftPepTextEdit->hide();
            ui->rightPepTextEdit->hide();
            ui->rightCppTextEdit->hide();
            ui->microTextEdit->show();
            ui->helpSplitter->widget(0)->hide();
            ui->helpSplitter->widget(1)->show();
            ui->copyToSourceButton->setText("Copy to Microcode");
#pragma message("TODO: decide on the official figure names.")
            if(row == eMFig101) {
                ui->microTextEdit->setText(Pep::resToString(":/help-micro/figures-micro/fig-1-01.pepmicro", true));
                ui->helpFigureLabel->setText("<b>Figure 1.01</b><code>  </code> A fragment demonstrating explicit & implict branching, as well as stopping the CPU.");
            }
            else if(row == eMFig102) {
                ui->microTextEdit->setText(Pep::resToString(":/help-micro/figures-micro/fig-1-02.pepmicro", true));
                ui->helpFigureLabel->setText("<b>Figure 1.02</b><code>  </code> A fragment using explicit branching to skip over code blocks.");
            }
            else if(row == eMFig103) {
                ui->microTextEdit->setText(Pep::resToString(":/help-micro/figures-micro/fig-1-03.pepmicro", true));
                ui->helpFigureLabel->setText("<b>Figure 1.03</b><code>  </code> A fragment using explicit branching to jump backwards and execute in a non-linear order.");
            }
            else if(row == eMFig201) {
                ui->microTextEdit->setText(Pep::resToString(":/help-micro/figures-micro/fig-2-01.pepmicro", true));
                ui->helpFigureLabel->setText("<b>Figure 2.01</b><code>  </code> A fragment using conditonal branching on C to set T1.");
            }
            else if(row == eMFig202) {
                ui->microTextEdit->setText(Pep::resToString(":/help-micro/figures-micro/fig-2-02.pepmicro", true));
                ui->helpFigureLabel->setText("<b>Figure 2.02</b><code>  </code> A fragment using conditional branching to test if A is odd.");
            }
            else if(row == eMFig203) {
                ui->microTextEdit->setText(Pep::resToString(":/help-micro/figures-micro/fig-2-03.pepmicro", true));
                ui->helpFigureLabel->setText("<b>Figure 2.03</b><code>  </code> A fragment using conditional branching to select correct memory fetch logic.");
            }
            else if(row == eMFig204) {
                ui->microTextEdit->setText(Pep::resToString(":/help-micro/figures-micro/fig-2-04.pepmicro", true));
                ui->helpFigureLabel->setText("<b>Figure 2.04</b><code>  </code> A fragment using showing the cost of unaligned memory access.");
            }
            else if(row == eMFig301) {
                ui->microTextEdit->setText(Pep::resToString(":/help-micro/figures-micro/fig-3-01.pepmicro", true));
                ui->helpFigureLabel->setText("<b>Figure 3.01</b><code>  </code> Unuary instruction fetch using the von Neumann cycle.");
            }
            else if(row == eMFig302) {
                ui->microTextEdit->setText(Pep::resToString(":/help-micro/figures-micro/fig-3-02.pepmicro", true));
                ui->helpFigureLabel->setText("<b>Figure 3.02</b><code>  </code> Unuary instruction / Non-unary instruction & operand fetch using the von Neumann cycle.");
            }
            else if(row == eMFig303) {
                ui->microTextEdit->setText(Pep::resToString(":/help-micro/figures-micro/fig-3-03.pepmicro", true));
                ui->helpFigureLabel->setText("<b>Figure 3.03</b><code>  </code> Add a prelude section to the von Neumann cycle.");
            }
            else if(row == eMFig401) {
                ui->microTextEdit->setText(Pep::resToString(":/help-micro/figures-micro/fig-4-01.pepmicro", true));
                ui->helpFigureLabel->setText("<b>Figure 4.01</b><code>  </code> Show operand immediate operand decoding");
            }
            else if(row == eMFig402) {
                ui->microTextEdit->setText(Pep::resToString(":/help-micro/figures-micro/fig-4-02.pepmicro", true));
                ui->helpFigureLabel->setText("<b>Figure 4.02</b><code>  </code> Show direct, indexed, stack relative, stack-indexed operand decoding");
            }
            else if(row == eMFig403) {
                ui->microTextEdit->setText(Pep::resToString(":/help-micro/figures-micro/fig-4-03.pepmicro", true));
                ui->helpFigureLabel->setText("<b>Figure 4.03</b><code>  </code> Show indirect, stack deferred operand decoding.");
            }
            else if(row == eMFig404) {
                ui->microTextEdit->setText(Pep::resToString(":/help-micro/figures-micro/fig-4-04.pepmicro", true));
                ui->helpFigureLabel->setText("<b>Figure 4.04</b><code>  </code> Show stack deferred indexed operand decoding.");
            }
        }
    }
    else if ((!isHelpSubCat && row == eASMEXAMPLES) || parentRow == eASMEXAMPLES) {
        if (!isHelpSubCat) {
            ui->helpSplitter->widget(1)->hide();
            ui->helpTopWebView->show();
            ui->helpTopWebView->load(QUrl("qrc:/help-micro/examples.html"));
        }
        else {
            ui->microTextEdit->hide();
            ui->leftPepTextEdit->show();
            ui->helpSplitter->widget(0)->hide();
            ui->helpSplitter->widget(1)->show();
            ui->copyToSourceButton->setText("Copy to Source");
            if (row == eFIG433) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0433.peph", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0433.pepb", false));
                ui->helpFigureLabel->setText("<b>Figure 4.33</b><code>  </code> A machine language program to output the characters <code>Hi</code>.");
                ui->rightPepTextEdit->show();
                ui->rightCppTextEdit->hide();
                ui->copyToSourceButton->setText("Copy to Object");

            }
            else if (row == eFIG435) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0435.peph", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0435.pepb", false));
                ui->helpFigureLabel->setText("<b>Figure 4.35</b><code>  </code> A machine language program to input two characters and output them in reverse order.");
                ui->rightPepTextEdit->show();
                ui->rightCppTextEdit->hide();
                ui->copyToSourceButton->setText("Copy to Object");
            }
            else if (row == eFIG436) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0436.peph", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0436.pepb", false));
                ui->helpFigureLabel->setText("<b>Figure 4.36</b><code>  </code> A machine language program to add 5 and 3 and output the single-character result.");
                ui->leftPepTextEdit->show();
                ui->rightPepTextEdit->show();
                ui->rightCppTextEdit->hide();
                ui->copyToSourceButton->setText("Copy to Object");
            }
            else if (row == eFIG437) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0437.peph", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0437.pepb", false));
                ui->helpFigureLabel->setText("<b>Figure 4.37</b><code>  </code> A machine language program that modifies itself. The add accumulator instruction changes to a subtract instruction.");
                ui->rightPepTextEdit->show();
                ui->rightCppTextEdit->hide();
                ui->copyToSourceButton->setText("Copy to Object");
            }
            else if (row == eFIG503) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0503.pep", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0433.peph", false));
                ui->helpFigureLabel->setText("<b>Figure 5.3</b><code>  </code> An assembly-language program to output <code>Hi</code>. It is the assembly-language version of Figure 4.33.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->show();
            }
            else if (row == eFIG506) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0506.pep", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0435.peph", false));
                ui->helpFigureLabel->setText("<b>Figure 5.6</b><code>  </code> An assembly language program to input two characters and output them in reverse order. It is the assembly language version of Figure 4.35.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->show();
            }
            else if (row == eFIG507) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0507.pep", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0436.peph", false));
                ui->helpFigureLabel->setText("<b>Figure 5.7</b><code>  </code> An assembly language program to add 3 and 5 and output the single-character result. It is the assembly language version of Figure 4.36.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->show();
            }
            else if (row == eFIG510) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0510.pep", false));
                ui->helpFigureLabel->setText("<b>Figure 5.10</b><code>  </code> A program to output <code>Hi</code> using immediate addressing.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG511) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0511.pep", false));
                ui->helpFigureLabel->setText("<b>Figure 5.11</b><code>  </code> A program to input a decimal value, add 1 to it, and output the sum.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG512) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0512.pep", false));
                ui->helpFigureLabel->setText("<b>Figure 5.12</b><code>  </code> A program identical to that of Figure 5.11 but with the <code>STRO</code> instruction.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG513) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0513.pep", false));
                ui->helpFigureLabel->setText("<b>Figure 5.13</b><code>  </code> A nonsense program to illustrate the interpretation of bit patterns.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG514a) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0514a.pep", false));
                ui->helpFigureLabel->setText("<b>Figure 5.14(a)</b><code>  </code> Two different source programs that produce the same object program and, therefore, the same output.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG514b) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-micro/figures-asm/fig0514b.pep", false));
                ui->helpFigureLabel->setText("<b>Figure 5.14(b)</b><code>  </code> Two different source programs that produce the same object program and, therefore, the same output.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG515) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0515.pep", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0512.pep", false));
                ui->helpFigureLabel->setText("<b>Figure 5.15</b><code>  </code> A program that adds 1 to a decimal value. It is identical to Figure 5.12 except that it uses symbols.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->show();
            }
            else if (row == eFIG516) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0516.pep", false));
                ui->helpFigureLabel->setText("<b>Figure 5.16</b><code>  </code> A nonsense program that illustrates the underlying von Neumann nature of the machine.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG519) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0519.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0519.c", false));
                ui->helpFigureLabel->setText("<b>Figure 5.19</b><code>  </code> The <code>printf()</code> function.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG522) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0522.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0522.c", false));
                ui->helpFigureLabel->setText("<b>Figure 5.22</b><code>  </code> The assignment statement with global variables.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG527) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0527.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0527.c", false));
                ui->helpFigureLabel->setText("<b>Figure 5.27</b><code>  </code> C constants.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG601) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0601.pep", false));
                ui->helpFigureLabel->setText("<b>Figure 6.1</b><code>  </code> Stack-relative addressing.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG604) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0604.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0604.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.4</b><code>  </code> Local variables.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG606) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0606.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0606.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.6</b><code>  </code> The <code>if</code> statement.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG608) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0608.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0608.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.8</b><code>  </code> The <code>if</code>/<code>else</code> statement.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG610) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0610.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0610.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.10</b><code>  </code> The <code>while</code> statement.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG612) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0612.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0612.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.12</b><code>  </code> The <code>do</code> statement.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG614) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0614.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0614.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.14</b><code>  </code> The <code>for</code> statement.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG616) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0616.pep", false));
                ui->helpFigureLabel->setText("<b>Figure 6.16</b><code>  </code> A mystery program.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG618) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0618.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0618.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.18</b><code>  </code> A procedure call with no parameters.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG621) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0621.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0621.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.21</b><code>  </code> Call-by-value parameters with global variables.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG623) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0623.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0623.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.23</b><code>  </code> Call-by-value parameters with local variables.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG625) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0625.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0625.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.25</b><code>  </code> A recursive nonvoid function.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG627) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0627.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0627.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.27</b><code>  </code> Call-by-reference parameters with global variables.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG629) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0629.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0629.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.29</b><code>  </code> Call-by-reference parameters with local variables.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG632) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0632.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0632.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.32</b><code>  </code> Translation of a boolean type.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG634) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0634.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0634.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.32</b><code>  </code> A global array.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG636) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0636.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0636.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.36</b><code>  </code> A local array.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG638) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0638.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0638.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.38</b><code>  </code> Passing a local array as a parameter.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG640) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0640.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0640.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.40</b><code>  </code> Translation of a <code>switch</code> statement.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG642) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0642.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0642.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.42</b><code>  </code> Translation of global pointers.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG644) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0644.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0644.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.44</b><code>  </code> Translation of local pointers.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG646) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0646.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0646.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.46</b><code>  </code> Translation of a structure.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG648) {
                ui->leftPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0648.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0648.c", false));
                ui->helpFigureLabel->setText("<b>Figure 6.48</b><code>  </code> Translation of a linked list.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
        }
    }
    else if (!isHelpSubCat && row == eOS) {         // Pep/9 Operating System
        ui->copyToSourceButton->setText("Copy to Source");
        ui->helpSplitter->widget(0)->hide();
        ui->helpSplitter->widget(1)->show();
        ui->leftPepTextEdit->setText(Pep::resToString(":/help-micro/alignedIO-OS.pep", false));
        ui->leftPepTextEdit->show();
        ui->rightCppTextEdit->hide();
        ui->rightPepTextEdit->hide();
        ui->microTextEdit->hide();
#pragma message ("TODO: Fix label text after publishing manuscript")
        ui->helpFigureLabel->setText("<b>Figures 8.2, 8.3, 8.6, 8.8, 8.10, 8.11</b><code>  </code> The Pep/9 operating system.");
    }
    else if (!isHelpSubCat && row == eMICROIMPL) {
        ui->copyToSourceButton->setText("Copy to Microcode");
        ui->helpSplitter->widget(0)->hide();
        ui->helpSplitter->widget(1)->show();
        ui->leftPepTextEdit->hide();
        ui->rightCppTextEdit->hide();
        ui->rightPepTextEdit->hide();
        ui->microTextEdit->setText(Pep::resToString(":/help-micro/pep9micro.pepmicro", true));
        ui->microTextEdit->show();
        ui->helpFigureLabel->setText("Microcoded implementation of the <i>Pep/9</i> instruction set.");
    }
}

void MicroHelpDialog::copy()
{
    if (ui->helpTopWebView->hasFocus()) {
        QApplication::clipboard()->setText(ui->helpTopWebView->selectedText());
    }
}

bool MicroHelpDialog::hasFocus()
{
    return ui->helpTopWebView->hasFocus();
}

