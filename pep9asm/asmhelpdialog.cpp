// File: helpdialog.cpp
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
#include <QClipboard>
#include "asmhelpdialog.h"
#include "ui_asmhelpdialog.h"
#include "pep.h"
#include "colors.h"

const int AsmHelpDialog::defaultHelpTreeWidth = 200;
AsmHelpDialog::AsmHelpDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::HelpDialog)
{
    ui->setupUi(this);

    connect(ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this,
            SLOT(onCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
    // Forward the helpCopyToMicrocodeButton_clicked() signal from this to the main window
    connect(ui->copyToSourceButton, &QAbstractButton::clicked, this, &AsmHelpDialog::copyToSourceClicked);

    ui->helpSplitter->widget(1)->hide();
    ui->treeWidget->expandAll();

    selectItem("Writing Programs");


    leftHighlighter = new PepASMHighlighter(PepColors::lightMode, ui->leftTextEdit->document());
    rightCppHighlighter = new CppHighlighter(PepColors::lightMode, ui->rightCppTextEdit->document());
    rightPepHighlighter = new PepASMHighlighter(PepColors::lightMode, ui->rightPepTextEdit->document());

    ui->copyToSourceButton->setFont(QFont(Pep::labelFont));
    if (Pep::getSystem() == "Linux") {
        ui->treeWidget->setFont(QFont(Pep::labelFont, 8));
    }
    else if (Pep::getSystem() == "Windows") {
        ui->treeWidget->setFont(QFont(Pep::labelFont, 8)); // I don't know if this is the proper font size.
    }

    ui->rightCppTextEdit->setFont(QFont(Pep::codeFont, Pep::codeFontSize));
    ui->rightPepTextEdit->setFont(QFont(Pep::codeFont, Pep::codeFontSize));
    ui->leftTextEdit->setFont(QFont(Pep::codeFont, Pep::codeFontSize));

    // Manual size allocation to prevent help web view from being hidden.
    QList<int> helpBalance;
    static const int desiredSize = size().width()-AsmHelpDialog::defaultHelpTreeWidth;
    helpBalance.append({defaultHelpTreeWidth, desiredSize});
    ui->splitter_3->setSizes(helpBalance);
}

AsmHelpDialog::~AsmHelpDialog()
{
    delete ui;
}

void AsmHelpDialog::selectItem(QString string) {
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        if ((*it)->text(0) == string) {
            (*it)->setSelected(true);
            ui->treeWidget->setCurrentItem((*it));
        } else {
            (*it)->setSelected(false);
        }
        ++it;
    }
}

void AsmHelpDialog::onCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*) {
    // Is this a subcategory?
    bool isHelpSubCat = ui->treeWidget->currentIndex().parent().isValid();
    // Parent row (if it has a parent, -1 else)
    int parentRow = ui->treeWidget->currentIndex().parent().row();
    // Row (if it has a parent, this is the child row)
    int row = ui->treeWidget->currentIndex().row();

//    qDebug() << "Selected: " << ui->treeWidget->currentIndex();

    if ((!isHelpSubCat && row == eWRITING) || parentRow == eWRITING) {
        ui->helpSplitter->widget(1)->hide();
        ui->webView->show();
        if (!isHelpSubCat) {                  // Writing Programs
            ui->webView->load(QUrl("qrc:/help-asm/writingprograms.html"));
        }
        else if (row == eMACHINE) {           // Writing Programs > Machine Language
            ui->webView->load(QUrl("qrc:/help-asm/machinelanguage.html"));
        }
        else if (row == eASSEMBLY) {          // Writing Programs > Assembly Language
            ui->webView->load(QUrl("qrc:/help-asm/assemblylanguage.html"));
        }
    }
    else if (!isHelpSubCat && row == eDEBUGGING) {
        ui->helpSplitter->widget(1)->hide();
        ui->webView->show();
        ui->webView->load(QUrl("qrc:/help-asm/debuggingprograms.html"));
    }
    else if (!isHelpSubCat && row == eTRAP) {  // Writing Trap Handlers
        ui->helpSplitter->widget(1)->hide();
        ui->webView->show();
        ui->webView->load(QUrl("qrc:/help-asm/writingtraphandlers.html"));
    }
    else if ((!isHelpSubCat && row == eREFERENCE)) {
        ui->helpSplitter->widget(1)->hide();
        ui->webView->show();
        ui->webView->load(QUrl("qrc:/help/pep9reference.html"));
    }
    else if ((!isHelpSubCat && row == eEXAMPLES) || parentRow == eEXAMPLES) {
        if (!isHelpSubCat) {
            ui->helpSplitter->widget(1)->hide();
            ui->webView->show();
            ui->webView->load(QUrl("qrc:/help-asm/examples.html"));
        }
        else {
            ui->helpSplitter->widget(0)->hide();
            ui->helpSplitter->widget(1)->show();
            ui->copyToSourceButton->setText("Copy to Source");
            if (row == eFIG433) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0433.peph", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0433.pepb", false));
                ui->figureLabel->setText("<b>Figure 4.33</b><code>  </code> A machine language program to output the characters <code>Hi</code>.");
                ui->rightPepTextEdit->show();
                ui->rightCppTextEdit->hide();
                ui->copyToSourceButton->setText("Copy to Object");

            }
            else if (row == eFIG435) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0435.peph", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0435.pepb", false));
                ui->figureLabel->setText("<b>Figure 4.35</b><code>  </code> A machine language program to input two characters and output them in reverse order.");
                ui->rightPepTextEdit->show();
                ui->rightCppTextEdit->hide();
                ui->copyToSourceButton->setText("Copy to Object");
            }
            else if (row == eFIG436) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0436.peph", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0436.pepb", false));
                ui->figureLabel->setText("<b>Figure 4.36</b><code>  </code> A machine language program to add 5 and 3 and output the single-character result.");
                ui->rightPepTextEdit->show();
                ui->rightCppTextEdit->hide();
                ui->copyToSourceButton->setText("Copy to Object");
            }
            else if (row == eFIG437) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0437.peph", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0437.pepb", false));
                ui->figureLabel->setText("<b>Figure 4.37</b><code>  </code> A machine language program that modifies itself. The add accumulator instruction changes to a subtract instruction.");
                ui->rightPepTextEdit->show();
                ui->rightCppTextEdit->hide();
                ui->copyToSourceButton->setText("Copy to Object");
            }
            else if (row == eFIG503) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0503.pep", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0433.peph", false));
                ui->figureLabel->setText("<b>Figure 5.3</b><code>  </code> An assembly-language program to output <code>Hi</code>. It is the assembly-language version of Figure 4.33.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->show();
            }
            else if (row == eFIG506) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0506.pep", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0435.peph", false));
                ui->figureLabel->setText("<b>Figure 5.6</b><code>  </code> An assembly language program to input two characters and output them in reverse order. It is the assembly language version of Figure 4.35.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->show();
            }
            else if (row == eFIG507) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0507.pep", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0436.peph", false));
                ui->figureLabel->setText("<b>Figure 5.7</b><code>  </code> An assembly language program to add 3 and 5 and output the single-character result. It is the assembly language version of Figure 4.36.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->show();
            }
            else if (row == eFIG510) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0510.pep", false));
                ui->figureLabel->setText("<b>Figure 5.10</b><code>  </code> A program to output <code>Hi</code> using immediate addressing.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG511) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0511.pep", false));
                ui->figureLabel->setText("<b>Figure 5.11</b><code>  </code> A program to input a decimal value, add 1 to it, and output the sum.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG512) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0512.pep", false));
                ui->figureLabel->setText("<b>Figure 5.12</b><code>  </code> A program identical to that of Figure 5.11 but with the <code>STRO</code> instruction.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG513) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0513.pep", false));
                ui->figureLabel->setText("<b>Figure 5.13</b><code>  </code> A nonsense program to illustrate the interpretation of bit patterns.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG514a) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0514a.pep", false));
                ui->figureLabel->setText("<b>Figure 5.14(a)</b><code>  </code> Two different source programs that produce the same object program and, therefore, the same output.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG514b) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0514b.pep", false));
                ui->figureLabel->setText("<b>Figure 5.14(b)</b><code>  </code> Two different source programs that produce the same object program and, therefore, the same output.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG515) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0515.pep", false));
                ui->rightPepTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0512.pep", false));
                ui->figureLabel->setText("<b>Figure 5.15</b><code>  </code> A program that adds 1 to a decimal value. It is identical to Figure 5.12 except that it uses symbols.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->show();
            }
            else if (row == eFIG516) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0516.pep", false));
                ui->figureLabel->setText("<b>Figure 5.16</b><code>  </code> A nonsense program that illustrates the underlying von Neumann nature of the machine.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG519) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0519.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0519.c", false));
                ui->figureLabel->setText("<b>Figure 5.19</b><code>  </code> The <code>printf()</code> function.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG522) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0522.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0522.c", false));
                ui->figureLabel->setText("<b>Figure 5.22</b><code>  </code> The assignment statement with global variables.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG527) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0527.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0527.c", false));
                ui->figureLabel->setText("<b>Figure 5.27</b><code>  </code> C constants.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG601) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0601.pep", false));
                ui->figureLabel->setText("<b>Figure 6.1</b><code>  </code> Stack-relative addressing.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG604) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0604.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0604.c", false));
                ui->figureLabel->setText("<b>Figure 6.4</b><code>  </code> Local variables.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG606) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0606.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0606.c", false));
                ui->figureLabel->setText("<b>Figure 6.6</b><code>  </code> The <code>if</code> statement.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG608) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0608.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0608.c", false));
                ui->figureLabel->setText("<b>Figure 6.8</b><code>  </code> The <code>if</code>/<code>else</code> statement.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG610) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0610.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0610.c", false));
                ui->figureLabel->setText("<b>Figure 6.10</b><code>  </code> The <code>while</code> statement.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG612) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0612.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0612.c", false));
                ui->figureLabel->setText("<b>Figure 6.12</b><code>  </code> The <code>do</code> statement.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG614) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0614.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0614.c", false));
                ui->figureLabel->setText("<b>Figure 6.14</b><code>  </code> The <code>for</code> statement.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG616) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0616.pep", false));
                ui->figureLabel->setText("<b>Figure 6.16</b><code>  </code> A mystery program.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG618) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0618.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0618.c", false));
                ui->figureLabel->setText("<b>Figure 6.18</b><code>  </code> A procedure call with no parameters.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG621) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0621.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0621.c", false));
                ui->figureLabel->setText("<b>Figure 6.21</b><code>  </code> Call-by-value parameters with global variables.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG623) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0623.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0623.c", false));
                ui->figureLabel->setText("<b>Figure 6.23</b><code>  </code> Call-by-value parameters with local variables.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG625) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0625.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0625.c", false));
                ui->figureLabel->setText("<b>Figure 6.25</b><code>  </code> A recursive nonvoid function.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG627) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0627.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0627.c", false));
                ui->figureLabel->setText("<b>Figure 6.27</b><code>  </code> Call-by-reference parameters with global variables.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG629) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0629.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0629.c", false));
                ui->figureLabel->setText("<b>Figure 6.29</b><code>  </code> Call-by-reference parameters with local variables.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG632) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0632.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0632.c", false));
                ui->figureLabel->setText("<b>Figure 6.32</b><code>  </code> Translation of a boolean type.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG634) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0634.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0634.c", false));
                ui->figureLabel->setText("<b>Figure 6.32</b><code>  </code> A global array.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG636) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0636.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0636.c", false));
                ui->figureLabel->setText("<b>Figure 6.36</b><code>  </code> A local array.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG638) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0638.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0638.c", false));
                ui->figureLabel->setText("<b>Figure 6.38</b><code>  </code> Passing a local array as a parameter.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG640) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0640.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0640.c", false));
                ui->figureLabel->setText("<b>Figure 6.40</b><code>  </code> Translation of a <code>switch</code> statement.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG642) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0642.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0642.c", false));
                ui->figureLabel->setText("<b>Figure 6.42</b><code>  </code> Translation of global pointers.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG644) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0644.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0644.c", false));
                ui->figureLabel->setText("<b>Figure 6.44</b><code>  </code> Translation of local pointers.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG646) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0646.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0646.c", false));
                ui->figureLabel->setText("<b>Figure 6.46</b><code>  </code> Translation of a structure.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
            else if (row == eFIG648) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0648.pep", false));
                ui->rightCppTextEdit->setText(Pep::resToString(":/help-asm/figures/fig0648.c", false));
                ui->figureLabel->setText("<b>Figure 6.48</b><code>  </code> Translation of a linked list.");
                ui->rightCppTextEdit->show();
                ui->rightPepTextEdit->hide();
            }
        }
    }
    else if ((!isHelpSubCat && row == ePROBLEMS) || parentRow == ePROBLEMS) {
        if (!isHelpSubCat) {
            ui->helpSplitter->widget(1)->hide();
            ui->webView->show();
            ui->webView->load(QUrl("qrc:/help-asm/problems.html"));
        }
        else {
            ui->helpSplitter->widget(0)->hide();
            ui->helpSplitter->widget(1)->show();
            ui->copyToSourceButton->setText("Copy to Source");
            if (row == eEXER804) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/exer0804.pep", false));
                ui->figureLabel->setText("<b>Exercise 8.4</b><code>  </code> An excercise for the <code>DECI</code> trap.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == ePROB826) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/prob0826.pep", false));
                ui->figureLabel->setText("<b>Problem 8.26</b><code>  </code> A test driver for the <code>ASL2</code> instruction.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == ePROB827) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/prob0827.pep", false));
                ui->figureLabel->setText("<b>Problem 8.27</b><code>  </code> A test driver for the <code>ASLMANY</code> instruction.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == ePROB828) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/prob0828.pep", false));
                ui->figureLabel->setText("<b>Problem 8.28</b><code>  </code> A test driver for the <code>MULA</code> instruction.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == ePROB829) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/prob0829.pep", false));
                ui->figureLabel->setText("<b>Problem 8.29</b><code>  </code> A test driver for the <code>STWADI</code> instruction.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == ePROB830) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/prob0830.pep", false));
                ui->figureLabel->setText("<b>Problem 8.30</b><code>  </code> A test driver for the <code>BOOLO</code> instruction.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == ePROB831) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/prob0831.pep", false));
                ui->figureLabel->setText("<b>Problem 8.31</b><code>  </code> A test driver for the <code>STKADD</code> instruction.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
            else if (row == ePROB832) {
                ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/prob0832.pep", false));
                ui->figureLabel->setText("<b>Problem 8.32</b><code>  </code> A test driver for the <code>XORA</code> instruction.");
                ui->rightCppTextEdit->hide();
                ui->rightPepTextEdit->hide();
            }
        }
    }
    else if (!isHelpSubCat && row == eOS) {         // Pep/9 Operating System
        ui->copyToSourceButton->setText("Copy to Source");
        ui->helpSplitter->widget(0)->hide();
        ui->helpSplitter->widget(1)->show();
        ui->leftTextEdit->setText(Pep::resToString(":/help-asm/figures/pep9os.pep", false));
        ui->rightCppTextEdit->hide();
        ui->rightPepTextEdit->hide();
        ui->figureLabel->setText("<b>Figures 8.2, 8.3, 8.6, 8.8, 8.10, 8.11</b><code>  </code> The Pep/9 operating system.");
    }
}

// Public functions called by main window help menu items:

void AsmHelpDialog::machineLanguageClicked()
{
    selectItem("Machine Language");
}

void AsmHelpDialog::assemblyLanguageClicked()
{
    selectItem("Assembly Language");
}

void AsmHelpDialog::debuggingProgramsClicked()
{
    selectItem("Debugging Programs");
}

void AsmHelpDialog::writingTrapHandlersClicked()
{
    selectItem("Writing Trap Handlers");
}

void AsmHelpDialog::pep9ReferenceClicked()
{
    selectItem("Pep/9 Reference");
}

void AsmHelpDialog::examplesClicked()
{
    selectItem("Examples");
}

void AsmHelpDialog::operatingSystemClicked()
{
    selectItem("Pep/9 Operating System");
}

// Helper Functions
QString AsmHelpDialog::getCode(Enu::EPane &destPane, Enu::EPane &inputDest, QString &input)
{
    bool isHelpSubCat = ui->treeWidget->currentIndex().parent().isValid();
    int row = ui->treeWidget->currentIndex().row();
    int parentRow = ui->treeWidget->currentIndex().parent().row();

    if (!isHelpSubCat && row == eOS) {         // Pep/9 Operating System
        destPane = Enu::EPane::ESource;
        return ui->leftTextEdit->toPlainText();
    }

    if (parentRow == eEXAMPLES) {
        if (row == eFIG433) {
            destPane = Enu::EPane::EObject;
            return Pep::resToString(":/help-asm/figures/fig0433.pepo", false);
        }
        else if (row == eFIG435) {
            destPane = Enu::EPane::EObject;
            inputDest = Enu::EPane::EBatchIO;
            input = "up";
            return Pep::resToString(":/help-asm/figures/fig0435.pepo", false);
        }
        else if (row == eFIG436) {
            destPane = Enu::EPane::EObject;
            return Pep::resToString(":/help-asm/figures/fig0436.pepo", false);
        }
        else if (row == eFIG437) {
            destPane = Enu::EPane::EObject;
            return Pep::resToString(":/help-asm/figures/fig0437.pepo", false);
        }
        else if (row == eFIG506) {
            input = "up";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG511) {
            input = "-479";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG512) {
            input = "-479";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG515) {
            input = "-479";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG522) {
            input = "M 419";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG527 || row == eFIG604) {
            input = "68 84";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG606) {
            input = "-25";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG608) {
            input = "75";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG610) {
            input = "Hello, world!*";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG616) {
            input = "3 -15 25";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG621 || row == eFIG623) {
            input = "12  3 13 17 34 27 23 25 29 16 10 0 2";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG632) {
            input = "25";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG634) {
            input = "60 70 80 90";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG636) {
            input = "2 26 -3 9";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG638) {
            input = "5  40 50 60 70 80";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG646) {
            input = "bj 32 m";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG648) {
            input = "10 20 30 40 -9999";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == eFIG627 || row == eFIG629 || row == eFIG640) {
            inputDest = Enu::EPane::ETerminal;
        }

    }
    if (parentRow == ePROBLEMS) {
        if (row == eEXER804) {
            input = "37";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == ePROB826) {
            input = "7";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == ePROB827) {
            input = "7 3";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == ePROB828) {
            input = "5 9";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == ePROB831) {
            input = "12 7";
            inputDest = Enu::EPane::EBatchIO;
        }
        else if (row == ePROB832) {
            input = "32773 6";
            inputDest = Enu::EPane::EBatchIO;
        }
    }
    destPane = Enu::EPane::ESource;
    return ui->leftTextEdit->toPlainText();
}

bool AsmHelpDialog::hasFocus()
{
    return ui->leftTextEdit->hasFocus() || ui->rightCppTextEdit->hasFocus() || ui->webView->hasFocus();
}

void AsmHelpDialog::copy()
{
    if (ui->leftTextEdit->hasFocus()) {
        ui->leftTextEdit->copy();
    } else if (ui->rightCppTextEdit->hasFocus()) {
        ui->rightCppTextEdit->copy();
    } else if (ui->webView->hasFocus()) {
        QApplication::clipboard()->setText(ui->webView->selectedText());
    }
}

void AsmHelpDialog::setCopyButtonDisabled(bool b)
{
    ui->copyToSourceButton->setDisabled(b);
}

void AsmHelpDialog::onFontChanged(QFont font)
{
    ui->leftTextEdit->setFont(font);
    ui->rightPepTextEdit->setFont(font);
    ui->rightCppTextEdit->setFont(font);
}

void AsmHelpDialog::onDarkModeChanged(bool darkMode)
{
    if(darkMode)
    {
        leftHighlighter->rebuildHighlightingRules(PepColors::darkMode);
        rightPepHighlighter->rebuildHighlightingRules(PepColors::darkMode);
        rightCppHighlighter->rebuildHighlightingRules(PepColors::darkMode);
    }
    else
    {
        leftHighlighter->rebuildHighlightingRules(PepColors::lightMode);
        rightPepHighlighter->rebuildHighlightingRules(PepColors::lightMode);
        rightCppHighlighter->rebuildHighlightingRules(PepColors::lightMode);
    }
    leftHighlighter->rehighlight();
    rightPepHighlighter->rehighlight();
    rightCppHighlighter->rehighlight();
}
