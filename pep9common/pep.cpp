// File: pep.cpp
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

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

#include <QFile>
#include <QString>
#include <QStringList>
#include <QtCore>

#include "pep.h"

using namespace Enu;

// Fonts
const QString Pep::codeFont = getSystem() == "Windows" ? "Courier" : (getSystem() == "Mac" ? "Courier" : "Ubuntu Mono,Courier New");
const int Pep::codeFontSize = getSystem() == "Windows" ? 9 : (getSystem() == "Mac" ? 11 : 11);
const int Pep::codeFontSizeSmall = getSystem() == "Windows" ? 7 : (getSystem() == "Mac" ? 10 : 10);
const int Pep::codeFontSizeLarge = getSystem() == "Windows" ? 9 : (getSystem() == "Mac" ? 11 : 11);
const int Pep::ioFontSize = getSystem() ==  "Windows" ? 10 : (getSystem() == "Mac" ? 13 : 10);
const QString Pep::labelFont = getSystem() == "Windows" ? "Verdana" : (getSystem() == "Mac" ? "Lucida Grande" : "Ubuntu");
const int Pep::labelFontSize = getSystem() == "Windows" ? 8 : (getSystem() == "Mac" ? 11 : 9);
const int Pep::labelFontSizeSmall = getSystem() == "Windows" ? 7 : (getSystem() == "Mac" ? 10 : 8);
const QString Pep::cpuFont = getSystem() == "Windows" ? "Verdana" : (getSystem() == "Mac" ? "Lucida Grande" : "Ubuntu");
const int Pep::cpuFontSize = getSystem() == "Windows" ? 8 : (getSystem() == "Mac" ? 11 : 8);

QString Pep::getSystem() {

    #ifdef Q_WS_X11
    return QString("Linux");
    s
    #elif defined Q_OS_OSX
    return QString("Mac");

    #elif defined Q_WS_QWS
    return QString("Embedded Linux");

    #elif defined Q_OS_WIN32
    return QString("Windows");

    #elif defined Q_WS_WIN
    return QString("Windows");

    #else
    return QString("No system");
    #endif
}

QString Pep::resToString(QString fileName, bool removeLineNumbers) {
    QFile file(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    QString inString = "";
    while (!in.atEnd()) {
        QString line = in.readLine();
        inString.append(line + "\n");
    }
    QStringList microcodeList;
    microcodeList = inString.split('\n');
    for (int i = 0; removeLineNumbers && i < microcodeList.size(); i++) {
        microcodeList[i].remove(QRegExp("^[0-9a-fA-F]+\\.?\\s*"));
    }
    inString = microcodeList.join("\n");
    return inString;
}

QString Pep::addCycleNumbers(QString codeString) {
    int lineNumber = 1;
    QStringList microcodeList = codeString.split("\n");
    for (int i = 0; i < microcodeList.size(); i++) {
        if (QRegExp("^//|^\\s*$|^unitpre|^unitpost", Qt::CaseInsensitive).indexIn(microcodeList.at(i)) != 0) {
            microcodeList[i].prepend(QString("%1. ").arg(lineNumber++));
        }
    }
    return microcodeList.join("\n");
}

QMap<Enu::EControlSignals, QString> Pep::decControlToMnemonMap;
QMap<Enu::EControlSignals, QString> Pep::memControlToMnemonMap;
QMap<Enu::EClockSignals, QString> Pep::clockControlToMnemonMap;
QMap<Enu::ECPUKeywords, QString> Pep::specificationToMnemonMap;
QMap<Enu::ECPUKeywords, QString> Pep::memSpecToMnemonMap;
QMap<Enu::ECPUKeywords, QString> Pep::regSpecToMnemonMap;
QMap<Enu::ECPUKeywords, QString> Pep::statusSpecToMnemonMap;
QMap<Enu::EBranchFunctions,QString> Pep::branchFuncToMnemonMap;
QMap<QString,Enu::EBranchFunctions> Pep::mnemonToBranchFuncMap;
QMap<QString, Enu::EControlSignals> Pep::mnemonToDecControlMap;
QMap<QString, Enu::EControlSignals> Pep::mnemonToMemControlMap;
QMap<QString, Enu::EClockSignals> Pep::mnemonToClockControlMap;
QMap<QString, Enu::ECPUKeywords> Pep::mnemonToSpecificationMap;
QMap<QString, Enu::ECPUKeywords> Pep::mnemonToMemSpecMap;
QMap<QString, Enu::ECPUKeywords> Pep::mnemonToRegSpecMap;
QMap<QString, Enu::ECPUKeywords> Pep::mnemonToStatusSpecMap;


void Pep::initMicroEnumMnemonMaps(CPUType cpuType, bool fullCtrlSection)
{
    branchFuncToMnemonMap.clear(); mnemonToBranchFuncMap.clear();
    if(fullCtrlSection) {
        mnemonToBranchFuncMap.insert("GT",uBRGT); branchFuncToMnemonMap.insert(uBRGT,"GT");
        mnemonToBranchFuncMap.insert("GE",uBRGE); branchFuncToMnemonMap.insert(uBRGE,"GE");
        mnemonToBranchFuncMap.insert("EQ",uBREQ); branchFuncToMnemonMap.insert(uBREQ,"EQ");
        mnemonToBranchFuncMap.insert("NE",uBRNE); branchFuncToMnemonMap.insert(uBRNE,"NE");
        mnemonToBranchFuncMap.insert("LE",uBRLE); branchFuncToMnemonMap.insert(uBRLE,"LE");
        mnemonToBranchFuncMap.insert("LT",uBRLT); branchFuncToMnemonMap.insert(uBRLT,"LT");
        mnemonToBranchFuncMap.insert("VBIT",uBRV); branchFuncToMnemonMap.insert(uBRV,"VBit");
        mnemonToBranchFuncMap.insert("CBIT",uBRC); branchFuncToMnemonMap.insert(uBRC,"CBit");
        mnemonToBranchFuncMap.insert("SBIT",uBRS); branchFuncToMnemonMap.insert(uBRS,"SBIt");
        mnemonToBranchFuncMap.insert("HASPREFETCH",IsPrefetchValid); branchFuncToMnemonMap.insert(IsPrefetchValid,"HasPrefetch");
        mnemonToBranchFuncMap.insert("ISUNARY",IsUnary); branchFuncToMnemonMap.insert(IsUnary,"IsUnary");
        mnemonToBranchFuncMap.insert("ISPCEVEN",IsPCEven); branchFuncToMnemonMap.insert(IsPCEven,"IsPCEven");
        mnemonToBranchFuncMap.insert("DECODEADDRSPEC",AddressingModeDecoder); branchFuncToMnemonMap.insert(AddressingModeDecoder,"DecodeAddrMode");
        mnemonToBranchFuncMap.insert("DECODEINSTRSPEC",InstructionSpecifierDecoder); branchFuncToMnemonMap.insert(InstructionSpecifierDecoder,"DecodeInstrSpec");
        mnemonToBranchFuncMap.insert("STOPCPU",Stop); branchFuncToMnemonMap.insert(Stop,"StopCPU");
    }
    mnemonToDecControlMap.clear();  decControlToMnemonMap.clear();
    mnemonToDecControlMap.insert("C", C); decControlToMnemonMap.insert(C,"C");
    mnemonToDecControlMap.insert("B", B); decControlToMnemonMap.insert(B,"B");
    mnemonToDecControlMap.insert("A", A); decControlToMnemonMap.insert(A,"A");
    mnemonToDecControlMap.insert("ANDZ", AndZ); decControlToMnemonMap.insert(AndZ,"ANDZ");
    if(fullCtrlSection) {
        mnemonToDecControlMap.insert("PVALID", PValid); decControlToMnemonMap.insert(PValid,"PValid");
    }
    mnemonToDecControlMap.insert("AMUX", AMux); decControlToMnemonMap.insert(AMux,"AMUX");
    mnemonToDecControlMap.insert("CMUX", CMux); decControlToMnemonMap.insert(CMux,"CMUX");
    mnemonToDecControlMap.insert("ALU", ALU); decControlToMnemonMap.insert(ALU,"ALU");
    mnemonToDecControlMap.insert("CSMUX", CSMux); decControlToMnemonMap.insert(CSMux,"CSMUX");
    if (cpuType == OneByteDataBus) {
        mnemonToDecControlMap.insert("MDRMUX", MDRMux); decControlToMnemonMap.insert(MDRMux,"MDRMUX");
    }
    else if (cpuType == TwoByteDataBus){
        mnemonToDecControlMap.insert("MARMUX", MARMux); decControlToMnemonMap.insert(MARMux,"MARMUX");
        mnemonToDecControlMap.insert("MDROMUX", MDROMux); decControlToMnemonMap.insert(MDROMux,"MDROMUX");
        mnemonToDecControlMap.insert("MDREMUX", MDREMux); decControlToMnemonMap.insert(MDREMux,"MDREMUX");
        mnemonToDecControlMap.insert("EOMUX", EOMux); decControlToMnemonMap.insert(EOMux,"EOMUX");
    }

    memControlToMnemonMap.clear();                      mnemonToMemControlMap.clear();
    memControlToMnemonMap.insert(MemWrite, "MemWrite"); mnemonToMemControlMap.insert("MEMWRITE", MemWrite);
    memControlToMnemonMap.insert(MemRead, "MemRead");   mnemonToMemControlMap.insert("MEMREAD", MemRead);

    clockControlToMnemonMap.clear();                    mnemonToClockControlMap.clear();
    clockControlToMnemonMap.insert(LoadCk, "LoadCk");   mnemonToClockControlMap.insert("LOADCK", LoadCk);
    clockControlToMnemonMap.insert(MARCk, "MARCk");     mnemonToClockControlMap.insert("MARCK", MARCk);
    clockControlToMnemonMap.insert(SCk, "SCk");         mnemonToClockControlMap.insert("SCK", SCk);
    clockControlToMnemonMap.insert(CCk, "CCk");         mnemonToClockControlMap.insert("CCK", CCk);
    clockControlToMnemonMap.insert(VCk, "VCk");         mnemonToClockControlMap.insert("VCK", VCk);
    clockControlToMnemonMap.insert(ZCk, "ZCk");         mnemonToClockControlMap.insert("ZCK", ZCk);
    clockControlToMnemonMap.insert(NCk, "NCk");         mnemonToClockControlMap.insert("NCK", NCk);
    if(fullCtrlSection) {
        clockControlToMnemonMap.insert(PValidCk, "PValidCk"); mnemonToClockControlMap.insert("PVALIDCK", PValidCk);
    }
    if (cpuType == OneByteDataBus) {
        clockControlToMnemonMap.insert(MDRCk, "MDRCk");     mnemonToClockControlMap.insert("MDRCK", MDRCk);
    }
    else if (cpuType == TwoByteDataBus){
        clockControlToMnemonMap.insert(MDROCk, "MDROCk");     mnemonToClockControlMap.insert("MDROCK", MDROCk);
        clockControlToMnemonMap.insert(MDRECk, "MDRECk");     mnemonToClockControlMap.insert("MDRECK", MDRECk);
    }

    specificationToMnemonMap.clear();                   mnemonToSpecificationMap.clear();
    specificationToMnemonMap.insert(Pre, "UnitPre:");   mnemonToSpecificationMap.insert("UNITPRE:", Pre);
    specificationToMnemonMap.insert(Post, "UnitPost:"); mnemonToSpecificationMap.insert("UNITPOST:", Post);

    memSpecToMnemonMap.clear();                         mnemonToMemSpecMap.clear();
    memSpecToMnemonMap.insert(Mem, "Mem");              mnemonToMemSpecMap.insert("MEM", Mem);

    regSpecToMnemonMap.clear();                         mnemonToRegSpecMap.clear();
    regSpecToMnemonMap.insert(Acc, "A");                  mnemonToRegSpecMap.insert("A", Acc);
    regSpecToMnemonMap.insert(X, "X");                  mnemonToRegSpecMap.insert("X", X);
    regSpecToMnemonMap.insert(SP, "SP");                mnemonToRegSpecMap.insert("SP", SP);
    regSpecToMnemonMap.insert(PC, "PC");                mnemonToRegSpecMap.insert("PC", PC);
    regSpecToMnemonMap.insert(IR, "IR");                mnemonToRegSpecMap.insert("IR", IR);
    regSpecToMnemonMap.insert(T1, "T1");                mnemonToRegSpecMap.insert("T1", T1);
    regSpecToMnemonMap.insert(T2, "T2");                mnemonToRegSpecMap.insert("T2", T2);
    regSpecToMnemonMap.insert(T3, "T3");                mnemonToRegSpecMap.insert("T3", T3);
    regSpecToMnemonMap.insert(T4, "T4");                mnemonToRegSpecMap.insert("T4", T4);
    regSpecToMnemonMap.insert(T5, "T5");                mnemonToRegSpecMap.insert("T5", T5);
    regSpecToMnemonMap.insert(T6, "T6");                mnemonToRegSpecMap.insert("T6", T6);
    regSpecToMnemonMap.insert(MARAREG, "MARA");         mnemonToRegSpecMap.insert("MARA", MARAREG);
    regSpecToMnemonMap.insert(MARBREG, "MARB");         mnemonToRegSpecMap.insert("MARB", MARBREG);
    regSpecToMnemonMap.insert(MDRREG, "MDR");           mnemonToRegSpecMap.insert("MDR", MDRREG);

    statusSpecToMnemonMap.clear();                      mnemonToStatusSpecMap.clear();
    statusSpecToMnemonMap.insert(N, "N");               mnemonToStatusSpecMap.insert("N", N);
    statusSpecToMnemonMap.insert(Z, "Z");               mnemonToStatusSpecMap.insert("Z", Z);
    statusSpecToMnemonMap.insert(V, "V");               mnemonToStatusSpecMap.insert("V", V);
    statusSpecToMnemonMap.insert(Cbit, "C");            mnemonToStatusSpecMap.insert("C", Cbit);
    statusSpecToMnemonMap.insert(S, "S");               mnemonToStatusSpecMap.insert("S", S);
}

quint8 Pep::numControlSignals()
{
    QMetaObject meta = Enu::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("EControlSignals"));
    return static_cast<quint8>(metaEnum.keyCount());
}

quint8 Pep::numClockSignals()
{
    QMetaObject meta = Enu::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("EClockSignals"));
    return static_cast<quint8>(metaEnum.keyCount());
}

/*
 * Begin Pep9 Sources
 */
// Default redefine mnemonics
const QString Pep::defaultUnaryMnemonic0 = "NOP0";
const QString Pep::defaultUnaryMnemonic1 = "NOP1";
const QString Pep::defaultNonUnaryMnemonic0 = "NOP";
const int Pep::defaultMnemon0AddrModes = static_cast<int>(EAddrMode::I);
const QString Pep::defaultNonUnaryMnemonic1 = "DECI";
const int Pep::defaultMnemon1AddrModes = static_cast<int>(EAddrMode::ALL)&(~static_cast<int>(EAddrMode::I));
const QString Pep::defaultNonUnaryMnemonic2 = "DECO";
const int Pep::defaultMnemon2AddrModes = static_cast<int>(EAddrMode::ALL);
const QString Pep::defaultNonUnaryMnemonic3 = "HEXO";
const int Pep::defaultMnemon3AddrModes = static_cast<int>(EAddrMode::ALL);
const QString Pep::defaultNonUnaryMnemonic4 = "STRO";
const int Pep::defaultMnemon4AddrModes = static_cast<int>(EAddrMode::D)|static_cast<int>(EAddrMode::N)|static_cast<int>(EAddrMode::S)|
        static_cast<int>(EAddrMode::SF)|static_cast<int>(EAddrMode::X);


int Pep::aaaAddressField(EAddrMode addressMode)
{
    if (addressMode == EAddrMode::I) return 0;
    if (addressMode == EAddrMode::D) return 1;
    if (addressMode == EAddrMode::N) return 2;
    if (addressMode == EAddrMode::S) return 3;
    if (addressMode == EAddrMode::SF) return 4;
    if (addressMode == EAddrMode::X) return 5;
    if (addressMode == EAddrMode::SX) return 6;
    if (addressMode == EAddrMode::SFX) return 7;
    return -1; // Should not occur;
}

int Pep::aAddressField(EAddrMode addressMode)
{

    if (addressMode == EAddrMode::I) return 0;
    if (addressMode == EAddrMode::X) return 1;
    return -1; // Should not occur;
}

QString Pep::intToAddrMode(EAddrMode addressMode) {
    if (addressMode == EAddrMode::I) return "i";
    if (addressMode == EAddrMode::D) return "d";
    if (addressMode == EAddrMode::N) return "n";
    if (addressMode == EAddrMode::S) return "s";
    if (addressMode == EAddrMode::SF) return "sf";
    if (addressMode == EAddrMode::X) return "x";
    if (addressMode == EAddrMode::SX) return "sx";
    if (addressMode == EAddrMode::SFX) return "sfx";
    return ""; // Should not occur
}

QString Pep::addrModeToCommaSpace(EAddrMode addressMode) {
    if (addressMode == EAddrMode::NONE) return "";
    if (addressMode == EAddrMode::I) return ", i";
    if (addressMode == EAddrMode::D) return ", d";
    if (addressMode == EAddrMode::N) return ", n";
    if (addressMode == EAddrMode::S) return ", s";
    if (addressMode == EAddrMode::SF) return ", sf";
    if (addressMode == EAddrMode::X) return ", x";
    if (addressMode == EAddrMode::SX) return ", sx";
    if (addressMode == EAddrMode::SFX) return ", sfx";
    return ""; // Should not occur
}

int Pep::operandDisplayFieldWidth(EMnemonic mnemon)
{
    switch(mnemon) {
    // All byte instructions that do not perform stores only need 1 byte
    // wide operands, which is 2 characters. STBr's operand is a memory address,
    // which still needs 2 bytes to be represented.
    case Enu::EMnemonic::LDBA:
        [[fallthrough]];
    case Enu::EMnemonic::LDBX:
        [[fallthrough]];
    case Enu::EMnemonic::CPBA:
        [[fallthrough]];
    case Enu::EMnemonic::CPBX:
        return 2;
    // All others use 2 bytes, which is 4 characters.
    default:
        return 4;

    }
}

// Maps between mnemonic enums and strings
QMap<Enu::EMnemonic, QString> Pep::enumToMnemonMap;
QMap<QString, Enu::EMnemonic> Pep::mnemonToEnumMap;
void Pep::initEnumMnemonMaps()
{
    enumToMnemonMap.clear(); mnemonToEnumMap.clear(); // Can be called from Redefine Mnemonics

    QMetaObject meta = Enu::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("EMnemonic"));
    QString tempqs;
    for(int it = 0; it < metaEnum.keyCount(); it++)
    {
        EMnemonic tempi = static_cast<EMnemonic>(metaEnum.value(it));
        tempqs = QString(metaEnum.key(it)).toUpper();
        enumToMnemonMap.insert(tempi, tempqs); mnemonToEnumMap.insert(tempqs, tempi);
    }

    //Lastly, override whatever the above enumerator put in for the redefinable mnemonics
    enumToMnemonMap.insert(EMnemonic::NOP, defaultNonUnaryMnemonic0); mnemonToEnumMap.insert(defaultNonUnaryMnemonic0, EMnemonic::NOP);
    enumToMnemonMap.insert(EMnemonic::NOP0, defaultUnaryMnemonic0); mnemonToEnumMap.insert(defaultUnaryMnemonic0, EMnemonic::NOP0);
    enumToMnemonMap.insert(EMnemonic::NOP1, defaultUnaryMnemonic1); mnemonToEnumMap.insert(defaultUnaryMnemonic1, EMnemonic::NOP1);
    enumToMnemonMap.insert(EMnemonic::DECI, defaultNonUnaryMnemonic1); mnemonToEnumMap.insert(defaultNonUnaryMnemonic1, EMnemonic::DECI);
    enumToMnemonMap.insert(EMnemonic::DECO, defaultNonUnaryMnemonic2); mnemonToEnumMap.insert(defaultNonUnaryMnemonic2, EMnemonic::DECO);
    enumToMnemonMap.insert(EMnemonic::HEXO, defaultNonUnaryMnemonic3); mnemonToEnumMap.insert(defaultNonUnaryMnemonic3, EMnemonic::HEXO);
    enumToMnemonMap.insert(EMnemonic::STRO, defaultNonUnaryMnemonic4); mnemonToEnumMap.insert(defaultNonUnaryMnemonic4, EMnemonic::STRO);
}

// Maps to characterize each instruction
QMap<Enu::EMnemonic, int> Pep::opCodeMap;
QMap<Enu::EMnemonic, bool> Pep::isUnaryMap;
QMap<Enu::EMnemonic, bool> Pep::addrModeRequiredMap;
QMap<Enu::EMnemonic, bool> Pep::isTrapMap;
void initMnemMapHelper(EMnemonic mnemon,int start,bool unary,bool addrModeReq,bool isTrap)
{
    Pep::opCodeMap.insert(mnemon, start); Pep::isUnaryMap.insert(mnemon, unary);
    Pep::addrModeRequiredMap.insert(mnemon, addrModeReq); Pep::isTrapMap.insert(mnemon, isTrap);
}

void Pep::initMnemonicMaps(bool NOP0IsTrap)
{
    if(NOP0IsTrap) {
        initMnemMapHelper(EMnemonic::NOP0, 38, true, false, true);
    }
    else {
        initMnemMapHelper(EMnemonic::NOP0, 38, true, false, false);
    }

    initMnemMapHelper(EMnemonic::ADDA, 96, false, true, false);
    initMnemMapHelper(EMnemonic::ADDX, 104, false, true, false);
    initMnemMapHelper(EMnemonic::ADDSP, 80, false, true, false);
    initMnemMapHelper(EMnemonic::ANDA, 128, false, true, false);
    initMnemMapHelper(EMnemonic::ANDX, 136, false, true, false);
    initMnemMapHelper(EMnemonic::ASLA, 10, true, false, false);
    initMnemMapHelper(EMnemonic::ASLX, 11, true, false, false);
    initMnemMapHelper(EMnemonic::ASRA, 12, true, false, false);
    initMnemMapHelper(EMnemonic::ASRX, 13, true, false, false);

    initMnemMapHelper(EMnemonic::BR, 18, false, false, false);
    initMnemMapHelper(EMnemonic::BRC, 34, false, false, false);
    initMnemMapHelper(EMnemonic::BREQ, 24, false, false, false);
    initMnemMapHelper(EMnemonic::BRGE, 28, false, false, false);
    initMnemMapHelper(EMnemonic::BRGT, 30, false, false, false);
    initMnemMapHelper(EMnemonic::BRLE, 20, false, false, false);
    initMnemMapHelper(EMnemonic::BRLT, 22, false, false, false);
    initMnemMapHelper(EMnemonic::BRNE, 26, false, false, false);
    initMnemMapHelper(EMnemonic::BRV, 32, false, false, false);

    initMnemMapHelper(EMnemonic::CALL, 36, false, false, false);
    initMnemMapHelper(EMnemonic::CPBA, 176, false, true, false);
    initMnemMapHelper(EMnemonic::CPBX, 184, false, true, false);
    initMnemMapHelper(EMnemonic::CPWA, 160, false, true, false);
    initMnemMapHelper(EMnemonic::CPWX, 168, false, true, false);

    initMnemMapHelper(EMnemonic::DECI, 48, false, true, true);
    initMnemMapHelper(EMnemonic::DECO, 56, false, true, true);

    initMnemMapHelper(EMnemonic::HEXO, 64, false, true, true);

    initMnemMapHelper(EMnemonic::LDBA, 208, false, true, false);
    initMnemMapHelper(EMnemonic::LDBX, 216, false, true, false);
    initMnemMapHelper(EMnemonic::LDWA, 192, false, true, false);
    initMnemMapHelper(EMnemonic::LDWX, 200, false, true, false);

    initMnemMapHelper(EMnemonic::MOVAFLG, 5, true, false, false);
    initMnemMapHelper(EMnemonic::MOVFLGA, 4, true, false, false);
    initMnemMapHelper(EMnemonic::MOVSPA, 3, true, false, false);

    //opCodeMap.insert(MOVAFLG, 5); isUnaryMap.insert(MOVAFLG, true); addrModeRequiredMap.insert(MOVAFLG, true); isTrapMap.insert(MOVAFLG, false);
    //opCodeMap.insert(MOVFLGA, 4); isUnaryMap.insert(MOVFLGA, true); addrModeRequiredMap.insert(MOVFLGA, true); isTrapMap.insert(MOVFLGA, false);
    //opCodeMap.insert(MOVSPA, 3); isUnaryMap.insert(MOVSPA, true); addrModeRequiredMap.insert(MOVSPA, true); isTrapMap.insert(MOVSPA, false);
    initMnemMapHelper(EMnemonic::NEGA, 8, true, false, false);
    initMnemMapHelper(EMnemonic::NEGX, 9, true, false, false);
    initMnemMapHelper(EMnemonic::NOP, 40, false, true, true);
    initMnemMapHelper(EMnemonic::NOP1, 39, true, false, true);
    initMnemMapHelper(EMnemonic::NOTA, 6, true, false, false);
    initMnemMapHelper(EMnemonic::NOTX, 7, true, false, false);

    initMnemMapHelper(EMnemonic::ORA, 144, false, true, false);
    initMnemMapHelper(EMnemonic::ORX, 152, false, true, false);

    initMnemMapHelper(EMnemonic::RET, 1, true, false, false);
    initMnemMapHelper(EMnemonic::RETTR, 2, true, false, false);
    initMnemMapHelper(EMnemonic::ROLA, 14, false, true, false);
    initMnemMapHelper(EMnemonic::ROLX, 15, true, false, false);
    initMnemMapHelper(EMnemonic::RORA, 16, true, false, false);
    initMnemMapHelper(EMnemonic::RORX, 17, true, false, false);

    initMnemMapHelper(EMnemonic::STBA, 240, false, true, false);
    initMnemMapHelper(EMnemonic::STBX, 248, false, true, false);
    initMnemMapHelper(EMnemonic::STWA, 224, false, true, false);
    initMnemMapHelper(EMnemonic::STWX, 232, false, true, false);
    initMnemMapHelper(EMnemonic::STOP, 0, true, false, false);
    initMnemMapHelper(EMnemonic::STRO, 72, false, true, true);
    initMnemMapHelper(EMnemonic::SUBA, 112, false, true, false);
    initMnemMapHelper(EMnemonic::SUBX, 120, false, true, false);
    initMnemMapHelper(EMnemonic::SUBSP, 88, false, true, false);
}

// Map to specify legal addressing modes for each instruction
QMap<Enu::EMnemonic, int > Pep::addrModesMap;
void Pep::initAddrModesMap()
{
        constexpr int all = static_cast<int>(EAddrMode::ALL);
    constexpr int IX = static_cast<int>(EAddrMode::I)|static_cast<int>(EAddrMode::X);
    constexpr int store = all & (~static_cast<int>(EAddrMode::I));
    // Nonunary instructions
    addrModesMap.insert(EMnemonic::ADDA, all);
    addrModesMap.insert(EMnemonic::ADDX, all);
    addrModesMap.insert(EMnemonic::ADDSP, all);
    addrModesMap.insert(EMnemonic::ANDA, all);
    addrModesMap.insert(EMnemonic::ANDX, all);
    addrModesMap.insert(EMnemonic::BR, IX);
    addrModesMap.insert(EMnemonic::BRC, IX);
    addrModesMap.insert(EMnemonic::BREQ, IX);
    addrModesMap.insert(EMnemonic::BRGE, IX);
    addrModesMap.insert(EMnemonic::BRGT, IX);
    addrModesMap.insert(EMnemonic::BRLE, IX);
    addrModesMap.insert(EMnemonic::BRLT,  IX);
    addrModesMap.insert(EMnemonic::BRNE, IX);
    addrModesMap.insert(EMnemonic::BRV, IX);
    addrModesMap.insert(EMnemonic::CALL, IX);
    addrModesMap.insert(EMnemonic::CPBA, all);
    addrModesMap.insert(EMnemonic::CPBX, all);
    addrModesMap.insert(EMnemonic::CPWA, all);
    addrModesMap.insert(EMnemonic::CPWX, all);
    addrModesMap.insert(EMnemonic::LDBA, all);
    addrModesMap.insert(EMnemonic::LDBX, all);
    addrModesMap.insert(EMnemonic::LDWA, all);
    addrModesMap.insert(EMnemonic::LDWX, all);
    addrModesMap.insert(EMnemonic::ORA, all);
    addrModesMap.insert(EMnemonic::ORX, all);
    addrModesMap.insert(EMnemonic::STBA, store);
    addrModesMap.insert(EMnemonic::STBX, store);
    addrModesMap.insert(EMnemonic::STWA, store);
    addrModesMap.insert(EMnemonic::STWX, store);
    addrModesMap.insert(EMnemonic::SUBA, all);
    addrModesMap.insert(EMnemonic::SUBX, all);
    addrModesMap.insert(EMnemonic::SUBSP, all);

    // Nonunary trap instructions
    addrModesMap.insert(EMnemonic::NOP, defaultMnemon0AddrModes);
    addrModesMap.insert(EMnemonic::DECI, defaultMnemon1AddrModes);
    addrModesMap.insert(EMnemonic::DECO, defaultMnemon2AddrModes);
    addrModesMap.insert(EMnemonic::HEXO, defaultMnemon3AddrModes);
    addrModesMap.insert(EMnemonic::STRO, defaultMnemon4AddrModes);
}

bool Pep::isStoreMnemonic(EMnemonic mnemon)
{
    return mnemon == EMnemonic::STBA ||
           mnemon == EMnemonic::STBX ||
           mnemon == EMnemonic::STWA ||
           mnemon == EMnemonic::STWX ||
           mnemon == EMnemonic::DECI;
}

// Decoder tables
QVector<Enu::EMnemonic> Pep::decodeMnemonic(256);
QVector<Enu::EAddrMode> Pep::decodeAddrMode(256);
void initDecoderTableAHelper(EMnemonic val,int startIdx)
{
    Pep::decodeMnemonic[startIdx] = val; Pep::decodeAddrMode[startIdx] = EAddrMode::I;
    Pep::decodeMnemonic[startIdx + 1] = val; Pep::decodeAddrMode[startIdx + 1] = EAddrMode::X;
}

void initDecoderTableAAAHelper(EMnemonic val,int startIdx)
{
        Pep::decodeMnemonic[startIdx + 0] = val; Pep::decodeAddrMode[startIdx + 0] = EAddrMode::I;
        Pep::decodeMnemonic[startIdx + 1] = val; Pep::decodeAddrMode[startIdx + 1] = EAddrMode::D;
        Pep::decodeMnemonic[startIdx + 2] = val; Pep::decodeAddrMode[startIdx + 2] = EAddrMode::N;
        Pep::decodeMnemonic[startIdx + 3] = val; Pep::decodeAddrMode[startIdx + 3] = EAddrMode::S;
        Pep::decodeMnemonic[startIdx + 4] = val; Pep::decodeAddrMode[startIdx + 4] = EAddrMode::SF;
        Pep::decodeMnemonic[startIdx + 5] = val; Pep::decodeAddrMode[startIdx + 5] = EAddrMode::X;
        Pep::decodeMnemonic[startIdx + 6] = val; Pep::decodeAddrMode[startIdx + 6] = EAddrMode::SX;
        Pep::decodeMnemonic[startIdx + 7] = val; Pep::decodeAddrMode[startIdx + 7] = EAddrMode::SFX;
}

void initDecoderTableHelperTrap(EMnemonic val,int startIdx,int distance){
    for(int it=0;it<distance;it++)
    {
        // Note that the trap instructions are all unary at the machine level
        Pep::decodeMnemonic[startIdx + it] = val; Pep::decodeAddrMode[startIdx + it] = EAddrMode::NONE;
    }
}

void Pep::initDecoderTables()
{
    decodeMnemonic[0] = EMnemonic::STOP; decodeAddrMode[0] = EAddrMode::NONE;
    decodeMnemonic[1] = EMnemonic::RET; decodeAddrMode[1] = EAddrMode::NONE;
    decodeMnemonic[2] = EMnemonic::RETTR; decodeAddrMode[2] = EAddrMode::NONE;
    decodeMnemonic[3] = EMnemonic::MOVSPA; decodeAddrMode[3] = EAddrMode::NONE;
    decodeMnemonic[4] = EMnemonic::MOVFLGA; decodeAddrMode[4] = EAddrMode::NONE;
    decodeMnemonic[5] = EMnemonic::MOVAFLG; decodeAddrMode[5] = EAddrMode::NONE;

    decodeMnemonic[6] = EMnemonic::NOTA; decodeAddrMode[6] = EAddrMode::NONE;
    decodeMnemonic[7] = EMnemonic::NOTX; decodeAddrMode[7] = EAddrMode::NONE;
    decodeMnemonic[8] = EMnemonic::NEGA; decodeAddrMode[8] = EAddrMode::NONE;
    decodeMnemonic[9] = EMnemonic::NEGX; decodeAddrMode[9] = EAddrMode::NONE;
    decodeMnemonic[10] = EMnemonic::ASLA; decodeAddrMode[10] = EAddrMode::NONE;
    decodeMnemonic[11] = EMnemonic::ASLX; decodeAddrMode[11] = EAddrMode::NONE;
    decodeMnemonic[12] = EMnemonic::ASRA; decodeAddrMode[12] = EAddrMode::NONE;
    decodeMnemonic[13] = EMnemonic::ASRX; decodeAddrMode[13] = EAddrMode::NONE;
    decodeMnemonic[14] = EMnemonic::ROLA; decodeAddrMode[14] = EAddrMode::NONE;
    decodeMnemonic[15] = EMnemonic::ROLX; decodeAddrMode[15] = EAddrMode::NONE;
    decodeMnemonic[16] = EMnemonic::RORA; decodeAddrMode[16] = EAddrMode::NONE;
    decodeMnemonic[17] = EMnemonic::RORX; decodeAddrMode[17] = EAddrMode::NONE;

    initDecoderTableAHelper(EMnemonic::BR, 18);
    initDecoderTableAHelper(EMnemonic::BRLE, 20);
    initDecoderTableAHelper(EMnemonic::BRLT, 22);
    initDecoderTableAHelper(EMnemonic::BREQ, 24);
    initDecoderTableAHelper(EMnemonic::BRNE, 26);
    initDecoderTableAHelper(EMnemonic::BRGE, 28);
    initDecoderTableAHelper(EMnemonic::BRGT, 30);
    initDecoderTableAHelper(EMnemonic::BRV, 32);
    initDecoderTableAHelper(EMnemonic::BRC, 34);
    initDecoderTableAHelper(EMnemonic::CALL, 36);

    initDecoderTableHelperTrap(EMnemonic::NOP0, 38, 1);
    initDecoderTableHelperTrap(EMnemonic::NOP1, 39, 1);
    initDecoderTableHelperTrap(EMnemonic::NOP, 40, 8);
    initDecoderTableHelperTrap(EMnemonic::DECI, 48, 8);
    initDecoderTableHelperTrap(EMnemonic::DECO, 56, 8);
    initDecoderTableHelperTrap(EMnemonic::HEXO, 64, 8);
    initDecoderTableHelperTrap(EMnemonic::STRO, 72, 8);

    initDecoderTableAAAHelper(EMnemonic::ADDSP, 80);
    initDecoderTableAAAHelper(EMnemonic::SUBSP, 88);
    initDecoderTableAAAHelper(EMnemonic::ADDA, 96);
    initDecoderTableAAAHelper(EMnemonic::ADDX, 104);
    initDecoderTableAAAHelper(EMnemonic::SUBA, 112);
    initDecoderTableAAAHelper(EMnemonic::SUBX, 120);
    initDecoderTableAAAHelper(EMnemonic::ANDA, 128);
    initDecoderTableAAAHelper(EMnemonic::ANDX, 136);
    initDecoderTableAAAHelper(EMnemonic::ORA, 144);
    initDecoderTableAAAHelper(EMnemonic::ORX, 152);
    initDecoderTableAAAHelper(EMnemonic::CPWA, 160);
    initDecoderTableAAAHelper(EMnemonic::CPWX, 168);
    initDecoderTableAAAHelper(EMnemonic::CPBA, 176);
    initDecoderTableAAAHelper(EMnemonic::CPBX, 184);
    initDecoderTableAAAHelper(EMnemonic::LDWA, 192);
    initDecoderTableAAAHelper(EMnemonic::LDWX, 200);
    initDecoderTableAAAHelper(EMnemonic::LDBA, 208);
    initDecoderTableAAAHelper(EMnemonic::LDBX, 216);
    initDecoderTableAAAHelper(EMnemonic::STWA, 224);
    initDecoderTableAAAHelper(EMnemonic::STWX, 232);
    initDecoderTableAAAHelper(EMnemonic::STBA, 240);
    initDecoderTableAAAHelper(EMnemonic::STBX, 248);
}

QMap<Enu::EMnemonic, QString> Pep::defaultEnumToMicrocodeInstrSymbol;
QMap<Enu::EAddrMode, QString> Pep::defaultEnumToMicrocodeAddrSymbol;
QVector<QString> Pep::instSpecToMicrocodeInstrSymbol;
QVector<QString> Pep::instSpecToMicrocodeAddrSymbol;
QString Pep::defaultStartSymbol;
void Pep::initMicroDecoderTables()
{
    defaultStartSymbol = "start";
    // Initialize insturction specifiers for microcode symbols
    defaultEnumToMicrocodeInstrSymbol.clear();

    QMetaObject meta = Enu::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("EMnemonic"));
    QString tempqs;
    for(int it = 0; it < metaEnum.keyCount(); it++)
    {
        EMnemonic tempi = static_cast<EMnemonic>(metaEnum.value(it));
        tempqs = QString(metaEnum.key(it)).toUpper();
        defaultEnumToMicrocodeInstrSymbol.insert(tempi, tempqs);
    }

    defaultEnumToMicrocodeInstrSymbol.insert(EMnemonic::NOP0, defaultUnaryMnemonic0);
    defaultEnumToMicrocodeInstrSymbol.insert(EMnemonic::NOP1, "opcode27");
    defaultEnumToMicrocodeInstrSymbol.insert(EMnemonic::NOP,  "opcode28");
    defaultEnumToMicrocodeInstrSymbol.insert(EMnemonic::DECI, "opcode30");
    defaultEnumToMicrocodeInstrSymbol.insert(EMnemonic::DECO, "opcode38");
    defaultEnumToMicrocodeInstrSymbol.insert(EMnemonic::HEXO, "opcode40");
    defaultEnumToMicrocodeInstrSymbol.insert(EMnemonic::STRO, "opcode48");

    // Initialize symbols for addressing modes
    defaultEnumToMicrocodeAddrSymbol.clear();
    metaEnum = meta.enumerator(meta.indexOfEnumerator("EAddrMode"));
    for(int it = 0; it < metaEnum.keyCount(); it++)
    {
        EAddrMode tempi = static_cast<EAddrMode>(metaEnum.value(it));
        if(tempi == Enu::EAddrMode::NONE) {
            tempqs = Pep::enumToMnemonMap[Enu::EMnemonic::STOP].toLower();
        }
        else {
            tempqs = QString(metaEnum.key(it)).toLower()+"Addr";
        }

        defaultEnumToMicrocodeAddrSymbol.insert(tempi, tempqs);
    }

    instSpecToMicrocodeInstrSymbol.resize(256);
    instSpecToMicrocodeAddrSymbol.resize(256);
    for(int it=0; it <= 255; it++) {
        instSpecToMicrocodeInstrSymbol[it] = defaultEnumToMicrocodeInstrSymbol[Pep::decodeMnemonic[it]].toLower();
        instSpecToMicrocodeAddrSymbol[it] = defaultEnumToMicrocodeAddrSymbol[Pep::decodeAddrMode[it]];
    }
}
