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

void addLabelToScene(QLabel** labelLoc, QGraphicsScene *scene, QString name, const QRect& geometry)
{
    *labelLoc = new QLabel(name);
    (*labelLoc)->setGeometry(geometry);
    (*labelLoc)->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(*labelLoc);
}

void addCenteredLabelToScene(QLabel** labelLoc, QGraphicsScene *scene, QString name, const QRect& geometry)
{
    addLabelToScene(labelLoc, scene,  name, geometry);
    (*labelLoc)->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}

void addDLabelToScene(QLabel** labelLoc, QGraphicsScene *scene, QString name, const QRect& geometry)
{
    *labelLoc = new QLabel(name);
    (*labelLoc)->setGeometry(geometry);
    (*labelLoc)->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    (*labelLoc)->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(*labelLoc);
}

void addLineEditToScene(QLineEdit** lineEdit, QGraphicsScene *scene, QRegExp validator, const QRect& geometry){
    (*lineEdit) = new QLineEdit();
    (*lineEdit)->setAlignment(Qt::AlignCenter);
    (*lineEdit)->setGeometry(geometry);
    (*lineEdit)->setValidator(new QRegExpValidator(validator, (*lineEdit)));
    (*lineEdit)->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(*lineEdit);
}

void addTLabel(TristateLabel** labelLoc, QGraphicsScene *scene, const QRect& geometry){
    (*labelLoc) = new TristateLabel(0, TristateLabel::Tristate);
    (*labelLoc)->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    (*labelLoc)->setGeometry(geometry);
    (*labelLoc)->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(*labelLoc);
}

void addStatusLabel(TristateLabel** labelLoc, QGraphicsScene* scene, const QRect& geometry){
    (*labelLoc) = new TristateLabel(0, TristateLabel::ZeroOne);
    (*labelLoc)->setText("0");
    (*labelLoc)->setGeometry(geometry);
    (*labelLoc)->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    (*labelLoc)->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(*labelLoc);
}

void addRegisterText(QVector<QLabel*> &labelVec, QGraphicsScene* scene, QString text, const QRect& geometry, const PepColors::Colors* colorScheme){
    QLabel* ph = new QLabel(text);
    ph->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ph->setPalette(QPalette(colorScheme->seqCircuitColor));
    ph->setGeometry(geometry);
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSizeSmall));
    scene->addWidget(ph);
    labelVec.append(ph);
}

void addEditableRegister(QLineEdit** edit, QVector<QLineEdit*> &editorVector, QGraphicsScene* scene,
                         QString text, QRegExp regex, const QRect& geometry, const PepColors::Colors* colorScheme){
    (*edit) = new QLineEdit(text);
    (*edit)->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    (*edit)->setGeometry(geometry);
    (*edit)->setValidator(new QRegExpValidator(regex,*edit));
    (*edit)->setPalette(QPalette(colorScheme->seqCircuitColor));
    (*edit)->setFrame(false);
    (*edit)->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(*edit);
    editorVector.append(*edit);
}

void addStaticRegister(QVector<QLabel*> &labelVec, QGraphicsScene* scene, QString text, const QRect& geometry, const PepColors::Colors* colorScheme){
    QLabel* ph = new QLabel(text);
    ph->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    ph->setGeometry(geometry);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(ph);
    labelVec.append(ph);
}

void addCheckToScene(QCheckBox** checkLoc, QGraphicsScene *scene,QString name, const QRect& geometry)
{
    *checkLoc = new QCheckBox(name);
    (*checkLoc)->setGeometry(geometry);
    (*checkLoc)->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(*checkLoc);
}

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
    addDLabelToScene(&MARMuxerDataLabel, scene, "MARMux", TwoByteShapes::MARMuxerDataLabel);
    addCheckToScene(&MDROCk, scene, "MDROCk", TwoByteShapes::MDROCkCheckbox);
    addCheckToScene(&MDRECk, scene, "MDRECk", TwoByteShapes::MDRECkCheckbox);
    addLabelToScene(&MDROMuxLabel, scene, "MDROMux", TwoByteShapes::MDROMuxLabel);
    addDLabelToScene(&MDROMuxerDataLabel, scene, "MDROMux", TwoByteShapes::MDROMuxerDataLabel);
    addTLabel(&MDROMuxTristateLabel, scene, TwoByteShapes::MDROMuxTristateLabel);
    addLabelToScene(&MDREMuxLabel, scene, "MDREMux", TwoByteShapes::MDREMuxLabel);
    addDLabelToScene(&MDREMuxerDataLabel, scene, "MDREMux", TwoByteShapes::MDREMuxerDataLabel);
    addTLabel(&MDREMuxTristateLabel, scene, TwoByteShapes::MDREMuxTristateLabel);
    addLabelToScene(&EOMuxLabel, scene, "EOMux", TwoByteShapes::EOMuxLabel);
    addDLabelToScene(&EOMuxerDataLabel, scene, "EOMux", TwoByteShapes::EOMuxerDataLabel);
    addTLabel(&EOMuxTristateLabel, scene, TwoByteShapes::EOMuxTristateLabel);

    addCenteredLabelToScene(&MDRELabel, scene, "0x00", TwoByteShapes::MDRELabel);
    addCenteredLabelToScene(&MDROLabel, scene, "0x00", TwoByteShapes::MDROLabel);
    addLabelToScene(&MARMuxLabel, scene, "MARMux", TwoByteShapes::MARMuxLabel);
    addTLabel(&MARMuxTristateLabel, scene, TwoByteShapes::MARMuxTristateLabel);

    // ************************************
    // end two byte exclusive stuff
    // ************************************

    //MARA & MARB
    addCenteredLabelToScene(&MARALabel, scene, "0x00", TwoByteShapes::MARALabel);
    addCenteredLabelToScene(&MARBLabel, scene, "0x00", TwoByteShapes::MARBLabel);
    MARALabel->setAutoFillBackground(false);
    MARBLabel->setAutoFillBackground(false);

    // LoadCk
    addCheckToScene(&loadCk, scene, "LoadCk", TwoByteShapes::loadCkCheckbox);

    // C
    // Note: the line edits must be added first, otherwise they cover the
    //  labels that go with them.
    QRegExp cbaRegExp("^((3[0-1])|([0-2][0-9])|([0-9]))$");
    addLineEditToScene(&cLineEdit, scene, cbaRegExp, TwoByteShapes::cLineEdit);
    addLabelToScene(&cLabel, scene, "C", TwoByteShapes::cLabel);

    // B
    addLineEditToScene(&bLineEdit, scene, cbaRegExp, TwoByteShapes::bLineEdit);
    addLabelToScene(&bLabel, scene, "B", TwoByteShapes::bLabel);

    // A
    addLineEditToScene(&aLineEdit, scene, cbaRegExp, TwoByteShapes::aLineEdit);
    addLabelToScene(&aLabel, scene, "A", TwoByteShapes::aLabel);

    // MARCk
    addCheckToScene(&MARCk, scene, "MARCk", TwoByteShapes::MARCkCheckbox);


    // AMux
    addLabelToScene(&aMuxLabel, scene, "AMux", TwoByteShapes::aMuxLabel);
    addDLabelToScene(&aMuxerDataLabel, scene, "AMux", TwoByteShapes::aMuxerDataLabel);
    addTLabel(&aMuxTristateLabel, scene, OneByteShapes::aMuxTristateLabel);

    // CMux
    addLabelToScene(&cMuxLabel, scene, "CMux", TwoByteShapes::cMuxLabel);
    addDLabelToScene(&cMuxerLabel, scene, "CMux", TwoByteShapes::cMuxerLabel);
    addTLabel(&cMuxTristateLabel, scene, OneByteShapes::cMuxTristateLabel);

    // ALU
    // keep this before the label that goes with it, or the line edit
    //  appears on top of the label
    addLineEditToScene(&ALULineEdit, scene, QRegExp("^((1[0-5])|(0[0-9])|[0-9])$"), TwoByteShapes::ALULineEdit);
    addLabelToScene(&ALULabel, scene, "ALU", TwoByteShapes::ALULabel);
    addCenteredLabelToScene(&ALUFunctionLabel, scene, "", TwoByteShapes::ALUFunctionLabel);

    // ALU shape
    ALUPoly = scene->addPolygon(OneByteShapes::ALUPoly,
                                QPen(QBrush(colorScheme->combCircuitBlue),
                                     2, Qt::SolidLine,
                                     Qt::SquareCap,
                                     Qt::MiterJoin),
                                QBrush(colorScheme->aluColor));
    ALUPoly->setZValue(-1);

    // CSMux
    addLabelToScene(&CSMuxLabel, scene, "CSMux", TwoByteShapes::CSMuxLabel);
    addDLabelToScene(&CSMuxerDataLabel, scene, "CSMux", TwoByteShapes::CSMuxerDataLabel);
    addTLabel(&CSMuxTristateLabel, scene, TwoByteShapes::CSMuxTristateLabel);

    // SCk
    addCheckToScene(&SCkCheckBox, scene, "SCk", TwoByteShapes::SCkCheckBox);
    addStatusLabel(&sBitLabel, scene, TwoByteShapes::sBitLabel);

    // CCk
    addCheckToScene(&CCkCheckBox, scene, "CCk", TwoByteShapes::CCkCheckBox);
    addStatusLabel(&cBitLabel, scene, TwoByteShapes::cBitLabel);

    // VCk
    addCheckToScene(&VCkCheckBox, scene, "VCk", TwoByteShapes::VCkCheckBox);
    addStatusLabel(&vBitLabel, scene, TwoByteShapes::vBitLabel);

    // AndZ
    addLabelToScene(&AndZLabel, scene, "AndZ", TwoByteShapes::AndZLabel);
    addTLabel(&AndZTristateLabel, scene, TwoByteShapes::AndZTristateLabel);
    addDLabelToScene(&AndZMuxLabel, scene, "AndZ", TwoByteShapes::AndZMuxLabel);

    // ZCk
    addCheckToScene(&ZCkCheckBox, scene, "ZCk", TwoByteShapes::ZCkCheckBox);
    addStatusLabel(&zBitLabel, scene, TwoByteShapes::zBitLabel);

    // NCk
    addCheckToScene(&NCkCheckBox, scene, "NCk", TwoByteShapes::NCkCheckBox);
    addStatusLabel(&nBitLabel, scene, TwoByteShapes::nBitLabel);

    // MemRead/Write
    addLabelToScene(&MemWriteLabel, scene, "MemWrite", TwoByteShapes::MemWriteLabel);
    addTLabel(&MemWriteTristateLabel, scene, TwoByteShapes::MemWriteTristateLabel);
    addLabelToScene(&MemReadLabel, scene, "MemRead", TwoByteShapes::MemReadLabel);
    addTLabel(&MemReadTristateLabel, scene, TwoByteShapes::MemReadTristateLabel);


    // Registers
    regBank = scene->addRect(OneByteShapes::RegBank, QPen(QBrush(QColor(Qt::red), Qt::SolidPattern),
                                        2, Qt::DotLine,
                                        Qt::SquareCap,
                                        Qt::MiterJoin), QBrush(colorScheme->seqCircuitColor));

    auto wordRegExp = QRegExp("(0x){0,1}([0-9a-fA-F]){0,4}");
    auto irRegExp = QRegExp("(0x){0,1}([0-9a-fA-F]){0,6}");
    auto byteRegExp = QRegExp("(0x){0,1}([0-9a-fA-F]){0,2}");
    QString dtext = "0x0000";
    QLabel *ph;

    addRegisterText(labelVec, scene, "0,1", OneByteShapes::getRegNoRect(1, 1), colorScheme);
    addRegisterText(labelVec, scene, "A", OneByteShapes::getRegLabelRect(1, 1), colorScheme);
    addEditableRegister(&aRegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::aRegLineEdit, colorScheme);

    addRegisterText(labelVec, scene, "2,3", OneByteShapes::getRegNoRect(1, 2), colorScheme);
    addRegisterText(labelVec, scene, "X", OneByteShapes::getRegLabelRect(1, 2), colorScheme);
    addEditableRegister(&xRegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::xRegLineEdit, colorScheme);

    addRegisterText(labelVec, scene, "4,5", OneByteShapes::getRegNoRect(1, 3), colorScheme);
    addRegisterText(labelVec, scene, "SP", OneByteShapes::getRegLabelRect(1, 3), colorScheme);
    addEditableRegister(&spRegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::spRegLineEdit, colorScheme);

    addRegisterText(labelVec, scene, "6,7", OneByteShapes::getRegNoRect(1, 4), colorScheme);
    addRegisterText(labelVec, scene, "PC", OneByteShapes::getRegLabelRect(1, 4), colorScheme);
    addEditableRegister(&pcRegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::pcRegLineEdit, colorScheme);

    addRegisterText(labelVec, scene, "8-10",OneByteShapes::getRegNoRect(2, 1), colorScheme);
    addRegisterText(labelVec, scene, "IR",OneByteShapes::getRegLabelRect(2, 1), colorScheme);
    addEditableRegister(&irRegLineEdit, editorVector, scene, "0x000000", irRegExp, TwoByteShapes::irRegLineEdit, colorScheme);

    addRegisterText(labelVec, scene, "11", OneByteShapes::getRegNoRect(2, 2), colorScheme);
    addRegisterText(labelVec, scene, "T1", OneByteShapes::getRegLabelRect(2, 2), colorScheme);
    addEditableRegister(&t1RegLineEdit, editorVector, scene, "0x00", byteRegExp, TwoByteShapes::t1RegLineEdit, colorScheme);

    addRegisterText(labelVec, scene, "12,13", OneByteShapes::getRegNoRect(2, 3), colorScheme);
    addRegisterText(labelVec, scene, "T2", OneByteShapes::getRegLabelRect(2, 3), colorScheme);
    addEditableRegister(&t2RegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::t2RegLineEdit, colorScheme);

    addRegisterText(labelVec, scene, "14,15",OneByteShapes::getRegNoRect(2, 4), colorScheme);
    addRegisterText(labelVec, scene, "T3",OneByteShapes::getRegLabelRect(2, 4), colorScheme);
    addEditableRegister(&t3RegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::t3RegLineEdit, colorScheme);

    addRegisterText(labelVec, scene, "16,17", OneByteShapes::getRegNoRect(3, 1), colorScheme);
    addRegisterText(labelVec, scene, "T4", OneByteShapes::getRegLabelRect(3, 1), colorScheme);
    addEditableRegister(&t4RegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::t4RegLineEdit, colorScheme);

    addRegisterText(labelVec, scene, "18,19", OneByteShapes::getRegNoRect(3, 2), colorScheme);
    addRegisterText(labelVec, scene, "T5", OneByteShapes::getRegLabelRect(3, 2), colorScheme);
    addEditableRegister(&t5RegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::t5RegLineEdit, colorScheme);

    addRegisterText(labelVec, scene, "20,21", OneByteShapes::getRegNoRect(3, 3), colorScheme);
    addRegisterText(labelVec, scene, "T6", OneByteShapes::getRegLabelRect(3, 3), colorScheme);
    addEditableRegister(&t6RegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::t6RegLineEdit, colorScheme);

    addRegisterText(labelVec, scene, "22,23", OneByteShapes::getRegNoRect(3, 4), colorScheme);
    addRegisterText(labelVec, scene, "M1", OneByteShapes::getRegLabelRect(3, 4), colorScheme);
    addStaticRegister(labelVec, scene, "0x0001", OneByteShapes::m1RegLabel, colorScheme);

    addRegisterText(labelVec, scene, "24,25", OneByteShapes::getRegNoRect(4, 1), colorScheme);
    addRegisterText(labelVec, scene, "M2", OneByteShapes::getRegLabelRect(4, 1), colorScheme);
    addStaticRegister(labelVec, scene, "0x0203", OneByteShapes::m2RegLabel, colorScheme);

    addRegisterText(labelVec, scene, "26,27", OneByteShapes::getRegNoRect(4, 2), colorScheme);
    addRegisterText(labelVec, scene, "M3", OneByteShapes::getRegLabelRect(4, 2), colorScheme);
    addStaticRegister(labelVec, scene, "0x0408", OneByteShapes::m3RegLabel, colorScheme);

    addRegisterText(labelVec, scene, "28,29", OneByteShapes::getRegNoRect(4, 3), colorScheme);
    addRegisterText(labelVec, scene, "M4", OneByteShapes::getRegLabelRect(4, 3), colorScheme);
    addStaticRegister(labelVec, scene, "0xF0F6", OneByteShapes::m4RegLabel, colorScheme);

    addRegisterText(labelVec, scene, "30,31", OneByteShapes::getRegNoRect(4, 4), colorScheme);
    addRegisterText(labelVec, scene, "M5", OneByteShapes::getRegLabelRect(4, 4), colorScheme);
    addStaticRegister(labelVec, scene, "0xFEFF", OneByteShapes::m5RegLabel, colorScheme);

    //outline around register bank
    regBankOutline = scene->addRect(OneByteShapes::RegBank, QPen(QBrush(QColor(colorScheme->arrowColorOn),
                                                       Qt::SolidPattern),
                                                2, Qt::SolidLine, Qt::SquareCap,
                                                Qt::MiterJoin));

    // do stuff based on the current model:
        // hide 1 byte bus stuff:
        // MARBus (MARA/MARB output bus)
        scene->addPolygon(TwoByteShapes::MARBus,
                          QPen(QBrush(colorScheme->arrowColorOn), 1), QBrush(colorScheme->combCircuitYellow));

        // ALU drawing:
        ALUPoly->moveBy(TwoByteShapes::controlOffsetX, TwoByteShapes::aluOffsetY);
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
    delete aMuxLabel;
    delete aMuxerDataLabel;
    delete aMuxerBorder;
    delete aMuxTristateLabel;
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
        return QRectF(0,0, 650+20, TwoByteShapes::BottomOfAlu+TwoByteShapes::MemReadYOffsetFromALU+TwoByteShapes::labelTriH+10);
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
    repaintMARCkTwoByteModel(painter);
    repaintAMuxSelect(painter); // Needs to be painted before buses
    repaintCMuxSelect(painter);
    painter->setPen(colorScheme->arrowColorOn);

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

    repaintCSMuxSelect(painter);

    repaintSCk(painter);
    repaintCCk(painter);
    repaintVCk(painter);
    repaintZCk(painter);
    repaintNCk(painter);
    repaintMemCommonTwoByte(painter);
    repaintMemReadTwoByteModel(painter);
    repaintMemWriteTwoByteModel(painter);
    repaintSBitOut(painter);
    repaintCBitOut(painter);
    repaintVBitOut(painter);
    repaintZBitOut(painter);
    repaintNBitOut(painter);

    repaintAndZSelect(painter);
    repaintALUSelectTwoByteModel(painter);

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
    painter->drawLines(TwoByteShapes::loadCkSelect._lines);

    for (int i = 0; i < TwoByteShapes::loadCkSelect._arrowheads.length(); i++) {
        painter->drawImage(TwoByteShapes::loadCkSelect._arrowheads.at(i),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
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
    painter->drawLines(TwoByteShapes::CSelect._lines);

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
    painter->drawLines(TwoByteShapes::BSelect._lines);

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
    painter->drawLines(TwoByteShapes::ASelect._lines);
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
        painter->drawLines(TwoByteShapes::AMuxSelect._lines);
        painter->drawImage(TwoByteShapes::AMuxSelect._arrowheads.first(), //Should more arrowheads be added, this will need to be a proper for loop.
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
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
        painter->drawPolygon(TwoByteShapes::AMuxBus);

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
        painter->drawLines(TwoByteShapes::CMuxSelect._lines);
        painter->drawImage(TwoByteShapes::CMuxSelect._arrowheads.first(),
                           color == Qt::gray ? arrowDownGray : arrowDown);
    // CMuxBus (output)
}

void CpuGraphicsItems::repaintSCk(QPainter *painter)
{
    QColor color;

    color = SCkCheckBox->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // line from checkbox to data
        painter->drawLine(TwoByteShapes::SBitSelect);
        // arrow
        painter->drawImage(QPoint(TwoByteShapes::sBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::SBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);

}

void CpuGraphicsItems::repaintCCk(QPainter *painter)
{
    QColor color;

    color = CCkCheckBox->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // line from checkbox to data
        painter->drawLine(TwoByteShapes::CBitSelect);
        // arrow
        painter->drawImage(QPoint(TwoByteShapes::cBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::CBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);

}

void CpuGraphicsItems::repaintVCk(QPainter *painter)
{
    QColor color;

    color = VCkCheckBox->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);
        painter->drawLine(TwoByteShapes::VBitSelect);
        painter->drawImage(QPoint(TwoByteShapes::vBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::VBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
}

void CpuGraphicsItems::repaintZCk(QPainter *painter)
{
    QColor color;

    color = ZCkCheckBox->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);
        painter->drawLine(TwoByteShapes::ZBitSelect);
        painter->drawImage(QPoint(TwoByteShapes::zBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::ZBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
}

void CpuGraphicsItems::repaintNCk(QPainter *painter)
{
    QColor color;

    color = NCkCheckBox->isChecked() ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);
        painter->drawLine(TwoByteShapes::NBitSelect);
        painter->drawImage(QPoint(TwoByteShapes::nBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::NBitSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
}

void CpuGraphicsItems::repaintSBitOut(QPainter *painter)
{
    sBitLabel->text() = dataSection->getStatusBit(Enu::STATUS_S)?"1":"0";

    QColor color = colorScheme->arrowColorOn;

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);
        // line from S bit to CSMux
        painter->drawLines(TwoByteShapes::SBitToCSMux._lines);
        // arrow:
        painter->drawImage(TwoByteShapes::SBitToCSMux._arrowheads.first(), arrowUp);
}

void CpuGraphicsItems::repaintCBitOut(QPainter *painter)
{
    cBitLabel->text() = dataSection->getStatusBit(Enu::STATUS_C)?"1":"0";

    QColor color = colorScheme->arrowColorOn;

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);
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
}

void CpuGraphicsItems::repaintVBitOut(QPainter *painter)
{
    vBitLabel->text() = dataSection->getStatusBit(Enu::STATUS_V)?"1":"0";

    QColor color = colorScheme->arrowColorOn;

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);
        painter->drawLines(TwoByteShapes::VBitOut._lines);

        painter->drawImage(TwoByteShapes::VBitOut._arrowheads.first(), arrowLeft);
}

void CpuGraphicsItems::repaintZBitOut(QPainter *painter)
{
    zBitLabel->text() = dataSection->getStatusBit(Enu::STATUS_Z)?"1":"0";

    QColor color = colorScheme->arrowColorOn;

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    QPoint point = QPoint(437,582);
        point.setX(point.x() + TwoByteShapes::controlOffsetX);
        point.setY(point.y() + TwoByteShapes::aluOffsetY);
        painter->drawEllipse(point, 2, 2);
        painter->drawLines(TwoByteShapes::ZBitOut._lines);

        painter->drawImage(TwoByteShapes::ZBitOut._arrowheads.first(), arrowLeft);
        painter->drawImage(TwoByteShapes::ZBitOut._arrowheads.last(), arrowUp);  // AndZ arrow upwards
}

void CpuGraphicsItems::repaintNBitOut(QPainter *painter)
{
    nBitLabel->text() = dataSection->getStatusBit(Enu::STATUS_N)?"1":"0";

    QPolygon poly;
    QColor color = colorScheme->arrowColorOn;

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);
        painter->drawLines(TwoByteShapes::NBitOut._lines);

        painter->drawImage(TwoByteShapes::NBitOut._arrowheads.first(), arrowLeft);
}

void CpuGraphicsItems::repaintCSMuxSelect(QPainter *painter)
{
    QColor color;

    color = CSMuxTristateLabel->text() != "" ? colorScheme->arrowColorOn : Qt::gray;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);
        // line from checkbox to data
        painter->drawLine(TwoByteShapes::CSMuxSelect);
        // arrow
        painter->drawImage(QPoint(TwoByteShapes::CSMuxerDataLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::CSMuxSelect.y1() - 3),
                           color == Qt::gray ? arrowLeftGray : arrowLeft);
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
        painter->drawLine(TwoByteShapes::AndZOut._lines[0]);
        painter->drawLine(TwoByteShapes::AndZOut._lines[1]);
        painter->drawImage(TwoByteShapes::AndZOut._arrowheads.first(),
                           color == Qt::gray ? arrowDownGray : arrowDown);

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
        painter->drawLine(TwoByteShapes::AndZOut._lines[2]);
        //The arrow is ~10 pixels long, and another 3 are needed for it to fit comfortably next to the box
        //The arrow is 8 pixels high, align the the center of the arrow with the middle of the box.
        painter->drawImage(QPoint(TwoByteShapes::zBitLabel.x()-13,TwoByteShapes::AndZMuxLabel.y()+TwoByteShapes::AndZMuxLabel.height()/2-4),
                           color == Qt::gray ? arrowRightGray : arrowRight);
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



