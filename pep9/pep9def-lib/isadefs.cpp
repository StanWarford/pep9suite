#include "isadefs.h"
namespace Pep9::ISA
{
// Default redefine mnemonics
const QString defaultUnaryMnemonic0 = "NOP0";
const QString defaultUnaryMnemonic1 = "NOP1";
const QString defaultNonUnaryMnemonic0 = "NOP";
const int defaultMnemon0AddrModes = static_cast<int>(EAddrMode::I);
const QString defaultNonUnaryMnemonic1 = "DECI";
const int defaultMnemon1AddrModes = static_cast<int>(EAddrMode::ALL)&(~static_cast<int>(EAddrMode::I));
const QString defaultNonUnaryMnemonic2 = "DECO";
const int defaultMnemon2AddrModes = static_cast<int>(EAddrMode::ALL);
const QString defaultNonUnaryMnemonic3 = "HEXO";
const int defaultMnemon3AddrModes = static_cast<int>(EAddrMode::ALL);
const QString defaultNonUnaryMnemonic4 = "STRO";
const int defaultMnemon4AddrModes = static_cast<int>(EAddrMode::D)|static_cast<int>(EAddrMode::N)|static_cast<int>(EAddrMode::S)|
        static_cast<int>(EAddrMode::SF)|static_cast<int>(EAddrMode::X);
int aaaAddressField(EAddrMode addressMode)
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

int aAddressField(EAddrMode addressMode)
{

    if (addressMode == EAddrMode::I) return 0;
    if (addressMode == EAddrMode::X) return 1;
    return -1; // Should not occur;
}

QString intToAddrMode(EAddrMode addressMode) {
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

QString addrModeToCommaSpace(EAddrMode addressMode) {
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

int operandDisplayFieldWidth(EMnemonic mnemon)
{
    switch(mnemon) {
    // All byte instructions that do not perform stores only need 1 byte
    // wide operands, which is 2 characters. STBr's operand is a memory address,
    // which still needs 2 bytes to be represented.
    case EMnemonic::LDBA:
        [[fallthrough]];
    case EMnemonic::LDBX:
        [[fallthrough]];
    case EMnemonic::CPBA:
        [[fallthrough]];
    case EMnemonic::CPBX:
        return 2;
    // All others use 2 bytes, which is 4 characters.
    default:
        return 4;

    }
}

// Maps between mnemonic enums and strings
QMap<EMnemonic, QString> enumToMnemonMap;
QMap<QString, EMnemonic> mnemonToEnumMap;
void initEnumMnemonMaps()
{
    enumToMnemonMap.clear(); mnemonToEnumMap.clear(); // Can be called from Redefine Mnemonics

    QMetaObject meta = Pep9::ISA::staticMetaObject;
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
QMap<EMnemonic, int> opCodeMap;
QMap<EMnemonic, bool> isUnaryMap;
QMap<EMnemonic, bool> addrModeRequiredMap;
QMap<EMnemonic, bool> isTrapMap;
void initMnemMapHelper(EMnemonic mnemon,int start,bool unary,bool addrModeReq,bool isTrap)
{
    opCodeMap.insert(mnemon, start); isUnaryMap.insert(mnemon, unary);
    addrModeRequiredMap.insert(mnemon, addrModeReq); isTrapMap.insert(mnemon, isTrap);
}

void initMnemonicMaps(bool NOP0IsTrap)
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
    initMnemMapHelper(EMnemonic::ROLA, 14, true, false, false);
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
QMap<EMnemonic, int > addrModesMap;
void initAddrModesMap()
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

bool isStoreMnemonic(EMnemonic mnemon)
{
    return mnemon == EMnemonic::STBA ||
           mnemon == EMnemonic::STBX ||
           mnemon == EMnemonic::STWA ||
           mnemon == EMnemonic::STWX ||
           mnemon == EMnemonic::DECI;
}

// Decoder tables
QVector<EMnemonic> decodeMnemonic(256);
QVector<EAddrMode> decodeAddrMode(256);
void initDecoderTableAHelper(EMnemonic val,int startIdx)
{
    decodeMnemonic[startIdx] = val; decodeAddrMode[startIdx] = EAddrMode::I;
    decodeMnemonic[startIdx + 1] = val; decodeAddrMode[startIdx + 1] = EAddrMode::X;
}

void initDecoderTableAAAHelper(EMnemonic val,int startIdx)
{
        decodeMnemonic[startIdx + 0] = val; decodeAddrMode[startIdx + 0] = EAddrMode::I;
        decodeMnemonic[startIdx + 1] = val; decodeAddrMode[startIdx + 1] = EAddrMode::D;
        decodeMnemonic[startIdx + 2] = val; decodeAddrMode[startIdx + 2] = EAddrMode::N;
        decodeMnemonic[startIdx + 3] = val; decodeAddrMode[startIdx + 3] = EAddrMode::S;
        decodeMnemonic[startIdx + 4] = val; decodeAddrMode[startIdx + 4] = EAddrMode::SF;
        decodeMnemonic[startIdx + 5] = val; decodeAddrMode[startIdx + 5] = EAddrMode::X;
        decodeMnemonic[startIdx + 6] = val; decodeAddrMode[startIdx + 6] = EAddrMode::SX;
        decodeMnemonic[startIdx + 7] = val; decodeAddrMode[startIdx + 7] = EAddrMode::SFX;
}

void initDecoderTableHelperTrap(EMnemonic val,int startIdx,int distance){
    for(int it=0;it<distance;it++)
    {
        // Note that the trap instructions are all unary at the machine level
        decodeMnemonic[startIdx + it] = val; decodeAddrMode[startIdx + it] = EAddrMode::NONE;
    }
}

void initDecoderTables()
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

}

QMap<Pep9::ISA::EMnemonic, QString> Pep9::ISA::defaultEnumToMicrocodeInstrSymbol;
QMap<Pep9::ISA::EAddrMode, QString> Pep9::ISA::defaultEnumToMicrocodeAddrSymbol;
QVector<QString> Pep9::ISA::instSpecToMicrocodeInstrSymbol;
QVector<QString> Pep9::ISA::instSpecToMicrocodeAddrSymbol;
QString Pep9::ISA::defaultStartSymbol;
void Pep9::ISA::initMicroDecoderTables()
{
    defaultStartSymbol = "start";
    // Initialize insturction specifiers for microcode symbols
    defaultEnumToMicrocodeInstrSymbol.clear();

    QMetaObject meta = Pep9::ISA::staticMetaObject;
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
        if(tempi == EAddrMode::NONE) {
            tempqs = enumToMnemonMap[EMnemonic::STOP].toLower();
        }
        else {
            tempqs = QString(metaEnum.key(it)).toLower()+"Addr";
        }

        defaultEnumToMicrocodeAddrSymbol.insert(tempi, tempqs);
    }
    auto x = defaultEnumToMicrocodeAddrSymbol;

    instSpecToMicrocodeInstrSymbol.resize(256);
    instSpecToMicrocodeAddrSymbol.resize(256);
    for(int it=0; it <= 255; it++) {
        instSpecToMicrocodeInstrSymbol[it] = defaultEnumToMicrocodeInstrSymbol[decodeMnemonic[it]].toLower();
        instSpecToMicrocodeAddrSymbol[it] = defaultEnumToMicrocodeAddrSymbol[decodeAddrMode[it]];
    }
}
