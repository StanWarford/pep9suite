#include "cpudatasection.h"
#include "code.h"
#include "memorysection.h"
#include "microcodeprogram.h"
CPUDataSection* CPUDataSection::_instance = nullptr;
CPUDataSection::CPUDataSection(QObject *parent): QObject(parent), memory(MemorySection::getInstance()),
    cpuFeatures(Enu::TwoByteDataBus), mainBusState(Enu::None),
    registerBank(32), memoryRegisters(6), controlSignals(20), clockSignals(10), hadDataError(false), errorMessage("")
{
    presetStaticRegisters();
}

bool CPUDataSection::aluFnIsUnary() const
{
    //The only alu functions that are unary are 0 & 10..15
    return controlSignals[Enu::ALU]==0||controlSignals[Enu::ALU]>=10;
}

bool CPUDataSection::getAMuxOutput(quint8& result) const
{
        if(controlSignals[Enu::AMux]==0)
        {
            //Which could come from MDRE when EOMux is 0
            if(controlSignals[Enu::EOMux]==0)
            {
                result=memoryRegisters[Enu::MEM_MDRE];
                return true;
            }
            //Or comes from MDRO if EOMux is 1
            else if(controlSignals[Enu::EOMux]==1)
            {
                result=memoryRegisters[Enu::MEM_MDRO];
                return true;
            }
            //Or has no has no output when EOMux is disabled
            else return false;

        }
        else if(controlSignals[Enu::AMux]==1)
        {
            return valueOnABus(result);
        }
        else return false;
}

bool CPUDataSection::calculateCSMuxOutput(bool &result) const
{
    //CSMux either outputs C when CS is 0
    if(controlSignals[Enu::CSMux]==0)
    {
        result=NZVCSbits&Enu::CMask;
        return true;
    }
    //Or outputs S when CS is 1
    else if(controlSignals[Enu::CSMux]==1)
    {
        result = NZVCSbits&Enu::SMask;
        return true;
    }
    //Otherwise it does not have valid output
    else return false;

}

bool CPUDataSection::calculateALUOutput(quint8 &res, quint8 &NZVC) const
{
    //This function should not set any errors.
    //Errors will be handled by step(..)
    quint8 a,b;
    bool carryIn = 0;
    bool hasA = getAMuxOutput(a), hasB = valueOnBBus(b);
    bool hasCIn = calculateCSMuxOutput(carryIn);
    if(!((aluFnIsUnary()&&hasA)||(hasA&&hasB)))
    {
        //The ALU output calculation would not be meaningful given its current function and inputs
        return false;
    }
    //Unless otherwise noted, do not return true (sucessfully) early, or the calculation for the NZ bits will be skipped
    switch(controlSignals[Enu::ALU])
    {
    case Enu::A_func: //A
        res=a;
        break;
    case Enu::ApB_func: //A plus B
        res=a+b;
        NZVC|= Enu::CMask*((int)(res<a||res<b)); //Carry out if result is unsigned less than a or b.
        //There is a signed overflow iff the high order bits of the input are the same,
        //and the inputs & output differs in sign.
        NZVC|= Enu::VMask*((~(a^b)&(a^res))>>7); //Dividing by 128 and >>7 are the same thing for unsigned integers
        break;
    case Enu::ApnBp1_func: //A plus ~B plus 1
        hasCIn = true;
        carryIn=1;
        //Intentional fallthrough
    case Enu::ApnBpCin_func: //A plus ~B plus Cin
        b=~b;
        //Intentional fallthrough
    case Enu::ApBpCin_func: //A plus B plus Cin
        if (!hasCIn) return false;
        res=a+b+(int)carryIn;
        NZVC|= Enu::CMask*((int)(res<a||res<b)); //Carry out if result is unsigned less than a or b.
        //There is a signed overflow iff the high order bits of the input are the same,
        //and the inputs & output differs in sign.
        NZVC|= Enu::VMask*((~(a^b)&(a^res))>>7);
        break;
    case Enu::AandB_func: //A*B
        res=a&b;
        break;
    case Enu::nAandB_func: //~(A*B)
        res=~(a&b);
        break;
    case Enu::AorB_func: //A+B
        res=a|b;
        break;
    case Enu::nAorB_func: //~(A+B)
        res=~(a|b);
        break;
    case Enu::AxorB_func: //A xor B
        res=a^b;
        break;
    case Enu::nA_func: //~A
        res=~a;
        break;
    case Enu::ASLA_func: //ASL A
        res=a<<1;
        NZVC|=Enu::CMask*((a & 0x80) >> 7);
        NZVC|=Enu::VMask*(((a<<1)^a)>>7); //Signed overflow if a<hi> doesn't match a<hi-1>
        break;
    case Enu::ROLA_func: //ROL A
        if (!hasCIn) return false;
        res=a<<1 | ((int) carryIn);
        NZVC|=Enu::CMask*((a & 0x80) >> 7);
        //NZVC|=Enu::VMask*(bool)(((a & 0x40) >> 6) ^ (NZVC&Enu::CMask));
        NZVC|=Enu::VMask*(((a<<1)^a)>>7); //Signed overflow if a<hi> doesn't match a<hi-1>
        break;
    case Enu::ASRA_func: //ASR A
        hasCIn = true;
        carryIn=a&128; //RORA and ASRA only differ by how the carryIn is calculated
        //Intentional fallthrough
    case Enu::RORA_func: //ROR a
        if (!hasCIn) return false;
        res = (a>>1)|(((int)carryIn)<<7); //No need to worry about sign extension on shift with unsigned a
        NZVC|=Enu::CMask*(a&1);
        break;
    case Enu::NZVCA_func: //Move A to NZVC
        res=0;
        NZVC|=Enu::NMask&a;
        NZVC|=Enu::ZMask&a;
        NZVC|=Enu::VMask&a;
        NZVC|=Enu::CMask&a;
        return true; //Must return early to avoid NZ calculation
    default: //If the default has been hit, then an invalid function was selected
        return false;
    }
    //Get boolean value for N, then shift to correct place
    NZVC |= (res>127) ? Enu::NMask : 0;
    //Get boolean value for Z, then shift to correct place
    NZVC |= (res==0) ? Enu::ZMask : 0;
    return true;

}

MemorySection *CPUDataSection::getMemorySection()
{
    return memory;
}

const MemorySection *CPUDataSection::getMemorySection() const
{
    return memory;
}

void CPUDataSection::setMemoryRegister(Enu::EMemoryRegisters reg, quint8 value)
{
    //Cache old memory value, so it be emitted with signal
    quint8 old = memoryRegisters[reg];
    if(old==value) return; //Don't continue if the new value is the old value
    memoryRegisters[reg]=value;
    emit memoryRegisterChanged(reg,old,value);
}

CPUDataSection* CPUDataSection::getInstance()
{
    //If no instance of the CPU data section yet exists, create it
    if(CPUDataSection::_instance==nullptr)
    {
        CPUDataSection::_instance = new CPUDataSection();
    }
    //Return the signle instance of the data section
    return CPUDataSection::_instance;
}

CPUDataSection::~CPUDataSection()
{
    //This code should not be called during the normal lifetime of Pep9CPU
}

Enu::CPUType CPUDataSection::getCPUFeatures() const
{
    return cpuFeatures;
}

quint8 CPUDataSection::getRegisterBankByte(quint8 registerNumber) const
{
    quint8 rval;
    if(registerNumber>Enu::maxRegisterNumber) return 0;
    else rval = registerBank[registerNumber];
    return rval;

}

quint16 CPUDataSection::getRegisterBankWord(quint8 registerNumber) const
{
    quint16 returnValue;
    if(registerNumber+1>Enu::maxRegisterNumber) returnValue=0;
    else
    {
        returnValue = ((quint16)registerBank[registerNumber])<<8;
        returnValue+=registerBank[registerNumber+1];
    }
    return returnValue;

}

quint8 CPUDataSection::getRegisterBankByte(CPURegisters registerNumber) const
{
    return getRegisterBankByte((quint8)registerNumber);
}

quint16 CPUDataSection::getRegisterBankWord(CPURegisters registerNumber) const
{
    return getRegisterBankWord((quint8)registerNumber);
}

quint8 CPUDataSection::getMemoryRegister(Enu::EMemoryRegisters registerNumber) const
{
    return memoryRegisters[registerNumber];
}

bool CPUDataSection::valueOnABus(quint8 &result) const
{
    if(controlSignals[Enu::A]==Enu::signalDisabled) return false;
    result=getRegisterBankByte(controlSignals[Enu::A]);
    return true;
}

bool CPUDataSection::valueOnBBus(quint8 &result) const
{
    if(controlSignals[Enu::B]==Enu::signalDisabled) return false;
    result=getRegisterBankByte(controlSignals[Enu::B]);
    return true;
}

bool CPUDataSection::valueOnCBus(quint8 &result) const
{
    if(controlSignals[Enu::CMux]==0)
    {
        //If CMux is 0, then the NZVC bits (minus S) are directly routed to result
        result = (NZVCSbits&(~Enu::SMask));
        return true;
    }
    else if(controlSignals[Enu::CMux]==1)
    {
        quint8 temp = 0; //Discard NZVC bits for this calculation, they are unecessary for calculating C's output
        //Otherwise the value of C depends solely on the ALU
        return calculateALUOutput(result,temp);
    }
    else return false;



}

Enu::MainBusState CPUDataSection::getMainBusState() const
{
    return mainBusState;
}

bool CPUDataSection::getStatusBit(Enu::EStatusBit statusBit) const
{
    switch(statusBit)
    {
    //Mask out bit of interest, then convert to bool
    case Enu::STATUS_N:
        return(NZVCSbits&Enu::NMask);
    case Enu::STATUS_Z:
        return(NZVCSbits&Enu::ZMask);
    case Enu::STATUS_V:
        return(NZVCSbits&Enu::VMask);
    case Enu::STATUS_C:
        return(NZVCSbits&Enu::CMask);
    case Enu::STATUS_S:
        return(NZVCSbits&Enu::SMask);
    }
}

void CPUDataSection::onSetStatusBit(Enu::EStatusBit statusBit, bool val)
{
    bool oldVal=false;
    switch(statusBit)
    {
    //Mask out the original value, and then or it with the properly shifted bit
    case Enu::STATUS_N:
        oldVal=NZVCSbits&Enu::NMask;
        NZVCSbits=(NZVCSbits&~Enu::NMask)|(val?1:0)*Enu::NMask;
        break;
    case Enu::STATUS_Z:
        oldVal=NZVCSbits&Enu::ZMask;
        NZVCSbits=(NZVCSbits&~Enu::ZMask)|(val?1:0)*Enu::ZMask;
        break;
    case Enu::STATUS_V:
        oldVal=NZVCSbits&Enu::VMask;
        NZVCSbits=(NZVCSbits&~Enu::VMask)|(val?1:0)*Enu::VMask;
        break;
    case Enu::STATUS_C:
        oldVal=NZVCSbits&Enu::CMask;
        NZVCSbits=(NZVCSbits&~Enu::CMask)|(val?1:0)*Enu::CMask;
        break;
    case Enu::STATUS_S:
        oldVal=NZVCSbits&Enu::SMask;
        NZVCSbits=(NZVCSbits&~Enu::SMask)|(val?1:0)*Enu::SMask;
        break;
    }
    if(oldVal != val) emit statusBitChanged(statusBit,val);
}

void CPUDataSection::onSetRegisterByte(quint8 reg, quint8 val)
{
    if(reg>21) return; //Don't allow static registers to be written to
    quint8 oldVal=registerBank[reg];
    registerBank[reg]=val;
    if(oldVal != val) emit registerChanged(reg,oldVal,val);
}

void CPUDataSection::onSetRegisterWord(quint8 reg, quint16 val)
{
   if(reg+1>21) return; //Don't allow static registers to be written to
   quint8 oldVal1 = registerBank[reg], oldVal2 = registerBank[reg+1];
   registerBank[reg] = val/256;
   registerBank[reg+1] = val%256;
   if(oldVal1 != val) emit registerChanged(reg,oldVal1,val/256);
   if(oldVal2 != val) emit registerChanged(reg,oldVal2,val%256);
}

void CPUDataSection::onSetMemoryRegister(Enu::EMemoryRegisters reg, quint8 val)
{
    quint8 oldVal = memoryRegisters[reg];
    memoryRegisters[reg] = val;
    if(oldVal != val) emit registerChanged(reg,oldVal,val);
}

void CPUDataSection::onSetClock(Enu::EClockSignals clock, bool value)
{
    bool old = clockSignals[clock];
    if(old == value) return;
    clockSignals[clock] = value;
    emit controlClockChanged();
}

void CPUDataSection::onSetControlSignal(Enu::EControlSignals control, quint8 value)
{
    quint8 old = controlSignals[control];
    if(old == value) return;
    controlSignals[control] = value;
    emit controlClockChanged();
}

bool CPUDataSection::setSignalsFromMicrocode(const MicroCode *line)
{
    if(line==nullptr)throw std::invalid_argument("CPUDataSection can't be passed a nullptr as a MicroCode line");
    //To start with, there are no
    bool clockVal,changed=false;
    quint8 sig;
    //For each control signal in the input line, set the appropriate control
    for(int it=0;it<controlSignals.length();it++)
    {
        sig=line->getControlSignal((Enu::EControlSignals)it);
        changed|=controlSignals[it]!=sig; //If signal aren't equal, then there was a change
        controlSignals[it]=sig;
    }
    //For each clock signal in the input microcode, set the appropriate clock
    for(int it=0;it<clockSignals.length();it++)
    {
        clockVal=line->getClockSignal((Enu::EClockSignals)it);
        changed|=clockSignals[it]!=clockVal; //If clocks aren't equal, then there was a change
        clockSignals[it]=clockVal;
    }
    if(changed) emit controlClockChanged(); //If any lines where changed, notify other components that there was a change
    return changed;
}

bool CPUDataSection::hadErrorOnStep() const
{
    return hadDataError;
}

QString CPUDataSection::getErrorMessage() const
{
    return errorMessage;
}

void CPUDataSection::setRegisterByte(quint8 reg, quint8 value)
{
    //Cache old register value
    quint8 old = registerBank[reg];
    if(old==value) return; //Don't continue if the new value is the old value
    onSetRegisterByte(reg,value);
    emit registerChanged(reg,old,value);
}

void CPUDataSection::setStatusBit(Enu::EStatusBit statusBit, bool val)
{
    quint8 old = NZVCSbits;
    onSetStatusBit(statusBit,val);
    //Check if old is equal to new after attempting to set it, as there isn't a simple test for bit math
    if((bool)(old&statusBit) == val)return; //Prevent signal from being emitted if no value changed
    emit statusBitChanged(statusBit,val);
}

void CPUDataSection::handleMainBusState() noexcept
{
    bool marChanged = false;
    quint8 a,b;
    if(clockSignals[Enu::MARCk]&&valueOnABus(a)&&valueOnBBus(b))
    {
        marChanged=!(a==memoryRegisters[Enu::MEM_MARA]&&b==memoryRegisters[Enu::MEM_MARB]);
    }
    switch(mainBusState)
    {
    case Enu::None:
        //One cannot change MAR contents and initiate a R/W on same cycle
        if(!marChanged)
        {
            if(controlSignals[Enu::MemRead]==1) mainBusState = Enu::MemReadFirstWait;
            else if(controlSignals[Enu::MemWrite]==1) mainBusState = Enu::MemWriteFirstWait;
        }
        break;
    case Enu::MemReadFirstWait:
        if(!marChanged&&controlSignals[Enu::MemRead]==1) mainBusState = Enu::MemReadSecondWait;
        else if(marChanged&&controlSignals[Enu::MemRead]==1); //Initiating a new read brings us back to first wait
        else if(controlSignals[Enu::MemWrite]==1) mainBusState = Enu::MemWriteFirstWait; //Switch from read to write.
        else mainBusState = Enu::None; //If neither are check, bus goes back to doing nothing
        break;
    case Enu::MemReadSecondWait:
        if(!marChanged&&controlSignals[Enu::MemRead]==1) mainBusState = Enu::MemReadReady;
        else if(marChanged&&controlSignals[Enu::MemRead]==1)mainBusState = Enu::MemReadFirstWait;
        else if(controlSignals[Enu::MemWrite]==1) mainBusState = Enu::MemWriteFirstWait;
        else mainBusState = Enu::None; //If neither are check, bus goes back to doing nothing
        break;
    case Enu::MemReadReady:
        if(!marChanged&&controlSignals[Enu::MemRead]==1)mainBusState=Enu::MemReadFirstWait; //Another MemRead will bring us back to first MemRead
        else if(marChanged&&controlSignals[Enu::MemRead]==1)mainBusState = Enu::MemReadFirstWait;
        else if(controlSignals[Enu::MemWrite]==1) mainBusState = Enu::MemWriteFirstWait;
        else mainBusState = Enu::None; //If neither are check, bus goes back to doing nothing
        break;
    case Enu::MemWriteFirstWait:
        if(!marChanged&&controlSignals[Enu::MemWrite]==1) mainBusState = Enu::MemWriteSecondWait;
        else if(marChanged&&controlSignals[Enu::MemWrite]==1); //Initiating a new write brings us back to first wait
        else if(controlSignals[Enu::MemRead]==1) mainBusState = Enu::MemReadFirstWait; //Switch from write to read.
        else mainBusState = Enu::None; //If neither are check, bus goes back to doing nothing
        break;
    case Enu::MemWriteSecondWait:
        if(!marChanged&&controlSignals[Enu::MemWrite]==1) mainBusState = Enu::MemWriteReady;
        else if(marChanged&&controlSignals[Enu::MemWrite]==1)mainBusState = Enu::MemWriteFirstWait; //Initiating a new write brings us back to first wait
        else if(controlSignals[Enu::MemRead]==1) mainBusState = Enu::MemReadFirstWait; //Switch from write to read.
        else mainBusState = Enu::None; //If neither are check, bus goes back to doing nothing
        break;
    case Enu::MemWriteReady:
        if(!marChanged&&controlSignals[Enu::MemWrite]==1)mainBusState=Enu::MemWriteFirstWait; //Another MemWrite will reset the bus state back to first MemWrite
        else if(marChanged&&controlSignals[Enu::MemWrite]==1)mainBusState = Enu::MemWriteFirstWait; //Initiating a new write brings us back to first wait
        else if(controlSignals[Enu::MemRead]==1) mainBusState = Enu::MemReadFirstWait; //Switch from write to read.
        else mainBusState = Enu::None; //If neither are check, bus goes back to doing nothing
        break;
    default:
        mainBusState=Enu::None;
        break;

    }
}

void CPUDataSection::stepTwoByte() noexcept
{
    //Update the bus state first, as the rest of the read / write functionality depends on it
    handleMainBusState();
    if(hadErrorOnStep()) return; //If the bus had an error, give up now

    //Set up all variables needed by stepping calculation
    Enu::EALUFunc aluFunc = (Enu::EALUFunc) controlSignals[Enu::ALU];
    quint8 a = 0,b = 0,c = 0,alu = 0,NZVC = 0;
    bool hasA = valueOnABus(a), hasB = valueOnBBus(b), hasC = valueOnCBus(c);
    bool statusBitError = false, hasALUOutput = calculateALUOutput(alu,NZVC);

    //Handle write to memory
    if(mainBusState == Enu::MemWriteReady)
    {
        quint16 address = (memoryRegisters[Enu::MEM_MARA]<<8)+memoryRegisters[Enu::MEM_MARB];
        address&=0xFFFE; //Memory access ignores lowest order bit
        memory->setMemoryWord(address,memoryRegisters[Enu::MEM_MDRE]*256+memoryRegisters[Enu::MEM_MDRO]);
    }

    //MARCk
    if(clockSignals[Enu::MARCk])
    {
        if(controlSignals[Enu::MARMux]==0)
        {
            //If MARMux is 0, route MDRE, MDRO to MARA, MARB
            setMemoryRegister(Enu::MEM_MARA,memoryRegisters[Enu::MEM_MDRE]);
            setMemoryRegister(Enu::MEM_MARB,memoryRegisters[Enu::MEM_MDRO]);
        }
        else if(controlSignals[Enu::MARMux]==1&&hasA&&hasB)
        {
            //If MARMux is 1, route A, B to MARA, MARB
            setMemoryRegister(Enu::MEM_MARA,a);
            setMemoryRegister(Enu::MEM_MARB,b);
        }
        else         //Otherwise MARCk is high, but no data flows through MARMux
        {
            hadDataError=true;
            errorMessage = "MARMux has no output but MARCk";
            return;
        }
    }

    //LoadCk
    if(clockSignals[Enu::LoadCk])
    {
        if(controlSignals[Enu::C] == Enu::signalDisabled)
        {
            hadDataError = true;
            errorMessage = "No destination register specified for LoadCk.";
        }
        else if(!hasC)
        {
            hadDataError = true;
            errorMessage = "No value on C Bus to clock in.";
        }
        else setRegisterByte(controlSignals[Enu::C],c);
    }

    //MDRECk
    if(clockSignals[Enu::MDRECk])
    {
        switch(controlSignals[Enu::MDREMux])
        {
        case 0: //Pick memory
        {
            quint16 address = (memoryRegisters[Enu::MEM_MARA]<<8)+memoryRegisters[Enu::MEM_MARB];
            address&=0xFFFE; //Memory access ignores lowest order bit
            if(mainBusState!=Enu::MemReadReady){
                hadDataError=true;
                errorMessage = "No value from data bus to write to MDRE";
                return;
            }
            else setMemoryRegister(Enu::MEM_MDRE,memory->getMemoryByte(address, true));
            break;
        }
        case 1: //Pick C Bus;
        {
            if(!hasC)
            {
                hadDataError=true;
                errorMessage = "No value on C bus to write to MDRE";
                return;
            }
            else setMemoryRegister(Enu::MEM_MDRE,c);
            break;
        }
        default:
            hadDataError=true;
            errorMessage = "No value to clock into MDRE";
            break;
        }

    }

    //MDRECk
    if(clockSignals[Enu::MDROCk])
    {
        switch(controlSignals[Enu::MDROMux])
        {
        case 0: //Pick memory
        {
            quint16 address = (memoryRegisters[Enu::MEM_MARA]<<8)+memoryRegisters[Enu::MEM_MARB];
            address&=0xFFFE; //Memory access ignores lowest order bit
            address+=1;
            if(mainBusState!=Enu::MemReadReady){
                hadDataError=true;
                errorMessage = "No value from data bus to write to MDRO";
                return;
            }
            else setMemoryRegister(Enu::MEM_MDRO,memory->getMemoryByte(address, true));
            break;
        }
        case 1: //Pick C Bus;
        {
            if(!hasC)
            {
                hadDataError=true;
                errorMessage = "No value on C bus to write to MDRO";
                return;
            }
            else setMemoryRegister(Enu::MEM_MDRO,c);
            break;
        }
        default:
            hadDataError=true;
            errorMessage = "No value to clock into MDRO";
            break;
        }

    }

    //NCk
    if(clockSignals[Enu::NCk])
    {
        //And against a instead of ALU output, since ALU output is technically 0
        if(aluFunc!=Enu::UNDEFINED_func && hasALUOutput) setStatusBit(Enu::STATUS_N,Enu::NMask & NZVC);
        else statusBitError = true;
    }

    //If no ALU output, don't set flags.
    //ZCk
    if(clockSignals[Enu::ZCk])
    {
        if(aluFunc!=Enu::UNDEFINED_func && hasALUOutput)
        {
            if(controlSignals[Enu::AndZ]==0)
            {
                setStatusBit(Enu::STATUS_Z,Enu::ZMask & NZVC);
            }
            else if(controlSignals[Enu::AndZ]==1)
            {
                setStatusBit(Enu::STATUS_Z,(bool)(Enu::ZMask & NZVC) && getStatusBit(Enu::STATUS_Z));
            }
            else statusBitError = true;
        }
        else statusBitError = true;
    }

    //VCk
    if(clockSignals[Enu::VCk])
    {
        if(aluFunc!=Enu::UNDEFINED_func && hasALUOutput) setStatusBit(Enu::STATUS_V,Enu::VMask & NZVC);
        else statusBitError = true;
    }

    //CCk
    if(clockSignals[Enu::CCk])
    {
        if(aluFunc!=Enu::UNDEFINED_func && hasALUOutput) setStatusBit(Enu::STATUS_C,Enu::CMask & NZVC);
        else statusBitError = true;
    }

    //SCk
    if(clockSignals[Enu::SCk])
    {
        if(aluFunc!=Enu::UNDEFINED_func && hasALUOutput) setStatusBit(Enu::STATUS_S,Enu::CMask & NZVC);
        else statusBitError = true;
    }

    if(statusBitError)
    {
        hadDataError = true;
        errorMessage = "ALU Error: No output from ALU to clock into status bits.";
    }
}

void CPUDataSection::presetStaticRegisters() noexcept
{
    //Pre-assign static registers according to CPU diagram
    registerBank[22]=0x00;
    registerBank[23]=0x01;
    registerBank[24]=0x02;
    registerBank[25]=0x03;
    registerBank[26]=0x04;
    registerBank[27]=0x08;
    registerBank[28]=0xF0;
    registerBank[29]=0xF6;
    registerBank[30]=0xFE;
    registerBank[31]=0xFF;
}

void CPUDataSection::clearControlSignals() noexcept
{
    //Set all control signals to disabled
    for(int it=0; it<controlSignals.length();it++)
    {
        controlSignals[it]=Enu::signalDisabled;
    }
}

void CPUDataSection::clearClockSignals() noexcept
{
    //Set all clock signals to low
    for(int it=0;it<clockSignals.length();it++)
    {
        clockSignals[it]=false;
    }
}

void CPUDataSection::clearRegisters() noexcept
{
    //Clear all registers in register bank, then restore the static values
    for(int it=0;it<registerBank.length();it++)
    {
        registerBank[it]=0;
    }
    presetStaticRegisters();

     //Clear all values from memory registers
    for(int it=0;it<memoryRegisters.length();it++)
    {
        memoryRegisters[it]=0;
    }
}

void CPUDataSection::clearErrors() noexcept
{
    hadDataError=false;
    errorMessage="";
}

void CPUDataSection::onStep() noexcept
{
    //If the error hasn't been handled by now, clear it
    clearErrors();
    stepTwoByte();
}

void CPUDataSection::onClock() noexcept
{
    //When the clock button is pushed, execute whatever control signals are set, and the clear their values
    onStep();
    clearClockSignals();
    clearControlSignals();
    emit controlClockChanged();
}

void CPUDataSection::onClearCPU() noexcept
{
    //Reset evey value associated with the CPU
    mainBusState = Enu::None;
    NZVCSbits=0;
    clearErrors();
    clearRegisters();
    clearClockSignals();
    clearControlSignals();
}
