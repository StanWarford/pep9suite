// File: objectcodepane.cpp
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

#include "microobjectcodepane.h"
#include "ui_microobjectcodepane.h"
#include "microcodeprogram.h"
#include "pep.h"
#include <QPainter>
#include <QDebug>
#include <QTextEdit>
#include <QTextItem>
#include <QTableWidget>
#include <QStringList>
#include <QKeyEvent>
#include <rotatedheaderview.h>
#include "cpucontrolsection.h"
#include "microcode.h"
#include "symboltable.h"
#include "symbolentry.h"
#include "symbolvalue.h"
MicroObjectCodePane::MicroObjectCodePane(QWidget *parent) :
    QWidget(parent), rowCount(0),model(new QStandardItemModel()),inSimulation(false),
    ui(new Ui::MicroObjectCodePane)
{
    ui->setupUi(this);
    QFont font(Pep::codeFont);
    font.setPointSize(Pep::codeFontSize);
    font.setStyleHint(QFont::TypeWriter);
    ui->codeTable->setModel(model);
    rotatedHeaderView= new RotatedHeaderView(Qt::Horizontal,ui->codeTable);
    rotatedHeaderView->setModel(ui->codeTable->model());
    selectionModel= new DisableSelectionModel(ui->codeTable->model());
    ui->codeTable->setSelectionModel(selectionModel);
    ui->codeTable->setHorizontalHeader(rotatedHeaderView);
    ui->codeTable->setFont(font);
    ui->codeTable->verticalHeader()->setDefaultSectionSize(12);
    ui->codeTable->verticalHeader()->setDefaultAlignment(Qt::AlignRight|Qt::AlignJustify);
    ui->codeTable->horizontalHeader()->setDefaultSectionSize(15);
    ui->codeTable->setShowGrid(false);
    model->setRowCount(0);
    initCPUModelState();
    //Connect to disabler
    connect(this, &MicroObjectCodePane::beginSimulation, selectionModel, &DisableSelectionModel::onBeginSimulation);
    connect(this, &MicroObjectCodePane::endSimulation, selectionModel, &DisableSelectionModel::onEndSimulation);

    //ui->codeTabl
}

MicroObjectCodePane::~MicroObjectCodePane()
{
    delete ui;
    delete program;
    delete model;
    delete rotatedHeaderView;
}

void MicroObjectCodePane::initCPUModelState()
{

    setObjectCode();
    clearSimulationView();
    assignHeaders();
}

void MicroObjectCodePane::setObjectCode()
{

    setObjectCode(new MicrocodeProgram(),nullptr);
}

void MicroObjectCodePane::setObjectCode(MicrocodeProgram* program,SymbolTable* symbolTable)
{
    assignHeaders();
    if(this->program==nullptr)
    {
        delete this->program;
    }
    this->program = program;
    this->symTable=symbolTable;
    int rowNum=0,colNum=0;
    QList<Enu::EControlSignals> controls = Pep::memControlToMnemonMap.keys();
    controls.append(Pep::decControlToMnemonMap.keys());
    QList<Enu::EClockSignals> clocks = Pep::clockControlToMnemonMap.keys();
    model->setRowCount(0);
    for(MicroCodeBase* row : program->getObjectCode())
    {
        if(!row->isMicrocode())
        {
           continue;
        }
        colNum=0;
        model->insertRow(rowNum);
        for(auto col : controls)
        {
            auto x = ((MicroCode*)row)->getControlSignal(col);
            if(x!=Enu::signalDisabled)
            {
                auto y = new QStandardItem(QString::number(x));
                y->setTextAlignment(Qt::AlignCenter);
                model->setItem(rowNum,colNum,y);
            }
            colNum++;
        }
        for(auto col : clocks)
        {
            auto x = ((MicroCode*)row)->getClockSignal(col);
            if(x!=false)
            {
                auto y = new QStandardItem(QString::number(x));
                y->setTextAlignment(Qt::AlignCenter);
                model->setItem(rowNum,colNum,y);
            }
            colNum++;
        }
        auto y = new QStandardItem(QString::number(((MicroCode*)row)->getBranchFunction()));
        y->setTextAlignment(Qt::AlignCenter);
        model->setItem(rowNum,colNum++,y);
        y = new QStandardItem(QString::number(((MicroCode*)row)->getTrueTarget()->getValue()));
        y->setTextAlignment(Qt::AlignCenter);
        model->setItem(rowNum,colNum++,y);
        y = new QStandardItem(QString::number(((MicroCode*)row)->getFalseTarget()->getValue()));
        y->setTextAlignment(Qt::AlignCenter);
        model->setItem(rowNum,colNum++,y);
        y = new QStandardItem(((MicroCode*)row)->getSymbol()->getName());
        y->setTextAlignment(Qt::AlignCenter);
        model->setItem(rowNum,colNum++,y);
        rowNum++;
    }
    ui->codeTable->resizeRowsToContents();
    ui->codeTable->resizeColumnsToContents();
}

void MicroObjectCodePane::highlightCurrentInstruction()
{
    rowCount = CPUControlSection::getInstance()->getLineNumber();
    selectionModel->forceSelectRow(rowCount);
    ui->codeTable->setCurrentIndex(model->index(rowCount,0));
    inSimulation=true;
}

void MicroObjectCodePane::clearSimulationView()
{
    ui->codeTable->clearSelection();
    rowCount=0;
    inSimulation=false;
}

void MicroObjectCodePane::copy()
{
    //ui->plainTextEdit->copy();
}

void MicroObjectCodePane::assignHeaders()
{
    QList<Enu::EControlSignals> controls = Pep::memControlToMnemonMap.keys();
    controls.append(Pep::decControlToMnemonMap.keys());
    QList<Enu::EClockSignals> clocks = Pep::clockControlToMnemonMap.keys();
    QMetaEnum nControls = QMetaEnum::fromType<Enu::EControlSignals>();
    QMetaEnum nClocks = QMetaEnum::fromType<Enu::EClockSignals>();
    QList<QString> headers;
    int size=controls.size()+clocks.size();
    for(auto x : controls)
    {
        headers.append(QString(nControls.valueToKey(x)));
    }
    for(auto x : clocks)
    {
        headers.append(QString(nClocks.valueToKey(x)));
    }
    headers.append("BRF");
    headers.append("T Trgt");
    headers.append("F Trgt");
    headers.append("Symbol");
    model->setColumnCount(size+4);
    model->setHorizontalHeaderLabels(headers);
    for(int x=0;x<size;x++)
    {
        model->horizontalHeaderItem(x)->setTextAlignment(Qt::AlignVCenter);
    }
    ui->codeTable->horizontalHeader()->setVisible(true);
    ui->codeTable->resizeColumnsToContents();
}

void MicroObjectCodePane::onSimulationStarted()
{
    emit beginSimulation();
}

void MicroObjectCodePane::onSimulationFinished()
{
    emit endSimulation();
}

void MicroObjectCodePane::onDarkModeChanged(bool)
{
    ui->codeTable->verticalHeader()->setFont(QFont(Pep::codeFont,Pep::codeFontSize));
    //ui->codeTable->resizeRowsToContents();
}

void MicroObjectCodePane::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MicroObjectCodePane::highlightOnFocus()
{
    if (ui->codeTable->hasFocus()) {
        ui->label->setAutoFillBackground(true);
    }
    else {
        ui->label->setAutoFillBackground(false);
    }
}
