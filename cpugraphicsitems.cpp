// File: cpugraphicsitems.cpp
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

#include "cpugraphicsitems.h"

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QGraphicsScene>
#include <QPainter>

#include <QGraphicsItem>

#include <QDebug>
#include "cpudatasection.h"
#include "pep.h"

#include "shapes_two_byte_data_bus.h"

CpuGraphicsItems::CpuGraphicsItems(Enu::CPUType type, QWidget *widgetParent,
                                                   QGraphicsItem *itemParent,
                                                   QGraphicsScene *scene)
    : QGraphicsItem(itemParent),labelVec(),editorVector(),
      parent(widgetParent),parentScene(scene),dataSection(CPUDataSection::getInstance()),
      colorScheme(&PepColors::lightMode)
{    
    // http://colrd.com/image-dna/23448/

    // save our current model for this set of items;
    model = type;
    // ************************************
    // two byte exclusive stuff
    // ************************************
    MARMuxerDataLabel = new QLabel("MARMux");
    MARMuxerDataLabel->setGeometry(TwoByteShapes::MARMuxerDataLabel);
    MARMuxerDataLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MARMuxerDataLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MARMuxerDataLabel);

    MDROCk = new QCheckBox("MDROCk");
    MDROCk->setGeometry(TwoByteShapes::MDROCkCheckbox);
    MDROCk->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDROCk);

    MDRECk = new QCheckBox("MDRECk");
    MDRECk->setGeometry(TwoByteShapes::MDRECkCheckbox);
    MDRECk->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDRECk);

    MDROMuxLabel = new QLabel("MDROMux");
    MDROMuxLabel->setGeometry(TwoByteShapes::MDROMuxLabel);
    MDROMuxLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDROMuxLabel);
    MDROMuxerDataLabel = new QLabel("MDROMux");
    MDROMuxerDataLabel->setGeometry(TwoByteShapes::MDROMuxerDataLabel);
    MDROMuxerDataLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MDROMuxerDataLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDROMuxerDataLabel);
    MDROMuxTristateLabel = new TristateLabel(0, TristateLabel::Tristate);
    MDROMuxTristateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MDROMuxTristateLabel->setGeometry(TwoByteShapes::MDROMuxTristateLabel);
    MDROMuxTristateLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDROMuxTristateLabel);

    MDREMuxLabel = new QLabel("MDREMux");
    MDREMuxLabel->setGeometry(TwoByteShapes::MDREMuxLabel);
    MDREMuxLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDREMuxLabel);
    MDREMuxerDataLabel = new QLabel("MDREMux");
    MDREMuxerDataLabel->setGeometry(TwoByteShapes::MDREMuxerDataLabel);
    MDREMuxerDataLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MDREMuxerDataLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDREMuxerDataLabel);
    MDREMuxTristateLabel = new TristateLabel(0, TristateLabel::Tristate);
    MDREMuxTristateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MDREMuxTristateLabel->setGeometry(TwoByteShapes::MDREMuxTristateLabel);
    MDREMuxTristateLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDREMuxTristateLabel);

    EOMuxLabel = new QLabel("EOMux");
    EOMuxLabel->setGeometry(TwoByteShapes::EOMuxLabel);
    EOMuxLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(EOMuxLabel);

    EOMuxerDataLabel = new QLabel("EOMux");
    EOMuxerDataLabel->setGeometry(TwoByteShapes::EOMuxerDataLabel);
    EOMuxerDataLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    EOMuxerDataLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(EOMuxerDataLabel);

    EOMuxTristateLabel = new TristateLabel(0, TristateLabel::Tristate);
    EOMuxTristateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    EOMuxTristateLabel->setGeometry(TwoByteShapes::EOMuxTristateLabel);
    EOMuxTristateLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(EOMuxTristateLabel);

    MDRELabel = new QLabel("0x00");
    MDRELabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MDRELabel->setGeometry(TwoByteShapes::MDRELabel);
    MDRELabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDRELabel);

    MDROLabel = new QLabel("0x00");
    MDROLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MDROLabel->setGeometry(TwoByteShapes::MDROLabel);
    MDROLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDROLabel);

    MARMuxLabel = new QLabel("MARMux");
    MARMuxLabel->setGeometry(TwoByteShapes::MARMuxLabel);
    MARMuxLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MARMuxLabel);
    MARMuxTristateLabel = new TristateLabel(0, TristateLabel::Tristate);
    MARMuxTristateLabel->setGeometry(TwoByteShapes::MARMuxTristateLabel);
    MARMuxTristateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MARMuxTristateLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MARMuxTristateLabel);


    // ************************************
    // end two byte exclusive stuff
    // ************************************


    // MARA & MARB
    MARALabel = new QLabel("0x00");
    MARALabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MARALabel->setAutoFillBackground(false);
    MARALabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MARALabel);
    MARBLabel = new QLabel("0x00");
    MARBLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MARBLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MARBLabel);

    // LoadCk
    loadCk = new QCheckBox("LoadCk");
    loadCk->setGeometry(OneByteShapes::loadCkCheckbox);
    loadCk->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(loadCk);

    // C
    // Note: the line edits must be added first, otherwise they cover the
    //  labels that go with them.
    QRegExp cbaRegExp("^((3[0-1])|([0-2][0-9])|([0-9]))$");
    cLineEdit = new QLineEdit();
    cLineEdit->setAlignment(Qt::AlignCenter);
    cLineEdit->setGeometry(OneByteShapes::cLineEdit);
    cLineEdit->setValidator(new QRegExpValidator(cbaRegExp, cLineEdit));
    cLineEdit->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(cLineEdit);
    cLabel = new QLabel("C");
    cLabel->setGeometry(OneByteShapes::cLabel);
    cLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(cLabel);

    // B
    bLineEdit = new QLineEdit();
    bLineEdit->setAlignment(Qt::AlignCenter);
    bLineEdit->setGeometry(OneByteShapes::bLineEdit);
    bLineEdit->setValidator(new QRegExpValidator(cbaRegExp, bLineEdit));
    bLineEdit->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(bLineEdit);
    bLabel = new QLabel("B");
    bLabel->setGeometry(OneByteShapes::bLabel);
    bLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(bLabel);

    // A
    aLineEdit = new QLineEdit();
    aLineEdit->setAlignment(Qt::AlignCenter);
    aLineEdit->setGeometry(OneByteShapes::aLineEdit);
    aLineEdit->setValidator(new QRegExpValidator(cbaRegExp, aLineEdit));
    aLineEdit->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(aLineEdit);
    aLabel = new QLabel("A");
    aLabel->setGeometry(OneByteShapes::aLabel);
    aLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(aLabel);

    // MARCk
    MARCk = new QCheckBox("MARCk");
    MARCk->setGeometry(OneByteShapes::MARCkCheckbox);
    MARCk->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MARCk);

    // MDRCk
    MDRCk = new QCheckBox("MDRCk");
    MDRCk->setGeometry(OneByteShapes::MDRCkCheckbox);
    MDRCk->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDRCk);

    // AMux
    aMuxLabel = new QLabel("AMux");
    aMuxLabel->setGeometry(OneByteShapes::aMuxLabel);
    aMuxLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(aMuxLabel);
    aMuxerDataLabel = new QLabel("AMux");
    aMuxerDataLabel->setGeometry(OneByteShapes::aMuxerDataLabel);
    aMuxerDataLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    aMuxerDataLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(aMuxerDataLabel);
    aMuxTristateLabel = new TristateLabel(0, TristateLabel::Tristate);
    aMuxTristateLabel->setGeometry(OneByteShapes::aMuxTristateLabel);
    aMuxTristateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    aMuxTristateLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(aMuxTristateLabel);

    // MDRMux
    MDRMuxLabel = new QLabel("MDRMux");
    MDRMuxLabel->setGeometry(OneByteShapes::MDRMuxLabel);
    MDRMuxLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDRMuxLabel);
    MDRMuxerDataLabel = new QLabel("MDRMux");
    MDRMuxerDataLabel->setGeometry(OneByteShapes::MDRMuxerDataLabel);
    MDRMuxerDataLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MDRMuxerDataLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDRMuxerDataLabel);
    MDRMuxTristateLabel = new TristateLabel(0, TristateLabel::Tristate);
    MDRMuxTristateLabel->setGeometry(OneByteShapes::MDRMuxTristateLabel);
    MDRMuxTristateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MDRMuxTristateLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDRMuxTristateLabel);
    MDRLabel = new QLabel("0x00");
    MDRLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MDRLabel->setGeometry(OneByteShapes::MDRLabel);
    MDRLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MDRLabel);


    // CMux
    cMuxLabel = new QLabel("CMux");
    cMuxLabel->setGeometry(OneByteShapes::cMuxLabel);
    cMuxLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(cMuxLabel);
    cMuxerLabel = new QLabel("CMux");
    cMuxerLabel->setGeometry(OneByteShapes::cMuxerLabel);
    cMuxerLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    cMuxerLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(cMuxerLabel);
    cMuxTristateLabel = new TristateLabel(0, TristateLabel::Tristate);
    cMuxTristateLabel->setGeometry(OneByteShapes::cMuxTristateLabel);
    cMuxTristateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    cMuxTristateLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(cMuxTristateLabel);

    // ALU
    // keep this before the label that goes with it, or the line edit
    //  appears on top of the label
    ALULineEdit = new QLineEdit();
    ALULineEdit->setAlignment(Qt::AlignCenter);
    ALULineEdit->setGeometry(OneByteShapes::ALULineEdit);
    ALULineEdit->setValidator(new QRegExpValidator(
                                  QRegExp("^((1[0-5])|(0[0-9])|[0-9])$"),
                                  ALULineEdit));
    ALULineEdit->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(ALULineEdit);
    ALULabel = new QLabel("ALU");
    ALULabel->setGeometry(OneByteShapes::ALULabel);
    ALULabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(ALULabel);

    ALUFunctionLabel = new QLabel("");
    ALUFunctionLabel->setGeometry(OneByteShapes::ALUFunctionLabel);
    ALUFunctionLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ALUFunctionLabel->setFont(QFont(Pep::labelFont, Pep::labelFontSizeSmall));
    scene->addWidget(ALUFunctionLabel);

    // ALU shape
    ALUPoly = scene->addPolygon(OneByteShapes::ALUPoly,
                                QPen(QBrush(colorScheme->combCircuitBlue),
                                     2, Qt::SolidLine,
                                     Qt::SquareCap,
                                     Qt::MiterJoin),
                                QBrush(colorScheme->aluColor));
    ALUPoly->setZValue(-1);

    // CSMux
    CSMuxLabel = new QLabel("CSMux");
    CSMuxLabel->setGeometry(OneByteShapes::CSMuxLabel);
    CSMuxLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(CSMuxLabel);
    CSMuxerDataLabel = new QLabel("CSMux");
    CSMuxerDataLabel->setGeometry(OneByteShapes::CSMuxerDataLabel);
    CSMuxerDataLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    CSMuxerDataLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(CSMuxerDataLabel);
    CSMuxTristateLabel = new TristateLabel(0, TristateLabel::Tristate);
    CSMuxTristateLabel->setGeometry(OneByteShapes::CSMuxTristateLabel);
    CSMuxTristateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    CSMuxTristateLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(CSMuxTristateLabel);

    // SCk
    SCkCheckBox = new QCheckBox ("SCk");
    SCkCheckBox->setGeometry(OneByteShapes::SCkCheckBox);
    SCkCheckBox->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(SCkCheckBox);
    sBitLabel = new TristateLabel(0, TristateLabel::ZeroOne);
    sBitLabel->setText("0");
    sBitLabel->setGeometry(OneByteShapes::sBitLabel);
    sBitLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    sBitLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(sBitLabel);

    // CCk
    CCkCheckBox = new QCheckBox ("CCk");
    CCkCheckBox->setGeometry(OneByteShapes::CCkCheckBox);
    CCkCheckBox->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(CCkCheckBox);
    cBitLabel = new TristateLabel(0, TristateLabel::ZeroOne);
    cBitLabel->setText("0");
    cBitLabel->setGeometry(OneByteShapes::cBitLabel);
    cBitLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    cBitLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(cBitLabel);

    // VCk
    VCkCheckBox = new QCheckBox("VCk");
    VCkCheckBox->setGeometry(OneByteShapes::VCkCheckBox);
    VCkCheckBox->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(VCkCheckBox);
    vBitLabel = new TristateLabel(0, TristateLabel::ZeroOne);
    vBitLabel->setText("0");
    vBitLabel->setGeometry(OneByteShapes::vBitLabel);
    vBitLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    vBitLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(vBitLabel);

    // AndZ
    AndZLabel = new QLabel("AndZ");
    AndZLabel->setGeometry(OneByteShapes::AndZLabel);
    AndZLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(AndZLabel);
    AndZTristateLabel = new TristateLabel(0, TristateLabel::Tristate);
    AndZTristateLabel->setGeometry(OneByteShapes::AndZTristateLabel);
    AndZTristateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    AndZTristateLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(AndZTristateLabel);

    AndZMuxLabel = new QLabel("AndZ");
    AndZMuxLabel->setGeometry(OneByteShapes::AndZMuxLabel);
    AndZMuxLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    AndZMuxLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(AndZMuxLabel);

    // ZCk
    ZCkCheckBox = new QCheckBox("ZCk");
    ZCkCheckBox->setGeometry(OneByteShapes::ZCkCheckBox);
    ZCkCheckBox->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(ZCkCheckBox);
    zBitLabel = new TristateLabel(0, TristateLabel::ZeroOne);
    zBitLabel->setText("0");
    zBitLabel->setGeometry(OneByteShapes::zBitLabel);
    zBitLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    zBitLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(zBitLabel);

    // NCk
    NCkCheckBox = new QCheckBox ("NCk");
    NCkCheckBox->setGeometry(OneByteShapes::NCkCheckBox);
    NCkCheckBox->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(NCkCheckBox);
    nBitLabel = new TristateLabel(0, TristateLabel::ZeroOne);
    nBitLabel->setText("0");
    nBitLabel->setGeometry(OneByteShapes::nBitLabel);
    nBitLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    nBitLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(nBitLabel);

    // MemRead/Write
    MemWriteLabel = new QLabel("MemWrite");
    MemWriteLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MemWriteLabel);
    MemWriteTristateLabel = new TristateLabel(0, TristateLabel::OneUndefined);
    MemWriteTristateLabel->setGeometry(OneByteShapes::MemWriteTristateLabel);
    MemWriteTristateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MemWriteTristateLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MemWriteTristateLabel);

    MemReadLabel = new QLabel("MemRead");
    MemReadLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MemReadLabel);
    MemReadTristateLabel = new TristateLabel(0, TristateLabel::OneUndefined);
    MemReadTristateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    MemReadTristateLabel->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(MemReadTristateLabel);


    // Registers
    regBank = scene->addRect(OneByteShapes::RegBank, QPen(QBrush(QColor(Qt::red), Qt::SolidPattern),
                                        2, Qt::DotLine,
                                        Qt::SquareCap,
                                        Qt::MiterJoin), QBrush(colorScheme->seqCircuitColor));

    QLabel *ph;
    ph = new QLabel("0,1");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegNoRect(1, 1));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("A");
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegLabelRect(1, 1));
    scene->addWidget(ph);
    labelVec.append(ph);
    aRegLineEdit = new QLineEdit("0x0000");
    aRegLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    aRegLineEdit->setGeometry(OneByteShapes::aRegLineEdit);
    aRegLineEdit->setValidator(new QRegExpValidator(
                                   QRegExp("(0x){0,1}([0-9a-fA-F]){0,4}"),
                                   aRegLineEdit));
    aRegLineEdit->setPalette(QPalette(colorScheme->seqCircuitColor));
    aRegLineEdit->setFrame(false);
    aRegLineEdit->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(aRegLineEdit);
    editorVector.append(aRegLineEdit);
    //    QObject::connect(A, SIGNAL(valueChanged()),
                           //this, SLOT(slotRegisterChanged()));

    ph = new QLabel("2,3");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegNoRect(1, 2));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("X");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegLabelRect(1, 2));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    xRegLineEdit = new QLineEdit("0x0000");
    xRegLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    xRegLineEdit->setGeometry(OneByteShapes::xRegLineEdit);
    xRegLineEdit->setValidator(new QRegExpValidator(
                                   QRegExp("(0x){0,1}([0-9a-fA-F]){0,4}"),
                                   xRegLineEdit));
    xRegLineEdit->setPalette(QPalette(colorScheme->seqCircuitColor));
    xRegLineEdit->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    xRegLineEdit->setFrame(false);
    scene->addWidget(xRegLineEdit);
    editorVector.append(xRegLineEdit);

    ph = new QLabel("4,5");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegNoRect(1, 3));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("SP");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegLabelRect(1, 3));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    spRegLineEdit = new QLineEdit("0x0000");
    spRegLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    spRegLineEdit->setGeometry(OneByteShapes::spRegLineEdit);
    spRegLineEdit->setValidator(new QRegExpValidator(
                                    QRegExp("(0x){0,1}[0-9a-fA-F]{0,4}"),
                                    spRegLineEdit));
    spRegLineEdit->setPalette(QPalette(colorScheme->seqCircuitColor));
    spRegLineEdit->setFrame(false);
    spRegLineEdit->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(spRegLineEdit);
    editorVector.append(spRegLineEdit);

    ph = new QLabel("6,7");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegNoRect(1, 4));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("PC");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegLabelRect(1, 4));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    pcRegLineEdit = new QLineEdit("0x0000");
    pcRegLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    pcRegLineEdit->setGeometry(OneByteShapes::pcRegLineEdit);
    pcRegLineEdit->setValidator(new QRegExpValidator(
                                    QRegExp("(0x){0,1}([0-9a-fA-F]){0,4}"),
                                    pcRegLineEdit));
    pcRegLineEdit->setPalette(QPalette(colorScheme->seqCircuitColor));
    pcRegLineEdit->setFrame(false);
    pcRegLineEdit->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(pcRegLineEdit);
    editorVector.append(pcRegLineEdit);


    ph = new QLabel("8-10");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegNoRect(2, 1));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("IR");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegLabelRect(2, 1));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    irRegLineEdit = new QLineEdit("0x000000");
    irRegLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    irRegLineEdit->setGeometry(OneByteShapes::irRegLineEdit);
    irRegLineEdit->setValidator(new QRegExpValidator(
                                    QRegExp("(0x){0,1}([0-9a-fA-F]){0,6}"),
                                    irRegLineEdit));
    irRegLineEdit->setPalette(QPalette(colorScheme->seqCircuitColor));
    irRegLineEdit->setFrame(false);
    irRegLineEdit->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(irRegLineEdit);
    editorVector.append(irRegLineEdit);

    ph = new QLabel("11");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegNoRect(2, 2));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("T1");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegLabelRect(2, 2));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    t1RegLineEdit = new QLineEdit("0x00");
    t1RegLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    t1RegLineEdit->setGeometry(OneByteShapes::t1RegLineEdit);
    t1RegLineEdit->setValidator(new QRegExpValidator(
                                    QRegExp("(0x){0,1}([0-9a-fA-F]){0,2}"),
                                    t1RegLineEdit));
    t1RegLineEdit->setPalette(QPalette(colorScheme->seqCircuitColor));
    t1RegLineEdit->setFrame(false);
    t1RegLineEdit->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(t1RegLineEdit);
    editorVector.append(t1RegLineEdit);

    ph = new QLabel("12,13");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegNoRect(2, 3));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("T2");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegLabelRect(2, 3));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    t2RegLineEdit = new QLineEdit("0x0000");
    t2RegLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    t2RegLineEdit->setGeometry(OneByteShapes::t2RegLineEdit);
    t2RegLineEdit->setValidator(new QRegExpValidator(
                                    QRegExp("(0x){0,1}([0-9a-fA-F]){0,4}"),
                                    t2RegLineEdit));
    t2RegLineEdit->setPalette(QPalette(colorScheme->seqCircuitColor));
    t2RegLineEdit->setFrame(false);
    t2RegLineEdit->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(t2RegLineEdit);
    editorVector.append(t2RegLineEdit);

    ph = new QLabel("14,15");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegNoRect(2, 4));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("T3");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegLabelRect(2, 4));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    t3RegLineEdit = new QLineEdit("0x0000");
    t3RegLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    t3RegLineEdit->setGeometry(OneByteShapes::t3RegLineEdit);
    t3RegLineEdit->setValidator(new QRegExpValidator(
                                    QRegExp("(0x){0,1}([0-9a-fA-F]){0,4}"),
                                    t3RegLineEdit));
    t3RegLineEdit->setPalette(QPalette(colorScheme->seqCircuitColor));
    t3RegLineEdit->setFrame(false);
    t3RegLineEdit->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(t3RegLineEdit);
    editorVector.append(t3RegLineEdit);

    ph = new QLabel("16,17");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegNoRect(3, 1));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("T4");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegLabelRect(3, 1));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    t4RegLineEdit = new QLineEdit("0x0000");
    t4RegLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    t4RegLineEdit->setGeometry(OneByteShapes::t4RegLineEdit);
    t4RegLineEdit->setValidator(new QRegExpValidator(
                                    QRegExp("(0x){0,1}([0-9a-fA-F]){0,4}"),
                                    t4RegLineEdit));
    t4RegLineEdit->setPalette(QPalette(colorScheme->seqCircuitColor));
    t4RegLineEdit->setFrame(false);
    t4RegLineEdit->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(t4RegLineEdit);
    editorVector.append(t4RegLineEdit);

    ph = new QLabel("18,19");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegNoRect(3, 2));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("T5");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegLabelRect(3, 2));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    t5RegLineEdit = new QLineEdit("0x0000");
    t5RegLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    t5RegLineEdit->setGeometry(OneByteShapes::t5RegLineEdit);
    t5RegLineEdit->setValidator(new QRegExpValidator(
                                    QRegExp("(0x){0,1}([0-9a-fA-F]){0,4}"),
                                    t5RegLineEdit));
    t5RegLineEdit->setPalette(QPalette(colorScheme->seqCircuitColor));
    t5RegLineEdit->setFrame(false);
    t5RegLineEdit->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(t5RegLineEdit);
    editorVector.append(t5RegLineEdit);

    ph = new QLabel("20,21");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegNoRect(3, 3));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("T6");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegLabelRect(3, 3));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    t6RegLineEdit = new QLineEdit("0x0000");
    t6RegLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    t6RegLineEdit->setGeometry(OneByteShapes::t6RegLineEdit);
    t6RegLineEdit->setValidator(new QRegExpValidator(
                                    QRegExp("(0x){0,1}([0-9a-fA-F]){0,4}"),
                                    t6RegLineEdit));
    t6RegLineEdit->setPalette(QPalette(colorScheme->seqCircuitColor));
    t6RegLineEdit->setFrame(false);
    t6RegLineEdit->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(t6RegLineEdit);
    editorVector.append(t6RegLineEdit);

    ph = new QLabel("22,23");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegNoRect(3, 4));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("M1");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(OneByteShapes::getRegLabelRect(3, 4));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("0x0001");
    ph->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    ph->setGeometry(OneByteShapes::m1RegLabel);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(ph);
    labelVec.append(ph);

    ph = new QLabel("24,25");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setGeometry(OneByteShapes::getRegNoRect(4, 1));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("M2");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setGeometry(OneByteShapes::getRegLabelRect(4, 1));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("0x0203");
    ph->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    ph->setGeometry(OneByteShapes::m2RegLabel);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(ph);
    labelVec.append(ph);

    ph = new QLabel("26,27");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setGeometry(OneByteShapes::getRegNoRect(4, 2));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("M3");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setGeometry(OneByteShapes::getRegLabelRect(4, 2));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("0x0408");
    ph->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    ph->setGeometry(OneByteShapes::m3RegLabel);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(ph);
    labelVec.append(ph);

    ph = new QLabel("28,29");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setGeometry(OneByteShapes::getRegNoRect(4, 3));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("M4");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setGeometry(OneByteShapes::getRegLabelRect(4, 3));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("0xF0F6");
    ph->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    ph->setGeometry(OneByteShapes::m4RegLabel);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(ph);
    labelVec.append(ph);

    ph = new QLabel("30,31");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setGeometry(OneByteShapes::getRegNoRect(4, 4));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("M5");
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setGeometry(OneByteShapes::getRegLabelRect(4, 4));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeLarge));
    ph->setFont(QFont(ph->font().family(), ph->font().pointSize(), QFont::Bold));
    scene->addWidget(ph);
    labelVec.append(ph);
    ph = new QLabel("0xFEFF");
    ph->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    ph->setGeometry(OneByteShapes::m5RegLabel);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(ph);
    labelVec.append(ph);

    //outline around register bank
    regBankOutline = scene->addRect(OneByteShapes::RegBank, QPen(QBrush(QColor(colorScheme->arrowColorOn),
                                                       Qt::SolidPattern),
                                                2, Qt::SolidLine, Qt::SquareCap,
                                                Qt::MiterJoin));

    // do stuff based on the current model:
    if (model == Enu::OneByteDataBus) {
        // hide 2 byte bus stuff:
        MDROCk->hide();
        MDRECk->hide();
        MDROMuxTristateLabel->hide();
        MDREMuxTristateLabel->hide();
        EOMuxTristateLabel->hide();
        MARMuxerDataLabel->hide();
        MARMuxTristateLabel->hide();
        MDROMuxerDataLabel->hide();
        MDREMuxerDataLabel->hide();
        MDROMuxLabel->hide();
        MDREMuxLabel->hide();
        EOMuxLabel->hide();
        EOMuxerDataLabel->hide();
        MDRELabel->hide();
        MDROLabel->hide();
        MARMuxLabel->hide();

        // MARBus (MARA/MARB output bus)
        // MAR
        MARALabel->setGeometry(OneByteShapes::MARALabel);
        MARBLabel->setGeometry(OneByteShapes::MARBLabel);

        // MDR
        //scene->addRect(OneByteShapes::MDRLabel);
        // MDR data section

        // MDRBus (output from MDR, right arrow):
        // note: left arrow gets drawn in repaintMemWrite

        // MemRead/Write
        MemWriteLabel->setGeometry(OneByteShapes::MemWriteLabel);
        MemReadLabel->setGeometry(OneByteShapes::MemReadLabel);
        MemWriteTristateLabel->setGeometry(OneByteShapes::MemWriteTristateLabel);
        MemReadTristateLabel->setGeometry(OneByteShapes::MemReadTristateLabel);
    }
    else if (model == Enu::TwoByteDataBus) {
        // hide 1 byte bus stuff:
        MDRCk->hide();
        MDRLabel->hide();
        MDRMuxerDataLabel->hide();
        MDRMuxLabel->hide();
        MDRMuxTristateLabel->hide();

        // MARBus (MARA/MARB output bus)
        scene->addPolygon(TwoByteShapes::MARBus,
                          QPen(QBrush(colorScheme->arrowColorOn), 1), QBrush(colorScheme->combCircuitYellow));

        // ALU drawing:
        ALUPoly->moveBy(TwoByteShapes::controlOffsetX, TwoByteShapes::aluOffsetY);

        // ***************************************
        // fix geometry for the two byte bus
        // ***************************************
        // MAR
        MARALabel->setGeometry(TwoByteShapes::MARALabel);
        MARBLabel->setGeometry(TwoByteShapes::MARBLabel);

        // register signals
        loadCk->setGeometry(TwoByteShapes::loadCkCheckbox);
        cLineEdit->setGeometry(TwoByteShapes::cLineEdit);
        cLabel->setGeometry(TwoByteShapes::cLabel);
        bLineEdit->setGeometry(TwoByteShapes::bLineEdit);
        bLabel->setGeometry(TwoByteShapes::bLabel);
        aLineEdit->setGeometry(TwoByteShapes::aLineEdit);
        aLabel->setGeometry(TwoByteShapes::aLabel);

        // misc control signals
        MARCk->setGeometry(TwoByteShapes::MARCkCheckbox);
        aMuxLabel->setGeometry(TwoByteShapes::aMuxLabel);
        aMuxerDataLabel->setGeometry(TwoByteShapes::aMuxerDataLabel);
        aMuxTristateLabel->setGeometry(TwoByteShapes::aMuxTristateLabel);
        cMuxLabel->setGeometry(TwoByteShapes::cMuxLabel);
        cMuxerLabel->setGeometry(TwoByteShapes::cMuxerLabel);
        cMuxTristateLabel->setGeometry(TwoByteShapes::cMuxTristateLabel);
        ALULineEdit->setGeometry(TwoByteShapes::ALULineEdit);
        ALULabel->setGeometry(TwoByteShapes::ALULabel);
        ALUFunctionLabel->setGeometry(TwoByteShapes::ALUFunctionLabel);

        // status bit control signals
        CSMuxLabel->setGeometry(TwoByteShapes::CSMuxLabel);
        CSMuxerDataLabel->setGeometry(TwoByteShapes::CSMuxerDataLabel);
        CSMuxTristateLabel->setGeometry(TwoByteShapes::CSMuxTristateLabel);
        SCkCheckBox->setGeometry(TwoByteShapes::SCkCheckBox);
        sBitLabel->setGeometry(TwoByteShapes::sBitLabel);
        CCkCheckBox->setGeometry(TwoByteShapes::CCkCheckBox);
        cBitLabel->setGeometry(TwoByteShapes::cBitLabel);
        VCkCheckBox->setGeometry(TwoByteShapes::VCkCheckBox);
        vBitLabel->setGeometry(TwoByteShapes::vBitLabel);
        AndZLabel->setGeometry(TwoByteShapes::AndZLabel);
        AndZTristateLabel->setGeometry(TwoByteShapes::AndZTristateLabel);
        AndZMuxLabel->setGeometry(TwoByteShapes::AndZMuxLabel);
        ZCkCheckBox->setGeometry(TwoByteShapes::ZCkCheckBox);
        zBitLabel->setGeometry(TwoByteShapes::zBitLabel);
        NCkCheckBox->setGeometry(TwoByteShapes::NCkCheckBox);
        nBitLabel->setGeometry(TwoByteShapes::nBitLabel);



        // Status Bits
        CSMuxerDataLabel->setGeometry(TwoByteShapes::CSMuxerDataLabel);
        sBitLabel->setGeometry(TwoByteShapes::sBitLabel);
        cBitLabel->setGeometry(TwoByteShapes::cBitLabel);
        vBitLabel->setGeometry(TwoByteShapes::vBitLabel);
        AndZMuxLabel->setGeometry(TwoByteShapes::AndZMuxLabel);
        zBitLabel->setGeometry(TwoByteShapes::zBitLabel);
        nBitLabel->setGeometry(TwoByteShapes::nBitLabel);


        // MemRead/Write
        MemWriteLabel->setGeometry(TwoByteShapes::MemWriteLabel);
        MemReadLabel->setGeometry(TwoByteShapes::MemReadLabel);
        MemWriteTristateLabel->setGeometry(TwoByteShapes::MemWriteTristateLabel);
        MemReadTristateLabel->setGeometry(TwoByteShapes::MemReadTristateLabel);
    }

    // Status Bits
    onDarkModeChanged(false);
}

CpuGraphicsItems::~CpuGraphicsItems()
{
    delete loadCk;
    delete cLabel;
    delete cLineEdit;
    delete bLabel;
    delete bLineEdit;
    delete aLabel;
    delete aLineEdit;
    delete MARCk;
    delete MARALabel;
    delete MARBLabel;
    delete MDRCk;
    delete aMuxLabel;
    delete aMuxerDataLabel;
    delete aMuxerBorder;
    delete aMuxTristateLabel;
    delete MDRMuxLabel;
    delete MDRMuxerDataLabel;
    delete MDRMuxTristateLabel;
    delete MDRLabel;
    delete cMuxLabel;
    delete cMuxTristateLabel;
    delete cMuxerLabel;
    delete ALULabel;
    delete ALULineEdit;
    delete ALUFunctionLabel;
    delete ALUPoly;
    delete CSMuxLabel;
    delete CSMuxerDataLabel;
    delete CSMuxTristateLabel;
    delete SCkCheckBox;
    delete CCkCheckBox;
    delete VCkCheckBox;
    delete AndZLabel;
    delete AndZTristateLabel;
    delete AndZMuxLabel;
    delete ZCkCheckBox;
    delete NCkCheckBox;
    delete nBitLabel;
    delete zBitLabel;
    delete vBitLabel;
    delete cBitLabel;
    delete sBitLabel;
    delete MemReadLabel;
    delete MemReadTristateLabel;
    delete MemWriteLabel;
    delete MemWriteTristateLabel;

    delete aRegLineEdit;
    delete xRegLineEdit;
    delete spRegLineEdit;
    delete pcRegLineEdit;
    delete irRegLineEdit;
    delete t1RegLineEdit;
    delete t2RegLineEdit;
    delete t3RegLineEdit;
    delete t4RegLineEdit;
    delete t5RegLineEdit;
    delete t6RegLineEdit;

    delete MDROCk;
    delete MDRECk;
    delete MDROMuxTristateLabel;
    delete MDREMuxTristateLabel;
    delete EOMuxTristateLabel;
    delete MARMuxTristateLabel;
    delete MDRELabel;
    delete MDROLabel;
}

QRectF CpuGraphicsItems::boundingRect() const
{
    if (dataSection->getCPUFeatures() == Enu::OneByteDataBus) {
        return QRectF(0,0, 650, 670);
    }
    else if (dataSection->getCPUFeatures() == Enu::TwoByteDataBus) {
        return QRectF(0,0, 650, TwoByteShapes::BottomOfAlu+TwoByteShapes::MemReadYOffsetFromALU+TwoByteShapes::labelTriH+10);
    }
    return QRectF(0,0, 650, 670);
}

bool CpuGraphicsItems::aluHasCorrectOutput()
{
    if (ALULineEdit->text() == "") {
        return false;
    }
    bool ok;
    int alu = ALULineEdit->text().toInt(&ok);
    (void)alu; //Useless cast to silence unused variable warning
    if (!ok) {
        qDebug() << "ALU text to int conversion failed - non-number in the ALU";
        return false;
    }

    if (dataSection->aluFnIsUnary()) { // unary
        if (aMuxTristateLabel->text() == "0") {
            return true;
        }
        else if (aMuxTristateLabel->text() == "1" && aLineEdit->text() != "") {
            return true;
        }
        else {
            return false;
        }
    }
    else { // nonunary
        if (aMuxTristateLabel->text() == "0" && bLineEdit->text() != "") {
            return true;
        }
        else if (aMuxTristateLabel->text() == "1" && aLineEdit->text() != ""
                 && bLineEdit->text() != "") {
            return true;
        }
        else {
            return false;
        }
    }
    return false;
}

void CpuGraphicsItems::paint(QPainter *painter,
                                     const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(colorScheme->arrowColorOn);
    drawDiagramFreeText(painter);
    if(true)
    {
        drawStaticRects(painter);
        drawALUPoly();
        drawRegisterBank();
    }
    // c,b,a select line text
    repaintLoadCk(painter);
    repaintCSelect(painter);
    repaintBSelect(painter);
    repaintASelect(painter);
    repaintMARCk(painter);
    repaintAMuxSelect(painter); // Needs to be painted before buses
    repaintCMuxSelect(painter);
    painter->setPen(colorScheme->arrowColorOn);

    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:

        //Paint the buses in the correct order for the One Byte Bus
        /*
         * In the one byte bus, the buses must be drawn in the folling order C, B, A.
         * The B bus overlaps with the A bus, and the C bus overlaps with the B bus.
         * So, this rendering order prevents graphical issues.
         */
        repaintCBusOneByteModel(painter);
        repaintBBusOneByteModel(painter);
        repaintABusOneByteModel(painter);

        repaintMDRMuxSelect(painter);

        repaintMDRCk(painter);
        break;
    case Enu::TwoByteDataBus:
        repaintMDROCk(painter);
        repaintMDRECk(painter);
        repaintEOMuxSelect(painter);
        repaintMDROSelect(painter);
        repaintMDRESelect(painter);
        repaintMARMUXToMARBuses(painter);
        repaintMDRMuxOutputBuses(painter);
        //Every select line above ALU firt first
        repaintMARMuxSelect(painter);
        repaintMDREToEOMuxBus(painter);
        repaintMDROToEOMuxBus(painter);
        repaintEOMuxOutpusBus(painter);
        repaintABusTwoByteModel(painter);
        repaintBBusTwoByteModel(painter);
        repaintCBusTwoByteModel(painter);

        break;
    default:
        break;
    }



    repaintCSMuxSelect(painter);

    repaintSCk(painter);
    repaintCCk(painter);
    repaintVCk(painter);
    repaintZCk(painter);
    repaintNCk(painter);
    repaintMemCommon(painter);
    repaintMemRead(painter);
    repaintMemWrite(painter);
    repaintSBitOut(painter);
    repaintCBitOut(painter);
    repaintVBitOut(painter);
    repaintZBitOut(painter);
    repaintNBitOut(painter);

    repaintAndZSelect(painter);
    repaintALUSelect(painter);

}

void CpuGraphicsItems::drawDiagramFreeText(QPainter *painter)
{
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(colorScheme->arrowColorOn);
    painter->drawText(7, 320, "Addr");
    painter->drawText(528,92, "5");
    painter->drawText(528,70, "5");
    painter->drawText(528,48, "5");
    painter->save();
    painter->rotate(-90);
    auto font= painter->font();
    font.setPointSize(font.pointSize()*1.3);
    painter->setFont(font);
    painter->drawText(-260,35, "System Bus");
    painter->restore();
    switch(dataSection->getCPUFeatures())
    {
    case Enu::CPUType::OneByteDataBus:
        painter->drawText(7, 395, "Data");
        painter->drawText(372,132, "ABus");
        painter->drawText(433,132, "BBus");
        painter->drawText(300,132, "CBus");
        // alu select line text
        painter->drawText(OneByteShapes::ctrlInputX - 23, ALULineEdit->y() + 5, "4");

        painter->drawText(368,388, "ALU");

        // NZVC data path text
        painter->drawText(314,531, "0");
        painter->drawText(314,541, "0");
        painter->drawText(314,551, "0");
        painter->drawText(314,561, "0");

        painter->drawText(OneByteShapes::MARALabel.x() - 37, OneByteShapes::MARALabel.y() + 13, "MARA");
        painter->drawText(OneByteShapes::MARBLabel.x() - 37, OneByteShapes::MARBLabel.y() + 13, "MARB");

        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawText(7, TwoByteShapes::DataArrowMidpointY+TwoByteShapes::DataArrowOuterYSpread+10, "Data");
        painter->drawText(427,132, "ABus");
        painter->drawText(483,132, "BBus");
        painter->drawText(350,132, "CBus");
        // alu select line text
        painter->drawText(TwoByteShapes::ctrlInputX - 23, ALULineEdit->y() + 5, "4");
        painter->drawText(368 + TwoByteShapes::controlOffsetX,
                          388 + TwoByteShapes::aluOffsetY, "ALU");

        // NZVC data path text
        painter->drawText(314 + TwoByteShapes::controlOffsetX,
                          531 + TwoByteShapes::aluOffsetY, "0");
        painter->drawText(314 + TwoByteShapes::controlOffsetX,
                          541 + TwoByteShapes::aluOffsetY, "0");
        painter->drawText(314 + TwoByteShapes::controlOffsetX,
                          551 + TwoByteShapes::aluOffsetY, "0");
        painter->drawText(314 + TwoByteShapes::controlOffsetX,
                          561 + TwoByteShapes::aluOffsetY, "0");
        painter->drawText(TwoByteShapes::MARALabel.x() - 37, TwoByteShapes::MARALabel.y() + 13, "MARA");
        painter->drawText(TwoByteShapes::MARBLabel.x() - 37, TwoByteShapes::MARBLabel.y() + 13, "MARB");
        break;
    }
}

void CpuGraphicsItems::drawLabels()
{

    QPalette seqColor = QPalette();
    seqColor.setColor(QPalette::Text,colorScheme->arrowColorOn);
    seqColor.setColor(QPalette::WindowText,colorScheme->arrowColorOn);
    seqColor.setColor(QPalette::Base,PepColors::transparent);
    seqColor.setColor(QPalette::Window,PepColors::transparent);

    QPalette combColor = QPalette();
    combColor.setColor(QPalette::WindowText,colorScheme->arrowColorOn);
    combColor.setColor(QPalette::Base,colorScheme->seqCircuitColor);
    combColor.setColor(QPalette::Window,colorScheme->seqCircuitColor);

    QPalette aluLabel = QPalette();
    aluLabel.setColor(QPalette::WindowText,colorScheme->arrowColorOn);
    aluLabel.setColor(QPalette::Base,PepColors::transparent);
    aluLabel.setColor(QPalette::Window,PepColors::transparent);

    //Set Line editors first
    cLineEdit->setPalette(seqColor);
    bLineEdit->setPalette(seqColor);
    aLineEdit->setPalette(seqColor);
    ALULineEdit->setPalette(seqColor);

    //Common labels
    cLabel->setPalette(seqColor);
    bLabel->setPalette(seqColor);
    aLabel->setPalette(seqColor);
    MARALabel->setPalette(combColor);
    MARBLabel->setPalette(combColor);
    MARCk->setPalette(seqColor);
    loadCk->setPalette(seqColor);
    aMuxLabel->setPalette(seqColor);
    aMuxerDataLabel->setPalette(seqColor);
    aMuxTristateLabel->setPalette(seqColor);
    cMuxerLabel->setPalette(seqColor);
    cMuxLabel->setPalette(seqColor);
    cMuxTristateLabel->setPalette(seqColor);
    ALULabel->setPalette(seqColor);
    ALUFunctionLabel->setPalette(aluLabel);
    CSMuxLabel->setPalette(seqColor);
    CSMuxerDataLabel->setPalette(seqColor);
    CSMuxTristateLabel->setPalette(seqColor);
    SCkCheckBox->setPalette(seqColor);
    sBitLabel->setPalette(combColor);
    CCkCheckBox->setPalette(seqColor);
    cBitLabel->setPalette(combColor);
    VCkCheckBox->setPalette(seqColor);
    vBitLabel->setPalette(combColor);
    AndZLabel->setPalette(seqColor);
    AndZMuxLabel->setPalette(seqColor);
    AndZTristateLabel->setPalette(seqColor);
    ZCkCheckBox->setPalette(seqColor);
    zBitLabel->setPalette(combColor);
    NCkCheckBox->setPalette(seqColor);
    nBitLabel->setPalette(combColor);


    MemReadLabel->setPalette(seqColor);
    MemReadTristateLabel->setPalette(seqColor);
    MemWriteLabel->setPalette(seqColor);
    MemWriteTristateLabel->setPalette(seqColor);
    //One byte exclusive labels
    MDRLabel->setPalette(combColor);
    MDRMuxLabel->setPalette(seqColor);
    MDRMuxTristateLabel->setPalette(seqColor);
    MDRMuxerDataLabel->setPalette(seqColor);
    MDRCk->setPalette(seqColor);


    //Two Byte Exclusive Labels
    MARMuxLabel->setPalette(seqColor);
    MARMuxTristateLabel->setPalette(seqColor);
    MARMuxerDataLabel->setPalette(seqColor);

    MDROLabel->setPalette(combColor);
    MDROMuxTristateLabel->setPalette(seqColor);
    MDROMuxLabel->setPalette(seqColor);
    MDROMuxerDataLabel->setPalette(seqColor);
    MDROCk->setPalette(seqColor);
    MDRELabel->setPalette(combColor);
    MDREMuxTristateLabel->setPalette(seqColor);
    MDREMuxLabel->setPalette(seqColor);
    MDREMuxerDataLabel->setPalette(seqColor);
    MDRECk->setPalette(seqColor);

    EOMuxLabel->setPalette(seqColor);
    EOMuxTristateLabel->setPalette(seqColor);
    EOMuxerDataLabel->setPalette(seqColor);
}

void CpuGraphicsItems::drawStaticRects(QPainter* paint)
{
    paint->setPen(QPen(colorScheme->arrowColorOn));
    paint->drawRect(QRectF(cBitLabel->pos(), cBitLabel->size())); // C
    paint->drawRect(QRectF(vBitLabel->pos(), vBitLabel->size())); // V
    paint->drawRect(QRectF(zBitLabel->pos(), zBitLabel->size())); // Z
    paint->drawRect(QRectF(nBitLabel->pos(), nBitLabel->size())); // N
    paint->drawRect(QRectF(sBitLabel->pos(), sBitLabel->size())); // S
    if(CPUDataSection::getInstance()->getCPUFeatures()==Enu::OneByteDataBus)
    {
        auto penOn = QPen(colorScheme->arrowColorOn);
        auto penOff = QPen(colorScheme->arrowColorOff);
        paint->setPen(penOn);
        paint->drawRect(OneByteShapes::MDRLabel);
        paint->drawRect(OneByteShapes::MARBLabel);
        paint->drawRect(OneByteShapes::MARALabel);
        paint->drawRect(OneByteShapes::MDRMuxerDataLabel);

        paint->drawRect(OneByteShapes::cMuxerLabel);
        paint->drawRect(OneByteShapes::aMuxerDataLabel);
        paint->drawRect(QRectF(CSMuxerDataLabel->pos(), CSMuxerDataLabel->size()));
        paint->drawRect(OneByteShapes::AndZMuxLabel);
        paint->drawLine(OneByteShapes::NZVCDataLine);
        // NZVC data path to CMux, vertical black line
        paint->setPen(penOff);
        paint->drawRect(QRectF(CSMuxTristateLabel->pos(), CSMuxTristateLabel->size()));
        paint->drawRect(QRectF(AndZTristateLabel->pos(), AndZTristateLabel->size()));
        paint->drawRect(QRectF(MemReadTristateLabel->pos(),
                              MemReadTristateLabel->size())); //gray
        paint->drawRect(QRectF(MemWriteTristateLabel->pos(),
                              MemWriteTristateLabel->size())); //gray

        paint->drawRect(OneByteShapes::aMuxTristateLabel); //gray
        paint->drawRect(OneByteShapes::cMuxTristateLabel); //gray
        paint->drawRect(OneByteShapes::MDRMuxTristateLabel); //gray
        paint->setPen(penOn);
        paint->setBrush(QBrush(colorScheme->combCircuitYellow));
        paint->drawRect(OneByteShapes::MDRBusOutRect);
        paint->drawPolygon(OneByteShapes::MDRBusOutArrow);
        paint->drawPolygon(OneByteShapes::NZVCDataPath);
        paint->drawPolygon(OneByteShapes::MARBus);
    }
    else if(CPUDataSection::getInstance()->getCPUFeatures()==Enu::TwoByteDataBus)
    {
        auto penOn = QPen(colorScheme->arrowColorOn);
        auto penOff = QPen(colorScheme->arrowColorOff);
        paint->setPen(penOn);
        paint->drawRect(TwoByteShapes::MARBLabel);
        paint->drawRect(TwoByteShapes::MARALabel);
        paint->drawRect(TwoByteShapes::cMuxerLabel);
        paint->drawRect(TwoByteShapes::aMuxerDataLabel);

        paint->drawRect(TwoByteShapes::MARMuxerDataLabel);
        paint->drawRect(TwoByteShapes::MDROMuxerDataLabel);
        paint->drawRect(TwoByteShapes::MDREMuxerDataLabel);
        paint->drawRect(TwoByteShapes::EOMuxerDataLabel);

        paint->drawRect(TwoByteShapes::MDROLabel);
        paint->drawRect(TwoByteShapes::MDRELabel);

        // NZVC data path to CMux, vertical black line
        paint->drawPolygon(TwoByteShapes::NZVCDataPath);
                          //QPen(QBrush(colorScheme->arrowColorOn), 1), QBrush(colorScheme->combCircuitYellow));
        paint->setPen(penOff);
                paint->drawRect(TwoByteShapes::AndZMuxLabel);
        paint->drawLine(QLine(TwoByteShapes::NZVCDataLine));
                paint->drawRect(QRectF(CSMuxerDataLabel->pos(), CSMuxerDataLabel->size()));
        paint->drawRect(QRectF(MemWriteTristateLabel->pos(),
                              MemWriteTristateLabel->size())); //gray
        paint->drawRect(QRectF(MemReadTristateLabel->pos(),
                              MemReadTristateLabel->size())); //gray

        paint->drawRect(TwoByteShapes::aMuxTristateLabel); //gray
        paint->drawRect(TwoByteShapes::cMuxTristateLabel); //gray
        paint->drawRect(QRectF(CSMuxTristateLabel->pos(), CSMuxTristateLabel->size())); //gray
        paint->drawRect(QRectF(AndZTristateLabel->pos(), AndZTristateLabel->size())); //gray
        paint->drawRect(TwoByteShapes::MDROMuxTristateLabel); //gray
        paint->drawRect(TwoByteShapes::MDREMuxTristateLabel); //gray
        paint->drawRect(TwoByteShapes::EOMuxTristateLabel); //gray
        paint->drawRect(TwoByteShapes::MARMuxTristateLabel); //gray
    }
}

void CpuGraphicsItems::drawALUPoly()
{
    ALUPoly->setBrush(QBrush(colorScheme->aluColor));
    ALUPoly->setPen(QPen(QBrush(colorScheme->aluOutline),
                         2, Qt::SolidLine,
                         Qt::SquareCap,
                         Qt::MiterJoin));
}

void CpuGraphicsItems::drawRegisterBank()
{
    regBank->setBrush(QBrush(colorScheme->seqCircuitColor));
    regBankOutline->setPen(QPen(QBrush(QColor(colorScheme->arrowColorOn),
                                       Qt::SolidPattern),
                                2, Qt::SolidLine, Qt::SquareCap,
                                Qt::MiterJoin));
    QPalette pal = QPalette();
    pal.setColor(QPalette::Text,colorScheme->arrowColorOn);
    pal.setColor(QPalette::WindowText,colorScheme->arrowColorOn);
    pal.setColor(QPalette::Base,PepColors::transparent);
    pal.setColor(QPalette::Background,PepColors::transparent);
    for(QLabel* lab : labelVec)
    {

        lab->setPalette(pal);
    }
    pal.setColor(QPalette::Base,colorScheme->backgroundFill);
    pal.setColor(QPalette::Background,PepColors::transparent);
    for(QLineEdit* edit : editorVector)
    {
        edit->setPalette(pal);
    }
}

void CpuGraphicsItems::repaintLoadCk(QPainter *painter)
{
    QColor color;

    color = loadCk->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawLines(OneByteShapes::loadCkSelect._lines);

        for (int i = 0; i < OneByteShapes::loadCkSelect._arrowheads.length(); i++) {
            painter->drawImage(OneByteShapes::loadCkSelect._arrowheads.at(i),
                               color == Qt::gray ? arrowLeftGray : arrowLeft);
        }
        break;
    case Enu::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::loadCkSelect._lines);

        for (int i = 0; i < TwoByteShapes::loadCkSelect._arrowheads.length(); i++) {
            painter->drawImage(TwoByteShapes::loadCkSelect._arrowheads.at(i),
                               color == Qt::gray ? arrowLeftGray : arrowLeft);
        }
        break;
    default:
        break;
    }
}

void CpuGraphicsItems::repaintCSelect(QPainter *painter)
{
    bool ok;
    QColor color;

    cLineEdit->text().toInt(&ok, 10);
    ok ? color = colorScheme->arrowColorOn : color = Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // Draw select lines
    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawLines(OneByteShapes::CSelect._lines);
        break;
    case Enu::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::CSelect._lines);
        break;
    default:
        break;
    }

    painter->drawImage(QPoint(499,47),
                       color == Qt::gray ? arrowLeftGray : arrowLeft);
}

void CpuGraphicsItems::repaintBSelect(QPainter *painter)
{
    bool ok;
    QColor color;

    bLineEdit->text().toInt(&ok, 10);

    color = ok ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // arrow
    painter->drawImage(QPoint(499,69),
                       color == Qt::gray ? arrowLeftGray : arrowLeft);

    // Draw select lines
    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawLines(OneByteShapes::BSelect._lines);
        break;
    case Enu::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::BSelect._lines);
        break;
    default:
        break;
    }

}

void CpuGraphicsItems::repaintASelect(QPainter *painter)
{
    bool ok;

    aLineEdit->text().toInt(&ok, 10);
    QColor color;
    color = ok ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // arrow
    painter->drawImage(QPoint(499,91),
                       color == Qt::gray ? arrowLeftGray : arrowLeft);

    // Draw select lines
    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawLines(OneByteShapes::ASelect._lines);
        break;
    case Enu::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::ASelect._lines);
        break;
    default:
        break;
    }
}

void CpuGraphicsItems::repaintMARCk(QPainter *painter)
{
    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        repaintMARCkOneByteModel(painter);
        break;
    case Enu::TwoByteDataBus:
        repaintMARCkTwoByteModel(painter);
        break;
    default:
        break;
    }
}

void CpuGraphicsItems::repaintAMuxSelect(QPainter *painter)
{
    QColor color;
    bool ok;
    int aMux = aMuxTristateLabel->text().toInt(&ok, 10);

    color = ok ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);
    // Draw AMux select depending on the enabled feature set
    switch(dataSection->getCPUFeatures())
    {
    case Enu::OneByteDataBus:
        painter->drawLines(OneByteShapes::AMuxSelect._lines);
        painter->drawImage(QPoint(380,300),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
        break;
    case Enu::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::AMuxSelect._lines);
        painter->drawImage(TwoByteShapes::AMuxSelect._arrowheads.first(), //Should more arrowheads be added, this will need to be a proper for loop.
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
        break;
    }
    QPalette pal = aMuxerDataLabel->palette();
    if (ok) {

        switch (aMux) {
        case (0):
            if(dataSection->getCPUFeatures()==Enu::TwoByteDataBus){
                if(EOMuxTristateLabel->text()=="0"){
                    color=colorScheme->combCircuitGreen;
                    pal.setColor(QPalette::Background,colorScheme->muxCircuitGreen);
                    aMuxerDataLabel->setPalette(pal);
                }
                else if(EOMuxTristateLabel->text()=="1"){
                    color=colorScheme->combCircuitYellow;
                    pal.setColor(QPalette::Background,colorScheme->muxCircuitYellow);
                    aMuxerDataLabel->setPalette(pal);
                }
                else
                {
                    color=colorScheme->backgroundFill;
                    pal.setColor(QPalette::Background,color);
                    aMuxerDataLabel->setPalette(pal);
                }
            }
            else
            {
                color=colorScheme->combCircuitYellow;
                pal.setColor(QPalette::Background,colorScheme->muxCircuitYellow);
                aMuxerDataLabel->setPalette(pal);
            }
            break;
        case (1):
            if (aLineEdit->text() == "") { // ABus.state == UNDEFINED
                color = colorScheme->backgroundFill;
                pal.setColor(QPalette::Background,color);
                aMuxerDataLabel->setPalette(pal);
            } else {
                color = colorScheme->combCircuitRed;
                pal.setColor(QPalette::Background,colorScheme->muxCircuitRed);
                aMuxerDataLabel->setPalette(pal);
            }
            break;
        }
    } else {
        color = colorScheme->backgroundFill;
        pal.setColor(QPalette::Background,color);
        aMuxerDataLabel->setPalette(pal);
    }
    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);

    // Draw AMux bus depending on the enabled feature set
    switch(dataSection->getCPUFeatures())
    {
    case Enu::OneByteDataBus:
        painter->drawPolygon(OneByteShapes::AMuxBus);
        break;
    case Enu::TwoByteDataBus:
        painter->drawPolygon(TwoByteShapes::AMuxBus);
        break;
    }

}

void CpuGraphicsItems::repaintMARMuxSelect(QPainter *painter)
{
    QColor color;
    bool ok;
    MARMuxTristateLabel->text().toInt(&ok, 10);

    color = ok ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // AMux Select
    painter->drawLines(TwoByteShapes::MARMuxSelect._lines);
    painter->drawImage(TwoByteShapes::MARMuxSelect._arrowheads.first(),
                       color == Qt::gray ? arrowDownGray : arrowDown);

}

void CpuGraphicsItems::repaintEOMuxSelect(QPainter *painter)
{
    QColor color;
    bool ok;
    EOMuxTristateLabel->text().toInt(&ok, 10);

    color = ok ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // AMux Select
    painter->drawLines(TwoByteShapes::EOMuxSelect._lines);

    painter->drawImage(TwoByteShapes::EOMuxSelect._arrowheads.first(),
                       color == Qt::gray ? arrowLeftGray : arrowLeft);

}

void CpuGraphicsItems::repaintCMuxSelect(QPainter *painter)
{
    QColor color;

    color = cMuxTristateLabel->text() != "" ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // CMux Select
    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawLines(OneByteShapes::CMuxSelect._lines);
        painter->drawImage(OneByteShapes::CMuxSelect._arrowheads.first(),
                           color == Qt::gray ? arrowDownGray : arrowDown);
        break;
    case Enu::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::CMuxSelect._lines);
        painter->drawImage(TwoByteShapes::CMuxSelect._arrowheads.first(),
                           color == Qt::gray ? arrowDownGray : arrowDown);
        break;
    default:
        break;
    }
    // CMuxBus (output)
}

void CpuGraphicsItems::repaintSCk(QPainter *painter)
{
    QColor color;

    color = SCkCheckBox->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // line from checkbox to data
    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawLine(OneByteShapes::SBitSelect);
        // arrow
        painter->drawImage(QPoint(OneByteShapes::sBitLabel.right() + OneByteShapes::arrowHOffset,
                                  OneByteShapes::SBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);

        break;
    case Enu::TwoByteDataBus:
        painter->drawLine(TwoByteShapes::SBitSelect);
        // arrow
        painter->drawImage(QPoint(TwoByteShapes::sBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::SBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);

        break;
    default:
        break;
    }

}

void CpuGraphicsItems::repaintCCk(QPainter *painter)
{
    QColor color;

    color = CCkCheckBox->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // line from checkbox to data
    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawLine(OneByteShapes::CBitSelect);
        // arrow
        painter->drawImage(QPoint(OneByteShapes::cBitLabel.right() + OneByteShapes::arrowHOffset,
                                  OneByteShapes::CBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
        break;
    case Enu::TwoByteDataBus:
        painter->drawLine(TwoByteShapes::CBitSelect);
        // arrow
        painter->drawImage(QPoint(TwoByteShapes::cBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::CBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
        break;
    default:
        break;
    }

}

void CpuGraphicsItems::repaintVCk(QPainter *painter)
{
    QColor color;

    color = VCkCheckBox->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawLine(OneByteShapes::VBitSelect);
        painter->drawImage(QPoint(OneByteShapes::vBitLabel.right() + OneByteShapes::arrowHOffset,
                                  OneByteShapes::VBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
        break;
    case Enu::TwoByteDataBus:
        painter->drawLine(TwoByteShapes::VBitSelect);
        painter->drawImage(QPoint(TwoByteShapes::vBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::VBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
        break;
    default:
        break;
    }

}

void CpuGraphicsItems::repaintZCk(QPainter *painter)
{
    QColor color;

    color = ZCkCheckBox->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawLine(OneByteShapes::ZBitSelect);
        painter->drawImage(QPoint(OneByteShapes::zBitLabel.right() + OneByteShapes::arrowHOffset,
                                  OneByteShapes::ZBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
        break;
    case Enu::TwoByteDataBus:
        painter->drawLine(TwoByteShapes::ZBitSelect);
        painter->drawImage(QPoint(TwoByteShapes::zBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::ZBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
        break;
    default:
        break;
    }

}

void CpuGraphicsItems::repaintNCk(QPainter *painter)
{
    QColor color;

    color = NCkCheckBox->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawLine(OneByteShapes::NBitSelect);
        painter->drawImage(QPoint(OneByteShapes::nBitLabel.right() + OneByteShapes::arrowHOffset,
                                  OneByteShapes::NBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
        break;
    case Enu::TwoByteDataBus:
        painter->drawLine(TwoByteShapes::NBitSelect);
        painter->drawImage(QPoint(TwoByteShapes::nBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::NBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
        break;
    default:
        break;
    }

}

void CpuGraphicsItems::repaintMemCommon(QPainter *painter)
{
    switch(dataSection->getCPUFeatures())
    {
    case Enu::OneByteDataBus:
        break;
    case Enu::TwoByteDataBus:
        repaintMemCommonTwoByte(painter);
        break;
    }
}

void CpuGraphicsItems::repaintMemRead(QPainter *painter)
{
    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        repaintMemReadOneByteModel(painter);
        break;
    case Enu::TwoByteDataBus:
        repaintMemReadTwoByteModel(painter);
        break;
    default:
        break;
    }
}

void CpuGraphicsItems::repaintMemWrite(QPainter *painter)
{
    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        repaintMemWriteOneByteModel(painter);
        break;
    case Enu::TwoByteDataBus:
        repaintMemWriteTwoByteModel(painter);
        break;
    default:
        break;
    }
}

void CpuGraphicsItems::repaintSBitOut(QPainter *painter)
{
    sBitLabel->text() = dataSection->getStatusBit(Enu::STATUS_S)?"1":"0";

    QColor color = colorScheme->arrowColorOn;

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    switch(dataSection->getCPUFeatures())
    {
    case Enu::OneByteDataBus:
        // line from S bit to CSMux
        painter->drawLines(OneByteShapes::SBitToCSMux._lines);
        // arrow:
        painter->drawImage(OneByteShapes::SBitToCSMux._arrowheads.first(), arrowUp);
        break;
    case Enu::TwoByteDataBus:
        // line from S bit to CSMux
        painter->drawLines(TwoByteShapes::SBitToCSMux._lines);
        // arrow:
        painter->drawImage(TwoByteShapes::SBitToCSMux._arrowheads.first(), arrowUp);
        break;
    }
}

void CpuGraphicsItems::repaintCBitOut(QPainter *painter)
{
    cBitLabel->text() = dataSection->getStatusBit(Enu::STATUS_C)?"1":"0";

    QColor color = colorScheme->arrowColorOn;

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        // line from C bit to NZVC bus
        painter->drawLines(OneByteShapes::CBitToNZVC._lines);
        // arrow to the NZVC data bus
        painter->drawImage(OneByteShapes::CBitToNZVC._arrowheads.first(), arrowLeft);

        // line from C bit to CSMux
        painter->drawLines(OneByteShapes::CBitToCSMux._lines);
        // arrow to the CSMux
        painter->drawImage(QPoint(OneByteShapes::CBitToCSMux._arrowheads.first()), arrowUp);

        // CIN line back to the ALU
        painter->drawLines(OneByteShapes::CInToALU._lines);
        // CIN arrow to the ALU
        painter->drawImage(OneByteShapes::CInToALU._arrowheads.first(), arrowLeft);
        break;
    case Enu::TwoByteDataBus:
        // line from C bit to NZVC bus
        painter->drawLines(TwoByteShapes::CBitToNZVC._lines);
        // arrow to the NZVC data bus
        painter->drawImage(TwoByteShapes::CBitToNZVC._arrowheads.first(), arrowLeft);

        // line from C bit to CSMux
        painter->drawLines(TwoByteShapes::CBitToCSMux._lines);
        // arrow to the CSMux
        painter->drawImage(QPoint(TwoByteShapes::CBitToCSMux._arrowheads.first()), arrowUp);

        // CIN line back to the ALU
        painter->drawLines(TwoByteShapes::CInToALU._lines);
        // CIN arrow to the ALU
        painter->drawImage(TwoByteShapes::CInToALU._arrowheads.first(), arrowLeft);
        break;
    default:
        break;
    }

}

void CpuGraphicsItems::repaintVBitOut(QPainter *painter)
{
    vBitLabel->text() = dataSection->getStatusBit(Enu::STATUS_V)?"1":"0";

    QColor color = colorScheme->arrowColorOn;

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawLines(OneByteShapes::VBitOut._lines);

        painter->drawImage(OneByteShapes::VBitOut._arrowheads.first(), arrowLeft);

        break;
    case Enu::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::VBitOut._lines);

        painter->drawImage(TwoByteShapes::VBitOut._arrowheads.first(), arrowLeft);

        break;
    default:
        break;
    }

}

void CpuGraphicsItems::repaintZBitOut(QPainter *painter)
{
    zBitLabel->text() = dataSection->getStatusBit(Enu::STATUS_Z)?"1":"0";

    QColor color = colorScheme->arrowColorOn;

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    QPoint point = QPoint(437,582);
    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawEllipse(point, 2, 2);
        painter->drawLines(OneByteShapes::ZBitOut._lines);

        painter->drawImage(OneByteShapes::ZBitOut._arrowheads.first(), arrowLeft);
        painter->drawImage(OneByteShapes::ZBitOut._arrowheads.last(), arrowUp);  // AndZ arrow upwards
        break;
    case Enu::TwoByteDataBus:
        point.setX(point.x() + TwoByteShapes::controlOffsetX);
        point.setY(point.y() + TwoByteShapes::aluOffsetY);
        painter->drawEllipse(point, 2, 2);
        painter->drawLines(TwoByteShapes::ZBitOut._lines);

        painter->drawImage(TwoByteShapes::ZBitOut._arrowheads.first(), arrowLeft);
        painter->drawImage(TwoByteShapes::ZBitOut._arrowheads.last(), arrowUp);  // AndZ arrow upwards
        break;
    default:
        break;
    }

}

void CpuGraphicsItems::repaintNBitOut(QPainter *painter)
{
    nBitLabel->text() = dataSection->getStatusBit(Enu::STATUS_N)?"1":"0";

    QPolygon poly;
    QColor color = colorScheme->arrowColorOn;

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawLines(OneByteShapes::NBitOut._lines);

        painter->drawImage(OneByteShapes::NBitOut._arrowheads.first(), arrowLeft);

        break;
    case Enu::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::NBitOut._lines);

        painter->drawImage(TwoByteShapes::NBitOut._arrowheads.first(), arrowLeft);

        break;
    default:
        break;
    }
}

void CpuGraphicsItems::repaintCSMuxSelect(QPainter *painter)
{
    QColor color;

    color = CSMuxTristateLabel->text() != "" ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        // line from checkbox to data
        painter->drawLine(OneByteShapes::CSMuxSelect);
        // arrow
        painter->drawImage(QPoint(OneByteShapes::CSMuxerDataLabel.right() + OneByteShapes::arrowHOffset,
                                  OneByteShapes::CSMuxSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
        break;
    case Enu::TwoByteDataBus:
        // line from checkbox to data
        painter->drawLine(TwoByteShapes::CSMuxSelect);
        // arrow
        painter->drawImage(QPoint(TwoByteShapes::CSMuxerDataLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::CSMuxSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
        break;
    default:
        break;
    }
}

void CpuGraphicsItems::repaintAndZSelect(QPainter *painter)
{
    QPolygon poly;

    QColor color = Qt::gray;

    if (AndZTristateLabel->text() != "") {
        color = colorScheme->arrowColorOn;
    }
    painter->setPen(color);
    painter->setBrush(color);

    // lines coming out of tristate label
    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawLine(OneByteShapes::AndZOut._lines[0]);
        painter->drawLine(OneByteShapes::AndZOut._lines[1]);

        painter->drawImage(OneByteShapes::AndZOut._arrowheads.first(),
                           color == Qt::gray ? arrowDownGray : arrowDown);

        break;
    case Enu::TwoByteDataBus:
        painter->drawLine(TwoByteShapes::AndZOut._lines[0]);
        painter->drawLine(TwoByteShapes::AndZOut._lines[1]);
        painter->drawImage(TwoByteShapes::AndZOut._arrowheads.first(),
                           color == Qt::gray ? arrowDownGray : arrowDown);

        break;
    default:
        break;
    }

    color = Qt::gray;
    if (aluHasCorrectOutput() && AndZTristateLabel->text() != "") {
        color = colorScheme->arrowColorOn;
    }
    else{
        color = Qt::gray;
    }
    painter->setPen(color);
    painter->setBrush(color);

    // AndZ out
    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        painter->drawLine(OneByteShapes::AndZOut._lines[2]);
        painter->drawImage(QPoint(OneByteShapes::zBitLabel.x()-13,OneByteShapes::AndZMuxLabel.y()+OneByteShapes::AndZMuxLabel.height()/2-4),
                           color == Qt::gray ? arrowRightGray : arrowRight);

        break;
    case Enu::TwoByteDataBus:
        painter->drawLine(TwoByteShapes::AndZOut._lines[2]);
        //The arrow is ~10 pixels long, and another 3 are needed for it to fit comfortably next to the box
        //The arrow is 8 pixels high, align the the center of the arrow with the middle of the box.
        painter->drawImage(QPoint(TwoByteShapes::zBitLabel.x()-13,TwoByteShapes::AndZMuxLabel.y()+TwoByteShapes::AndZMuxLabel.height()/2-4),
                           color == Qt::gray ? arrowRightGray : arrowRight);

        break;
    default:
        break;
    }


}

void CpuGraphicsItems::repaintALUSelect(QPainter *painter)
{
    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:
        repaintALUSelectOneByteModel(painter);
        break;
    case Enu::TwoByteDataBus:
        repaintALUSelectTwoByteModel(painter);
        break;
    default:
        break;
    }
}

void CpuGraphicsItems::repaintMDRMuxSelect(QPainter *painter)
{
    QColor color;
    QPalette pal = MDRMuxerDataLabel->palette();
    painter->setPen(colorScheme->arrowColorOn);
    if(MDRCk->isChecked()){
        if(MDRMuxTristateLabel->text()=="0"&&dataSection->getMainBusState()==Enu::MemReadSecondWait){
            color = colorScheme->combCircuitGreen;
            pal.setColor(QPalette::Background,colorScheme->muxCircuitGreen);
            MDRMuxerDataLabel->setPalette(pal);
        }
        else if(MDRMuxTristateLabel->text()=="1"&&cMuxTristateLabel->text()=="0"){
            color = colorScheme->combCircuitYellow;
            pal.setColor(QPalette::Background,colorScheme->muxCircuitYellow);
            MDRMuxerDataLabel->setPalette(pal);
        }
        else if(MDRMuxTristateLabel->text()=="1"&&cMuxTristateLabel->text()=="1"&&aluHasCorrectOutput()){
            color = colorScheme->combCircuitBlue;
            pal.setColor(QPalette::Background,colorScheme->muxCircuitBlue);
            MDRMuxerDataLabel->setPalette(pal);
        }
        else{
            color = colorScheme->backgroundFill;
            pal.setColor(QPalette::Background,color);
            MDRMuxerDataLabel->setPalette(pal);
        }

    }
    else{
        color = colorScheme->backgroundFill;
        pal.setColor(QPalette::Background,color);
        MDRMuxerDataLabel->setPalette(pal);
    }

    painter->setBrush(color);
    // MDRMuxOutBus (MDRMux to MDR arrow)
    painter->drawPolygon(OneByteShapes::MDRMuxOutBus);

    // finish up by drawing select lines:
    color = Qt::gray;
    if (MDRMuxTristateLabel->text() != "") {
        color = colorScheme->arrowColorOn;
    }
    painter->setPen(color);
    painter->setBrush(color);

    // MDRMux Select
    painter->drawLine(257,303, 265,303); painter->drawLine(265,303, 265,324);
    painter->drawLine(265,324, 279,324); painter->drawLine(291,324, 335,324);
    painter->drawLine(347,324, 416,324); painter->drawLine(428,324, 543,324);

    painter->drawImage(QPoint(249,300),
                       color == Qt::gray ? arrowLeftGray : arrowLeft);
}


// ***************************************************************************
// One byte model-specific functionality:
// ***************************************************************************

void CpuGraphicsItems::repaintMARCkOneByteModel(QPainter *painter)
{
    QColor color;

    color = MARCk->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // MARCk
    painter->drawLines(OneByteShapes::MARCk._lines);

    painter->drawEllipse(QPoint(235,177), 2, 2);

    painter->drawImage(OneByteShapes::MARCk._arrowheads.first(),
                       color == Qt::gray ? arrowUpGray : arrowUp);
    painter->drawImage(OneByteShapes::MARCk._arrowheads.last(),
                       color == Qt::gray ? arrowDownGray : arrowDown);
}

void CpuGraphicsItems::repaintMDRCk(QPainter *painter)
{
    QColor color;

    switch (dataSection->getCPUFeatures()) {
    case Enu::OneByteDataBus:

        color = MDRCk->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
        painter->setPen(QPen(QBrush(color), 1));
        painter->setBrush(color);

        // MDRCk
        painter->drawLines(OneByteShapes::MDRCk._lines);

        painter->drawImage(OneByteShapes::MDRCk._arrowheads.first(),
                           color == Qt::gray ? arrowDownGray : arrowDown);
        break;
    case Enu::TwoByteDataBus:

        break;
    default:
        break;
    }

}

void CpuGraphicsItems::repaintALUSelectOneByteModel(QPainter *painter)
{
    QColor color;

    color = ALULineEdit->text() != "" ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // ALU Select
    painter->drawLines(OneByteShapes::ALUSelect._lines);

    painter->drawImage(OneByteShapes::ALUSelect._arrowheads.first(),
                       color == Qt::gray ? arrowLeftGray : arrowLeft);

    painter->setPen(colorScheme->arrowColorOn);

    if (ALULineEdit->text() != "" && ALULineEdit->text() != "15") {
        if (aMuxTristateLabel->text() == "0" && dataSection->aluFnIsUnary()) {
            painter->setBrush(colorScheme->combCircuitBlue);
        }
        else if (aMuxTristateLabel->text() == "0" && bLineEdit->text() != "") {
            painter->setBrush(colorScheme->combCircuitBlue);
        }
        else if (aMuxTristateLabel->text() == "1") {
            if (aLineEdit->text() != "" && dataSection->aluFnIsUnary()) {
                painter->setBrush(colorScheme->combCircuitBlue);
            }
            else if (aLineEdit->text() != "" && bLineEdit->text() != "") {
                painter->setBrush(colorScheme->combCircuitBlue);
            }
            else {
                painter->setBrush(colorScheme->backgroundFill);
            }
        }
        else {
            painter->setBrush(colorScheme->backgroundFill);
        }
    }
    else {
        painter->setBrush(colorScheme->backgroundFill);
    }

    // CBus
    painter->drawPolygon(OneByteShapes::ALUOutBus);

    // Draw status bit lines
    color = aluHasCorrectOutput() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(color);
    painter->setBrush(color);

    painter->drawLines(OneByteShapes::ALUSelectOut._lines);

    for (int i = 0; i < OneByteShapes::ALUSelectOut._arrowheads.length(); i++) {
        painter->drawImage(OneByteShapes::ALUSelectOut._arrowheads.at(i),
                           color == Qt::gray ? arrowRightGray : arrowRight);
    }

    // S ellipse
    painter->drawEllipse(QPoint(416,446), 2, 2); //437+9
}

void CpuGraphicsItems::repaintMemReadOneByteModel(QPainter *painter)
{
    QPolygon poly;
    QColor color;
    bool isHigh = MemReadTristateLabel->text() == "1";

    // Draw memread select line
    if (isHigh) {
        MemWriteTristateLabel->setDisabled(true);
        color = colorScheme->arrowColorOn;
    }
    else {
        MemWriteTristateLabel->setDisabled(false);
        color = Qt::gray;
    }
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // memRead line from label to bus:
    painter->drawLine(OneByteShapes::MemReadSelect);

    painter->drawImage(QPoint(OneByteShapes::DataBus.right()+5,
                              OneByteShapes::MemReadSelect.y1() - 3),
                       color == Qt::gray ? arrowLeftGray : arrowLeft);

    if (MemWriteTristateLabel->text() == "1") {
        // Do not paint main bus if MemWrite is isHigh
        return;
    }

    // Draw ADDR bus stuff:
    isHigh ? color = colorScheme->combCircuitYellow : color = colorScheme->backgroundFill;

    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);

    // Main Bus
    // ADDR bus:
    painter->drawRect(OneByteShapes::AddrBus);
    // left arrow to addr:
    painter->drawPolygon(OneByteShapes::AddrArrow);

    painter->setBrush(colorScheme->backgroundFill);

    // Draw DATA bus stuff:
    if (isHigh && (dataSection->getMainBusState() == Enu::MemReadReady ||
                dataSection->getMainBusState() == Enu::MemReadSecondWait)) {
        color = colorScheme->combCircuitGreen;
    }
    else {
        color = colorScheme->backgroundFill;
    }
    painter->setBrush(color);

    // Data bus:
    painter->drawRect(OneByteShapes::DataBus);

    // Mem Data Bus
    poly.clear();
    // arrowhead into the main bus:
    if (color == colorScheme->combCircuitGreen) {
        // square end (when reading):
        poly << QPoint(3, 365) << QPoint(3, 375);
    }
    else {
        // arrowhead (normally):
        poly << QPoint(13, 365) << QPoint(13, 360) << QPoint(3, 370)
             << QPoint(13, 380) << QPoint(13, 375);
    }
    poly << QPoint(29, 375) << QPoint(29, 380) << QPoint(39, 370)
         << QPoint(29, 360) << QPoint(29, 365);
    painter->drawPolygon(poly);

    // right arrow from Bus to MDRMux:
    painter->drawPolygon(OneByteShapes::DataToMDRMuxBus);

}

void CpuGraphicsItems::repaintMemWriteOneByteModel(QPainter *painter)
{
    QPolygon poly;
    QColor color;
    bool isHigh = MemWriteTristateLabel->text() == "1";

    // Draw memwrite select line
    if (isHigh) {
        MemReadTristateLabel->setDisabled(true);
        color = colorScheme->arrowColorOn;
    }
    else {
        MemReadTristateLabel->setDisabled(false);
        color = Qt::gray;
    }

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // memWrite line from the label to the bus:
    painter->drawLine(OneByteShapes::MemWriteSelect);
    painter->drawImage(QPoint(OneByteShapes::DataBus.right()+5,
                              OneByteShapes::MemWriteSelect.y1() - 3),
                       color == Qt::gray ? arrowLeftGray : arrowLeft);

    // draw line from memWrite to MDR out:
    painter->drawEllipse(QPoint(OneByteShapes::DataBus.right()+25,
                                OneByteShapes::MemWriteSelect.y1()),
                         2, 2);
    painter->drawLine(OneByteShapes::DataBus.right()+25, OneByteShapes::MemWriteSelect.y1() - 3,
                      OneByteShapes::DataBus.right()+25,345);
    // memWrite line from the label to the bus:
    painter->drawLine(OneByteShapes::DataBus.right()+25,333, OneByteShapes::DataBus.right()+25,280); //268+12
    painter->drawImage(QPoint(OneByteShapes::DataBus.right()+22,271), //96-3 //268+12-9
                       color == Qt::gray ? arrowUpGray : arrowUp);

    // repaint the MDR-to-main-bus line, based on if MemWrite is set or not
    // note: it should be lighter (disabled) when MemWrite is not set.
    color = colorScheme->combCircuitGreen;
    if (!isHigh) {
        color = color.lighter(120);
    }
    painter->setBrush(color);
    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));

    // mdr to data bus
    painter->drawPolygon(OneByteShapes::MDRToDataBus);


    if (MemReadTristateLabel->text() == "1") {
        // Do not paint main bus if MemRead is high
        return;
    }

    // Draw ADDR bus stuff:
    if (isHigh) {
        // qDebug() << "mainBusState: " << dataSection->getMainBusState();
        // ADDR bus is yellow if the bus is high
        color = colorScheme->combCircuitYellow;
    }
    else {
        color = colorScheme->backgroundFill;
    }

    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);

    // Main Bus
    // Main ADDR bus:
    painter->drawRect(OneByteShapes::AddrBus);
    // left arrow from addr bus to memory:
    painter->drawPolygon(OneByteShapes::AddrArrow);

    // Draw DATA bus stuff:
    // figure out the color:
    if (isHigh && (dataSection->getMainBusState() == Enu::MemWriteReady ||
                dataSection->getMainBusState() == Enu::MemWriteSecondWait)) {
        color = colorScheme->combCircuitGreen;
    }
    else {
        color = colorScheme->backgroundFill;
    }
    painter->setBrush(color);

    // Main Data bus:
    painter->drawRect(OneByteShapes::DataBus);

    // Mem Data Bus (bidirectional arrow)
    poly.clear();
    // arrowhead:
    poly << QPoint(13, 365) << QPoint(13, 360) << QPoint(3, 370)
         << QPoint(13, 380) << QPoint(13, 375);
    // other end of the bus:
    if (color == QColor(16, 150, 24)) {
        // flat end
        poly << QPoint(40, 375) << QPoint(40, 365);
    }
    else {
        // arrowhead
        poly << QPoint(29, 375) << QPoint(29, 380) << QPoint(39, 370)
             << QPoint(29, 360) << QPoint(29, 365);
    }
    painter->drawPolygon(poly);

    // Main Bus to MDRMux is ALWAYS white on a memWrite:
    painter->setBrush(colorScheme->backgroundFill);

    // right arrow from Bus to MDRMux:
    painter->drawPolygon(OneByteShapes::DataToMDRMuxBus);

}

void CpuGraphicsItems::repaintABusOneByteModel(QPainter *painter)
{
    bool ok;
    aLineEdit->text().toInt(&ok, 10);
    QColor color;
    color = ok ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    color = ok ? colorScheme->combCircuitRed : colorScheme->backgroundFill;

    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);
    // ABus
    painter->drawPolygon(OneByteShapes::ABus1);
    painter->drawPolygon(OneByteShapes::ABus2);
}

void CpuGraphicsItems::repaintBBusOneByteModel(QPainter *painter)
{
    bool ok;
    bLineEdit->text().toInt(&ok, 10);;
    QColor color;
    color = ok ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    color = ok ? colorScheme->combCircuitRed : colorScheme->backgroundFill;

    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);
    // ABus
    painter->drawPolygon(OneByteShapes::BBus1);
    painter->drawPolygon(OneByteShapes::BBus2);
    painter->drawPolygon(OneByteShapes::BBusRect);
}

void CpuGraphicsItems::repaintCBusOneByteModel(QPainter *painter)
{
    QColor color;
    QPalette pal = cMuxerLabel->palette();
    if (cMuxTristateLabel->text() == "0") {
        color = colorScheme->combCircuitYellow;
        pal.setColor(QPalette::Background,colorScheme->muxCircuitYellow);
        cMuxerLabel->setPalette(pal);
    }
    else if (cMuxTristateLabel->text() == "1") {
        if (!aluHasCorrectOutput() || ALULineEdit->text() == "15") {
            // CBus.state == UNDEFINED or NZVC A
            qDebug() << "WARNING: CMux select: There is no ALU output";
            color = colorScheme->backgroundFill;
            pal.setColor(QPalette::Background,color);
            cMuxerLabel->setPalette(pal);
        }
        else {
            color = colorScheme->combCircuitBlue;
            pal.setColor(QPalette::Background,colorScheme->muxCircuitBlue);
            cMuxerLabel->setPalette(pal);
        }
    }
    else {
        color = colorScheme->backgroundFill;
        pal.setColor(QPalette::Background,color);
        cMuxerLabel->setPalette(pal);
    }
    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);
    painter->drawPolygon(OneByteShapes::CBus);
}



// ***************************************************************************
// Two byte model-specific functionality:
// ***************************************************************************

void CpuGraphicsItems::repaintMARCkTwoByteModel(QPainter *painter)
{
    QColor color;

    color = MARCk->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // MARCk
    painter->drawLines(TwoByteShapes::MARCk._lines);


    painter->drawEllipse(QPoint(TwoByteShapes::MARCk._lines.last().x1(),TwoByteShapes::MARMuxerDataLabel.y()+TwoByteShapes::MARMuxerDataLabel.height()/2), 2, 2);
    painter->drawImage(TwoByteShapes::MARCk._arrowheads[0],
            color == Qt::gray ? arrowUpGray : arrowUp);
    painter->drawImage(TwoByteShapes::MARCk._arrowheads[1],
            color == Qt::gray ? arrowDownGray : arrowDown);
}

void CpuGraphicsItems::repaintMDROCk(QPainter *painter)
{
    QColor color;
    color = MDROCk->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // MDROdd Clock
    painter->drawLines(TwoByteShapes::MDROck._lines);
    painter->drawImage(TwoByteShapes::MDROck._arrowheads.first(),
                       color == Qt::gray ? arrowDownGray : arrowDown);
}

void CpuGraphicsItems::repaintMDRECk(QPainter *painter)
{
    QColor color;
    color = MDRECk->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // MDREven Clock
    painter->drawLines(TwoByteShapes::MDREck._lines);
    painter->drawImage(TwoByteShapes::MDREck._arrowheads.first(),
                       color == Qt::gray ? arrowDownGray : arrowDown);

}

void CpuGraphicsItems::repaintEOMuxOutpusBus(QPainter *painter)
{
    QColor color = colorScheme->backgroundFill;
    QPalette pal = EOMuxerDataLabel->palette();
    if(EOMuxTristateLabel->text()=="1"){
        color=colorScheme->combCircuitYellow;
        pal.setColor(QPalette::Background,colorScheme->muxCircuitYellow);
        EOMuxerDataLabel->setPalette(pal);
    }
    else if(EOMuxTristateLabel->text()=="0"){
        color=colorScheme->combCircuitGreen;
        pal.setColor(QPalette::Background,colorScheme->muxCircuitGreen);
        EOMuxerDataLabel->setPalette(pal);
    }
    else
    {
        pal.setColor(QPalette::Background,color);
        EOMuxerDataLabel->setPalette(pal);
    }
    painter->setPen(colorScheme->arrowColorOn);
    painter->setBrush(color);
    painter->drawPolygon(TwoByteShapes::EOMuxOutputBus);
}

void CpuGraphicsItems::repaintALUSelectTwoByteModel(QPainter *painter)
{
    QColor color;

    color = ALULineEdit->text() != "" ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // ALU Select
    painter->drawLines(TwoByteShapes::ALUSelect._lines);

    painter->drawImage(TwoByteShapes::ALUSelect._arrowheads.first(),
                       color == Qt::gray ? arrowLeftGray : arrowLeft);

    painter->setPen(colorScheme->arrowColorOn);

    if (ALULineEdit->text() != "" && ALULineEdit->text() != "15") {
        if (aMuxTristateLabel->text() == "0" && dataSection->aluFnIsUnary()) {
            painter->setBrush(colorScheme->combCircuitBlue);
        }
        else if (aMuxTristateLabel->text() == "0" && bLineEdit->text() != "") {
            painter->setBrush(colorScheme->combCircuitBlue);
        }
        else if (aMuxTristateLabel->text() == "1") {
            if (aLineEdit->text() != "" && dataSection->aluFnIsUnary()) {
                painter->setBrush(colorScheme->combCircuitBlue);
            }
            else if (aLineEdit->text() != "" && bLineEdit->text() != "") {
                painter->setBrush(colorScheme->combCircuitBlue);
            }
            else {
                painter->setBrush(colorScheme->backgroundFill);
            }
        }
        else {
            painter->setBrush(colorScheme->backgroundFill);
        }
    }
    else {
        painter->setBrush(colorScheme->backgroundFill);
    }

    // CBus
    painter->drawPolygon(TwoByteShapes::ALUOutBus);

    // Draw status bit lines
    painter->setPen(aluHasCorrectOutput() ? colorScheme->arrowColorOn : Qt::gray);
    painter->setBrush(aluHasCorrectOutput() ? colorScheme->arrowColorOn : Qt::gray);

    painter->drawLines(TwoByteShapes::ALUSelectOut._lines);

    for (int i = 0; i < TwoByteShapes::ALUSelectOut._arrowheads.length(); i++) {
        painter->drawImage(TwoByteShapes::ALUSelectOut._arrowheads.at(i),
                           color == Qt::gray ? arrowRightGray : arrowRight);
    }

    // S ellipse
    painter->drawEllipse(QPoint(416 + TwoByteShapes::controlOffsetX,
                                TwoByteShapes::sBitLabel.y() + TwoByteShapes::selectYOffset),
                         2, 2);

}

void CpuGraphicsItems::repaintMemCommonTwoByte(QPainter *painter)
{
    QColor color;
    bool readIsHigh = MemReadTristateLabel->text()=="1",writeIsHigh = MemWriteTristateLabel->text()=="1";

    if (readIsHigh||writeIsHigh) {
        // qDebug() << "mainBusState: " << dataSection->getMainBusState();
        // ADDR bus is yellow if the bus is high
        color = colorScheme->combCircuitYellow;
    }
    else {
        color = colorScheme->backgroundFill;
    }

    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);

    // Main Bus
    // Main ADDR bus:
    painter->drawRect(TwoByteShapes::AddrBus);
    // left arrow from addr bus to memory:
    painter->drawPolygon(TwoByteShapes::AddrArrow);
    // Draw memwrite select line
    if (writeIsHigh) {
        MemReadTristateLabel->setDisabled(true);
        color = colorScheme->arrowColorOn;
    }
    else {
        MemReadTristateLabel->setDisabled(false);
        color = Qt::gray;
    }

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // memWrite line from the label to the bus:
    painter->drawLine(TwoByteShapes::MemWriteSelect); //611+8
    painter->drawImage(QPoint(TwoByteShapes::DataBus.right()+5,
                              TwoByteShapes::MemWriteSelect.y1() - 3),
                       color == Qt::gray ? arrowLeftGray : arrowLeft);

    // draw line from memWrite to MDRO out:
    painter->drawEllipse(QPoint(TwoByteShapes::DataBus.right()+20,
                                TwoByteShapes::MemWriteSelect.y1()),
                         2, 2);
    painter->drawLine(TwoByteShapes::DataBus.right()+20,TwoByteShapes::MemWriteSelect.y1(), TwoByteShapes::DataBus.right()+20,TwoByteShapes::MDROLabel.bottom()); //611+8
    // memWrite line from the label to the bus:
    painter->drawImage(QPoint(TwoByteShapes::DataBus.right()+17, //96-3
                              //The bottom of the bus is 5 pixels below the label's midpoint, and add 3 pixels for visual comfort.
                              TwoByteShapes::MDROLabel.y()+TwoByteShapes::MDROLabel.height()/2+5+3),
                       color == Qt::gray ? arrowUpGray : arrowUp);
    painter->drawImage(QPoint(TwoByteShapes::DataBus.right()+17,//96-3
                              //The bottom of the bus is 5 pixels below the label's midpoint, and add 3 pixels for visual comfort.
                              TwoByteShapes::MDRELabel.y()+TwoByteShapes::MDRELabel.height()/2+5+3),
                       color == Qt::gray ? arrowUpGray : arrowUp);

    // Draw memread select line
    if (readIsHigh) {
        MemWriteTristateLabel->setDisabled(true);
        color = colorScheme->arrowColorOn;
    }
    else {
        MemWriteTristateLabel->setDisabled(false);
        color = Qt::gray;
    }
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // memRead line from label to bus:
    painter->drawLine(TwoByteShapes::MemReadSelect);

    painter->drawImage(QPoint(TwoByteShapes::DataBus.right()+5,
                              TwoByteShapes::MemReadSelect.y1() - 3),
                       color == Qt::gray ? arrowLeftGray : arrowLeft);
    QPolygon poly;

    //Pick data bus and data arrow color
    if(MemReadTristateLabel->text()=="1" && (dataSection->getMainBusState() == Enu::MemReadReady ||
                                             dataSection->getMainBusState() == Enu::MemReadSecondWait)){
        color=colorScheme->combCircuitRed;
    }
    else if(MemWriteTristateLabel->text()=="1" && (dataSection->getMainBusState() == Enu::MemWriteReady ||
                                                   dataSection->getMainBusState() == Enu::MemWriteSecondWait)){
        color = colorScheme->combCircuitGreen;
    }
    else{
        color =colorScheme->backgroundFill;
    }
    painter->setPen(colorScheme->arrowColorOn);
    painter->setBrush(color);
    // Left end points
    if (MemReadTristateLabel->text()=="1" && (dataSection->getMainBusState() == Enu::MemReadReady ||
                   dataSection->getMainBusState() == Enu::MemReadSecondWait)) {
        // square end (when reading):
        poly << QPoint(TwoByteShapes::DataArrowLeftX+TwoByteShapes::arrowHOffset,
                       TwoByteShapes::DataArrowMidpointY-TwoByteShapes::DataArrowInnerYSpread) // Arrow Top Inner point
             << QPoint(TwoByteShapes::DataArrowLeftX+TwoByteShapes::arrowHOffset,
                       TwoByteShapes::DataArrowMidpointY+TwoByteShapes::DataArrowInnerYSpread); // Arrow Bottom Inner point
    }
    else {
        poly << QPoint(TwoByteShapes::DataArrowLeftX+TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY-TwoByteShapes::DataArrowInnerYSpread)  // Arrow Top Inner point
             << QPoint(TwoByteShapes::DataArrowLeftX+TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY-TwoByteShapes::DataArrowOuterYSpread)  // Arrow Top Outer point
             << QPoint(TwoByteShapes::DataArrowLeftX, TwoByteShapes::DataArrowMidpointY)   // Arrow Middle point
             << QPoint(TwoByteShapes::DataArrowLeftX+TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY+TwoByteShapes::DataArrowOuterYSpread)  // Arrow Bottom Outer point
             << QPoint(TwoByteShapes::DataArrowLeftX+TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY+TwoByteShapes::DataArrowInnerYSpread); // Arrow Bottom Inner point
    }
    // Right end points
    if (MemWriteTristateLabel->text()=="1" && (dataSection->getMainBusState() == Enu::MemWriteReady ||
                                               dataSection->getMainBusState() == Enu::MemWriteSecondWait)) {
        // square end (when writing):
        poly << QPoint(TwoByteShapes::DataArrowRightX+1,
                       TwoByteShapes::DataArrowMidpointY+TwoByteShapes::DataArrowInnerYSpread) // Arrow Bottom Inner point
             << QPoint(TwoByteShapes::DataArrowRightX+1,
                       TwoByteShapes::DataArrowMidpointY-TwoByteShapes::DataArrowInnerYSpread); // Arrow Top Inner point
    }
    else {
        poly << QPoint(TwoByteShapes::DataArrowRightX-TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY+TwoByteShapes::DataArrowInnerYSpread) // Arrow Bottom Inner point
             << QPoint(TwoByteShapes::DataArrowRightX-TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY+TwoByteShapes::DataArrowOuterYSpread) // Arrow Bottom Outer point
             << QPoint(TwoByteShapes::DataArrowRightX, TwoByteShapes::DataArrowMidpointY) // Arrow Middle point
             << QPoint(TwoByteShapes::DataArrowRightX-TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY-TwoByteShapes::DataArrowOuterYSpread) // Arrow Top Outer point
             << QPoint(TwoByteShapes::DataArrowRightX-TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY-TwoByteShapes::DataArrowInnerYSpread); // Arrow Top Inner point
    }
    painter->drawPolygon(poly);

    //Do right end or arrow


}

void CpuGraphicsItems::repaintMemReadTwoByteModel(QPainter *painter)
{
    QColor color;
    bool isHigh = MemReadTristateLabel->text() == "1";

    // right arrow from Bus to MDR{O,E}Mux:
    if (MemWriteTristateLabel->text() == "1") {
        // Do not paint main bus if MemWrite is isHigh

        return;
    }
    if (isHigh && (dataSection->getMainBusState() == Enu::MemReadReady ||
                dataSection->getMainBusState() == Enu::MemReadSecondWait)) {
        color = colorScheme->combCircuitRed;
    }
    else {
        color = colorScheme->backgroundFill;
    }
    painter->setBrush(color);
    // Data bus:
    painter->drawRect(TwoByteShapes::DataBus);

    // Mem Data Bus
    painter->drawPolygon(TwoByteShapes::DataToMDROMuxBus);
    painter->drawPolygon(TwoByteShapes::DataToMDREMuxBus);


}

void CpuGraphicsItems::repaintMemWriteTwoByteModel(QPainter *painter)
{
    QColor color;
    bool isHigh = MemWriteTristateLabel->text() == "1";

    // repaint the MDR-to-main-bus line, based on if MemWrite is set or not
    // note: it should be lighter (disabled) when MemWrite is not set.
    color = colorScheme->combCircuitGreen;
    if (!isHigh) {
        color = color.lighter(120);
    }
    painter->setBrush(color);
    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    // mdr to data bus
    painter->drawPolygon(TwoByteShapes::MDROToDataBus);
    painter->drawPolygon(TwoByteShapes::MDREToDataBus);

    if (MemReadTristateLabel->text() == "1") {
        // Do not paint main bus if MemRead is high
        return;
    }

    // Draw ADDR bus stuff:
    // Draw DATA bus stuff:
    // figure out the color:
    if (isHigh && (dataSection->getMainBusState() == Enu::MemWriteReady ||
                dataSection->getMainBusState() == Enu::MemWriteSecondWait)) {
        color = colorScheme->combCircuitGreen;
    }
    else {
        color = colorScheme->backgroundFill;
    }
    painter->setPen(colorScheme->arrowColorOn);
    painter->setBrush(color);

    // Main Data bus:
    painter->drawRect(TwoByteShapes::DataBus);

    // Mem Data Bus (bidirectional arrow)
    // Main Bus to MDRMux is ALWAYS white on a memWrite:
    painter->setBrush(colorScheme->backgroundFill);

    // right arrow from Bus to MDR{O,E}Mux:
    painter->drawPolygon(TwoByteShapes::DataToMDROMuxBus);
    painter->drawPolygon(TwoByteShapes::DataToMDREMuxBus);

}

void CpuGraphicsItems::repaintMARMUXToMARBuses(QPainter *painter)
{
    //Needs conditional painting based on the state of the bus.
    bool marckIsHigh = MARCk->isChecked();
    QColor colorTop=colorScheme->backgroundFill,colorBottom=colorScheme->backgroundFill;
    if(marckIsHigh && MARMuxTristateLabel->text()=="0"){
        colorTop= colorScheme->combCircuitYellow;
        colorBottom = colorScheme->combCircuitGreen;
    }
    else if(marckIsHigh && MARMuxTristateLabel->text()=="1")
    {
        colorTop = colorBottom = colorScheme->combCircuitRed;
    }
    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(colorBottom);
    painter->drawPolygon(TwoByteShapes::MARMuxToMARABus);
    painter->setBrush(colorTop);
    painter->drawPolygon(TwoByteShapes::MARMuxToMARBBus);
}

void CpuGraphicsItems::repaintMDRESelect(QPainter *painter)
{
    //Determine if control line is high, so that the color may be set accordingly
    bool MDREIsHigh=MDREMuxTristateLabel->text()=="1"||MDREMuxTristateLabel->text()=="0";
    QColor MDREColor=MDREIsHigh?colorScheme->arrowColorOn:Qt::gray;
    QPalette pal =MDREMuxerDataLabel->palette();
    //Paint MDRESelect's lines and arrow in the appropriate color
    painter->setPen(MDREColor);
    painter->drawLines(TwoByteShapes::MDREMuxSelect._lines);
    painter->drawImage(TwoByteShapes::MDREMuxSelect._arrowheads.first(),
                       MDREColor == Qt::gray ? arrowLeftGray : arrowLeft);
    if(MDRECk->isChecked()){
        if(MDREMuxTristateLabel->text()=="0"&&dataSection->getMainBusState()==Enu::MemReadSecondWait){
            pal.setColor(QPalette::Background,colorScheme->muxCircuitRed);
            MDREMuxerDataLabel->setPalette(pal);
        }
        else if(MDREMuxTristateLabel->text()=="1"&&cMuxTristateLabel->text()=="0"){
            pal.setColor(QPalette::Background,colorScheme->muxCircuitYellow);
            MDREMuxerDataLabel->setPalette(pal);
        }
        else if(MDREMuxTristateLabel->text()=="1"&&cMuxTristateLabel->text()=="1"&&aluHasCorrectOutput()){
            pal.setColor(QPalette::Background,colorScheme->muxCircuitBlue);
            MDREMuxerDataLabel->setPalette(pal);
        }
        else{
            pal.setColor(QPalette::Background,colorScheme->backgroundFill);
            MDREMuxerDataLabel->setPalette(pal);
        }

    }
    else{
        pal.setColor(QPalette::Background,colorScheme->backgroundFill);
        MDREMuxerDataLabel->setPalette(pal);
    }
}

void CpuGraphicsItems::repaintMDROSelect(QPainter *painter)
{
    //Determine if control line is high, so that the color may be set accordingly
    bool MDROIsHigh=MDROMuxTristateLabel->text()=="1"||MDROMuxTristateLabel->text()=="0";
    QColor MDROColor=MDROIsHigh?colorScheme->arrowColorOn:Qt::gray;
    QPalette pal = MDROMuxerDataLabel->palette();
    //Paint MDROSelect's lines and arrow in the appropriate color
    painter->setPen(MDROColor);
    painter->drawLines(TwoByteShapes::MDROMuxSelect._lines);
    painter->drawImage(TwoByteShapes::MDROMuxSelect._arrowheads.first(),
                       MDROColor == Qt::gray ? arrowLeftGray : arrowLeft);
    if(MDROCk->isChecked()){
        if(MDROMuxTristateLabel->text()=="0"&&dataSection->getMainBusState()==Enu::MemReadSecondWait){
            pal.setColor(QPalette::Background,colorScheme->muxCircuitRed);
            MDROMuxerDataLabel->setPalette(pal);
        }
        else if(MDROMuxTristateLabel->text()=="1"&&cMuxTristateLabel->text()=="0"){
            pal.setColor(QPalette::Background,colorScheme->muxCircuitYellow);
            MDROMuxerDataLabel->setPalette(pal);
        }
        else if(MDROMuxTristateLabel->text()=="1"&&cMuxTristateLabel->text()=="1"&&aluHasCorrectOutput()){
            pal.setColor(QPalette::Background,colorScheme->muxCircuitBlue);
            MDROMuxerDataLabel->setPalette(pal);
        }
        else{
            pal.setColor(QPalette::Background,colorScheme->backgroundFill);
            MDROMuxerDataLabel->setPalette(pal);
        }

    }
    else{
        pal.setColor(QPalette::Background,colorScheme->backgroundFill);
        MDROMuxerDataLabel->setPalette(pal);
    }
}

void CpuGraphicsItems::repaintMDRMuxOutputBuses(QPainter *painter)
{
    // MDRMuxOutBus (MDRMux to MDR arrow)
    QColor colorMDRE = colorScheme->backgroundFill, colorMDRO = colorScheme->backgroundFill;
    // Depending on which input is selected on the MDRMuxes, the color might need to change.
    // For now red seems to be the most logical color to use all the time.
    QString MDREText = MDREMuxTristateLabel->text(), MDROText = MDROMuxTristateLabel->text();
    if(MDRECk->isChecked()&&(MDREText=="1"||MDREText=="0")){
         //If the muxer is enabled, and data can be clocked in to the register, pick an appropriate color
        if(MDREMuxTristateLabel->text()=="0"&&dataSection->getMainBusState()==Enu::MemReadSecondWait){
            colorMDRE=colorScheme->combCircuitRed;
        }
        else if(MDREMuxTristateLabel->text()=="1"&&cMuxTristateLabel->text()=="0"){
            colorMDRE=colorScheme->combCircuitYellow;
        }
        else if(MDREMuxTristateLabel->text()=="1"&&cMuxTristateLabel->text()=="1"&&aluHasCorrectOutput()){
            colorMDRE=colorScheme->combCircuitBlue;
        }
        else{
            colorMDRE = colorScheme->backgroundFill;
        }
    }
    if(MDROCk->isChecked()&&(MDROText=="1"||MDROText=="0")){
         //If the muxer is enabled, and data can be clocked in to the register, pick an appropriate color
        if(MDROMuxTristateLabel->text()=="0"&&dataSection->getMainBusState()==Enu::MemReadSecondWait){
            colorMDRO=colorScheme->combCircuitRed;
        }
        else if(MDROMuxTristateLabel->text()=="1"&&cMuxTristateLabel->text()=="0"){
            colorMDRO=colorScheme->combCircuitYellow;
        }
        else if(MDROMuxTristateLabel->text()=="1"&&cMuxTristateLabel->text()=="1"&&aluHasCorrectOutput()){
            colorMDRO=colorScheme->combCircuitBlue;
        }
        else{
            colorMDRO = colorScheme->backgroundFill;
        }
    }
    painter->setPen(colorScheme->arrowColorOn);
    painter->setBrush(colorMDRE);
    painter->drawPolygon(TwoByteShapes::MDREMuxOutBus);
    painter->setBrush(colorMDRO);
    painter->drawPolygon(TwoByteShapes::MDROMuxOutBus);
}

void CpuGraphicsItems::repaintMDREToEOMuxBus(QPainter *painter){
    QColor color = colorScheme->backgroundFill;
    if(MARMuxTristateLabel->text()=="0"||EOMuxTristateLabel->text()=="1"||EOMuxTristateLabel->text()=="0"){
        color=colorScheme->combCircuitGreen;
    }
    painter->setPen(colorScheme->arrowColorOn);
    painter->setBrush(color);
    painter->drawPolygon(TwoByteShapes::MDREToEOMuxBus);
}

void CpuGraphicsItems::repaintMDROToEOMuxBus(QPainter *painter){
    QColor color=colorScheme->backgroundFill;
    if(MARMuxTristateLabel->text()=="0"||EOMuxTristateLabel->text()=="1"||EOMuxTristateLabel->text()=="0"){
        color=colorScheme->combCircuitYellow;
    }
    painter->setPen(colorScheme->arrowColorOn);
    painter->setBrush(color);
    painter->drawPolygon(TwoByteShapes::MDROToEOMuxBus);
}

void CpuGraphicsItems::repaintABusTwoByteModel(QPainter *painter)
{
    bool ok;
    aLineEdit->text().toInt(&ok, 10);
    QColor color;
    color = ok ? colorScheme->combCircuitRed : colorScheme->backgroundFill;
    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);

    // ABus
    painter->drawPolygon(TwoByteShapes::ABus);
}

void CpuGraphicsItems::repaintBBusTwoByteModel(QPainter *painter)
{
    bool ok;
    bLineEdit->text().toInt(&ok, 10);
    QColor color;
    color = ok ? colorScheme->combCircuitRed : colorScheme->backgroundFill;
    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);

    // BBus
    painter->drawPolygon(TwoByteShapes::BBus);
}

void CpuGraphicsItems::repaintCBusTwoByteModel(QPainter *painter)
{
    QColor color;
    QPalette pal = cMuxerLabel->palette();
    if (cMuxTristateLabel->text() == "0") {
        color = colorScheme->combCircuitYellow;
        pal.setColor(QPalette::Background,colorScheme->muxCircuitYellow);
        cMuxerLabel->setPalette(pal);
    }
    else if (cMuxTristateLabel->text() == "1") {
        if (!aluHasCorrectOutput() || ALULineEdit->text() == "15") {
            // CBus.state == UNDEFINED or NZVC A
            qDebug() << "WARNING!: CMux select: There is no ALU output";
            color = colorScheme->backgroundFill;
            pal.setColor(QPalette::Background,color);
            cMuxerLabel->setPalette(pal);
        }
        else {
            color = colorScheme->combCircuitBlue;
            pal.setColor(QPalette::Background,colorScheme->muxCircuitBlue);
            cMuxerLabel->setPalette(pal);
        }
    }
    else {
        color = colorScheme->backgroundFill;
        pal.setColor(QPalette::Background,color);
        cMuxerLabel->setPalette(pal);
    }
    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);
    painter->drawPolygon(TwoByteShapes::CBus);
}

void CpuGraphicsItems::onDarkModeChanged(bool darkMode)
{
    this->darkMode=darkMode;
    if(darkMode)
    {
        colorScheme = &(PepColors::darkMode);
    }
    else
    {
        colorScheme = &(PepColors::lightMode);
    }
    // set up arrow heads:
    arrowLeft = QImage(colorScheme->arrowImageOn);
    arrowRight = arrowLeft.mirrored(true, false);
    QTransform t;
    t.rotate(90);
    arrowUp = arrowLeft.transformed(t);
    t.rotate(-180);
    arrowDown = arrowLeft.transformed(t);

    arrowLeftGray = QImage(colorScheme->arrowImageOff);
    arrowRightGray = arrowLeftGray.mirrored(true, false);
    QTransform t_gray;
    t_gray.rotate(90);
    arrowUpGray = arrowLeftGray.transformed(t_gray);
    t_gray.rotate(-180);
    arrowDownGray = arrowLeftGray.transformed(t_gray);
    drawLabels();
}



