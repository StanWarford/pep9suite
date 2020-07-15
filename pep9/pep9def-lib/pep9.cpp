#include "pep9.h"

Pep9::Pep9(bool NOP0IsTrap): QObject(), APepVersion(), decodeMnemonic(256), decodeAddrMode(256)
{
    initMnemonicMaps(NOP0IsTrap);
    initDecoderTables();
}

Pep9::~Pep9() = default;

PepCore::CPURegisters_number_t Pep9::get_global_register_number(APepVersion::global_registers reg) const
{
    switch(reg) {
    case APepVersion::global_registers::A:
        return to_uint8_t(Pep9::CPURegisters::A);
    case APepVersion::global_registers::X:
        return to_uint8_t(Pep9::CPURegisters::X);
    case APepVersion::global_registers::SP:
        return to_uint8_t(Pep9::CPURegisters::SP);
    case APepVersion::global_registers::PC:
        return to_uint8_t(Pep9::CPURegisters::PC);
    case APepVersion::global_registers::IS:
        return to_uint8_t(Pep9::CPURegisters::IS);
    case APepVersion::global_registers::OS:
        return to_uint8_t(Pep9::CPURegisters::OS);
    }
}

bool Pep9::isInstructionUnary(quint8 instr) const
{
    return this->isUnaryMap[decodeMnemonic[instr]];
}

quint8 Pep9::maxRegisterNumber() const
{
    return 32;
}

void Pep9::initMnemonicMaps(bool NOP0IsTrap)
{
    auto initMnemMapHelper = [this](Pep9::EMnemonic mnemon, int start, bool unary, bool addrModeReq, bool isTrap) {
        opCodeMap.insert(mnemon, start); isUnaryMap.insert(mnemon, unary);
        addrModeRequiredMap.insert(mnemon, addrModeReq); isTrapMap.insert(mnemon, isTrap);
    };

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

void Pep9::initAddrModesMap()
{

}

bool Pep9::isStoreMnemonic(Pep9::EMnemonic mnemon)
{
    return mnemon == EMnemonic::STBA ||
           mnemon == EMnemonic::STBX ||
           mnemon == EMnemonic::STWA ||
           mnemon == EMnemonic::STWX ||
           mnemon == EMnemonic::DECI;
}

void Pep9::initDecoderTables()
{

    auto initDecoderTableAHelper = [this](EMnemonic val,int startIdx)
    {
        decodeMnemonic[startIdx] = val; decodeAddrMode[startIdx] = EAddrMode::I;
        decodeMnemonic[startIdx + 1] = val; decodeAddrMode[startIdx + 1] = EAddrMode::X;
    };

    auto initDecoderTableAAAHelper = [this](EMnemonic val,int startIdx)
    {
            decodeMnemonic[startIdx + 0] = val; decodeAddrMode[startIdx + 0] = EAddrMode::I;
            decodeMnemonic[startIdx + 1] = val; decodeAddrMode[startIdx + 1] = EAddrMode::D;
            decodeMnemonic[startIdx + 2] = val; decodeAddrMode[startIdx + 2] = EAddrMode::N;
            decodeMnemonic[startIdx + 3] = val; decodeAddrMode[startIdx + 3] = EAddrMode::S;
            decodeMnemonic[startIdx + 4] = val; decodeAddrMode[startIdx + 4] = EAddrMode::SF;
            decodeMnemonic[startIdx + 5] = val; decodeAddrMode[startIdx + 5] = EAddrMode::X;
            decodeMnemonic[startIdx + 6] = val; decodeAddrMode[startIdx + 6] = EAddrMode::SX;
            decodeMnemonic[startIdx + 7] = val; decodeAddrMode[startIdx + 7] = EAddrMode::SFX;
    };

    auto initDecoderTableHelperTrap = [this](EMnemonic val,int startIdx,int distance){
        for(int it=0;it<distance;it++)
        {
            // Note that the trap instructions are all unary at the machine level
            decodeMnemonic[startIdx + it] = val; decodeAddrMode[startIdx + it] = EAddrMode::NONE;
        }
    };

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

uint8_t to_uint8_t(Pep9::CPURegisters reg)
{
    return static_cast<uint8_t>(reg);
}
