#include "cpudefs.h"

namespace Pep9::uarch {
    QMap<Pep9::uarch::EControlSignals, QString> decControlToMnemonMap;
    QMap<Pep9::uarch::EControlSignals, QString> memControlToMnemonMap;
    QMap<Pep9::uarch::EClockSignals, QString> clockControlToMnemonMap;
    QMap<Pep9::uarch::ECPUKeywords, QString> specificationToMnemonMap;
    QMap<Pep9::uarch::ECPUKeywords, QString> memSpecToMnemonMap;
    QMap<Pep9::uarch::ECPUKeywords, QString> regSpecToMnemonMap;
    QMap<Pep9::uarch::ECPUKeywords, QString> statusSpecToMnemonMap;
    QMap<Pep9::uarch::EBranchFunctions,QString> branchFuncToMnemonMap;
    QMap<QString, Pep9::uarch::EBranchFunctions> mnemonToBranchFuncMap;
    QMap<QString, Pep9::uarch::EControlSignals> mnemonToDecControlMap;
    QMap<QString, Pep9::uarch::EControlSignals> mnemonToMemControlMap;
    QMap<QString, Pep9::uarch::EClockSignals> mnemonToClockControlMap;
    QMap<QString, Pep9::uarch::ECPUKeywords> mnemonToSpecificationMap;
    QMap<QString, Pep9::uarch::ECPUKeywords> mnemonToMemSpecMap;
    QMap<QString, Pep9::uarch::ECPUKeywords> mnemonToRegSpecMap;
    QMap<QString, Pep9::uarch::ECPUKeywords> mnemonToStatusSpecMap;
    QVector<QString> instSpecToMicrocodeInstrSymbol(256);
    QVector<QString> instSpecToMicrocodeAddrSymbol(256);


}


void Pep9::uarch::initMicroEnumMnemonMaps(PepCore::CPUType cpuType, bool fullCtrlSection)
{
    using namespace Pep9::uarch;

    branchFuncToMnemonMap.clear();
    mnemonToBranchFuncMap.clear();
    mnemonToDecControlMap.clear();
    decControlToMnemonMap.clear();
    statusSpecToMnemonMap.clear();
    mnemonToStatusSpecMap.clear();
    memControlToMnemonMap.clear();
    mnemonToMemControlMap.clear();
    clockControlToMnemonMap.clear();
    mnemonToClockControlMap.clear();
    specificationToMnemonMap.clear();
    mnemonToSpecificationMap.clear();
    memSpecToMnemonMap.clear();
    mnemonToMemSpecMap.clear();
    regSpecToMnemonMap.clear();
    mnemonToRegSpecMap.clear();

    if(fullCtrlSection) {
        mnemonToBranchFuncMap.insert("GT",EBranchFunctions::uBRGT);
        branchFuncToMnemonMap.insert(EBranchFunctions::uBRGT,"GT");
        mnemonToBranchFuncMap.insert("GE",EBranchFunctions::uBRGE);
        branchFuncToMnemonMap.insert(EBranchFunctions::uBRGE,"GE");
        mnemonToBranchFuncMap.insert("EQ",EBranchFunctions::uBREQ);
        branchFuncToMnemonMap.insert(EBranchFunctions::uBREQ,"EQ");
        mnemonToBranchFuncMap.insert("NE",EBranchFunctions::uBRNE);
        branchFuncToMnemonMap.insert(EBranchFunctions::uBRNE,"NE");
        mnemonToBranchFuncMap.insert("LE",EBranchFunctions::uBRLE);
        branchFuncToMnemonMap.insert(EBranchFunctions::uBRLE,"LE");
        mnemonToBranchFuncMap.insert("LT",EBranchFunctions::uBRLT);
        branchFuncToMnemonMap.insert(EBranchFunctions::uBRLT,"LT");
        mnemonToBranchFuncMap.insert("VBIT",EBranchFunctions::uBRV);
        branchFuncToMnemonMap.insert(EBranchFunctions::uBRV,"VBit");
        mnemonToBranchFuncMap.insert("CBIT",EBranchFunctions::uBRC);
        branchFuncToMnemonMap.insert(EBranchFunctions::uBRC,"CBit");
        mnemonToBranchFuncMap.insert("SBIT",EBranchFunctions::uBRS);
        branchFuncToMnemonMap.insert(EBranchFunctions::uBRS,"SBIt");
        mnemonToBranchFuncMap.insert("HASPREFETCH",EBranchFunctions::IsPrefetchValid);
        branchFuncToMnemonMap.insert(EBranchFunctions::IsPrefetchValid,"HasPrefetch");
        mnemonToBranchFuncMap.insert("ISUNARY",EBranchFunctions::IsUnary);
        branchFuncToMnemonMap.insert(EBranchFunctions::IsUnary,"IsUnary");
        mnemonToBranchFuncMap.insert("PCISODD",EBranchFunctions::PCisOdd);
        branchFuncToMnemonMap.insert(EBranchFunctions::PCisOdd,"PCisOdd");
        mnemonToBranchFuncMap.insert("DECODEADDRSPEC",EBranchFunctions::AddressingModeDecoder);
        branchFuncToMnemonMap.insert(EBranchFunctions::AddressingModeDecoder,"DecodeAddrMode");
        mnemonToBranchFuncMap.insert("DECODEINSTRSPEC",EBranchFunctions::InstructionSpecifierDecoder);
        branchFuncToMnemonMap.insert(EBranchFunctions::InstructionSpecifierDecoder,"DecodeInstrSpec");
        mnemonToBranchFuncMap.insert("STOPCPU",EBranchFunctions::Stop);
        branchFuncToMnemonMap.insert(EBranchFunctions::Stop,"StopCPU");
    }



    mnemonToDecControlMap.insert("C", EControlSignals::C);
    decControlToMnemonMap.insert(EControlSignals::C,"C");
    mnemonToDecControlMap.insert("B", EControlSignals::B);
    decControlToMnemonMap.insert(EControlSignals::B,"B");
    mnemonToDecControlMap.insert("A", EControlSignals::A);
    decControlToMnemonMap.insert(EControlSignals::A,"A");
    mnemonToDecControlMap.insert("ANDZ", EControlSignals::AndZ);
    decControlToMnemonMap.insert(EControlSignals::AndZ,"ANDZ");
    if(fullCtrlSection) {
        mnemonToDecControlMap.insert("PVALID", EControlSignals::PValid);
        decControlToMnemonMap.insert(EControlSignals::PValid,"PValid");
    }
    mnemonToDecControlMap.insert("AMUX", EControlSignals::AMux);
    decControlToMnemonMap.insert(EControlSignals::AMux,"AMUX");
    mnemonToDecControlMap.insert("CMUX", EControlSignals::CMux);
    decControlToMnemonMap.insert(EControlSignals::CMux,"CMUX");
    mnemonToDecControlMap.insert("ALU", EControlSignals::ALU);
    decControlToMnemonMap.insert(EControlSignals::ALU,"ALU");
    mnemonToDecControlMap.insert("CSMUX", EControlSignals::CSMux);
    decControlToMnemonMap.insert(EControlSignals::CSMux,"CSMUX");

    if (cpuType == PepCore::CPUType::OneByteDataBus) {
        mnemonToDecControlMap.insert("MDRMUX", EControlSignals::MDRMux);
        decControlToMnemonMap.insert(EControlSignals::MDRMux,"MDRMUX");
    }
    else if (cpuType == PepCore::CPUType::TwoByteDataBus){
        mnemonToDecControlMap.insert("MARMUX", EControlSignals::MARMux);
        decControlToMnemonMap.insert(EControlSignals::MARMux,"MARMUX");
        mnemonToDecControlMap.insert("MDROMUX", EControlSignals::MDROMux);
        decControlToMnemonMap.insert(EControlSignals::MDROMux,"MDROMUX");
        mnemonToDecControlMap.insert("MDREMUX", EControlSignals::MDREMux);
        decControlToMnemonMap.insert(EControlSignals::MDREMux,"MDREMUX");
        mnemonToDecControlMap.insert("EOMUX", EControlSignals::EOMux);
        decControlToMnemonMap.insert(EControlSignals::EOMux,"EOMUX");
    }

    memControlToMnemonMap.insert(EControlSignals::MemWrite, "MemWrite");
    mnemonToMemControlMap.insert("MEMWRITE", EControlSignals::MemWrite);
    memControlToMnemonMap.insert(EControlSignals::MemRead, "MemRead");
    mnemonToMemControlMap.insert("MEMREAD", EControlSignals::MemRead);

    clockControlToMnemonMap.insert(EClockSignals::LoadCk, "LoadCk");
    mnemonToClockControlMap.insert("LOADCK", EClockSignals::LoadCk);
    clockControlToMnemonMap.insert(EClockSignals::MARCk, "MARCk");
    mnemonToClockControlMap.insert("MARCK", EClockSignals::MARCk);
    clockControlToMnemonMap.insert(EClockSignals::SCk, "SCk");
    mnemonToClockControlMap.insert("SCK", EClockSignals::SCk);
    clockControlToMnemonMap.insert(EClockSignals::CCk, "CCk");
    mnemonToClockControlMap.insert("CCK", EClockSignals::CCk);
    clockControlToMnemonMap.insert(EClockSignals::VCk, "VCk");
    mnemonToClockControlMap.insert("VCK", EClockSignals::VCk);
    clockControlToMnemonMap.insert(EClockSignals::ZCk, "ZCk");
    mnemonToClockControlMap.insert("ZCK", EClockSignals::ZCk);
    clockControlToMnemonMap.insert(EClockSignals::NCk, "NCk");
    mnemonToClockControlMap.insert("NCK", EClockSignals::NCk);
    if(fullCtrlSection) {
        clockControlToMnemonMap.insert(EClockSignals::PValidCk, "PValidCk");
        mnemonToClockControlMap.insert("PVALIDCK", EClockSignals::PValidCk);
    }
    if (cpuType == PepCore::CPUType::OneByteDataBus) {
        clockControlToMnemonMap.insert(EClockSignals::MDRCk, "MDRCk");
        mnemonToClockControlMap.insert("MDRCK", EClockSignals::MDRCk);
    }
    else if (cpuType == PepCore::CPUType::TwoByteDataBus){
        clockControlToMnemonMap.insert(EClockSignals::MDROCk, "MDROCk");
        mnemonToClockControlMap.insert("MDROCK", EClockSignals::MDROCk);
        clockControlToMnemonMap.insert(EClockSignals::MDRECk, "MDRECk");
        mnemonToClockControlMap.insert("MDRECK", EClockSignals::MDRECk);
    }

    specificationToMnemonMap.insert(ECPUKeywords::Pre, "UnitPre:");
    mnemonToSpecificationMap.insert("UNITPRE:", ECPUKeywords::Pre);
    specificationToMnemonMap.insert(ECPUKeywords::Post, "UnitPost:");
    mnemonToSpecificationMap.insert("UNITPOST:", ECPUKeywords::Post);

    memSpecToMnemonMap.insert(ECPUKeywords::Mem, "Mem");
    mnemonToMemSpecMap.insert("MEM", ECPUKeywords::Mem);

    regSpecToMnemonMap.insert(ECPUKeywords::Acc, "A");
    mnemonToRegSpecMap.insert("A", ECPUKeywords::Acc);
    regSpecToMnemonMap.insert(ECPUKeywords::X, "X");
    mnemonToRegSpecMap.insert("X", ECPUKeywords::X);
    regSpecToMnemonMap.insert(ECPUKeywords::SP, "SP");
    mnemonToRegSpecMap.insert("SP", ECPUKeywords::SP);
    regSpecToMnemonMap.insert(ECPUKeywords::PC, "PC");
    mnemonToRegSpecMap.insert("PC", ECPUKeywords::PC);
    regSpecToMnemonMap.insert(ECPUKeywords::IR, "IR");
    mnemonToRegSpecMap.insert("IR", ECPUKeywords::IR);
    regSpecToMnemonMap.insert(ECPUKeywords::T1, "T1");
    mnemonToRegSpecMap.insert("T1", ECPUKeywords::T1);
    regSpecToMnemonMap.insert(ECPUKeywords::T2, "T2");
    mnemonToRegSpecMap.insert("T2", ECPUKeywords::T2);
    regSpecToMnemonMap.insert(ECPUKeywords::T3, "T3");
    mnemonToRegSpecMap.insert("T3", ECPUKeywords::T3);
    regSpecToMnemonMap.insert(ECPUKeywords::T4, "T4");
    mnemonToRegSpecMap.insert("T4", ECPUKeywords::T4);
    regSpecToMnemonMap.insert(ECPUKeywords::T5, "T5");
    mnemonToRegSpecMap.insert("T5", ECPUKeywords::T5);
    regSpecToMnemonMap.insert(ECPUKeywords::T6, "T6");
    mnemonToRegSpecMap.insert("T6", ECPUKeywords::T6);
    regSpecToMnemonMap.insert(ECPUKeywords::MARAREG, "MARA");
    mnemonToRegSpecMap.insert("MARA", ECPUKeywords::MARAREG);
    regSpecToMnemonMap.insert(ECPUKeywords::MARBREG, "MARB");
    mnemonToRegSpecMap.insert("MARB", ECPUKeywords::MARBREG);
    regSpecToMnemonMap.insert(ECPUKeywords::MDRREG, "MDR");
    mnemonToRegSpecMap.insert("MDR", ECPUKeywords::MDRREG);

    statusSpecToMnemonMap.insert(ECPUKeywords::N, "N");
    mnemonToStatusSpecMap.insert("N", ECPUKeywords::N);
    statusSpecToMnemonMap.insert(ECPUKeywords::Z, "Z");
    mnemonToStatusSpecMap.insert("Z", ECPUKeywords::Z);
    statusSpecToMnemonMap.insert(ECPUKeywords::V, "V");
    mnemonToStatusSpecMap.insert("V", ECPUKeywords::V);
    statusSpecToMnemonMap.insert(ECPUKeywords::Cbit, "C");
    mnemonToStatusSpecMap.insert("C", ECPUKeywords::Cbit);
    statusSpecToMnemonMap.insert(ECPUKeywords::S, "S");
    mnemonToStatusSpecMap.insert("S", ECPUKeywords::S);

    auto z = Pep9::uarch::mnemonToDecControlMap;
    z.size();
}

quint8 Pep9::uarch::numControlSignals()
{
    QMetaObject meta = Pep9::uarch::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("EControlSignals"));
    return static_cast<quint8>(metaEnum.keyCount());
}

quint8 Pep9::uarch::numClockSignals()
{
    QMetaObject meta = Pep9::uarch::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("EClockSignals"));
    return static_cast<quint8>(metaEnum.keyCount());
}
