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
#include "pep.h"

#include "shapes_two_byte_data_bus.h"
#include "cpudata.h"

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

void addTLabel(TristateLabel** labelLoc, QGraphicsScene *scene, const QRect& geometry){
    (*labelLoc) = new TristateLabel(nullptr, TristateLabel::Tristate);
    (*labelLoc)->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    (*labelLoc)->setGeometry(geometry);
    (*labelLoc)->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    scene->addWidget(*labelLoc);
}

void addStatusLabel(TristateLabel** labelLoc, QGraphicsScene* scene, const QRect& geometry){
    (*labelLoc) = new TristateLabel(nullptr, TristateLabel::ZeroOne);
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
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(ph);
    ph->raise();
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

void addStaticRegister(QVector<QLabel*> &labelVec, QGraphicsScene* scene, QString text, const QRect& geometry, const PepColors::Colors*){
    QLabel* ph = new QLabel(text);
    ph->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    ph->setGeometry(geometry);
    ph->setPalette(QPalette(PepColors::transparent));
    ph->setFont (QFont(Pep::codeFont, Pep::codeFontSize));
    scene->addWidget(ph);
    labelVec.append(ph);
}

void addCheckToScene(QCheckBox** checkLoc, QVector<QCheckBox*> &checkVector, QGraphicsScene *scene, QString name, const QRect& geometry)
{
    *checkLoc = new QCheckBox(name);
    (*checkLoc)->setGeometry(geometry);
    (*checkLoc)->setFont (QFont(Pep::labelFont, Pep::labelFontSize));
    checkVector.append(*checkLoc);
    scene->addWidget(*checkLoc);
}

CpuGraphicsItems::CpuGraphicsItems(Enu::CPUType type, CPUDataSection *dataSection, QWidget *widgetParent,
                                                   QGraphicsItem *itemParent,
                                                   QGraphicsScene *scene)
    : QGraphicsItem(itemParent),parent(widgetParent), parentScene(scene), dataSection(dataSection),
      type(type),colorScheme(&PepColors::lightMode), checkVector(), framedVector(),
      labelVec(), editorVector()

{    
    // http://colrd.com/image-dna/23448/
    // Silence compiler warnings about unused variables.
    // We don't need them at this moment, but they were needed in earlier
    // versions of PepCPU, and I don't want to forget that we used to capture them.
    static_cast<void>(parent);
    static_cast<void>(parentScene);

    // ************************************
    // one  byte exclusive items
    // ************************************
    addCheckToScene(&MDRCk, checkVector, scene, "MDRCk", OneByteShapes::MDRCkCheckbox);
    addLabelToScene(&MDRMuxLabel, scene, "MDRMux", OneByteShapes::MDRMuxLabel);
    addDLabelToScene(&MDRMuxerDataLabel, scene, "MDRMux", OneByteShapes::MDRMuxerDataLabel);
    addTLabel(&MDRMuxTristateLabel, scene, OneByteShapes::MDRMuxTristateLabel);
    addCenteredLabelToScene(&MDRLabel, scene, "0x00", OneByteShapes::MDRLabel);
    // ************************************
    // end one byte exclusive items
    // ************************************

    // ************************************
    // two byte exclusive items
    // ************************************
    addDLabelToScene(&MARMuxerDataLabel, scene, "MARMux", TwoByteShapes::MARMuxerDataLabel);
    addCheckToScene(&MDROCk, checkVector, scene, "MDROCk", TwoByteShapes::MDROCkCheckbox);
    addCheckToScene(&MDRECk, checkVector, scene, "MDRECk", TwoByteShapes::MDRECkCheckbox);
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

    // Hide two byte items
    MARMuxLabel->setVisible(false);
    MARMuxerDataLabel->setVisible(false);
    MARMuxTristateLabel->setVisible(false);
    MDROCk->setVisible(false); MDRECk->setVisible(false);
    MDROMuxLabel->setVisible(false); MDREMuxLabel->setVisible(false);
    MDROMuxerDataLabel->setVisible(false);MDREMuxerDataLabel->setVisible(false);
    MDROMuxTristateLabel->setVisible(false); MDREMuxTristateLabel->setVisible(false);
    MDROLabel->setVisible(false); MDRELabel->setVisible(false);
    EOMuxLabel->setVisible(false);
    EOMuxerDataLabel->setVisible(false);
    EOMuxTristateLabel->setVisible(false);

    // ************************************
    // end two byte exclusive items
    // ************************************

    //MARA & MARB
    addCenteredLabelToScene(&MARALabel, scene, "0x00", OneByteShapes::MARALabel);
    addCenteredLabelToScene(&MARBLabel, scene, "0x00", OneByteShapes::MARBLabel);
    MARALabel->setAutoFillBackground(false);
    MARBLabel->setAutoFillBackground(false);

    // LoadCk
    addCheckToScene(&loadCk, checkVector, scene, "LoadCk", OneByteShapes::loadCkCheckbox);

    // C
    // Note: the line edits must be added first, otherwise they cover the
    //  labels that go with them.
    QRegExp cbaRegExp("^((3[0-1])|([0-2][0-9])|([0-9]))$");
    addEditableRegister(&cLineEdit, framedVector, scene, "", cbaRegExp, OneByteShapes::cLineEdit, colorScheme);
    addLabelToScene(&cLabel, scene, "C", OneByteShapes::cLabel);
    cLineEdit->setAlignment(Qt::AlignCenter);

    // B
    addEditableRegister(&bLineEdit,framedVector, scene, "", cbaRegExp, OneByteShapes::bLineEdit, colorScheme);
    addLabelToScene(&bLabel, scene, "B", OneByteShapes::bLabel);
    bLineEdit->setAlignment(Qt::AlignCenter);

    // A
    addEditableRegister(&aLineEdit, framedVector, scene, "", cbaRegExp, OneByteShapes::aLineEdit, colorScheme);
    addLabelToScene(&aLabel, scene, "A", OneByteShapes::aLabel);
    aLineEdit->setAlignment(Qt::AlignCenter);

    // MARCk
    addCheckToScene(&MARCk, checkVector, scene, "MARCk", OneByteShapes::MARCkCheckbox);


    // AMux
    addLabelToScene(&aMuxLabel, scene, "AMux", OneByteShapes::aMuxLabel);
    addDLabelToScene(&aMuxerDataLabel, scene, "AMux", OneByteShapes::aMuxerDataLabel);
    addTLabel(&aMuxTristateLabel, scene, OneByteShapes::aMuxTristateLabel);

    // CMux
    addLabelToScene(&cMuxLabel, scene, "CMux", OneByteShapes::cMuxLabel);
    addDLabelToScene(&cMuxerLabel, scene, "CMux", OneByteShapes::cMuxerLabel);
    addTLabel(&cMuxTristateLabel, scene, OneByteShapes::cMuxTristateLabel);

    // ALU
    // keep this before the label that goes with it, or the line edit
    //  appears on top of the label
    addEditableRegister(&ALULineEdit, framedVector, scene, "",
                       QRegExp("^((1[0-5])|(0[0-9])|[0-9])$"), OneByteShapes::ALULineEdit, colorScheme);
    addLabelToScene(&ALULabel, scene, "ALU", OneByteShapes::ALULabel);
    addCenteredLabelToScene(&ALUFunctionLabel, scene, "", OneByteShapes::ALUFunctionLabel);
    ALULineEdit->setAlignment(Qt::AlignCenter);

    // ALU shape
    ALUPoly = scene->addPolygon(OneByteShapes::ALUPoly,
                                QPen(QBrush(colorScheme->combCircuitBlue),
                                     2, Qt::SolidLine,
                                     Qt::SquareCap,
                                     Qt::MiterJoin),
                                QBrush(colorScheme->aluColor));
    ALUPoly->setZValue(-1);

    // CSMux
    addLabelToScene(&CSMuxLabel, scene, "CSMux", OneByteShapes::CSMuxLabel);
    addDLabelToScene(&CSMuxerDataLabel, scene, "CSMux", OneByteShapes::CSMuxerDataLabel);
    addTLabel(&CSMuxTristateLabel, scene, OneByteShapes::CSMuxTristateLabel);

    // SCk
    addCheckToScene(&SCkCheckBox, checkVector, scene, "SCk", OneByteShapes::SCkCheckBox);
    addStatusLabel(&sBitLabel, scene, OneByteShapes::sBitLabel);

    // CCk
    addCheckToScene(&CCkCheckBox, checkVector, scene, "CCk", OneByteShapes::CCkCheckBox);
    addStatusLabel(&cBitLabel, scene, OneByteShapes::cBitLabel);

    // VCk
    addCheckToScene(&VCkCheckBox, checkVector, scene, "VCk", OneByteShapes::VCkCheckBox);
    addStatusLabel(&vBitLabel, scene, OneByteShapes::vBitLabel);

    // AndZ
    addLabelToScene(&AndZLabel, scene, "AndZ", OneByteShapes::AndZLabel);
    addTLabel(&AndZTristateLabel, scene, OneByteShapes::AndZTristateLabel);
    addDLabelToScene(&AndZMuxLabel, scene, "AndZ", OneByteShapes::AndZMuxLabel);

    // ZCk
    addCheckToScene(&ZCkCheckBox, checkVector, scene, "ZCk", OneByteShapes::ZCkCheckBox);
    addStatusLabel(&zBitLabel, scene, OneByteShapes::zBitLabel);

    // NCk
    addCheckToScene(&NCkCheckBox, checkVector, scene, "NCk", OneByteShapes::NCkCheckBox);
    addStatusLabel(&nBitLabel, scene, OneByteShapes::nBitLabel);

    // MemRead/Write
    addLabelToScene(&MemWriteLabel, scene, "MemWrite", OneByteShapes::MemWriteLabel);
    addTLabel(&MemWriteTristateLabel, scene, OneByteShapes::MemWriteTristateLabel);
    addLabelToScene(&MemReadLabel, scene, "MemRead", OneByteShapes::MemReadLabel);
    addTLabel(&MemReadTristateLabel, scene, OneByteShapes::MemReadTristateLabel);


    // Registers
    regBank = scene->addRect(OneByteShapes::RegBank, QPen(QBrush(colorScheme->seqCircuitColor, Qt::SolidPattern),
                                        2, Qt::DotLine,
                                        Qt::SquareCap,
                                        Qt::MiterJoin), QBrush(colorScheme->seqCircuitColor));

    auto wordRegExp = QRegExp("(0x){0,1}([0-9a-fA-F]){0,4}");
    auto irRegExp = QRegExp("(0x){0,1}([0-9a-fA-F]){0,6}");
    auto byteRegExp = QRegExp("(0x){0,1}([0-9a-fA-F]){0,2}");
    QString dtext = "0x0000";
    addEditableRegister(&aRegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::aRegLineEdit, colorScheme);
    addEditableRegister(&xRegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::xRegLineEdit, colorScheme);
    addEditableRegister(&spRegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::spRegLineEdit, colorScheme);
    addEditableRegister(&pcRegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::pcRegLineEdit, colorScheme);
    addEditableRegister(&irRegLineEdit, editorVector, scene, "0x000000", irRegExp, TwoByteShapes::irRegLineEdit, colorScheme);
    addEditableRegister(&t1RegLineEdit, editorVector, scene, "0x00", byteRegExp, TwoByteShapes::t1RegLineEdit, colorScheme);
    addEditableRegister(&t2RegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::t2RegLineEdit, colorScheme);
    addEditableRegister(&t3RegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::t3RegLineEdit, colorScheme);
    addEditableRegister(&t4RegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::t4RegLineEdit, colorScheme);
    addEditableRegister(&t5RegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::t5RegLineEdit, colorScheme);
    addEditableRegister(&t6RegLineEdit, editorVector, scene, dtext, wordRegExp, TwoByteShapes::t6RegLineEdit, colorScheme);
    addStaticRegister(labelVec, scene, "0x0001", OneByteShapes::m1RegLabel, colorScheme);
    addStaticRegister(labelVec, scene, "0x0203", OneByteShapes::m2RegLabel, colorScheme);
    addStaticRegister(labelVec, scene, "0x0408", OneByteShapes::m3RegLabel, colorScheme);
    addStaticRegister(labelVec, scene, "0xF0F6", OneByteShapes::m4RegLabel, colorScheme);
    addStaticRegister(labelVec, scene, "0xFEFF", OneByteShapes::m5RegLabel, colorScheme);

    addRegisterText(labelVec, scene, "0,1", OneByteShapes::getRegNoRect(1, 1), colorScheme);
    addRegisterText(labelVec, scene, "A", OneByteShapes::getRegLabelRect(1, 1), colorScheme);

    addRegisterText(labelVec, scene, "2,3", OneByteShapes::getRegNoRect(1, 2), colorScheme);
    addRegisterText(labelVec, scene, "X", OneByteShapes::getRegLabelRect(1, 2), colorScheme);

    addRegisterText(labelVec, scene, "4,5", OneByteShapes::getRegNoRect(1, 3), colorScheme);
    addRegisterText(labelVec, scene, "SP", OneByteShapes::getRegLabelRect(1, 3), colorScheme);

    addRegisterText(labelVec, scene, "6,7", OneByteShapes::getRegNoRect(1, 4), colorScheme);
    addRegisterText(labelVec, scene, "PC", OneByteShapes::getRegLabelRect(1, 4), colorScheme);

    addRegisterText(labelVec, scene, "8-10",OneByteShapes::getRegNoRect(2, 1), colorScheme);
    addRegisterText(labelVec, scene, "IR",OneByteShapes::getRegLabelRect(2, 1), colorScheme);

    addRegisterText(labelVec, scene, "11", OneByteShapes::getRegNoRect(2, 2), colorScheme);
    addRegisterText(labelVec, scene, "T1", OneByteShapes::getRegLabelRect(2, 2), colorScheme);

    addRegisterText(labelVec, scene, "12,13", OneByteShapes::getRegNoRect(2, 3), colorScheme);
    addRegisterText(labelVec, scene, "T2", OneByteShapes::getRegLabelRect(2, 3), colorScheme);

    addRegisterText(labelVec, scene, "14,15",OneByteShapes::getRegNoRect(2, 4), colorScheme);
    addRegisterText(labelVec, scene, "T3",OneByteShapes::getRegLabelRect(2, 4), colorScheme);

    addRegisterText(labelVec, scene, "16,17", OneByteShapes::getRegNoRect(3, 1), colorScheme);
    addRegisterText(labelVec, scene, "T4", OneByteShapes::getRegLabelRect(3, 1), colorScheme);

    addRegisterText(labelVec, scene, "18,19", OneByteShapes::getRegNoRect(3, 2), colorScheme);
    addRegisterText(labelVec, scene, "T5", OneByteShapes::getRegLabelRect(3, 2), colorScheme);

    addRegisterText(labelVec, scene, "20,21", OneByteShapes::getRegNoRect(3, 3), colorScheme);
    addRegisterText(labelVec, scene, "T6", OneByteShapes::getRegLabelRect(3, 3), colorScheme);

    addRegisterText(labelVec, scene, "22,23", OneByteShapes::getRegNoRect(3, 4), colorScheme);
    addRegisterText(labelVec, scene, "M1", OneByteShapes::getRegLabelRect(3, 4), colorScheme);

    addRegisterText(labelVec, scene, "24,25", OneByteShapes::getRegNoRect(4, 1), colorScheme);
    addRegisterText(labelVec, scene, "M2", OneByteShapes::getRegLabelRect(4, 1), colorScheme);

    addRegisterText(labelVec, scene, "26,27", OneByteShapes::getRegNoRect(4, 2), colorScheme);
    addRegisterText(labelVec, scene, "M3", OneByteShapes::getRegLabelRect(4, 2), colorScheme);

    addRegisterText(labelVec, scene, "28,29", OneByteShapes::getRegNoRect(4, 3), colorScheme);
    addRegisterText(labelVec, scene, "M4", OneByteShapes::getRegLabelRect(4, 3), colorScheme);

    addRegisterText(labelVec, scene, "30,31", OneByteShapes::getRegNoRect(4, 4), colorScheme);
    addRegisterText(labelVec, scene, "M5", OneByteShapes::getRegLabelRect(4, 4), colorScheme);

    //outline around register bank
    regBankOutline = scene->addRect(OneByteShapes::RegBank, QPen(QBrush(QColor(colorScheme->arrowColorOn),
                                                       Qt::SolidPattern),
                                                2, Qt::SolidLine, Qt::SquareCap,
                                                Qt::MiterJoin));

    if(type == Enu::TwoByteDataBus) {
        CPUTypeChanged(type);
    }

    darkModeChanged(false, "");
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

    // Delete register line edits
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

    // Delete one byte items
    delete MDRCk;
    delete MDRMuxLabel;
    delete MDRMuxerDataLabel;
    delete MDRMuxTristateLabel;
    delete MDRLabel;

    // Delete two byte items
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
    if (type == Enu::OneByteDataBus) {
        return QRectF(0,0, 650 + 20, 670);
    }
    else if (type == Enu::TwoByteDataBus) {
        return QRectF(0, 0, 650 + 20, TwoByteShapes::BottomOfAlu + TwoByteShapes::MemReadYOffsetFromALU + TwoByteShapes::labelTriH + 10);
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
    (void) alu; //Cast to silence unused variable warning
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
    }
    else { // nonunary
        if (aMuxTristateLabel->text() == "0" && bLineEdit->text() != "") {
            return true;
        }
        else if (aMuxTristateLabel->text() == "1" && aLineEdit->text() != ""
                 && bLineEdit->text() != "") {
            return true;
        }
    }
    return false;
}

void CpuGraphicsItems::paint(QPainter *painter,
                                     const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(colorScheme->arrowColorOn);

    drawStaticRects(painter);
    // c,b,a select line text
    repaintLoadCk(painter);
    repaintCSelect(painter);
    repaintBSelect(painter);
    repaintASelect(painter);
    repaintMARCk(painter);
    repaintAMuxSelect(painter); // Needs to be painted before buses
    repaintCMuxSelect(painter);
    painter->setPen(colorScheme->arrowColorOn);

    switch(type) {
    case Enu::CPUType::OneByteDataBus:
        repaintMDRMuxSelect(painter);
        repaintMDRCk(painter);
        /*
         * Paint the buses in the correct order for the One Byte Bus
         * In the one byte bus, the buses must be drawn in the following order C, B, A.
         * The B bus overlaps with the A bus, and the C bus overlaps with the B bus.
         * So, this rendering order prevents graphical issues.
         */
        repaintCBusOneByte(painter);
        repaintBBusOneByte(painter);
        repaintABusOneByte(painter);
        break;
    case Enu::CPUType::TwoByteDataBus:
        repaintMDROCk(painter);
        repaintMDRECk(painter);
        repaintEOMuxSelect(painter);
        repaintMDROSelect(painter);
        repaintMDRESelect(painter);
        repaintMARMUXToMARBuses(painter);
        repaintMDRMuxOutputBuses(painter);
        // Repaint every select line above ALU firt first
        repaintMARMuxSelect(painter);
        repaintMDREToEOMuxBus(painter);
        repaintMDROToEOMuxBus(painter);
        repaintEOMuxOutpusBus(painter);
        repaintBBusTwoByte(painter);
        repaintABusTwoByte(painter);
        repaintCBusTwoByte(painter);
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

    drawDiagramFreeText(painter);
    //Option to disable updating the static data on certain renders
    if(true) {
        drawALUPoly();
        drawRegisterBank();
    }

}

void CpuGraphicsItems::drawDiagramFreeText(QPainter *painter)
{
#pragma message ("Remove magic numbers")
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(colorScheme->arrowColorOn);
    painter->drawText(7, 320, "Addr");
    painter->drawText(OneByteShapes::RegBank.right() + OneByteShapes::arrowHDepth + 11, 92, "5");
    painter->drawText(OneByteShapes::RegBank.right() + OneByteShapes::arrowHDepth + 11, 70, "5");
    painter->drawText(OneByteShapes::RegBank.right() + OneByteShapes::arrowHDepth + 11, 48, "5");
    painter->save();
    painter->rotate(-90);
    auto font= painter->font();
    font.setPointSize(static_cast<int>(font.pointSize()*1.3));
    painter->setFont(font);
    painter->drawText(-260, 35, "System Bus");
    painter->restore();
    int NZVCXBase = 314, NZVCYBase = 531;
    int dBusY = 395, busY = 132;
    int aBusX = 372, bBusX = 433, cBusX = 300;
    switch(type)
    {
    case Enu::CPUType::OneByteDataBus:
        // alu select line text
        painter->drawText(OneByteShapes::ctrlInputX - 23, ALULineEdit->y() + 5, "4");
        painter->drawText(368,388, "ALU");
        painter->drawText(OneByteShapes::MARALabel.x() - 37, OneByteShapes::MARALabel.y() + 13, "MARA");
        painter->drawText(OneByteShapes::MARBLabel.x() - 37, OneByteShapes::MARBLabel.y() + 13, "MARB");

        break;
    case Enu::CPUType::TwoByteDataBus:
        // Adjusst base point of NZVC bits
        NZVCXBase += TwoByteShapes::controlOffsetX;
        NZVCYBase += TwoByteShapes::aluOffsetY;

        // Adjust base point of a, b, c, data buses
        dBusY = TwoByteShapes::DataArrowMidpointY+TwoByteShapes::DataArrowOuterYSpread+10;
        aBusX +=50;
        bBusX += 50;
        cBusX += 50;

        // alu select line text
        painter->drawText(TwoByteShapes::ctrlInputX - 23, ALULineEdit->y() + 5, "4");
        painter->drawText(368 + TwoByteShapes::controlOffsetX,
                          388 + TwoByteShapes::aluOffsetY, "ALU");

        painter->drawText(TwoByteShapes::MARALabel.x() - 37, TwoByteShapes::MARALabel.y() + 13, "MARA");
        painter->drawText(TwoByteShapes::MARBLabel.x() - 37, TwoByteShapes::MARBLabel.y() + 13, "MARB");
        break;
    }
    painter->drawText(NZVCXBase, NZVCYBase, "0");
    painter->drawText(NZVCXBase, NZVCYBase + 10, "0");
    painter->drawText(NZVCXBase, NZVCYBase + 20, "0");
    painter->drawText(NZVCXBase, NZVCYBase + 30, "0");
    painter->drawText(7, dBusY, "Data");
    painter->drawText(aBusX, busY, "ABus");
    painter->drawText(bBusX, busY, "BBus");
    painter->drawText(cBusX, busY, "CBus");

}

void CpuGraphicsItems::drawLabels()
{

    QPalette seqColor = QPalette();
    seqColor.setColor(QPalette::Text, colorScheme->textColor);
    seqColor.setColor(QPalette::WindowText, colorScheme->textColor);
    seqColor.setColor(QPalette::Base, colorScheme->backgroundFill);
    seqColor.setColor(QPalette::Window, PepColors::transparent);

    QPalette combColor = QPalette();
    combColor.setColor(QPalette::WindowText, colorScheme->textColor);
    combColor.setColor(QPalette::Base, colorScheme->seqCircuitColor);
    combColor.setColor(QPalette::Window, colorScheme->seqCircuitColor);

    QPalette aluLabel = QPalette();
    aluLabel.setColor(QPalette::WindowText, colorScheme->textColor);
    aluLabel.setColor(QPalette::Base, PepColors::transparent);
    aluLabel.setColor(QPalette::Window, PepColors::transparent);

    // Set Line editors first
    QString lineStyle = QString("QLineEdit, QCheckbox::indicator { border: 1px solid rgb(%1, %2, %3);\
                                 background: rgb(%4, %5, %6); color: rgb(%7, %8, %9)}")
            .arg(colorScheme->arrowColorOff.red(), 3, 10)
            .arg(colorScheme->arrowColorOff.green(), 3, 10)
            .arg(colorScheme->arrowColorOff.blue(), 3, 10)
            .arg(colorScheme->backgroundFill.red(), 3, 10)
            .arg(colorScheme->backgroundFill.green(), 3, 10)
            .arg(colorScheme->backgroundFill.blue(), 3, 10)
            .arg(colorScheme->textColor.red(), 3, 10)
            .arg(colorScheme->textColor.green(), 3, 10)
            .arg(colorScheme->textColor.blue(), 3, 10);

    cLineEdit->setStyleSheet(lineStyle);
    bLineEdit->setStyleSheet(lineStyle);
    aLineEdit->setStyleSheet(lineStyle);
    ALULineEdit->setStyleSheet(lineStyle);

    // Common labels
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

    // One Byte Exclusive Labels
    MDRLabel->setPalette(combColor);
    MDRMuxTristateLabel->setPalette(seqColor);
    MDRMuxLabel->setPalette(seqColor);
    MDRMuxerDataLabel->setPalette(seqColor);
    MDRCk->setPalette(seqColor);

    // Two Byte Exclusive Labels
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
    if(type == Enu::CPUType::OneByteDataBus) {
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
    else if(type == Enu::CPUType::TwoByteDataBus) {
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
    pal.setColor(QPalette::Text, colorScheme->arrowColorOn);
    pal.setColor(QPalette::WindowText, colorScheme->arrowColorOn);
    pal.setColor(QPalette::Base, colorScheme->backgroundFill);
    pal.setColor(QPalette::Background, PepColors::transparent);
    for(QLineEdit* edit : editorVector) {
        edit->setPalette(pal);
    }
    pal.setColor(QPalette::Base, PepColors::transparent);
    pal.setColor(QPalette::Background, PepColors::transparent);
    for(QLabel* label : labelVec) {
        label->raise();
        label->setPalette(pal);
    }
}

void CpuGraphicsItems::repaintLoadCk(QPainter *painter)
{
    QColor color;

    color = loadCk->isChecked() ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLines(OneByteShapes::loadCkSelect._lines);
        for (int i = 0; i < OneByteShapes::loadCkSelect._arrowheads.length(); i++) {
            painter->drawImage(OneByteShapes::loadCkSelect._arrowheads.at(i),
                               color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        }
        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::loadCkSelect._lines);
        for (int i = 0; i < TwoByteShapes::loadCkSelect._arrowheads.length(); i++) {
            painter->drawImage(TwoByteShapes::loadCkSelect._arrowheads.at(i),
                               color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        }
        break;
    }
}

void CpuGraphicsItems::repaintCSelect(QPainter *painter)
{
    bool ok;
    cLineEdit->text().toInt(&ok, 10);

    QColor color = ok ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // Draw select lines
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLines(OneByteShapes::CSelect._lines);
        painter->drawImage(OneByteShapes::CSelect._arrowheads.at(0),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::CSelect._lines);
        painter->drawImage(TwoByteShapes::CSelect._arrowheads.at(0),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    }
}

void CpuGraphicsItems::repaintBSelect(QPainter *painter)
{
    bool ok;
    bLineEdit->text().toInt(&ok, 10);

    QColor color = ok ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // Draw select lines
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLines(OneByteShapes::BSelect._lines);
        painter->drawImage(OneByteShapes::BSelect._arrowheads.at(0),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::BSelect._lines);
        painter->drawImage(TwoByteShapes::BSelect._arrowheads.at(0),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    }
}

void CpuGraphicsItems::repaintASelect(QPainter *painter)
{
    bool ok;
    aLineEdit->text().toInt(&ok, 10);

    QColor color = ok ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // Draw select lines
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLines(OneByteShapes::ASelect._lines);
        painter->drawImage(OneByteShapes::ASelect._arrowheads.at(0),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::ASelect._lines);
        painter->drawImage(TwoByteShapes::ASelect._arrowheads.at(0),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    }
}

void CpuGraphicsItems::repaintAMuxSelect(QPainter *painter)
{
    bool ok;
    int aMux = aMuxTristateLabel->text().toInt(&ok, 10);

    QColor color = ok ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);
    // Draw AMux select depending on the enabled feature set
    switch(type)
    {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLines(OneByteShapes::AMuxSelect._lines);
        painter->drawImage(QPoint(380,300),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::AMuxSelect._lines);
        painter->drawImage(TwoByteShapes::AMuxSelect._arrowheads.first(), //Should more arrowheads be added, this will need to be a proper for loop.
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    }
    // Set the color of the amux based on the data flow
    QPalette pal = aMuxerDataLabel->palette();
    if (ok) {
        switch (aMux) { // Decide to route through left or right value
        case 0:
            if(dataSection->getCPUType() == Enu::TwoByteDataBus) { // If using 2-byte cpu
                if(EOMuxTristateLabel->text() == "0"){ // Then AMux depends on EOMux
                    color = colorScheme->combCircuitGreen;
                    pal.setColor(QPalette::Background,colorScheme->muxCircuitGreen);
                    aMuxerDataLabel->setPalette(pal);
                }
                else if(EOMuxTristateLabel->text() == "1") {
                    color = colorScheme->combCircuitYellow;
                    pal.setColor(QPalette::Background, colorScheme->muxCircuitYellow);
                    aMuxerDataLabel->setPalette(pal);
                }
                else { //When AMux is routing from left, but EOMux is inactive
                    color = colorScheme->backgroundFill;
                    pal.setColor(QPalette::Background, color);
                    aMuxerDataLabel->setPalette(pal);
                }
            }
            else {
                color = colorScheme->combCircuitYellow;
                pal.setColor(QPalette::Background, colorScheme->muxCircuitYellow);
                aMuxerDataLabel->setPalette(pal);
            }
            break;
        case 1:
            if (aLineEdit->text() == "") { // ABus.state == UNDEFINED
                color = colorScheme->backgroundFill;
                pal.setColor(QPalette::Background, color);
                aMuxerDataLabel->setPalette(pal);
            }
            else {
                color = colorScheme->combCircuitRed;
                pal.setColor(QPalette::Background, colorScheme->muxCircuitRed);
                aMuxerDataLabel->setPalette(pal);
            }
            break;
        }
    }
    else {
        color = colorScheme->backgroundFill;
        pal.setColor(QPalette::Background,color);
        aMuxerDataLabel->setPalette(pal);
    }
    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);

    // Draw AMux bus depending on the enabled feature set
    switch(type)
    {
    case Enu::CPUType::OneByteDataBus:
        painter->drawPolygon(OneByteShapes::AMuxBus);
        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawPolygon(TwoByteShapes::AMuxBus);
        break;
    }

}

void CpuGraphicsItems::repaintMARMuxSelect(QPainter *painter)
{
    bool ok;
    MARMuxTristateLabel->text().toInt(&ok, 10);

    QColor color = ok ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // AMux Select
    painter->drawLines(TwoByteShapes::MARMuxSelect._lines);
    painter->drawImage(TwoByteShapes::MARMuxSelect._arrowheads.first(),
                       color == colorScheme->arrowColorOff ? arrowDownGray : arrowDown);


}

void CpuGraphicsItems::repaintEOMuxSelect(QPainter *painter)
{
    bool ok;
    EOMuxTristateLabel->text().toInt(&ok, 10);

    QColor color = ok ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // AMux Select
    painter->drawLines(TwoByteShapes::EOMuxSelect._lines);

    painter->drawImage(TwoByteShapes::EOMuxSelect._arrowheads.first(),
                       color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);

}

void CpuGraphicsItems::repaintCMuxSelect(QPainter *painter)
{
    bool ok;
    cMuxTristateLabel->text().toInt(&ok, 10);

    QColor color = ok ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // CMux Select, depending on feature set
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLines(OneByteShapes::CMuxSelect._lines);
        painter->drawImage(OneByteShapes::CMuxSelect._arrowheads.first(),
                           color == colorScheme->arrowColorOff ? arrowDownGray : arrowDown);
        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::CMuxSelect._lines);
        painter->drawImage(TwoByteShapes::CMuxSelect._arrowheads.first(),
                           color == colorScheme->arrowColorOff ? arrowDownGray : arrowDown);
        break;
    }
}

void CpuGraphicsItems::repaintSCk(QPainter *painter)
{
    QColor color = SCkCheckBox->isChecked() ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // line from checkbox to data
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLine(OneByteShapes::SBitSelect);
        // arrow
        painter->drawImage(QPoint(OneByteShapes::sBitLabel.right() + OneByteShapes::arrowHOffset,
                                  OneByteShapes::SBitSelect.y1() - 3),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);

        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLine(TwoByteShapes::SBitSelect);
        // arrow
        painter->drawImage(QPoint(TwoByteShapes::sBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::SBitSelect.y1() - 3),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    }
}

void CpuGraphicsItems::repaintCCk(QPainter *painter)
{
    QColor color = CCkCheckBox->isChecked() ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // line from checkbox to data
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLine(OneByteShapes::CBitSelect);
        // arrow
        painter->drawImage(QPoint(OneByteShapes::cBitLabel.right() + OneByteShapes::arrowHOffset,
                                  OneByteShapes::CBitSelect.y1() - 3),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLine(TwoByteShapes::CBitSelect);
        // arrow
        painter->drawImage(QPoint(TwoByteShapes::cBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::CBitSelect.y1() - 3),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    }

}

void CpuGraphicsItems::repaintVCk(QPainter *painter)
{
    QColor color = VCkCheckBox->isChecked() ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLine(OneByteShapes::VBitSelect);
        painter->drawImage(QPoint(OneByteShapes::vBitLabel.right() + OneByteShapes::arrowHOffset,
                                  OneByteShapes::VBitSelect.y1() - 3),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLine(TwoByteShapes::VBitSelect);
        painter->drawImage(QPoint(TwoByteShapes::vBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::VBitSelect.y1() - 3),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    }
}

void CpuGraphicsItems::repaintZCk(QPainter *painter)
{
    QColor color = ZCkCheckBox->isChecked() ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLine(OneByteShapes::ZBitSelect);
        painter->drawImage(QPoint(OneByteShapes::zBitLabel.right() + OneByteShapes::arrowHOffset,
                                  OneByteShapes::ZBitSelect.y1() - 3),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLine(TwoByteShapes::ZBitSelect);
        painter->drawImage(QPoint(TwoByteShapes::zBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::ZBitSelect.y1() - 3),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    }
}

void CpuGraphicsItems::repaintNCk(QPainter *painter)
{
    QColor color = NCkCheckBox->isChecked() ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLine(OneByteShapes::NBitSelect);
        painter->drawImage(QPoint(OneByteShapes::nBitLabel.right() + OneByteShapes::arrowHOffset,
                                  OneByteShapes::NBitSelect.y1() - 3),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLine(TwoByteShapes::NBitSelect);
        painter->drawImage(QPoint(TwoByteShapes::nBitLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::NBitSelect.y1() - 3),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    }
}

void CpuGraphicsItems::repaintSBitOut(QPainter *painter)
{
    sBitLabel->text() = dataSection->getStatusBit(Enu::STATUS_S)?"1":"0";

    QColor color = colorScheme->arrowColorOn;

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    switch(type)
    {
    case Enu::CPUType::OneByteDataBus:
        // line from S bit to CSMux
        painter->drawLines(OneByteShapes::SBitToCSMux._lines);
        // arrow:
        painter->drawImage(OneByteShapes::SBitToCSMux._arrowheads.first(), arrowUp);
        break;
    case Enu::CPUType::TwoByteDataBus:
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

    switch (type) {
    case Enu::CPUType::OneByteDataBus:
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
    case Enu::CPUType::TwoByteDataBus:
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
    }
}

void CpuGraphicsItems::repaintVBitOut(QPainter *painter)
{
    vBitLabel->text() = dataSection->getStatusBit(Enu::STATUS_V)?"1":"0";

    QColor color = colorScheme->arrowColorOn;

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLines(OneByteShapes::VBitOut._lines);

        painter->drawImage(OneByteShapes::VBitOut._arrowheads.first(), arrowLeft);

        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::VBitOut._lines);

        painter->drawImage(TwoByteShapes::VBitOut._arrowheads.first(), arrowLeft);

        break;
    }
}

void CpuGraphicsItems::repaintZBitOut(QPainter *painter)
{
    zBitLabel->text() = dataSection->getStatusBit(Enu::STATUS_Z)?"1":"0";

    QColor color = colorScheme->arrowColorOn;

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

#pragma message ("Remove magic numbers")
    QPoint point = QPoint(437,582);
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawEllipse(point, 2, 2);
        painter->drawLines(OneByteShapes::ZBitOut._lines);

        painter->drawImage(OneByteShapes::ZBitOut._arrowheads.first(), arrowLeft);
        painter->drawImage(OneByteShapes::ZBitOut._arrowheads.last(), arrowUp);  // AndZ arrow upwards
        break;
    case Enu::CPUType::TwoByteDataBus:
        point.setX(point.x() + TwoByteShapes::controlOffsetX);
        point.setY(point.y() + TwoByteShapes::aluOffsetY);
        painter->drawEllipse(point, 2, 2);
        painter->drawLines(TwoByteShapes::ZBitOut._lines);

        painter->drawImage(TwoByteShapes::ZBitOut._arrowheads.first(), arrowLeft);
        painter->drawImage(TwoByteShapes::ZBitOut._arrowheads.last(), arrowUp);  // AndZ arrow upwards
        break;
    }
}

void CpuGraphicsItems::repaintNBitOut(QPainter *painter)
{
    nBitLabel->text() = dataSection->getStatusBit(Enu::STATUS_N)?"1":"0";

    QColor color = colorScheme->arrowColorOn;

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLines(OneByteShapes::NBitOut._lines);
        painter->drawImage(OneByteShapes::NBitOut._arrowheads.first(), arrowLeft);
        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLines(TwoByteShapes::NBitOut._lines);
        painter->drawImage(TwoByteShapes::NBitOut._arrowheads.first(), arrowLeft);
        break;
    }
}

void CpuGraphicsItems::repaintCSMuxSelect(QPainter *painter)
{
    QColor color;

    color = CSMuxTristateLabel->text() != "" ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        // line from checkbox to data
        painter->drawLine(OneByteShapes::CSMuxSelect);
        // arrow
        painter->drawImage(QPoint(OneByteShapes::CSMuxerDataLabel.right() + OneByteShapes::arrowHOffset,
                                  OneByteShapes::CSMuxSelect.y1() - 3),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    case Enu::CPUType::TwoByteDataBus:
        // line from checkbox to data
        painter->drawLine(TwoByteShapes::CSMuxSelect);
        // arrow
        painter->drawImage(QPoint(TwoByteShapes::CSMuxerDataLabel.right() + TwoByteShapes::arrowHOffset,
                                  TwoByteShapes::CSMuxSelect.y1() - 3),
                           color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
        break;
    }
}

void CpuGraphicsItems::repaintAndZSelect(QPainter *painter)
{
    QPolygon poly;

    QColor color = colorScheme->arrowColorOff;

    if (AndZTristateLabel->text() != "") {
        color = colorScheme->arrowColorOn;
    }
    painter->setPen(color);
    painter->setBrush(color);

    // lines coming out of tristate label
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLine(OneByteShapes::AndZOut._lines[0]);
        painter->drawLine(OneByteShapes::AndZOut._lines[1]);

        painter->drawImage(OneByteShapes::AndZOut._arrowheads.first(),
                           color == colorScheme->arrowColorOff ? arrowDownGray : arrowDown);
        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLine(TwoByteShapes::AndZOut._lines[0]);
        painter->drawLine(TwoByteShapes::AndZOut._lines[1]);
        painter->drawImage(TwoByteShapes::AndZOut._arrowheads.first(),
                           color == colorScheme->arrowColorOff ? arrowDownGray : arrowDown);
        break;
    }

    color = colorScheme->arrowColorOff;
    if (aluHasCorrectOutput() && AndZTristateLabel->text() != "") {
        color = colorScheme->arrowColorOn;
    }
    else{
        color = colorScheme->arrowColorOff;
    }
    painter->setPen(color);
    painter->setBrush(color);

    // AndZ out
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        painter->drawLine(OneByteShapes::AndZOut._lines[2]);
        painter->drawImage(OneByteShapes::AndZOut._arrowheads[1],
                color == colorScheme->arrowColorOff ? arrowRightGray : arrowRight);
        break;
    case Enu::CPUType::TwoByteDataBus:
        painter->drawLine(TwoByteShapes::AndZOut._lines[2]);
        //The arrow is ~10 pixels long, and another 3 are needed for it to fit comfortably next to the box
        //The arrow is 8 pixels high, align the the center of the arrow with the middle of the box.
        painter->drawImage(TwoByteShapes::AndZOut._arrowheads[1],
                color == colorScheme->arrowColorOff ? arrowRightGray : arrowRight);
        break;
    }
}

// ***************************************************************************
// Dispatch to one or two byte:
// ***************************************************************************
void CpuGraphicsItems::repaintALUSelect(QPainter *painter)
{
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        repaintALUSelectOneByte(painter);
        break;
    case Enu::CPUType::TwoByteDataBus:
        repaintALUSelectTwoByte(painter);
        break;
    }
}

void CpuGraphicsItems::repaintMARCk(QPainter *painter)
{
    switch (type) {
    case Enu::OneByteDataBus:
        repaintMARCkOneByte(painter);
        break;
    case Enu::TwoByteDataBus:
        repaintMARCkTwoByte(painter);
        break;
    }
}

void CpuGraphicsItems::repaintMemCommon(QPainter *painter)
{
    switch(type)
    {
    case Enu::CPUType::OneByteDataBus:
        repaintMemCommonOneByte(painter);
        break;
    case Enu::TwoByteDataBus:
        repaintMemCommonTwoByte(painter);
        break;
    }
}

void CpuGraphicsItems::repaintMemRead(QPainter *painter)
{
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        repaintMemReadOneByte(painter);
        break;
    case Enu::TwoByteDataBus:
        repaintMemReadTwoByte(painter);
        break;
    default:
        break;
    }
}

void CpuGraphicsItems::repaintMemWrite(QPainter *painter)
{
    switch (type) {
    case Enu::CPUType::OneByteDataBus:
        repaintMemWriteOneByte(painter);
        break;
    case Enu::TwoByteDataBus:
        repaintMemWriteTwoByte(painter);
        break;
    default:
        break;
    }
}

// ***************************************************************************
// One byte model-specific functionality:
// ***************************************************************************

void CpuGraphicsItems::repaintMDRCk(QPainter *painter)
{
    QColor color;
    color = MDRCk->isChecked() ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // MDRCk
    painter->drawLines(OneByteShapes::MDRCk._lines);

    painter->drawImage(OneByteShapes::MDRCk._arrowheads.first(),
                       color == colorScheme->arrowColorOff ? arrowDownGray : arrowDown);

}

void CpuGraphicsItems::repaintMDRMuxSelect(QPainter *painter)
{
    QColor color;
    QPalette pal = MDRMuxerDataLabel->palette();
    painter->setPen(colorScheme->arrowColorOn);
    if(MDRCk->isChecked()) {
        if(MDRMuxTristateLabel->text() == "0" && dataSection->getMainBusState() == Enu::MemReadSecondWait) {
            color = colorScheme->combCircuitGreen;
            pal.setColor(QPalette::Background,colorScheme->muxCircuitGreen);
            MDRMuxerDataLabel->setPalette(pal);
        }
        else if(MDRMuxTristateLabel->text() == "1" && cMuxTristateLabel->text() == "0") {
            color = colorScheme->combCircuitYellow;
            pal.setColor(QPalette::Background, colorScheme->muxCircuitYellow);
            MDRMuxerDataLabel->setPalette(pal);
        }
        else if(MDRMuxTristateLabel->text() == "1" && cMuxTristateLabel->text() == "1" && aluHasCorrectOutput()) {
            color = colorScheme->combCircuitBlue;
            pal.setColor(QPalette::Background, colorScheme->muxCircuitBlue);
            MDRMuxerDataLabel->setPalette(pal);
        }
        else {
            color = colorScheme->backgroundFill;
            pal.setColor(QPalette::Background,color);
            MDRMuxerDataLabel->setPalette(pal);
        }

    }
    else {
        color = colorScheme->backgroundFill;
        pal.setColor(QPalette::Background, color);
        MDRMuxerDataLabel->setPalette(pal);
    }

    painter->setBrush(color);
    // MDRMuxOutBus (MDRMux to MDR arrow)
    painter->drawPolygon(OneByteShapes::MDRMuxOutBus);

    // finish up by drawing select lines:
    color = colorScheme->arrowColorOff;
    if (MDRMuxTristateLabel->text() != "") {
        color = colorScheme->arrowColorOn;
    }
    painter->setPen(color);
    painter->setBrush(color);

    // MDRMux Select
    painter->drawLine(257,303, 265,303); painter->drawLine(265,303, 265,324);
    painter->drawLine(265,324, 279,324); painter->drawLine(291,324, 335,324);
    painter->drawLine(347,324, 416,324); painter->drawLine(428,324, OneByteShapes::CommonPositions::ctrlInputX - 7,324);

    painter->drawImage(QPoint(249,300),
                       color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
}

void CpuGraphicsItems::repaintMARCkOneByte(QPainter *painter)
{
    QColor color;

    color = MARCk->isChecked() ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // MARCk
    painter->drawLines(OneByteShapes::MARCk._lines);

    painter->drawEllipse(QPoint(235,177), 2, 2);

    painter->drawImage(OneByteShapes::MARCk._arrowheads.first(),
                       color == colorScheme->arrowColorOff ? arrowUpGray : arrowUp);
    painter->drawImage(OneByteShapes::MARCk._arrowheads.last(),
                       color == colorScheme->arrowColorOff ? arrowDownGray : arrowDown);
}

void CpuGraphicsItems::repaintALUSelectOneByte(QPainter *painter)
{
    QColor color;

    color = ALULineEdit->text() != "" ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // ALU Select
    painter->drawLines(OneByteShapes::ALUSelect._lines);

    painter->drawImage(OneByteShapes::ALUSelect._arrowheads.first(),
                       color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);

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
    color = aluHasCorrectOutput() ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(color);
    painter->setBrush(color);

    painter->drawLines(OneByteShapes::ALUSelectOut._lines);

    for (int i = 0; i < OneByteShapes::ALUSelectOut._arrowheads.length(); i++) {
        painter->drawImage(OneByteShapes::ALUSelectOut._arrowheads.at(i),
                           color == colorScheme->arrowColorOff ? arrowRightGray : arrowRight);
    }

    // S ellipse
    painter->drawEllipse(QPoint(416,446), 2, 2); //437+9
}

void CpuGraphicsItems::repaintMemCommonOneByte(QPainter */*painter*/)
{
    // Has not been split into common parts of read / write
}

void CpuGraphicsItems::repaintMemReadOneByte(QPainter *painter)
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
        color = colorScheme->arrowColorOff;
    }
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // memRead line from label to bus:
    painter->drawLine(OneByteShapes::MemReadSelect);

    painter->drawImage(QPoint(OneByteShapes::DataBus.right()+5,
                              OneByteShapes::MemReadSelect.y1() - 3),
                       color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);

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

void CpuGraphicsItems::repaintMemWriteOneByte(QPainter *painter)
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
        color = colorScheme->arrowColorOff;
    }

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // memWrite line from the label to the bus:
    painter->drawLine(OneByteShapes::MemWriteSelect);
    painter->drawImage(QPoint(OneByteShapes::DataBus.right()+5,
                              OneByteShapes::MemWriteSelect.y1() - 3),
                       color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);

    // draw line from memWrite to MDR out:
    painter->drawEllipse(QPoint(OneByteShapes::DataBus.right()+25,
                                OneByteShapes::MemWriteSelect.y1()),
                         2, 2);
    painter->drawLine(OneByteShapes::DataBus.right()+25, OneByteShapes::MemWriteSelect.y1() - 3,
                      OneByteShapes::DataBus.right()+25,345);
    // memWrite line from the label to the bus:
    painter->drawLine(OneByteShapes::DataBus.right()+25,333, OneByteShapes::DataBus.right()+25,280); //268+12
    painter->drawImage(QPoint(OneByteShapes::DataBus.right()+22,271), //96-3 //268+12-9
                       color == colorScheme->arrowColorOff ? arrowUpGray : arrowUp);

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

void CpuGraphicsItems::repaintABusOneByte(QPainter *painter)
{
    bool ok;
    aLineEdit->text().toInt(&ok, 10);
    QColor color;
    color = ok ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    color = ok ? colorScheme->combCircuitRed : colorScheme->backgroundFill;

    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);
    // ABus
    painter->drawPolygon(OneByteShapes::ABus1);
    painter->drawPolygon(OneByteShapes::ABus2);
}

void CpuGraphicsItems::repaintBBusOneByte(QPainter *painter)
{
    bool ok;
    bLineEdit->text().toInt(&ok, 10);;
    QColor color;
    color = ok ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
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

void CpuGraphicsItems::repaintCBusOneByte(QPainter *painter)
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

void CpuGraphicsItems::repaintMARCkTwoByte(QPainter *painter)
{
    QColor color = MARCk->isChecked() ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // MARCk
    painter->drawLines(TwoByteShapes::MARCk._lines);


    painter->drawEllipse(QPoint(TwoByteShapes::MARCk._lines.last().x1(),
                                TwoByteShapes::MARMuxerDataLabel.y() + TwoByteShapes::MARMuxerDataLabel.height()/2), 2, 2);
    painter->drawImage(TwoByteShapes::MARCk._arrowheads[0],
            color == colorScheme->arrowColorOff ? arrowUpGray : arrowUp);
    painter->drawImage(TwoByteShapes::MARCk._arrowheads[1],
            color == colorScheme->arrowColorOff ? arrowDownGray : arrowDown);
    painter->setBrush(colorScheme->combCircuitYellow);
    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->drawPolygon(TwoByteShapes::MARBus);
}

void CpuGraphicsItems::repaintMDROCk(QPainter *painter)
{
    QColor color = MDROCk->isChecked() ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // MDROdd Clock
    painter->drawLines(TwoByteShapes::MDROck._lines);
    painter->drawImage(TwoByteShapes::MDROck._arrowheads.first(),
                       color == colorScheme->arrowColorOff ? arrowDownGray : arrowDown);
}

void CpuGraphicsItems::repaintMDRECk(QPainter *painter)
{
    QColor color = MDRECk->isChecked() ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // MDREven Clock
    painter->drawLines(TwoByteShapes::MDREck._lines);
    painter->drawImage(TwoByteShapes::MDREck._arrowheads.first(),
                       color == colorScheme->arrowColorOff ? arrowDownGray : arrowDown);

}

void CpuGraphicsItems::repaintEOMuxOutpusBus(QPainter *painter)
{
    QColor color = colorScheme->backgroundFill;
    QPalette pal = EOMuxerDataLabel->palette();
    if(EOMuxTristateLabel->text() == "1") {
        color=colorScheme->combCircuitYellow;
        pal.setColor(QPalette::Background, colorScheme->muxCircuitYellow);
    }
    else if(EOMuxTristateLabel->text() == "0") {
        color = colorScheme->combCircuitGreen;
        pal.setColor(QPalette::Background, colorScheme->muxCircuitGreen);
    }
    else {
        pal.setColor(QPalette::Background,color);
    }
    EOMuxerDataLabel->setPalette(pal);
    painter->setPen(colorScheme->arrowColorOn);
    painter->setBrush(color);
    painter->drawPolygon(TwoByteShapes::EOMuxOutputBus);
}

void CpuGraphicsItems::repaintALUSelectTwoByte(QPainter *painter)
{
    QColor color = ALULineEdit->text() != "" ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // ALU Select
    painter->drawLines(TwoByteShapes::ALUSelect._lines);

    painter->drawImage(TwoByteShapes::ALUSelect._arrowheads.first(),
                       color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);

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
    painter->setPen(aluHasCorrectOutput() ? colorScheme->arrowColorOn : colorScheme->arrowColorOff);
    painter->setBrush(aluHasCorrectOutput() ? colorScheme->arrowColorOn : colorScheme->arrowColorOff);

    painter->drawLines(TwoByteShapes::ALUSelectOut._lines);

    for (int i = 0; i < TwoByteShapes::ALUSelectOut._arrowheads.length(); i++) {
        painter->drawImage(TwoByteShapes::ALUSelectOut._arrowheads.at(i),
                           color == colorScheme->arrowColorOff ? arrowRightGray : arrowRight);
    }

    // S ellipse
#pragma message("Remove magic numbers")
    painter->drawEllipse(QPoint(416 + TwoByteShapes::controlOffsetX,
                                TwoByteShapes::sBitLabel.y() + TwoByteShapes::selectYOffset),
                         2, 2);

}

void CpuGraphicsItems::repaintMemCommonTwoByte(QPainter *painter)
{
    QColor color;
    bool readIsHigh = MemReadTristateLabel->text() == "1", writeIsHigh = MemWriteTristateLabel->text() == "1";

    // If exactly one of read or write is high
    if (readIsHigh ^ writeIsHigh) {
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
    // Main ADDR bus
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
        color = colorScheme->arrowColorOff;
    }

    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // memWrite line from the label to the bus:
    painter->drawLine(TwoByteShapes::MemWriteSelect); //611+8
    painter->drawImage(QPoint(TwoByteShapes::DataBus.right() + 5,
                              TwoByteShapes::MemWriteSelect.y1() - 3),
                       color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);

    // draw line from memWrite to MDRO out:
    painter->drawEllipse(QPoint(TwoByteShapes::DataBus.right() + 20,
                                TwoByteShapes::MemWriteSelect.y1()),
                         2, 2);
    painter->drawLine(TwoByteShapes::DataBus.right() + 20,
                      TwoByteShapes::MemWriteSelect.y1(),
                      TwoByteShapes::DataBus.right() + 20,
                      TwoByteShapes::MDROLabel.bottom()); //611+8
    // memWrite line from the label to the bus:
    painter->drawImage(QPoint(TwoByteShapes::DataBus.right() + 17, //96-3
                              //The bottom of the bus is 5 pixels below the label's midpoint, and add 3 pixels for visual comfort.
                              TwoByteShapes::MDROLabel.y() + TwoByteShapes::MDROLabel.height()/2 + 5 + 3),
                       color == colorScheme->arrowColorOff ? arrowUpGray : arrowUp);
    painter->drawImage(QPoint(TwoByteShapes::DataBus.right() + 17,//96-3
                              //The bottom of the bus is 5 pixels below the label's midpoint, and add 3 pixels for visual comfort.
                              TwoByteShapes::MDRELabel.y() + TwoByteShapes::MDRELabel.height()/2 + 5 + 3),
                       color == colorScheme->arrowColorOff ? arrowUpGray : arrowUp);

    // Draw memread select line
    if (readIsHigh) {
        MemWriteTristateLabel->setDisabled(true);
        color = colorScheme->arrowColorOn;
    }
    else {
        MemWriteTristateLabel->setDisabled(false);
        color = colorScheme->arrowColorOff;
    }
    painter->setPen(QPen(QBrush(color), 1));
    painter->setBrush(color);

    // memRead line from label to bus:
    painter->drawLine(TwoByteShapes::MemReadSelect);

    painter->drawImage(QPoint(TwoByteShapes::DataBus.right()+5,
                              TwoByteShapes::MemReadSelect.y1() - 3),
                       color == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
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
        color = colorScheme->backgroundFill;
    }
    painter->setPen(colorScheme->arrowColorOn);
    painter->setBrush(color);
    // Left end points
    if (MemReadTristateLabel->text() == "1" && (dataSection->getMainBusState() == Enu::MemReadReady ||
                   dataSection->getMainBusState() == Enu::MemReadSecondWait)) {
        // square end (when reading):
        poly << QPoint(TwoByteShapes::DataArrowLeftX + TwoByteShapes::arrowHOffset,
                       TwoByteShapes::DataArrowMidpointY - TwoByteShapes::DataArrowInnerYSpread) // Arrow Top Inner point
             << QPoint(TwoByteShapes::DataArrowLeftX + TwoByteShapes::arrowHOffset,
                       TwoByteShapes::DataArrowMidpointY + TwoByteShapes::DataArrowInnerYSpread); // Arrow Bottom Inner point
    }
    else {
        poly << QPoint(TwoByteShapes::DataArrowLeftX + TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY - TwoByteShapes::DataArrowInnerYSpread)  // Arrow Top Inner point
             << QPoint(TwoByteShapes::DataArrowLeftX + TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY - TwoByteShapes::DataArrowOuterYSpread)  // Arrow Top Outer point
             << QPoint(TwoByteShapes::DataArrowLeftX, TwoByteShapes::DataArrowMidpointY)   // Arrow Middle point
             << QPoint(TwoByteShapes::DataArrowLeftX + TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY + TwoByteShapes::DataArrowOuterYSpread)  // Arrow Bottom Outer point
             << QPoint(TwoByteShapes::DataArrowLeftX + TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY + TwoByteShapes::DataArrowInnerYSpread); // Arrow Bottom Inner point
    }
    // Right end points
    if (MemWriteTristateLabel->text()== "1"  && (dataSection->getMainBusState() == Enu::MemWriteReady ||
                                               dataSection->getMainBusState() == Enu::MemWriteSecondWait)) {
        // square end (when writing):
        poly << QPoint(TwoByteShapes::DataArrowRightX + 1,
                       TwoByteShapes::DataArrowMidpointY + TwoByteShapes::DataArrowInnerYSpread) // Arrow Bottom Inner point
             << QPoint(TwoByteShapes::DataArrowRightX + 1,
                       TwoByteShapes::DataArrowMidpointY-TwoByteShapes::DataArrowInnerYSpread); // Arrow Top Inner point
    }
    else {
        poly << QPoint(TwoByteShapes::DataArrowRightX - TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY + TwoByteShapes::DataArrowInnerYSpread) // Arrow Bottom Inner point
             << QPoint(TwoByteShapes::DataArrowRightX - TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY + TwoByteShapes::DataArrowOuterYSpread) // Arrow Bottom Outer point
             << QPoint(TwoByteShapes::DataArrowRightX, TwoByteShapes::DataArrowMidpointY) // Arrow Middle point
             << QPoint(TwoByteShapes::DataArrowRightX - TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY - TwoByteShapes::DataArrowOuterYSpread) // Arrow Top Outer point
             << QPoint(TwoByteShapes::DataArrowRightX - TwoByteShapes::DataArrowDepth,
                       TwoByteShapes::DataArrowMidpointY - TwoByteShapes::DataArrowInnerYSpread); // Arrow Top Inner point
    }
    painter->drawPolygon(poly);
}

void CpuGraphicsItems::repaintMemReadTwoByte(QPainter *painter)
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

void CpuGraphicsItems::repaintMemWriteTwoByte(QPainter *painter)
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

    // Figure out the color:
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
    QColor colorTop = colorScheme->backgroundFill, colorBottom = colorScheme->backgroundFill;
    if(marckIsHigh && MARMuxTristateLabel->text() == "0"){
        colorTop = colorScheme->combCircuitYellow;
        colorBottom = colorScheme->combCircuitGreen;
    }
    else if(marckIsHigh && MARMuxTristateLabel->text() == "1" && !aLineEdit->text().isEmpty() && !bLineEdit->text().isEmpty())
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
    bool MDREIsHigh = MDREMuxTristateLabel->text() == "1" || MDREMuxTristateLabel->text() == "0";
    QColor MDREColor = MDREIsHigh ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    QPalette pal = MDREMuxerDataLabel->palette();
    //Paint MDRESelect's lines and arrow in the appropriate color
    painter->setPen(MDREColor);
    painter->drawLines(TwoByteShapes::MDREMuxSelect._lines);
    painter->drawImage(TwoByteShapes::MDREMuxSelect._arrowheads.first(),
                       MDREColor == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
    if(MDRECk->isChecked()) {
        if(MDREMuxTristateLabel->text() == "0" && dataSection->getMainBusState() == Enu::MemReadSecondWait) {
            pal.setColor(QPalette::Background, colorScheme->muxCircuitRed);
        }
        else if(MDREMuxTristateLabel->text() == "1" && cMuxTristateLabel->text() == "0") {
            pal.setColor(QPalette::Background, colorScheme->muxCircuitYellow);
        }
        else if(MDREMuxTristateLabel->text() == "1" && cMuxTristateLabel->text() == "1"
                && aluHasCorrectOutput()) {
            pal.setColor(QPalette::Background, colorScheme->muxCircuitBlue);
        }
        else {
            pal.setColor(QPalette::Background, colorScheme->backgroundFill);
        }
    }
    else {
        pal.setColor(QPalette::Background,colorScheme->backgroundFill);
    }
    MDREMuxerDataLabel->setPalette(pal);
}

void CpuGraphicsItems::repaintMDROSelect(QPainter *painter)
{
    //Determine if control line is high, so that the color may be set accordingly
    bool MDROIsHigh = MDROMuxTristateLabel->text() == "1" || MDROMuxTristateLabel->text() == "0";
    QColor MDROColor = MDROIsHigh ? colorScheme->arrowColorOn : colorScheme->arrowColorOff;
    QPalette pal = MDROMuxerDataLabel->palette();
    //Paint MDROSelect's lines and arrow in the appropriate color
    painter->setPen(MDROColor);
    painter->drawLines(TwoByteShapes::MDROMuxSelect._lines);
    painter->drawImage(TwoByteShapes::MDROMuxSelect._arrowheads.first(),
                       MDROColor == colorScheme->arrowColorOff ? arrowLeftGray : arrowLeft);
    if(MDROCk->isChecked()){
        if(MDROMuxTristateLabel->text() == "0" && dataSection->getMainBusState() == Enu::MemReadSecondWait) {
            pal.setColor(QPalette::Background,colorScheme->muxCircuitRed);
        }
        else if(MDROMuxTristateLabel->text() == "1" && cMuxTristateLabel->text() == "0") {
            pal.setColor(QPalette::Background,colorScheme->muxCircuitYellow);
        }
        else if(MDROMuxTristateLabel->text() == "1" && cMuxTristateLabel->text() == "1"
                && aluHasCorrectOutput()) {
            pal.setColor(QPalette::Background,colorScheme->muxCircuitBlue);
        }
        else{
            pal.setColor(QPalette::Background, colorScheme->backgroundFill);
        }
    }
    else {
        pal.setColor(QPalette::Background, colorScheme->backgroundFill);
    }
    MDROMuxerDataLabel->setPalette(pal);
}

void CpuGraphicsItems::repaintMDRMuxOutputBuses(QPainter *painter)
{
    // MDRMuxOutBus (MDRMux to MDR arrow)
    QColor colorMDRE = colorScheme->backgroundFill, colorMDRO = colorScheme->backgroundFill;
    // Depending on which input is selected on the MDRMuxes, the color might need to change.
    // For now red seems to be the most logical color to use all the time.
    QString MDREText = MDREMuxTristateLabel->text(), MDROText = MDROMuxTristateLabel->text();
    if(MDRECk->isChecked() && (MDREText == "1" || MDREText == "0")){
         //If the muxer is enabled, and data can be clocked in to the register, pick an appropriate color
        if(MDREMuxTristateLabel->text() == "0" && dataSection->getMainBusState() == Enu::MemReadSecondWait) {
            colorMDRE = colorScheme->combCircuitRed;
        }
        else if(MDREMuxTristateLabel->text() == "1" && cMuxTristateLabel->text() == "0") {
            colorMDRE = colorScheme->combCircuitYellow;
        }
        else if(MDREMuxTristateLabel->text() == "1" && cMuxTristateLabel->text() == "1" && aluHasCorrectOutput()) {
            colorMDRE = colorScheme->combCircuitBlue;
        }
        else {
            colorMDRE = colorScheme->backgroundFill;
        }
    }
    if(MDROCk->isChecked() && (MDROText == "1" || MDROText == "0")) {
         //If the muxer is enabled, and data can be clocked in to the register, pick an appropriate color
        if(MDROMuxTristateLabel->text() == "0" && dataSection->getMainBusState() == Enu::MemReadSecondWait) {
            colorMDRO=colorScheme->combCircuitRed;
        }
        else if(MDROMuxTristateLabel->text()=="1"&&cMuxTristateLabel->text()=="0") {
            colorMDRO=colorScheme->combCircuitYellow;
        }
        else if(MDROMuxTristateLabel->text() == "1" && cMuxTristateLabel->text() == "1"
                && aluHasCorrectOutput()) {
            colorMDRO=colorScheme->combCircuitBlue;
        }
        else {
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
    if(MARMuxTristateLabel->text() == "0"|| EOMuxTristateLabel->text() == "1"
            || EOMuxTristateLabel->text() == "0") {
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

void CpuGraphicsItems::repaintABusTwoByte(QPainter *painter)
{
    bool ok;
    aLineEdit->text().toInt(&ok, 10);
    QColor color = ok ? colorScheme->combCircuitRed : colorScheme->backgroundFill;
    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);

    // ABus
    painter->drawPolygon(TwoByteShapes::ABus);
}

void CpuGraphicsItems::repaintBBusTwoByte(QPainter *painter)
{
    bool ok;
    bLineEdit->text().toInt(&ok, 10);
    QColor color = ok ? colorScheme->combCircuitRed : colorScheme->backgroundFill;
    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);

    // BBus
    painter->drawPolygon(TwoByteShapes::BBus);
}

void CpuGraphicsItems::repaintCBusTwoByte(QPainter *painter)
{
    QColor color;
    QPalette pal = cMuxerLabel->palette();
    if (cMuxTristateLabel->text() == "0") {
        color = colorScheme->combCircuitYellow;
        pal.setColor(QPalette::Background,colorScheme->muxCircuitYellow);
    }
    else if (cMuxTristateLabel->text() == "1") {
        if (!aluHasCorrectOutput() || ALULineEdit->text() == "15") {
            // CBus.state == UNDEFINED or NZVC A
            qDebug() << "WARNING!: CMux select: There is no ALU output";
            color = colorScheme->backgroundFill;
            pal.setColor(QPalette::Background,color);
        }
        else {
            color = colorScheme->combCircuitBlue;
            pal.setColor(QPalette::Background,colorScheme->muxCircuitBlue);
        }
    }
    else {
        color = colorScheme->backgroundFill;
        pal.setColor(QPalette::Background,color);
    }
    cMuxerLabel->setPalette(pal);
    painter->setPen(QPen(QBrush(colorScheme->arrowColorOn), 1));
    painter->setBrush(color);
    painter->drawPolygon(TwoByteShapes::CBus);
}

// ***************************************************************************
// Public Slosts
// ***************************************************************************

void CpuGraphicsItems::darkModeChanged(bool darkMode, QString styleSheet)
{
    this->darkMode = darkMode;
    if(darkMode) {
        colorScheme = &(PepColors::darkMode);
    }
    else {
        colorScheme = &(PepColors::lightMode);
    }
    for(auto checkbox : this->checkVector) {
        checkbox->setStyleSheet(styleSheet);
        checkbox->setAutoFillBackground(false);
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

void CpuGraphicsItems::CPUTypeChanged(Enu::CPUType newType)
{
    //if(type == newType) return;
    if(newType == Enu::CPUType::TwoByteDataBus) {
        // Hide one byte items
        MDRCk->setVisible(false);
        MDRMuxLabel->setVisible(false);
        MDRMuxerDataLabel->setVisible(false);
        MDRMuxTristateLabel->setVisible(false);
        MDRLabel->setVisible(false);

        // Make two byte things visible
        MARMuxLabel->setVisible(true);
        MARMuxerDataLabel->setVisible(true);
        MARMuxTristateLabel->setVisible(true);
        MDROCk->setVisible(true); MDRECk->setVisible(true);
        MDROMuxLabel->setVisible(true); MDREMuxLabel->setVisible(true);
        MDROMuxerDataLabel->setVisible(true);MDREMuxerDataLabel->setVisible(true);
        MDROMuxTristateLabel->setVisible(true); MDREMuxTristateLabel->setVisible(true);
        MDROLabel->setVisible(true); MDRELabel->setVisible(true);
        EOMuxLabel->setVisible(true);
        EOMuxerDataLabel->setVisible(true);
        EOMuxTristateLabel->setVisible(true);

        // Move things from one byte position to two byte position

        ALUPoly->setPolygon(OneByteShapes::ALUPoly.translated(TwoByteShapes::controlOffsetX, TwoByteShapes::aluOffsetY));
        MARALabel->setGeometry(TwoByteShapes::MARALabel);
        MARBLabel->setGeometry(TwoByteShapes::MARBLabel);
        loadCk->setGeometry(TwoByteShapes::loadCkCheckbox);
        cLineEdit->setGeometry(TwoByteShapes::cLineEdit);
        cLabel->setGeometry(TwoByteShapes::cLabel);
        bLineEdit->setGeometry(TwoByteShapes::bLineEdit);
        bLabel->setGeometry(TwoByteShapes::bLabel);
        aLineEdit->setGeometry(TwoByteShapes::aLineEdit);
        aLabel->setGeometry(TwoByteShapes::aLabel);
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
        MemWriteLabel->setGeometry(TwoByteShapes::MemWriteLabel);
        MemWriteTristateLabel->setGeometry(TwoByteShapes::MemWriteTristateLabel);
        MemReadLabel->setGeometry(TwoByteShapes::MemReadLabel);
        MemReadTristateLabel->setGeometry(TwoByteShapes::MemReadTristateLabel);
    }
    else if(newType == Enu::CPUType::OneByteDataBus) {
        // Hide two byte items
        MARMuxLabel->setVisible(false);
        MARMuxerDataLabel->setVisible(false);
        MARMuxTristateLabel->setVisible(false);
        MDROCk->setVisible(false); MDRECk->setVisible(false);
        MDROMuxLabel->setVisible(false); MDREMuxLabel->setVisible(false);
        MDROMuxerDataLabel->setVisible(false);MDREMuxerDataLabel->setVisible(false);
        MDROMuxTristateLabel->setVisible(false); MDREMuxTristateLabel->setVisible(false);
        MDROLabel->setVisible(false); MDRELabel->setVisible(false);
        EOMuxLabel->setVisible(false);
        EOMuxerDataLabel->setVisible(false);
        EOMuxTristateLabel->setVisible(false);

        // Make one byte things visible
        MDRCk->setVisible(true);
        MDRMuxLabel->setVisible(true);
        MDRMuxerDataLabel->setVisible(true);
        MDRMuxTristateLabel->setVisible(true);
        MDRLabel->setVisible(true);

        // Move things from two byte position to one byte position
        ALUPoly->setPolygon(OneByteShapes::ALUPoly);
        MARALabel->setGeometry(OneByteShapes::MARALabel);
        MARBLabel->setGeometry(OneByteShapes::MARBLabel);
        loadCk->setGeometry(OneByteShapes::loadCkCheckbox);
        cLineEdit->setGeometry(OneByteShapes::cLineEdit);
        cLabel->setGeometry(OneByteShapes::cLabel);
        bLineEdit->setGeometry(OneByteShapes::bLineEdit);
        bLabel->setGeometry(OneByteShapes::bLabel);
        aLineEdit->setGeometry(OneByteShapes::aLineEdit);
        aLabel->setGeometry(OneByteShapes::aLabel);
        MARCk->setGeometry(OneByteShapes::MARCkCheckbox);
        aMuxLabel->setGeometry(OneByteShapes::aMuxLabel);
        aMuxerDataLabel->setGeometry(OneByteShapes::aMuxerDataLabel);
        aMuxTristateLabel->setGeometry(OneByteShapes::aMuxTristateLabel);
        cMuxLabel->setGeometry(OneByteShapes::cMuxLabel);
        cMuxerLabel->setGeometry(OneByteShapes::cMuxerLabel);
        cMuxTristateLabel->setGeometry(OneByteShapes::cMuxTristateLabel);
        ALULineEdit->setGeometry(OneByteShapes::ALULineEdit);
        ALULabel->setGeometry(OneByteShapes::ALULabel);
        ALUFunctionLabel->setGeometry(OneByteShapes::ALUFunctionLabel);
        CSMuxLabel->setGeometry(OneByteShapes::CSMuxLabel);
        CSMuxerDataLabel->setGeometry(OneByteShapes::CSMuxerDataLabel);
        CSMuxTristateLabel->setGeometry(OneByteShapes::CSMuxTristateLabel);
        SCkCheckBox->setGeometry(OneByteShapes::SCkCheckBox);
        sBitLabel->setGeometry(OneByteShapes::sBitLabel);
        CCkCheckBox->setGeometry(OneByteShapes::CCkCheckBox);
        cBitLabel->setGeometry(OneByteShapes::cBitLabel);
        VCkCheckBox->setGeometry(OneByteShapes::VCkCheckBox);
        vBitLabel->setGeometry(OneByteShapes::vBitLabel);
        AndZLabel->setGeometry(OneByteShapes::AndZLabel);
        AndZTristateLabel->setGeometry(OneByteShapes::AndZTristateLabel);
        AndZMuxLabel->setGeometry(OneByteShapes::AndZMuxLabel);
        ZCkCheckBox->setGeometry(OneByteShapes::ZCkCheckBox);
        zBitLabel->setGeometry(OneByteShapes::zBitLabel);
        NCkCheckBox->setGeometry(OneByteShapes::NCkCheckBox);
        nBitLabel->setGeometry(OneByteShapes::nBitLabel);
        MemWriteLabel->setGeometry(OneByteShapes::MemWriteLabel);
        MemWriteTristateLabel->setGeometry(OneByteShapes::MemWriteTristateLabel);
        MemReadLabel->setGeometry(OneByteShapes::MemReadLabel);
        MemReadTristateLabel->setGeometry(OneByteShapes::MemReadTristateLabel);
    }
    type = newType;
}



