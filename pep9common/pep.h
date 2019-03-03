// File: pep.h
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
#ifndef PEP_H
#define PEP_H

#include <QString>
#include <QMap>
#include <QColor>

#include "enu.h"
class Pep
{
public:
    // Fonts:
    static const QString codeFont;
    static const int codeFontSize;
    static const int codeFontSizeSmall;
    static const int codeFontSizeLarge;
    static const int ioFontSize;
    static const QString labelFont;
    static const int labelFontSize;
    static const int labelFontSizeSmall;
    static const QString cpuFont;
    static const int cpuFontSize;
    static QString getSystem();

    // Function to read text from a resource file
    static QString resToString(QString fileName);
    static QString addCycleNumbers(QString codeString);

    // Maps between mnemonic enums and strings
    static QMap<Enu::EControlSignals, QString> decControlToMnemonMap;
    static QMap<Enu::EControlSignals, QString> memControlToMnemonMap;
    static QMap<Enu::EClockSignals, QString> clockControlToMnemonMap;
    static QMap<Enu::ECPUKeywords, QString> specificationToMnemonMap;
    static QMap<Enu::ECPUKeywords, QString> memSpecToMnemonMap;
    static QMap<Enu::ECPUKeywords, QString> regSpecToMnemonMap;
    static QMap<Enu::ECPUKeywords, QString> statusSpecToMnemonMap;
    static QMap<Enu::EBranchFunctions,QString> branchFuncToMnemonMap;
    static QMap<QString, Enu::EBranchFunctions> mnemonToBranchFuncMap;
    static QMap<QString, Enu::EControlSignals> mnemonToDecControlMap;
    static QMap<QString, Enu::EControlSignals> mnemonToMemControlMap;
    static QMap<QString, Enu::EClockSignals> mnemonToClockControlMap;
    static QMap<QString, Enu::ECPUKeywords> mnemonToSpecificationMap;
    static QMap<QString, Enu::ECPUKeywords> mnemonToMemSpecMap;
    static QMap<QString, Enu::ECPUKeywords> mnemonToRegSpecMap;
    static QMap<QString, Enu::ECPUKeywords> mnemonToStatusSpecMap;
    static void initMicroEnumMnemonMaps(Enu::CPUType cpuType, bool fullCtrlSection);
    static quint8 numControlSignals();
    static quint8 numClockSignals();

    /*
     * Begin Pep9 source code
     */
    // Default redefine mnemonics
    static const QString defaultUnaryMnemonic0;
    static const QString defaultUnaryMnemonic1;
    static const QString defaultNonUnaryMnemonic0;
    static const int defaultMnemon0AddrModes;
    static const QString defaultNonUnaryMnemonic1;
    static const int defaultMnemon1AddrModes;
    static const QString defaultNonUnaryMnemonic2;
    static const int defaultMnemon2AddrModes;
    static const QString defaultNonUnaryMnemonic3;
    static const int defaultMnemon3AddrModes;
    static const QString defaultNonUnaryMnemonic4;
    static const int defaultMnemon4AddrModes;


    // Functions for computing instruction specifiers
    static int aaaAddressField(Enu::EAddrMode addressMode);
    static int aAddressField(Enu::EAddrMode addressMode);
    static QString intToAddrMode(Enu::EAddrMode addressMode);
    static QString addrModeToCommaSpace(Enu::EAddrMode addressMode);

    // Maps between mnemonic enums and strings
    static QMap<Enu::EMnemonic, QString> enumToMnemonMap;
    static QMap<QString, Enu::EMnemonic> mnemonToEnumMap;
    static void initEnumMnemonMaps();

    // Maps to characterize each instruction
    static QMap<Enu::EMnemonic, int> opCodeMap;
    static QMap<Enu::EMnemonic, bool> isUnaryMap;
    static QMap<Enu::EMnemonic, bool> addrModeRequiredMap;
    static QMap<Enu::EMnemonic, bool> isTrapMap;
    static void initMnemonicMaps();

    // Map to specify legal addressing modes for each instruction
    static QMap<Enu::EMnemonic, int> addrModesMap;
    static void initAddrModesMap();

    // Decoder tables
    static QVector<Enu::EMnemonic> decodeMnemonic;
    static QVector<Enu::EAddrMode> decodeAddrMode;
    static void initDecoderTables();

    // Microprogram decoder table
    // Map mnemonic to the symbol in microcode which implements that instruction.
    static QMap<Enu::EMnemonic, QString> enumToMicrocodeInstrSymbol;
    // Map mnemonic to the symbopl in microcode which implements that iunstruction.
    static QMap<Enu::EAddrMode, QString> enumToMicrocodeAddrSymbol;
    static void initMicroDecoderTables();
};
#endif // PEP_H
