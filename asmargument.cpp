#include "asmargument.h"
#include "symboltable.h"
#include "symbolentry.h"
#include "symbolvalue.h"
SymbolRefArgument::SymbolRefArgument(QSharedPointer<SymbolEntry> sRefValue): symbolRefValue(sRefValue)
{

}

int SymbolRefArgument::getArgumentValue() const
{
    return symbolRefValue->getValue();
}

QString SymbolRefArgument::getArgumentString() const
{
    return symbolRefValue->getName();
}

HexArgument::HexArgument(int hValue) :hexValue(hValue)
{


}

int HexArgument::getArgumentValue() const
{
    return hexValue;
}

QString HexArgument::getArgumentString() const
{
    return "0x" + QString("%1").arg(hexValue, 4, 16, QLatin1Char('0')).toUpper();
}

StringArgument::StringArgument(QString sValue): stringValue(sValue)
{

}

int StringArgument::getArgumentValue() const
{
    return IsaAsm::string2ArgumentToInt(stringValue);
}

QString StringArgument::getArgumentString() const
{
    return stringValue;
}

CharArgument::CharArgument(QString cValue) : charValue(cValue)
{

}

int CharArgument::getArgumentValue() const
{
    return IsaAsm::charStringToInt(charValue);
}

QString CharArgument::getArgumentString() const
{
    return charValue;
}

DecArgument::DecArgument(int dValue): decValue(dValue)
{

}

int DecArgument::getArgumentValue() const
{
    return decValue;
}

QString DecArgument::getArgumentString() const
{
    int temp = decValue >= 32768 ? decValue - 65536 : decValue;
    return QString("%1").arg(temp);
}

UnsignedDecArgument::UnsignedDecArgument(int dValue): decValue(dValue)
{

}

int UnsignedDecArgument::getArgumentValue() const
{
    return decValue;
}

QString UnsignedDecArgument::getArgumentString() const
{
    return QString("%1").arg(decValue);
}
