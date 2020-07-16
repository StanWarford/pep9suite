#include "pep9asmcode.h"
#include "assembler/asmargument.h"

#include "isadefs.h"

UnaryInstruction::UnaryInstruction(const UnaryInstruction &other) : AsmCode(other)
{
    this->breakpoint = other.breakpoint;
    this->mnemonic = other.mnemonic;
}

NonUnaryInstruction::NonUnaryInstruction(const NonUnaryInstruction &other) : AsmCode(other)
{
    this->mnemonic = other.mnemonic;
    this->addressingMode = other.addressingMode;
    this->argument = other.argument;
    this->breakpoint = other.breakpoint;
}

UnaryInstruction &UnaryInstruction::operator=(UnaryInstruction other)
{
    swap(*this, other);
    return *this;
}

NonUnaryInstruction &NonUnaryInstruction::operator=(NonUnaryInstruction other)
{
    swap(*this, other);
    return *this;
}

AsmCode *UnaryInstruction::cloneAsmCode() const
{
    return new UnaryInstruction(*this);
}

AsmCode *NonUnaryInstruction::cloneAsmCode() const
{
    return new NonUnaryInstruction(*this);
}

void UnaryInstruction::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    objectCode.append(Pep9::ISA::opCodeMap.value(mnemonic));
}

void NonUnaryInstruction::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    int instructionSpecifier = Pep9::ISA::opCodeMap.value(mnemonic);
    if (Pep9::ISA::addrModeRequiredMap.value(mnemonic)) {
        instructionSpecifier += Pep9::ISA::aaaAddressField(addressingMode);
    }
    else {
        instructionSpecifier += Pep9::ISA::aAddressField(addressingMode);
    }
    objectCode.append(instructionSpecifier);
    int operandSpecifier = argument->getArgumentValue();
    objectCode.append(operandSpecifier / 256);
    objectCode.append(operandSpecifier % 256);
}


bool UnaryInstruction::hasBreakpoint() const
{
    return breakpoint;
}

void UnaryInstruction::setBreakpoint(bool b)
{
    breakpoint = b;
}

Pep9::ISA::EMnemonic UnaryInstruction::getMnemonic() const
{
    return this->mnemonic;
}

void UnaryInstruction::setMnemonic(Pep9::ISA::EMnemonic mnemonic)
{
    this->mnemonic = mnemonic;
}

bool UnaryInstruction::tracksTraceTags() const
{
    // With the inclusion of MOVASP, this will become conditional.
    return false;
}

bool NonUnaryInstruction::hasBreakpoint() const
{
    return breakpoint;
}

void NonUnaryInstruction::setBreakpoint(bool b)
{
    breakpoint = b;
}

bool NonUnaryInstruction::hasSymbolicOperand() const
{
    return dynamic_cast<SymbolRefArgument*>(argument.get()) != nullptr;
}

QSharedPointer<const SymbolEntry> NonUnaryInstruction::getSymbolicOperand() const
{
    return dynamic_cast<SymbolRefArgument*>(argument.get())->getSymbolValue();
}

QSharedPointer<AsmArgument>NonUnaryInstruction::getArgument() const
{
    return argument;
}

void NonUnaryInstruction::setArgument(QSharedPointer<AsmArgument> argument)
{
    this->argument = std::move(argument);
}

bool NonUnaryInstruction::tracksTraceTags() const
{
    switch(mnemonic){
    case Pep9::ISA::EMnemonic::ADDSP:
        return true;
    case Pep9::ISA::EMnemonic::SUBSP:
        return true;
    case Pep9::ISA::EMnemonic::CALL:
        if(hasSymbolicOperand()
           && getSymbolicOperand()->getName().compare("malloc", Qt::CaseInsensitive) == 0) {
            return true;
        }
        return false;
    default:
        return false;
    }
}

Pep9::ISA::EMnemonic NonUnaryInstruction::getMnemonic() const
{
    return mnemonic;
}

void NonUnaryInstruction::setMnemonic(Pep9::ISA::EMnemonic mnemonic)
{
    this->mnemonic = mnemonic;
}

Pep9::ISA::EAddrMode NonUnaryInstruction::getAddressingMode() const
{
    return addressingMode;
}

void NonUnaryInstruction::setAddressingMode(Pep9::ISA::EAddrMode addressingMode)
{
    this->addressingMode = addressingMode;
}

QString UnaryInstruction::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    // Potentially skip codegen
    QString codeStr;
    if(emitObjectCode) {
        codeStr = QString("%1").arg(Pep9::ISA::opCodeMap.value(mnemonic), 2, 16, QLatin1Char('0')).toUpper();
    }
    else {
        codeStr = "  ";
    }

    QString lineStr = QString("%1%2%3")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -7, QLatin1Char(' '))
                      .arg(getAssemblerSource());
    /*if(ForceShowAnnotation) {
        for(const auto& command : getTraceData()) {
            lineStr.append(QString(" %1").arg(command.toString()));
        }
    }*/
    return lineStr;
}

QString UnaryInstruction::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString mnemonStr = Pep9::ISA::enumToMnemonMap.value(mnemonic);
    QString lineStr = QString("%1%2%3")
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(mnemonStr, -8, QLatin1Char(' '))
                      .arg("            " + comment);
    return lineStr;
}

QString NonUnaryInstruction::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    int temp = Pep9::ISA::opCodeMap.value(mnemonic);
    temp += Pep9::ISA::addrModeRequiredMap.value(mnemonic) ? Pep9::ISA::aaaAddressField(addressingMode) : Pep9::ISA::aAddressField(addressingMode);
    // Potentially skip codegen
    QString codeStr;
    QString oprndNumStr;
    if(emitObjectCode) {
        codeStr = QString("%1").arg(temp, 2, 16, QLatin1Char('0')).toUpper();
        oprndNumStr = QString("%1").arg(argument->getArgumentValue(), 4, 16, QLatin1Char('0')).toUpper();
    }
    else {
        codeStr = "  ";
        oprndNumStr = " ";
    }

    QString lineStr = QString("%1%2%3%4")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -2)
                      .arg(oprndNumStr, -5, QLatin1Char(' '))
                      .arg(getAssemblerSource(), -29);
    /*if(ForceShowAnnotation) {
        for(const auto& command : getTraceData()) {
            lineStr.append(QString(" %1").arg(command.toString()));
        }
    }*/
    return lineStr;
}

QString NonUnaryInstruction::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString mnemonStr = Pep9::ISA::enumToMnemonMap.value(mnemonic);
    QString oprndStr = argument->getArgumentString();
    if (Pep9::ISA::addrModeRequiredMap.value(mnemonic)) {
        oprndStr.append("," + Pep9::ISA::intToAddrMode(addressingMode));
    }
    else if (addressingMode == Pep9::ISA::EAddrMode::X) {
        oprndStr.append("," + Pep9::ISA::intToAddrMode(addressingMode));
    }
    QString lineStr = QString("%1%2%3%4")
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(mnemonStr, -8, QLatin1Char(' '))
                      .arg(oprndStr, -12)
                      .arg(comment);
    return lineStr;
}

quint16 UnaryInstruction::objectCodeLength() const
{
    return 1;
}

quint16 NonUnaryInstruction::objectCodeLength() const
{
    return 3;
}
