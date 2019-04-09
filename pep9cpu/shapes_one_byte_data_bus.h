// File: shapes_one_byte_data_bus.h
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
#ifndef SHAPES_ONE_BYTE_DATA_BUS_H
#define SHAPES_ONE_BYTE_DATA_BUS_H

#include <QPair>
#include <QPoint>
#include <QLine>
#include <QVector>
#include <QPolygon>
#include <QImage>

#include <QVector>

#include "pep.h"


class Arrow {
public:
    Arrow (QVector<QPoint> arrowheads, QVector<QLine> lines) {
        _arrowheads = arrowheads;
        _lines = lines;
    }
    QVector<QPoint> _arrowheads;
    QVector<QLine> _lines;

    Arrow translated(int dx, int dy) const {
        QVector<QPoint> arrowHeads;
        QVector<QLine> lines;

        for (int i = 0; i < _arrowheads.length(); i++) {
            arrowHeads.append(_arrowheads.at(i) + QPoint(dx, dy));
        }
        for (int i = 0; i < _lines.length(); i++) {
            lines.append(_lines.at(i).translated(dx, dy));
        }

        Arrow retVal(arrowHeads, lines);
        return retVal;

    }
};
namespace OneByteShapes
{

// generic shapes:
enum Shapes {
    checkW    = 60,
    checkH    = 20,
    check2W   = 80,
    check2H   = checkH,
    labelW    = 42,
    labelH    = 20,
    dataLabelW = 69,
    dataLabelH = 19,
    labelTriW = 25,
    labelTriH = labelH,
    lineEditW = 25,
    lineEditH = 21,
    regLineEditW = 60,
    regLineEditH = 19,

    arrowHDepth = 20, // really 15, but 20 with arrowHOffset
    arrowHOffset = 5,
    arrowLeftOff = 12,
    selectYOffset = 9,
    selectSlashOffset = 5,
    incrementerOffset = 10,

    aluSelOff = 57,
    selLineOff = 15,

};

enum RegPos {
    rowHeight = 28, // Original was 28
    columnWidth = 123 + 10, // Original was 123
    Row1Y     = 10,
    Row2Y     = Row1Y+rowHeight,
    Row3Y     = Row2Y+rowHeight,
    Row4Y     = Row3Y+rowHeight,
    Col1X     = 56,
    Col2X     = Col1X+columnWidth,
    Col3X     = Col2X+columnWidth+9, // Needs additional room, due to the triple width register
    Col4X     = Col3X+columnWidth,
};

enum CommonPositions {
    ctrlInputX = 570,
    ctrlLabelX = ctrlInputX + 29,
    interfaceRegsX = 175,
    statusBitsX = 476,
};

const QRect RegBank = QRect(5, 5, Col4X+ regLineEditW - 10, 113);
//    const QPolygon checkbox  = QPolygon(QRect(0,0, 60, 20));
//    const QPolygon checkbox2 = QPolygon(QRect(0,0, 80, 20));
//    const QPolygon label     = QPolygon(QRect(0,0, 42, 20));
//    const QPolygon dataLabel = QPolygon(QRect(0,0, 69, 19));
//    const QPolygon triLabel  = QPolygon(QRect(0,0, 25, 20));
//    const QPolygon lineEdit  = QPolygon(QRect(0,0, 25, 21));

// input/label/control section:
const QRect loadCkCheckbox          = QRect(ctrlInputX, 18, check2W,   check2H);
const QRect cLineEdit               = QRect(ctrlInputX, 39, lineEditW, lineEditH);
const QRect cLabel                  = QRect(ctrlLabelX, 41, labelW,    labelH);
const QRect bLineEdit               = QRect(ctrlInputX, 61, lineEditW, lineEditH);
const QRect bLabel                  = QRect(ctrlLabelX, 63, labelW,    labelH);
const QRect aLineEdit               = QRect(ctrlInputX, 83, lineEditW, lineEditH);
const QRect aLabel                  = QRect(ctrlLabelX, 85, labelW,    labelH);

const QRect MARCkCheckbox           = QRect(ctrlInputX, 169, check2W, check2H);
const QRect MARBLabel               = QRect(interfaceRegsX, 132, dataLabelW, dataLabelH);
const QRect MARALabel               = QRect(interfaceRegsX, 202, dataLabelW, dataLabelH);
const QRect MDRCkCheckbox           = QRect(ctrlInputX, 225, check2W, check2H);

const QRect MARIncrementer          = QRect(MARBLabel.right() + incrementerOffset,
                                            MARBLabel.top(),
                                            MARALabel.bottom() - MARBLabel.top(),
                                            MARALabel.bottom() - MARBLabel.top());


const QRect MDRMuxTristateLabel     = QRect(ctrlInputX, 316, labelTriW, labelTriH);
const QRect MDRMuxLabel             = QRect(ctrlLabelX, MDRMuxTristateLabel.y(), dataLabelW, labelH);
const QRect MDRMuxerDataLabel       = QRect(interfaceRegsX,293, dataLabelW, dataLabelH);
const QRect MDRLabel                = QRect(interfaceRegsX, 254, dataLabelW, dataLabelH);

const QRect aMuxTristateLabel       = QRect(ctrlInputX, 295, labelTriW, 21);
const QRect aMuxLabel               = QRect(ctrlLabelX, aMuxTristateLabel.y(), labelW, labelH);
const QRect aMuxerDataLabel         = QRect(306, 293, dataLabelW, dataLabelH);
const QRect cMuxTristateLabel       = QRect(ctrlInputX, 348, labelTriW, labelTriH);
const QRect cMuxLabel               = QRect(ctrlLabelX, cMuxTristateLabel.y(), labelW, labelH);
const QRect cMuxerLabel             = QRect(250, 374, dataLabelW, dataLabelH);
const QRect ALULineEdit             = QRect(ctrlInputX,  368, 26,     lineEditH);
const QRect ALULabel                = QRect(ctrlLabelX,  370, 31,     labelH);
const QRect ALUFunctionLabel        = QRect(332, 355, 98, 20);

const QRect CSMuxLabel              = QRect(ctrlLabelX,  399, labelW, labelH);
const QRect CSMuxerDataLabel        = QRect(statusBitsX+19-69, 399, dataLabelW, dataLabelH);
const QRect CSMuxTristateLabel      = QRect(ctrlInputX,  399, 25,     21);
const QRect SCkCheckBox             = QRect(ctrlInputX,  437, checkW, checkH);
const QRect sBitLabel               = QRect(statusBitsX, 437, 19,     dataLabelH);
const QRect CCkCheckBox             = QRect(ctrlInputX,  464, checkW, checkH);
const QRect cBitLabel               = QRect(statusBitsX, 463, 19,     dataLabelH);
const QRect VCkCheckBox             = QRect(ctrlInputX,  491, checkW, checkH);
const QRect vBitLabel               = QRect(statusBitsX, 491, 19,     dataLabelH);
const QRect AndZLabel               = QRect(ctrlLabelX,  517, 45,     20);
const QRect AndZTristateLabel       = QRect(ctrlInputX, 517, labelTriW,labelTriH);
const QRect AndZMuxLabel            = QRect(416, 544, 41,21);
const QRect ZCkCheckBox             = QRect(ctrlInputX, 544, 60, 20);
const QRect zBitLabel               = QRect(statusBitsX, 544, 19, dataLabelH);
const QRect NCkCheckBox             = QRect(ctrlInputX, 586, checkW, checkH);
const QRect nBitLabel               = QRect(statusBitsX, 586, 19, dataLabelH);

const QRect MemWriteLabel           = QRect(ctrlLabelX, 611, check2W, check2H);
const QRect MemWriteTristateLabel   = QRect(ctrlInputX, 611, labelTriW, labelTriH);
const QRect MemReadLabel            = QRect(ctrlLabelX, 631, check2W, check2H);
const QRect MemReadTristateLabel    = QRect(ctrlInputX, 631, labelTriW, labelTriH);

// registers

// lines and shapes:
const QRect AddrBus = QRect(40, 151, 20, 500);
const QRect DataBus = QRect(AddrBus.x()+AddrBus.width(), AddrBus.top() + 100, 10, 400);

const Arrow loadCkSelect = Arrow(QVector<QPoint>() << QPoint(RegBank.right() + arrowHOffset, 24),
                                 QVector<QLine>() << QLine(ctrlInputX - 7, loadCkCheckbox.y()+9,
                                                           RegBank.right() + arrowHOffset, loadCkCheckbox.y()+9));
const Arrow CSelect = Arrow(QVector<QPoint>() << QPoint(RegBank.right() + arrowHOffset, 47),
                            QVector<QLine>()  <<  QLine(ctrlInputX - 7, cLabel.y() + selectYOffset,
                                                        RegBank.right() + arrowHOffset, cLabel.y() + selectYOffset)
                            <<  QLine(RegBank.right() + arrowHDepth + 6, cLabel.y() + selectYOffset - 5,
                                      RegBank.right() + arrowHDepth + 16, cLabel.y() + selectYOffset + 5));
const Arrow BSelect = Arrow(QVector<QPoint>() << QPoint(RegBank.right() + arrowHOffset, 69),
                            QVector<QLine>()  <<  QLine(ctrlInputX - 7, bLabel.y() + selectYOffset,
                                                        RegBank.right() + arrowHOffset, bLabel.y() + selectYOffset)
                            <<  QLine(RegBank.right() + arrowHDepth + 6, bLabel.y() + selectYOffset - selectSlashOffset,
                                      RegBank.right() + arrowHDepth + 16, bLabel.y() + selectYOffset + selectSlashOffset));
const Arrow ASelect = Arrow(QVector<QPoint>() << QPoint(RegBank.right() + arrowHOffset, 91),
                            QVector<QLine>()  <<  QLine(ctrlInputX - 7, aLabel.y() + selectYOffset,
                                                        RegBank.right() + arrowHOffset, aLabel.y() + selectYOffset)
                            <<  QLine(RegBank.right() + arrowHDepth + 6, aLabel.y() + selectYOffset - selectSlashOffset,
                                      RegBank.right() + arrowHDepth + 16, aLabel.y() + selectYOffset + selectSlashOffset));
const Arrow MARCk = Arrow(QVector<QPoint>() << QPoint(232,155)
                          << QPoint(232,191),
                          QVector<QLine > () << QLine(428,177, ctrlInputX - 7,177)
                          << QLine(367,177, 416,177)
                          << QLine(291,177, 355,177)
                          << QLine(235,177, 279,177)
                          << QLine(235,163, 235,191));
const Arrow MDRCk = Arrow(QVector<QPoint>() << QPoint(207,241),
                          QVector<QLine>() << QLine(210,233, 279,233)
                          << QLine(291,233, 355,233)
                          << QLine(367,233, 416,233)
                          << QLine(428,233, ctrlInputX - 7, 233)
                          << QLine(210,233, 210,241));
const Arrow AMuxSelect = Arrow(QVector<QPoint>() << QPoint(380,300),
                               QVector<QLine>()  << QLine(388, 303, 416, 303)
                               << QLine(428, 303, ctrlInputX - 7, 303));
const QPolygon AMuxBus = QPolygon(QVector<QPoint>() << QPoint(336,312)
                                  << QPoint(336,331)
                                  << QPoint(331,331)
                                  << QPoint(341,341)
                                  << QPoint(351,331)
                                  << QPoint(346,331)
                                  << QPoint(346,312));
const Arrow CMuxSelect = Arrow(QVector<QPoint>() << QPoint(cMuxerLabel.left()+7,cMuxerLabel.top()-12),
                               QVector<QLine>() << QLine(449,355, ctrlInputX - 7,355)
                               << QLine(315,355, 290,355)
                               << QLine(280,355, 260,355)
                               << QLine(260,355, 260,365));
const QPolygon CMuxBus = QPolygon(QVector<QPoint>() << QPoint(290,374)
                                  << QPoint(290,130) << QPoint(295,130)
                                  << QPoint(285,120) << QPoint(275,130)
                                  << QPoint(280,130) << QPoint(280,334)
                                  << QPoint(240,334) << QPoint(240,326)
                                  << QPoint(245,326) << QPoint(235,316)
                                  << QPoint(225,326) << QPoint(230,326)
                                  << QPoint(230,344) << QPoint(280,344)
                                  << QPoint(280,374));

//Use an enumeration for points that other shapes (like AMux) rely on, so that it is easier to re-arrange the diagram.
enum ALUPolyNumbers
{
    ALUUpperLeftLine_LeftPoint = 314,
    ALUUpperLeftLine_RightPoint = 366,
    ALUUpperRightLine_LeftPoint = 394,
    ALUUpperRightLine_RightPoint = 447,
    ALUTopBound = 342,
    ALUBottomBound = 394,
};
const QPolygon ALUPoly = QPolygon(QVector<QPoint>() << QPoint(ALUUpperLeftLine_LeftPoint,ALUTopBound)
                                  << QPoint(ALUUpperLeftLine_RightPoint,ALUTopBound) << QPoint(370,353)
                                  << QPoint(390,353) << QPoint(ALUUpperRightLine_LeftPoint,ALUTopBound)
                                  << QPoint(ALUUpperRightLine_RightPoint,ALUTopBound) << QPoint(421,ALUBottomBound)
                                  << QPoint(340,ALUBottomBound));

// the two shapes that make up the arrow out to the right of the MDR
const QRect MDRBusOutRect = QRect(244, 258, 36, 10);
const QPolygon MDRBusOutArrow = QPolygon(QVector<QPoint>() << QPoint(290,258)
                                         << QPoint(326,258) << QPoint(326,280)
                                         << QPoint(331,280) << QPoint(321,290)
                                         << QPoint(311,280) << QPoint(316,280)
                                         << QPoint(316,268) << QPoint(290,268));

const QPolygon MARBus = QPolygon(QVector<QPoint>()
                                 << QPoint(205,151)
                                 << QPoint(205,167)
                                 // arrow:
                                 << QPoint(AddrBus.x()+AddrBus.width()+arrowHDepth,167)
                                 << QPoint(AddrBus.x()+AddrBus.width()+arrowHDepth,162)
                                 << QPoint(AddrBus.x()+AddrBus.width()+arrowHOffset,177)
                                 << QPoint(AddrBus.x()+AddrBus.width()+arrowHDepth,192)
                                 << QPoint(AddrBus.x()+AddrBus.width()+arrowHDepth,187)
                                 << QPoint(205,187)
                                 << QPoint(205,202)
                                 << QPoint(215,202)
                                 // black line in the middle:
                                 << QPoint(215,151)
                                 << QPoint(215,177)
                                 << QPoint(OneByteShapes::AddrBus.right()+arrowHDepth,177)
                                 << QPoint(215,177)
                                 << QPoint(215,151));
const QPolygon NZVCDataPath = QPolygon(QVector<QPoint>() << QPoint(310,513)
                                       << QPoint(269,513) << QPoint(269,407)
                                       << QPoint(274,407) << QPoint(264,397)
                                       << QPoint(254,407) << QPoint(259,407)
                                       << QPoint(259,523) << QPoint(310,523));



const int regNumberOffset = Pep::getSystem() == "Mac" ? 65 : 65; // 54 in some spots?
const int regLabelOffset  = 24;
const QSize regLabelSize  = QSize(20,22);
const QSize regNumberSize = Pep::getSystem() == "Mac" ? QSize(42, 22) : QSize(42, 22);
const QRect aRegLineEdit  = QRect(Col1X, Row1Y, regLineEditW, regLineEditH);
const QRect xRegLineEdit  = QRect(Col1X, Row2Y, regLineEditW, regLineEditH);
const QRect spRegLineEdit = QRect(Col1X, Row3Y, regLineEditW, regLineEditH);
const QRect pcRegLineEdit = QRect(Col1X, Row4Y, regLineEditW, regLineEditH);

const QRect irRegLineEdit = QRect(Col2X, Row1Y, regLineEditW + 15, regLineEditH);
const QRect t1RegLineEdit = QRect(Col2X, Row2Y, regLineEditW - 15, regLineEditH);
const QRect t2RegLineEdit = QRect(Col2X, Row3Y, regLineEditW, regLineEditH);
const QRect t3RegLineEdit = QRect(Col2X, Row4Y, regLineEditW, regLineEditH);

const QRect t4RegLineEdit = QRect(Col3X, Row1Y, regLineEditW, regLineEditH);
const QRect t5RegLineEdit = QRect(Col3X, Row2Y, regLineEditW, regLineEditH);
const QRect t6RegLineEdit = QRect(Col3X, Row3Y, regLineEditW, regLineEditH);
const QRect m1RegLabel    = QRect(Col3X, Row4Y, regLineEditW - 10, regLineEditH);

const QRect m2RegLabel    = QRect(Col4X, Row1Y, regLineEditW - 10, regLineEditH);
const QRect m3RegLabel    = QRect(Col4X, Row2Y, regLineEditW - 10, regLineEditH);
const QRect m4RegLabel    = QRect(Col4X, Row3Y, regLineEditW - 10, regLineEditH);
const QRect m5RegLabel    = QRect(Col4X, Row4Y, regLineEditW - 10, regLineEditH);

const QPolygon BBus1 = QPolygon(QVector<QPoint>()   << QPoint(417, 118)
                                << QPoint(417, 136) << QPoint(366, 136)
                                << QPoint(366, 146) << QPoint(417, 146)
                                << QPoint(417, 331) << QPoint(412, 331)
                                << QPoint(422, 341) << QPoint(432, 331)
                                << QPoint(427, 331) << QPoint(427, 118));
const QRect BBusRect = QRect(290, 136, 67, 11);
const QPolygon BBus2 = QPolygon(QVector<QPoint>()   << QPoint(280, 136)
                                << QPoint(258, 136) << QPoint(258, 131)
                                << QPoint(248, 141) << QPoint(258, 151)
                                << QPoint(258, 146) << QPoint(280, 146)
                                << QPoint(280, 136));

const QPolygon ABus1 = QPolygon(QVector<QPoint>()  << QPoint(356,118)
                                << QPoint(356,207) << QPoint(290,207)
                                << QPoint(290,217) << QPoint(356,217)
                                << QPoint(356,280) << QPoint(351,280)
                                << QPoint(361,290) << QPoint(371,280)
                                << QPoint(366,280) << QPoint(366,118));
const QPolygon ABus2 = QPolygon(QVector<QPoint>()   << QPoint(280, 207)
                                << QPoint(258, 207) << QPoint(258, 202)
                                << QPoint(248, 212) << QPoint(258, 222)
                                << QPoint(258, 217) << QPoint(280, 217)
                                << QPoint(280, 207));

//    const QPolygon AMuxBus = QPolygon(QVector<QPoint>()  << QPoint(336, 312)
//                                      << QPoint(336, 331) << QPoint(331, 331)
//                                      << QPoint(341, 341) << QPoint(351, 331)
//                                      << QPoint(346, 331) << QPoint(346, 312));

const QPolygon CBus = QPolygon(QVector<QPoint>()  << QPoint(290,374)
                               << QPoint(290,130) << QPoint(295,130)
                               << QPoint(285,120) << QPoint(275,130)
                               << QPoint(280,130) << QPoint(280,334)
                               << QPoint(240,334) << QPoint(240,326)
                               << QPoint(245,326) << QPoint(235,316)
                               << QPoint(225,326) << QPoint(230,326)
                               << QPoint(230,344) << QPoint(280,344)
                               << QPoint(280,374));


const QPolygon AddrArrow = QPolygon(QVector<QPoint>()
                                    // arrowhead
                                    << QPoint(18, 330) << QPoint(18, 325)
                                    << QPoint(3,  340) << QPoint(18, 355)
                                    << QPoint(18, 350)
                                    // blunt end at the bus:
                                    << QPoint(40, 350) << QPoint(40, 330));

const QPolygon DataToMDRMuxBus = QPolygon(QVector<QPoint>()
                                          // foot:
                                          << QPoint(190, 344)
                                          << QPoint(70,  344) << QPoint(70,  334)
                                          << QPoint(180, 334)
                                          // arrowhead:
                                          << QPoint(180, 326) << QPoint(175, 326)
                                          << QPoint(185, 316) << QPoint(195, 326)
                                          << QPoint(190, 326));

const QPolygon MDRToDataBus = QPolygon(QVector<QPoint>()  << QPoint(175, 258)
                                       << QPoint(83, 258) << QPoint(83, 253)
                                       << QPoint(73, 263) << QPoint(83, 273)
                                       << QPoint(83, 268) << QPoint(175, 268));

const QPolygon ALUOutBus = QPolygon(QVector<QPoint>() << QPoint(346, 395)
                                    << QPoint(346, 414) << QPoint(314, 414)
                                    << QPoint(314, 407) << QPoint(319, 407)
                                    << QPoint(309, 397) << QPoint(299, 407)
                                    << QPoint(304, 407) << QPoint(304, 424)
                                    << QPoint(356, 424) << QPoint(356, 395));

const QPolygon MDRMuxOutBus = QPolygon(QVector<QPoint>()
                                       << QPoint(205, MDRMuxerDataLabel.y()) // 293
                                       << QPoint(205, 286) << QPoint(200, 286)
                                       << QPoint(210, 276) << QPoint(220,286)
                                       << QPoint(215, 286)
                                       << QPoint(215,MDRMuxerDataLabel.y()));

const Arrow ALUSelect      = Arrow(QVector<QPoint>() <<
                                   QPoint(ALUPoly.boundingRect().right() - 13,
                                          ALUPoly.boundingRect().bottom() - 21),
                                   QVector<QLine>() << QLine(439,376,
                                                             ctrlInputX - 7,
                                                             ALULineEdit.y() + selectYOffset - 1)
                                   << QLine(ctrlInputX - 17,
                                            ALULineEdit.y() + 13,
                                            ctrlInputX - 27,
                                            ALULineEdit.y() + 3)); // diagonal line

const Arrow ALUSelectOut = Arrow(QVector<QPoint>() <<
                                 QPoint(nBitLabel.left() - arrowLeftOff,
                                        nBitLabel.y() + arrowHOffset + 1) << // N
                                 QPoint(AndZMuxLabel.left() - arrowLeftOff,
                                        zBitLabel.y() + arrowHOffset + 1) << // Z
                                 QPoint(vBitLabel.left() - arrowLeftOff,
                                        vBitLabel.y() + arrowHOffset + 1) << // V
                                 QPoint(cBitLabel.left() - arrowLeftOff,
                                        cBitLabel.y() + arrowHOffset + 1) << // C
                                 QPoint(sBitLabel.left() - arrowLeftOff,
                                        sBitLabel.y() + arrowHOffset + 1),   // S
                                 QVector<QLine>() <<
                                 // N
                                 QLine(ALUPoly.boundingRect().left() + aluSelOff,
                                       ALUPoly.boundingRect().bottom(),
                                       ALUPoly.boundingRect().left() + aluSelOff,
                                       nBitLabel.y() + selectYOffset) << //586+8
                                 QLine(ALUPoly.boundingRect().left() + aluSelOff,
                                       nBitLabel.y() + selectYOffset,
                                       nBitLabel.left() - arrowLeftOff,
                                       nBitLabel.y() + selectYOffset) << //586+8
                                 // Z
                                 QLine(ALUPoly.boundingRect().left() + aluSelOff + selLineOff,
                                       ALUPoly.boundingRect().bottom(),
                                       ALUPoly.boundingRect().left() + aluSelOff + selLineOff,
                                       zBitLabel.y() + selectYOffset) <<
                                 QLine(ALUPoly.boundingRect().left() + aluSelOff + selLineOff,
                                       zBitLabel.y() + selectYOffset,
                                       AndZMuxLabel.left() - arrowLeftOff,
                                       zBitLabel.y() + selectYOffset) <<

                                 // V
                                 QLine(ALUPoly.boundingRect().left() + aluSelOff + selLineOff * 2,
                                       ALUPoly.boundingRect().bottom(),
                                       ALUPoly.boundingRect().left() + aluSelOff + selLineOff * 2,
                                       vBitLabel.y() + selectYOffset) <<
                                 QLine(ALUPoly.boundingRect().left() + aluSelOff + selLineOff * 2,
                                       vBitLabel.y() + selectYOffset,
                                       vBitLabel.left() - arrowLeftOff,
                                       vBitLabel.y() + selectYOffset) <<

                                 // C
                                 QLine(ALUPoly.boundingRect().left() + aluSelOff + selLineOff * 3,
                                       ALUPoly.boundingRect().bottom(),
                                       ALUPoly.boundingRect().left() + aluSelOff + selLineOff * 3,
                                       cBitLabel.y() + selectYOffset) <<
                                 QLine(ALUPoly.boundingRect().left() + aluSelOff + selLineOff * 3,
                                       cBitLabel.y() + selectYOffset,
                                       cBitLabel.left() - arrowLeftOff,
                                       cBitLabel.y() + selectYOffset) <<
                                 // S
                                 QLine(ALUPoly.boundingRect().left() + aluSelOff + selLineOff * 3,
                                       sBitLabel.y() + selectYOffset,
                                       sBitLabel.left() - arrowLeftOff,
                                       sBitLabel.y() + selectYOffset)
                                 );

const QLine CSMuxSelect    = QLine(CSMuxerDataLabel.right() + arrowHOffset,
                                   CSMuxLabel.y() + selectYOffset +1,
                                   ctrlInputX - 7,
                                   CSMuxLabel.y() + selectYOffset + 1);
const QLine SBitSelect     = QLine(sBitLabel.right() + arrowHOffset,
                                   sBitLabel.y() + selectYOffset,
                                   ctrlInputX - 7,
                                   sBitLabel.y() + selectYOffset);
const QLine CBitSelect     = QLine(cBitLabel.right() + arrowHOffset,
                                   cBitLabel.y() + selectYOffset,
                                   ctrlInputX - 7,
                                   cBitLabel.y() + selectYOffset);
const QLine VBitSelect     = QLine(vBitLabel.right() + arrowHOffset,
                                   vBitLabel.y() + selectYOffset,
                                   ctrlInputX - 7,
                                   vBitLabel.y() + selectYOffset);
const QLine ZBitSelect     = QLine(zBitLabel.right() + arrowHOffset,
                                   zBitLabel.y() + selectYOffset,
                                   ctrlInputX - 7,
                                   zBitLabel.y() + selectYOffset);
const QLine NBitSelect     = QLine(nBitLabel.right() + arrowHOffset,
                                   nBitLabel.y() + selectYOffset,
                                   ctrlInputX - 7,
                                   nBitLabel.y() + selectYOffset);

const QLine NZVCDataLine   = QLine(310, 477, 310, 559);
const Arrow CBitToNZVC        = Arrow(QVector<QPoint>() <<
                                   QPoint(314,483),
                                   QVector<QLine>()
                                   << QLine(487,482, 487,486)
                                   << QLine(330,486, 322,486)
                                   << QLine(330,486, 487,486));
const Arrow CBitToCSMux   = Arrow(QVector<QPoint>() << QPoint(431, 421),
                                  QVector<QLine>()
                                  << QLine(487,459, 487,463)
                                  << QLine(434,459, 487,459)
                                  << QLine(434,426, 434,459));
const Arrow CInToALU      = Arrow(QVector<QPoint>() << QPoint(428, 386),
                                  QVector<QLine>()
                                  << QLine(461,389, 433,389)
                                  << QLine(461,399, 461,389));
const Arrow SBitToCSMux   = Arrow(QVector<QPoint>() << QPoint(484,421),
                                  QVector<QLine>()
                                  << QLine(487,437, 487,429));
const Arrow ZBitOut       = Arrow(QVector<QPoint>()
                                  << QPoint(314,503)
                                  << QPoint(434,566),
                                  QVector<QLine>()
                                  << QLine(487,563, 487,582) // vertical line to Z bit
                                  << QLine(487,582, 341,582) // long horizontal line
                                  << QLine(341,582, 341,506) // vertical line closest to arrowhead
                                  << QLine(341,506, 322,506) // line from arrowhead on left
                                  << QLine(437,582, 437,574));
const Arrow VBitOut       = Arrow(QVector<QPoint>()
                                  << QPoint(314,493),
                                  QVector<QLine>()
                                  << QLine(487,510, 487,514) // bitty bit
                                  << QLine(487,514, 352,514)
                                  << QLine(352,513, 352,496)
                                  << QLine(352,496, 322,496));  // short line from the arrow
const Arrow NBitOut       = Arrow(QVector<QPoint>()
                                  << QPoint(314,514),
                                  QVector<QLine>()
                                  << QLine(487,605, 487,609)
                                  << QLine(487,609, 330,609)
                                  << QLine(330,609, 330,517)
                                  << QLine(330,517, 322,517));

const Arrow AndZOut       = Arrow(QVector<QPoint>()
                                  << QPoint(AndZMuxLabel.x() + AndZMuxLabel.width()/2 - arrowHOffset/2, AndZMuxLabel.top() - selectYOffset - 3 )
                                  << QPoint(zBitLabel.x() - 12,
                                            AndZMuxLabel.y() + AndZMuxLabel.height()/2 - 3),
                                  QVector<QLine>()
                                  // Connects arrow head to horizontal line
                                  << QLine(AndZMuxLabel.x() + AndZMuxLabel.width()/2, AndZTristateLabel.y() + AndZTristateLabel.height()/2,
                                         AndZMuxLabel.x() + AndZMuxLabel.width()/2, AndZMuxLabel.y() - arrowHOffset)
                                  // Horizontal line from label to arrowhead.
                                  << QLine(AndZMuxLabel.x() + AndZMuxLabel.width()/2, AndZTristateLabel.y() + AndZTristateLabel.height()/2,
                                           ctrlInputX - 7, AndZTristateLabel.y() + AndZTristateLabel.height()/2)
                                  // Line from ANDZ circuit to Z bit.
                                  << QLine(AndZMuxLabel.right(), AndZMuxLabel.y() + AndZMuxLabel.height()/2,
                                           zBitLabel.left() - arrowHOffset, AndZMuxLabel.y() + AndZMuxLabel.height()/2));

const QLine MemReadSelect  = QLine(DataBus.right()   + arrowHOffset,
                                   MemReadLabel.y() + selectYOffset,
                                   ctrlInputX - 7,
                                   MemReadLabel.y() + selectYOffset);
const QLine MemWriteSelect = QLine(DataBus.right()   + arrowHOffset,
                                   MemWriteLabel.y() + selectYOffset,
                                   ctrlInputX - 7,
                                   MemWriteLabel.y() + selectYOffset);

const QRect getRegRect(int row, int col) {
    QRect rect = QRect();
    int x, y;
    switch (row) {
    case 1:
        x = Col1X;
        break;
    case 2:
        x = Col2X;
        break;
    case 3:
        x = Col3X;
        break;
    case 4:
        x = Col4X;
        break;
    default:
        x = Col1X;
        break;
    }

    switch (col) {
    case 1:
        y = Row1Y;
        break;
    case 2:
        y = Row2Y;
        break;
    case 3:
        y = Row3Y;
        break;
    case 4:
        y = Row4Y;
        break;
    default:
        y = Row1Y;
        break;
    }

    rect.setX(x);
    rect.setY(y);

    return rect;
}

const QRect getRegLabelRect(int row, int col) {
    QRect rect = getRegRect(row, col);
    rect.setX(rect.x() - regLabelOffset);
    rect.setHeight(regLabelSize.height());
    rect.setWidth(regLabelSize.width());

    return rect;
}

const QRect getRegNoRect(int row, int col) {
    QRect rect = getRegRect(row, col);
    rect.setX(rect.x() - regNumberOffset);
    rect.setHeight(regNumberSize.height());
    rect.setWidth(regNumberSize.width());

    return rect;
}

}

#endif // SHAPES_ONE_BYTE_DATA_BUS_H

