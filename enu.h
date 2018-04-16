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
#include <QException>
namespace Enu
{
    Q_NAMESPACE
    static const quint8 maxRegisterNumber = 31;
    static const quint8 signalDisabled= 255;
    class InvalidCPUMode : public QException
    {
        void raise() const { throw *this; }
        InvalidCPUMode *clone() const { return new InvalidCPUMode(*this); }
    };
    // Instruction mnemonics
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
    Q_ENUM_NS(MainBusState)
    enum EBranchFunctions{
        Unconditional=0,
        BRGT=1,BRGE=2,BREQ=3,BRLE=4,BRLT=5,
        BRNE=6,BRV=7,BRC=8,BRS=9,
        BRU=10, //Branch if instruction specifier is unary
        IJT=13, //Instruction jump table
        PCE=14, //Program counter even?
        Stop=15,
        Assembler_Assigned=16
    };
    Q_ENUM_NS(EBranchFunctions)
    enum EControlSignals
    {
        MemRead, MemWrite,
        A,B,EOMux, MARMux,MARA, MARB, AMux, ALU,CSMux, AndZ,
        CMux, C,
        MDRMux, MDROMux, MDREMux,MDR, MDRE, MDRO,
    };
    Q_ENUM_NS(EControlSignals)
    enum EClockSignals{
        NCk,ZCk,VCk,CCk,SCk,MARCk,LoadCk,MDRCk, MDROCk, MDRECk,
    };
    Q_ENUM_NS(EClockSignals)
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

        Pre=255-1, Post=255-2,
        Mem=255-3, Acc=255-24, X=255-4, SP=255-5, PC=255-6, IR=255-7,
        T1=255-8, T2=255-9, T3=255-10, T4=255-11, T5=255-12, T6=255-13,
        N=255-15, Z=255-16, V=255-17,Cbit=255-25, S=255-18,
        MARAREG=255-19,MARBREG=255-20,
        MDRREG=255-21,MDREREG=255-22,MDROREG=255-23
    };

//Q_ENUM_NS(EMnemonic); //This is a declaration, despite whatever QT Creator says.


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

}

#endif // ENU_H
