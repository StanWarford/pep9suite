// File: enu.h
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
#ifndef ENU_H
#define ENU_H
#include <QtCore>
#include <QtGlobal>
#include <QException>
namespace Enu
{
    Q_NAMESPACE;

    static const quint8 maxRegisterNumber = 31;
    static const quint8 signalDisabled= 255;
    class InvalidCPUMode : public QException
    {
        void raise() const { throw *this; }
        InvalidCPUMode *clone() const { return new InvalidCPUMode(*this); }
    };

    enum EMask // For ALU function 15
    {
        SMask = 0x10,
        NMask = 0x08,
        ZMask = 0x04,
        VMask = 0x02,
        CMask = 0x01,
    };

    enum MainBusState {
        None,
        MemReadFirstWait,
        MemReadSecondWait,
        MemReadReady,
        MemWriteFirstWait,
        MemWriteSecondWait,
        MemWriteReady,
    };
    Q_ENUM_NS(MainBusState);

    enum EBranchFunctions{
        Unconditional = 0,
        uBRGT = 1, uBRGE = 2, uBREQ = 3, uBRLE = 4, uBRLT = 5,
        uBRNE = 6, uBRV = 7, uBRC = 8, uBRS = 9,
        IsPrefetchValid = 10,
        IsUnary = 11,
        IsPCEven = 12,
        AddressingModeDecoder = 13, //Adressing modes jump table
        InstructionSpecifierDecoder = 14, //Instruction jump table
        Stop=15,
        Assembler_Assigned=16
    };
    Q_ENUM_NS(EBranchFunctions);

    // Instruction mnemonics
    enum EControlSignals
    {
        MemRead, MemWrite,
        A,B,EOMux, MARMux,MARA, MARB, AMux, ALU,CSMux, AndZ,
        CMux, C,
        MDRMux, MDROMux, MDREMux,MDR, MDRE, MDRO,
        PValid,
    };
    Q_ENUM_NS(EControlSignals);
    enum EClockSignals{
        NCk,ZCk,VCk,CCk,SCk,MARCk,LoadCk,MDRCk, MDROCk, MDRECk,
        PValidCk,
    };
    Q_ENUM_NS(EClockSignals);

    enum EMemoryRegisters
    {
        MEM_MARA,MEM_MARB,MEM_MDR,MEM_MDRO,MEM_MDRE
    };
    Q_ENUM_NS(EMemoryRegisters);

    enum EStatusBit
    {
        STATUS_N,STATUS_Z,STATUS_V,STATUS_C,STATUS_S
    };
    Q_ENUM_NS(EStatusBit);

    enum EALUFunc
    {
        A_func=0,ApB_func=1,ApBpCin_func=2,ApnBp1_func=3,
        ApnBpCin_func=4,AandB_func=5,nAandB_func=6,AorB_func=7,
        nAorB_func=8,AxorB_func=9,nA_func=10,ASLA_func=11,
        ROLA_func=12,ASRA_func=13,RORA_func=14,NZVCA_func=15,
        UNDEFINED_func=255,
    };

    enum EKeywords {
        Pre, Post,
        Mem, Acc, X, SP, PC, IR,
        T1, T2, T3, T4, T5, T6,
        N, Z, V,Cbit, S,
        MARAREG, MARBREG,
        MDRREG, MDREREG, MDROREG
    };

    enum CPUType {
        OneByteDataBus,
        TwoByteDataBus
    };

    // For our drawing/shapes classes:
    enum Direction {
        Up,
        Down,
        Left,
        Right,
    };


    /*
     * Enums Specific to Pep9
     */


    enum class EMnemonic: int
    {
        ADDA, ADDX, ADDSP, ANDA, ANDX, ASLA, ASLX, ASRA, ASRX,
        BR, BRC, BREQ, BRGE, BRGT, BRLE, BRLT, BRNE, BRV,
        CALL, CPBA, CPBX, CPWA, CPWX,
        DECI, DECO,
        HEXO,
        LDBA, LDBX, LDWA, LDWX,
        MOVAFLG, MOVFLGA, MOVSPA,
        NEGA, NEGX, NOP, NOP0, NOP1, NOTA, NOTX,
        ORA, ORX,
        RET, RETTR, ROLA, ROLX, RORA, RORX,
        STBA, STBX, STWA, STWX, STOP, STRO, SUBA, SUBX, SUBSP

    };
    Q_ENUM_NS(EMnemonic);
    // Addressing modes for instructions
    enum class EAddrMode: int
    {
        NONE = 0,
        I = 1,
        D = 2,
        N = 4,
        S = 8,
        SF = 16,
        X = 32,
        SX = 64,
        SFX = 128,
        ALL = 255
    };
    Q_ENUM_NS(EAddrMode);
    // Format for symbols
    enum class ESymbolFormat
    {
        F_NONE, F_1C, F_1D, F_2D, F_1H, F_2H
    };

    enum class EExecState
    {
        EStart,
        ERun, ERunAwaitIO,
        EDebugAwaitIO, EDebugAwaitClick, EDebugRunToBP, EDebugSingleStep
    };

    enum class EWaiting
    {
        ERunWaiting,
        EDebugSSWaiting,
        EDebugResumeWaiting,
    };

    enum class EPane
    {
        ESource,
        EObject,
        EListing,
        EListingTrace,
        EMemoryTrace,
        EBatchIO,
        ETerminal,
        EMicrocode,
        EDataSection,
    };
    Q_ENUM_NS(EPane);

    // Enums specific to Pep9Micro
    enum class DebugLevels: quint16
    {  DEFAULT = 1,
       NONE = 0, MINIMAL = 1, ALL = 2, END
    };
}
#endif // ENU_H
