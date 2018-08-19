// File: asm.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.
    
    Copyright (C) 2009  J. Stanley Warford, Pepperdine University

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
#include "isaasm.h"
#include "asmargument.h"
#include "asmcode.h"
#include "symboltable.h"
#include "symbolentry.h"
#include "symbolvalue.h"
// Regular expressions for lexical analysis
QRegExp IsaAsm::rxAddrMode("^((,)(\\s*)(i|d|x|n|s(?![fx])|sx(?![f])|sf|sfx){1}){1}");
QRegExp IsaAsm::rxCharConst("^((\')(?![\'])(([^\'\\\\]){1}|((\\\\)([\'|b|f|n|r|t|v|\"|\\\\]))|((\\\\)(([x|X])([0-9|A-F|a-f]{2}))))(\'))");
QRegExp IsaAsm::rxComment("^((;{1})(.)*)");
QRegExp IsaAsm::rxDecConst("^((([+|-]{0,1})([0-9]+))|^(([1-9])([0-9]*)))");
QRegExp IsaAsm::rxDotCommand("^((.)(([A-Z|a-z]{1})(\\w)*))");
QRegExp IsaAsm::rxHexConst("^((0(?![x|X]))|((0)([x|X])([0-9|A-F|a-f])+)|((0)([0-9]+)))");
QRegExp IsaAsm::rxIdentifier("^((([A-Z|a-z|_]{1})(\\w*))(:){0,1})");
QRegExp IsaAsm::rxStringConst("^((\")((([^\"\\\\])|((\\\\)([\'|b|f|n|r|t|v|\"|\\\\]))|((\\\\)(([x|X])([0-9|A-F|a-f]{2}))))*)(\"))");

// Regular expressions for trace tag analysis
QRegExp IsaAsm::rxFormatTag("(#((1c)|(1d)|(1h)|(2d)|(2h))((\\d)+a)?(\\s|$))");
QRegExp IsaAsm::rxSymbolTag("#([a-zA-Z][a-zA-Z0-9]{0,7})");
QRegExp IsaAsm::rxArrayMultiplier("((\\d)+)a");

bool IsaAsm::getToken(QString &sourceLine, ELexicalToken &token, QString &tokenString)
{
    sourceLine = sourceLine.trimmed();
    if (sourceLine.length() == 0) {
        token = LT_EMPTY;
        tokenString = "";
        return true;
    }
    QChar firstChar = sourceLine[0];
    rxAddrMode.setCaseSensitivity (Qt::CaseInsensitive);  // Make rxAddrMode not case sensitive
    if (firstChar == ',') {
        if (rxAddrMode.indexIn(sourceLine) == -1) {
            tokenString = ";ERROR: Malformed addressing mode.";
            return false;
        }
        token = LT_ADDRESSING_MODE;
        tokenString = rxAddrMode.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar == '\'') {
        if (rxCharConst.indexIn(sourceLine) == -1) {
            tokenString = ";ERROR: Malformed character constant.";
            return false;
        }
        token = LT_CHAR_CONSTANT;
        tokenString = rxCharConst.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar == ';') {
        if (rxComment.indexIn(sourceLine) == -1) {
            // This error should not occur, as any characters are allowed in a comment.
            tokenString = ";ERROR: Malformed comment";
            return false;
        }
        token = LT_COMMENT;
        tokenString = rxComment.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (startsWithHexPrefix(sourceLine)) {
        if (rxHexConst.indexIn(sourceLine) == -1) {
            tokenString = ";ERROR: Malformed hex constant.";
            return false;
        }
        token = LT_HEX_CONSTANT;
        tokenString = rxHexConst.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if ((firstChar.isDigit() || firstChar == '+' || firstChar == '-')) {
        if (rxDecConst.indexIn(sourceLine) == -1) {
            tokenString = ";ERROR: Malformed decimal constant.";
            return false;
        }
        token = LT_DEC_CONSTANT;
        tokenString = rxDecConst.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar == '.') {
        if (rxDotCommand.indexIn(sourceLine) == -1) {
            tokenString = ";ERROR: Malformed dot command.";
            return false;
        }
        token = LT_DOT_COMMAND;
        tokenString = rxDotCommand.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar.isLetter() || firstChar == '_') {
        if (rxIdentifier.indexIn(sourceLine) == -1) {
            // This error should not occur, as one-character identifiers are valid.
            tokenString = ";ERROR: Malformed identifier.";
            return false;
        }
        tokenString = rxIdentifier.capturedTexts()[0];
        token = tokenString.endsWith(':') ? LT_SYMBOL_DEF : LT_IDENTIFIER;
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar == '\"') {
        if (rxStringConst.indexIn(sourceLine) == -1) {
            tokenString = ";ERROR: Malformed string constant.";
            return false;
        }
        token = LT_STRING_CONSTANT;
        tokenString = rxStringConst.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    tokenString = ";ERROR: Syntax error.";
    return false;
}

QList<QString> IsaAsm::listOfReferencedSymbols;
QList<int> IsaAsm::listOfReferencedSymbolLineNums;

bool IsaAsm::startsWithHexPrefix(QString str)
{
    if (str.length() < 2) return false;
    if (str[0] != '0') return false;
    if (str[1] == 'x' || str[1] == 'X') return true;
    return false;
}

Enu::EAddrMode IsaAsm::stringToAddrMode(QString str)
{
    str.remove(0, 1); // Remove the comma.
    str = str.trimmed().toUpper();
    if (str == "I") return Enu::EAddrMode::I;
    if (str == "D") return Enu::EAddrMode::D;
    if (str == "N") return Enu::EAddrMode::N;
    if (str == "S") return Enu::EAddrMode::S;
    if (str == "SF") return Enu::EAddrMode::SF;
    if (str == "X") return Enu::EAddrMode::X;
    if (str == "SX") return Enu::EAddrMode::SX;
    if (str == "SFX") return Enu::EAddrMode::SFX;
    return Enu::EAddrMode::NONE;
}

int IsaAsm::charStringToInt(QString str)
{
    str.remove(0, 1); // Remove the leftmost single quote.
    str.chop(1); // Remove the rightmost single quote.
    int value;
    IsaAsm::unquotedStringToInt(str, value);
    return value;
}

int IsaAsm::string2ArgumentToInt(QString str) {
    int valueA, valueB;
    str.remove(0, 1); // Remove the leftmost double quote.
    str.chop(1); // Remove the rightmost double quote.
    IsaAsm::unquotedStringToInt(str, valueA);
    if (str.length() == 0) {
        return valueA;
    }
    else {
        IsaAsm::unquotedStringToInt(str, valueB);
        valueA = 256 * valueA + valueB;
        if (valueA < 0) {
            valueA += 65536; // Stored as two-byte unsigned.
        }
        return valueA;
    }
}

void IsaAsm::unquotedStringToInt(QString &str, int &value)
{
    QString s;
    if (str.startsWith("\\x") || str.startsWith("\\X")) {
        str.remove(0, 2); // Remove the leading \x or \X
        s = str.left(2);
        str.remove(0, 2); // Get the two-char hex number
        bool ok;
        value = s.toInt(&ok, 16);
    }
    else if (str.startsWith("\\")) {
        str.remove(0, 1); // Remove the leading bash
        s = str.left(1);
        str.remove(0,1);
        if (s == "b") { // backspace
            value = 8;
        }
        else if (s == "f") { // form feed
            value = 12;
        }
        else if (s == "n") { // line feed (new line)
            value = 10;
        }
        else if (s == "r") { // carriage return
            value = 13;
        }
        else if (s == "t") { // horizontal tab
            value = 9;
        }
        else if (s == "v") { // vertical tab
            value = 11;
        }
        else {
            value = QChar(s[0]).toLatin1();
        }
    }
    else {
        s = str.left(1);
        str.remove(0, 1);
        value = QChar(s[0]).toLatin1();
    }
    value += value < 0 ? 256 : 0;
}

int IsaAsm::byteStringLength(QString str)
{
    str.remove(0, 1); // Remove the leftmost double quote.
    str.chop(1); // Remove the rightmost double quote.
    int length = 0;
    while (str.length() > 0) {
        if (str.startsWith("\\x") || str.startsWith("\\X")) {
            str.remove(0, 4); // Remove the \xFF
        }
        else if (str.startsWith("\\")) {
            str.remove(0, 2); // Remove the quoted character
        }
        else {
            str.remove(0, 1); // Remove the single character
        }
        length++;
    }
    return length;
}

Enu::ESymbolFormat IsaAsm::formatTagType(QString formatTag) {
    if (formatTag.startsWith("#1c")) return Enu::ESymbolFormat::F_1C;
    if (formatTag.startsWith("#1d")) return Enu::ESymbolFormat::F_1D;
    if (formatTag.startsWith("#2d")) return Enu::ESymbolFormat::F_2D;
    if (formatTag.startsWith("#1h")) return Enu::ESymbolFormat::F_1H;
    if (formatTag.startsWith("#2h")) return Enu::ESymbolFormat::F_2H;
    return Enu::ESymbolFormat::F_NONE; // Should not occur
}

int IsaAsm::tagNumBytes(Enu::ESymbolFormat symbolFormat) {
    switch (symbolFormat) {
    case Enu::ESymbolFormat::F_1C: return 1;
    case Enu::ESymbolFormat::F_1D: return 1;
    case Enu::ESymbolFormat::F_2D: return 2;
    case Enu::ESymbolFormat::F_1H: return 1;
    case Enu::ESymbolFormat::F_2H: return 2;
    case Enu::ESymbolFormat::F_NONE: return 0;
    default: return -1; // Should not occur.
    }
}

int IsaAsm::formatMultiplier(QString formatTag) {
    int pos = IsaAsm::rxArrayMultiplier.indexIn(formatTag);
    if (pos > -1) {
        QString multiplierTag = IsaAsm::rxArrayMultiplier.cap(0);
        multiplierTag.chop(1); // Remove the last character 'a' from the array tag.
        return multiplierTag.toInt();
    }
    else {
        return 1;
    }
}

bool IsaAsm::processSourceLine(SymbolTable* symTable, ROMInfo& info, int& byteCount, QString sourceLine, int lineNum, AsmCode *&code, QString &errorString, bool &dotEndDetected, bool hasBreakpoint)
{
    IsaAsm::ELexicalToken token; // Passed to getToken.
    QString tokenString; // Passed to getToken.
    QString localSymbolDef = ""; // Saves symbol definition for processing in the following state.
    Enu::EMnemonic localEnumMnemonic; // Key to Pep:: table lookups.

    // The concrete code objects asssigned to code.
    UnaryInstruction *unaryInstruction = nullptr;
    NonUnaryInstruction *nonUnaryInstruction = nullptr;
    DotAddrss *dotAddrss = nullptr;
    DotAlign *dotAlign = nullptr;
    DotAscii *dotAscii = nullptr;
    DotBlock *dotBlock = nullptr;
    DotBurn *dotBurn = nullptr;
    DotByte *dotByte = nullptr;
    DotEnd *dotEnd = nullptr;
    DotEquate *dotEquate = nullptr;
    DotWord *dotWord = nullptr;
    CommentOnly *commentOnly = nullptr;
    BlankLine *blankLine = nullptr;
    dotEndDetected = false;
    IsaAsm::ParseState state = IsaAsm::PS_START;
    do {
        if (!getToken(sourceLine, token, tokenString)) {
            errorString = tokenString;
            return false;
        }
        switch (state) {
        case IsaAsm::PS_START:
            if (token == IsaAsm::LT_IDENTIFIER) {
                if (Pep::mnemonToEnumMap.contains(tokenString.toUpper())) {
                    localEnumMnemonic = Pep::mnemonToEnumMap.value(tokenString.toUpper());
                    if (Pep::isUnaryMap.value(localEnumMnemonic)) {
                        unaryInstruction = new UnaryInstruction;
                        unaryInstruction->mnemonic = localEnumMnemonic;
                        unaryInstruction->breakpoint = hasBreakpoint;
                        code = unaryInstruction;
                        code->memAddress = byteCount;                        
                        byteCount += 1; // One byte generated for unary instruction.
                        state = IsaAsm::PS_CLOSE;
                    }
                    else {
                        nonUnaryInstruction = new NonUnaryInstruction;
                        nonUnaryInstruction->mnemonic = localEnumMnemonic;
                        nonUnaryInstruction->breakpoint = hasBreakpoint;
                        code = nonUnaryInstruction;
                        code->memAddress = byteCount;
                        byteCount += 3; // Three bytes generated for nonunary instruction.
                        state = IsaAsm::PS_INSTRUCTION;
                    }
                }
                else {
                    errorString = ";ERROR: Invalid mnemonic.";
                    return false;
                }
            }
            else if (token == IsaAsm::LT_DOT_COMMAND) {
                tokenString.remove(0, 1); // Remove the period
                tokenString = tokenString.toUpper();
                if (tokenString == "ADDRSS") {
                    dotAddrss = new DotAddrss;
                    code = dotAddrss;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_ADDRSS;
                }
                else if (tokenString == "ALIGN") {
                    dotAlign = new DotAlign;
                    code = dotAlign;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_ALIGN;
                }
                else if (tokenString == "ASCII") {
                    dotAscii = new DotAscii;
                    code = dotAscii;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_ASCII;
                }
                else if (tokenString == "BLOCK") {
                    dotBlock = new DotBlock;
                    code = dotBlock;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_BLOCK;
                }
                else if (tokenString == "BURN") {
                    dotBurn = new DotBurn;
                    code = dotBurn;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_BURN;
                }
                else if (tokenString == "BYTE") {
                    dotByte = new DotByte;
                    code = dotByte;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_BYTE;
                }
                else if (tokenString == "END") {
                    dotEnd = new DotEnd;
                    code = dotEnd;
                    // End symbol does not have a memory address
                    code->memAddress = byteCount;
                    dotEndDetected = true;
                    state = IsaAsm::PS_DOT_END;
                }
                else if (tokenString == "EQUATE") {
                    dotEquate = new DotEquate;
                    code = dotEquate;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_EQUATE;
                }
                else if (tokenString == "WORD") {
                    dotWord = new DotWord;
                    code = dotWord;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_WORD;
                }
                else {
                    errorString = ";ERROR: Invalid dot command.";
                    return false;
                }
            }
            else if (token == IsaAsm::LT_SYMBOL_DEF) {
                tokenString.chop(1); // Remove the colon
                if (tokenString.length() > 8) {
                    errorString = ";ERROR: Symbol " + tokenString + " cannot have more than eight characters.";
                    return false;
                }
                // If the symbol is already defined, then there is an compilation error.
                if (symTable->exists(tokenString) && symTable->getValue(tokenString)->isDefined()) {
                    symTable->getValue(tokenString)->setMultiplyDefined();
                    errorString = ";ERROR: Symbol " + tokenString + " was previously defined.";
                    return false;
                }
                localSymbolDef = tokenString;
                if(!symTable->exists(tokenString)) symTable->insertSymbol(tokenString);
                symTable->setValue(localSymbolDef, QSharedPointer<SymbolValueLocation>::create(byteCount));
                state = IsaAsm::PS_SYMBOL_DEF;
            }
            else if (token == IsaAsm::LT_COMMENT) {
                commentOnly = new CommentOnly;
                commentOnly->comment = tokenString;
                code = commentOnly;
                // Comments don't have a memory address
                code->memAddress = -1;
                state = IsaAsm::PS_COMMENT;
            }
            else if (token == IsaAsm::LT_EMPTY) {
                blankLine = new BlankLine;
                code = blankLine;
                // Neither do empty lines
                code->memAddress = -1;
                code->sourceCodeLine = lineNum;
                state = IsaAsm::PS_FINISH;
            }
            else {
                errorString = ";ERROR: Line must start with symbol definition, mnemonic, dot command, or comment.";
                return false;
            }
            break;

        case IsaAsm::PS_SYMBOL_DEF:
            if (token == IsaAsm::LT_IDENTIFIER){
                if (Pep::mnemonToEnumMap.contains(tokenString.toUpper())) {
                    localEnumMnemonic = Pep::mnemonToEnumMap.value(tokenString.toUpper());
                    if (Pep::isUnaryMap.value(localEnumMnemonic)) {
                        unaryInstruction = new UnaryInstruction;
                        unaryInstruction->symbolEntry = symTable->getValue(localSymbolDef);
                        unaryInstruction->mnemonic = localEnumMnemonic;
                        unaryInstruction->breakpoint = hasBreakpoint;
                        code = unaryInstruction;
                        code->memAddress = byteCount;
                        byteCount += 1; // One byte generated for unary instruction.
                        state = IsaAsm::PS_CLOSE;
                    }
                    else {
                        nonUnaryInstruction = new NonUnaryInstruction;
                        nonUnaryInstruction->symbolEntry = symTable->getValue(localSymbolDef);
                        nonUnaryInstruction->mnemonic = localEnumMnemonic;
                        nonUnaryInstruction->breakpoint = hasBreakpoint;
                        code = nonUnaryInstruction;
                        code->memAddress = byteCount;
                        byteCount += 3; // Three bytes generated for unary instruction.
                        state = IsaAsm::PS_INSTRUCTION;
                    }
                }
                else {
                    errorString = ";ERROR: Invalid mnemonic.";
                    return false;
                }
            }
            else if (token == IsaAsm::LT_DOT_COMMAND) {
                tokenString.remove(0, 1); // Remove the period
                tokenString = tokenString.toUpper();
                if (tokenString == "ADDRSS") {
                    dotAddrss = new DotAddrss;
                    dotAddrss->symbolEntry = symTable->getValue(localSymbolDef);
                    code = dotAddrss;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_ADDRSS;
                }
                else if (tokenString == "ASCII") {
                    dotAscii = new DotAscii;
                    dotAscii->symbolEntry = symTable->getValue(localSymbolDef);
                    code = dotAscii;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_ASCII;
                }
                else if (tokenString == "BLOCK") {
                    dotBlock = new DotBlock;
                    dotBlock->symbolEntry = symTable->getValue(localSymbolDef);
                    code = dotBlock;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_BLOCK;
                }
                else if (tokenString == "BURN") {
                    dotBurn = new DotBurn;
                    dotBurn->symbolEntry = symTable->getValue(localSymbolDef);
                    code = dotBurn;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_BURN;
                }
                else if (tokenString == "BYTE") {
                    dotByte = new DotByte;
                    dotByte->symbolEntry = symTable->getValue(localSymbolDef);
                    code = dotByte;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_BYTE;
                }
                else if (tokenString == "END") {
                    dotEnd = new DotEnd;
                    dotEnd->symbolEntry = symTable->getValue(localSymbolDef);
                    code = dotEnd;
                    code->memAddress = byteCount;
                    dotEndDetected = true;
                    state = IsaAsm::PS_DOT_END;
                }
                else if (tokenString == "EQUATE") {
                    dotEquate = new DotEquate;
                    dotEquate->symbolEntry = symTable->getValue(localSymbolDef);
                    code = dotEquate;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_EQUATE;
                }
                else if (tokenString == "WORD") {
                    dotWord = new DotWord;
                    dotWord->symbolEntry = symTable->getValue(localSymbolDef);
                    code = dotWord;
                    code->memAddress = byteCount;
                    state = IsaAsm::PS_DOT_WORD;
                }
                else {
                    errorString = ";ERROR: Invalid dot command.";
                    return false;
                }
            }
            else {
                errorString = ";ERROR: Must have mnemonic or dot command after symbol definition.";
                return false;
            }
            break;

        case IsaAsm::PS_INSTRUCTION:
            if (token == IsaAsm::LT_IDENTIFIER) {
                if (tokenString.length() > 8) {
                    errorString = ";ERROR: Symbol " + tokenString + " cannot have more than eight characters.";
                    return false;
                }
                if(!symTable->exists(tokenString)) symTable->insertSymbol(tokenString);
                nonUnaryInstruction->argument = new SymbolRefArgument(symTable->getValue(tokenString));
                IsaAsm::listOfReferencedSymbols.append(tokenString);
                IsaAsm::listOfReferencedSymbolLineNums.append(lineNum);
                state = IsaAsm::PS_ADDRESSING_MODE;
            }
            else if (token == IsaAsm::LT_STRING_CONSTANT) {
                if (IsaAsm::byteStringLength(tokenString) > 2) {
                    errorString = ";ERROR: String operands must have length at most two.";
                    return false;
                }
                nonUnaryInstruction->argument = new StringArgument(tokenString);
                state = IsaAsm::PS_ADDRESSING_MODE;
            }
            else if (token == IsaAsm::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                int value = tokenString.toInt(&ok, 16);
                if (value < 65536) {
                    nonUnaryInstruction->argument = new HexArgument(value);
                    state = IsaAsm::PS_ADDRESSING_MODE;
                }
                else {
                    errorString = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
                    return false;
                }
            }
            else if (token == IsaAsm::LT_DEC_CONSTANT) {
                bool ok;
                int value = tokenString.toInt(&ok, 10);
                if ((-32768 <= value) && (value <= 65535)) {
                    if (value < 0) {
                        value += 65536; // Stored as two-byte unsigned.
                        nonUnaryInstruction->argument = new DecArgument(value);
                    }
                    else {
                        nonUnaryInstruction->argument = new UnsignedDecArgument(value);
                    }
                    state = IsaAsm::PS_ADDRESSING_MODE;
                }
                else {
                    errorString = ";ERROR: Decimal constant is out of range (-32768..65535).";
                    return false;
                }
            }
            else if (token == IsaAsm::LT_CHAR_CONSTANT) {
                nonUnaryInstruction->argument = new CharArgument(tokenString);
                state = IsaAsm::PS_ADDRESSING_MODE;
            }
            else {
                errorString = ";ERROR: Operand specifier expected after mnemonic.";
                return false;
            }
            break;

        case IsaAsm::PS_ADDRESSING_MODE:
            if (token == IsaAsm::LT_ADDRESSING_MODE) {
                Enu::EAddrMode addrMode = IsaAsm::stringToAddrMode(tokenString);
                if ((static_cast<int>(addrMode) & Pep::addrModesMap.value(localEnumMnemonic)) == 0) { // Nested parens required.
                    errorString = ";ERROR: Illegal addressing mode for this instruction.";
                    return false;
                }
                nonUnaryInstruction->addressingMode = addrMode;
                state = IsaAsm::PS_CLOSE;
            }
            else if (Pep::addrModeRequiredMap.value(localEnumMnemonic)) {
                errorString = ";ERROR: Addressing mode required for this instruction.";
                return false;
            }
            else { // Must be branch type instruction with no addressing mode. Assign default addressing mode.
                nonUnaryInstruction->addressingMode = Enu::EAddrMode::I;
                if (token == IsaAsm::LT_COMMENT) {
                    code->comment = tokenString;
                    state = IsaAsm::PS_COMMENT;
                }
                else if (token == IsaAsm::LT_EMPTY) {
                    code->sourceCodeLine = lineNum;
                    state = IsaAsm::PS_FINISH;
                }
                else {
                    errorString = ";ERROR: Comment expected following instruction.";
                    return false;
                }
            }
            break;

        case IsaAsm::PS_DOT_ADDRSS:
            if (token == IsaAsm::LT_IDENTIFIER) {
                if (tokenString.length() > 8) {
                    errorString = ";ERROR: Symbol " + tokenString + " cannot have more than eight characters.";
                    return false;
                }
                if(!symTable->exists(tokenString)) symTable->insertSymbol(tokenString);
                dotAddrss->argument = new SymbolRefArgument(symTable->getValue(tokenString));
                IsaAsm::listOfReferencedSymbols.append(tokenString);
                IsaAsm::listOfReferencedSymbolLineNums.append(lineNum);
                byteCount += 2;
                state = IsaAsm::PS_CLOSE;
            }
            else {
                errorString = ";ERROR: .ADDRSS requires a symbol argument.";
                return false;
            }
            break;

        case IsaAsm::PS_DOT_ALIGN:
            if (token == IsaAsm::LT_DEC_CONSTANT) {
                bool ok;
                int value = tokenString.toInt(&ok, 10);
                if (value == 2 || value == 4 || value == 8) {
                    int numBytes = (value - byteCount % value) % value;
                    dotAlign->argument = new UnsignedDecArgument(value);
                    dotAlign->numBytesGenerated = new UnsignedDecArgument(numBytes);
                    byteCount += numBytes;
                    state = IsaAsm::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Decimal constant is out of range (2, 4, 8).";
                    return false;
                }
            }
            else {
                errorString = ";ERROR: .ALIGN requires a decimal constant 2, 4, or 8.";
                return false;
            }
            break;

        case IsaAsm::PS_DOT_ASCII:
            if (token == IsaAsm::LT_STRING_CONSTANT) {
                dotAscii->argument = new StringArgument(tokenString);
                byteCount += IsaAsm::byteStringLength(tokenString);
                state = IsaAsm::PS_CLOSE;
            }
            else {
                errorString = ";ERROR: .ASCII requires a string constant argument.";
                return false;
            }
            break;

        case IsaAsm::PS_DOT_BLOCK:
            if (token == IsaAsm::LT_DEC_CONSTANT) {
                bool ok;
                int value = tokenString.toInt(&ok, 10);
                if ((0 <= value) && (value <= 65535)) {
                    if (value < 0) {
                        value += 65536; // Stored as two-byte unsigned.
                        dotBlock->argument = new DecArgument(value);
                    }
                    else {
                        dotBlock->argument = new UnsignedDecArgument(value);
                    }
                    byteCount += value;
                    state = IsaAsm::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Decimal constant is out of range (0..65535).";
                    return false;
                }
            }
            else if (token == IsaAsm::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                int value = tokenString.toInt(&ok, 16);
                if (value < 65536) {
                    dotBlock->argument = new HexArgument(value);
                    byteCount += value;
                    state = IsaAsm::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
                    return false;
                }
            }
            else {
                errorString = ";ERROR: .BLOCK requires a decimal or hex constant argument.";
                return false;
            }
            break;

        case IsaAsm::PS_DOT_BURN:
            if (token == IsaAsm::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                int value = tokenString.toInt(&ok, 16);
                if (value < 65536) {
                    dotBurn->argument = new HexArgument(value);
                    info.burnCount++;
                    info.burnValue = value;
                    info.startROMAddress = byteCount;
                    state = IsaAsm::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
                    return false;
                }
            }
            else {
                errorString = ";ERROR: .BURN requires a hex constant argument.";
                return false;
            }
            break;

        case IsaAsm::PS_DOT_BYTE:
            if (token == IsaAsm::LT_CHAR_CONSTANT) {
                dotByte->argument = new CharArgument(tokenString);
                byteCount += 1;
                state = IsaAsm::PS_CLOSE;
            }
            else if (token == IsaAsm::LT_DEC_CONSTANT) {
                bool ok;
                int value = tokenString.toInt(&ok, 10);
                if ((-128 <= value) && (value <= 255)) {
                    if (value < 0) {
                        value += 256; // value stored as one-byte unsigned.
                    }
                    dotByte->argument = new DecArgument(value);
                    byteCount += 1;
                    state = IsaAsm::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Decimal constant is out of byte range (-128..255).";
                    return false;
                }
            }
            else if (token == IsaAsm::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                int value = tokenString.toInt(&ok, 16);
                if (value < 256) {
                    dotByte->argument = new HexArgument(value);
                    byteCount += 1;
                    state = IsaAsm::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Hex constant is out of byte range (0x00..0xFF).";
                    return false;
                }
            }
            else if (token == IsaAsm::LT_STRING_CONSTANT) {
                if (IsaAsm::byteStringLength(tokenString) > 1) {
                    errorString = ";ERROR: .BYTE string operands must have length one.";
                    return false;
                }
                dotByte->argument = new StringArgument(tokenString);
                byteCount += 1;
                state = IsaAsm::PS_CLOSE;
            }
            else {
                errorString = ";ERROR: .BYTE requires a char, dec, hex, or string constant argument.";
                return false;
            }
            break;

        case IsaAsm::PS_DOT_END:
            if (token == IsaAsm::LT_COMMENT) {
                dotEnd->comment = tokenString;
                code->sourceCodeLine = lineNum;
                state = IsaAsm::PS_FINISH;
            }
            else if (token == IsaAsm::LT_EMPTY) {
                dotEnd->comment = "";
                code->sourceCodeLine = lineNum;
                state = IsaAsm::PS_FINISH;
            }
            else {
                errorString = ";ERROR: Only a comment can follow .END.";
                return false;
            }
            break;

        case IsaAsm::PS_DOT_EQUATE:
            if (dotEquate->symbolEntry == nullptr) {
                errorString = ";ERROR: .EQUATE must have a symbol definition.";
                return false;
            }
            else if (token == IsaAsm::LT_DEC_CONSTANT) {
                bool ok;
                int value = tokenString.toInt(&ok, 10);
                if ((-32768 <= value) && (value <= 65535)) {

                    if (value < 0) {
                        value += 65536; // Stored as two-byte unsigned.
                        dotEquate->argument = new DecArgument(value);
                    }
                    else {
                        dotEquate->argument = new UnsignedDecArgument(value);
                    }
                    dotEquate->symbolEntry->setValue( QSharedPointer<SymbolValueNumeric>::create(value));
                    state = IsaAsm::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Decimal constant is out of range (-32768..65535).";
                    return false;
                }
            }
            else if (token == IsaAsm::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                int value = tokenString.toInt(&ok, 16);
                if (value < 65536) {
                    dotEquate->argument = new HexArgument(value);
                    dotEquate->symbolEntry->setValue( QSharedPointer<SymbolValueNumeric>::create(value));
                    state = IsaAsm::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
                    return false;
                }
            }
            else if (token == IsaAsm::LT_STRING_CONSTANT) {
                if (IsaAsm::byteStringLength(tokenString) > 2) {
                    errorString = ";ERROR: .EQUATE string operand must have length at most two.";
                    return false;
                }
                dotEquate->argument = new StringArgument(tokenString);
                dotEquate->symbolEntry->setValue( QSharedPointer<SymbolValueNumeric>::create(IsaAsm::string2ArgumentToInt(tokenString)));
                state = IsaAsm::PS_CLOSE;
            }
            else if (token == IsaAsm::LT_CHAR_CONSTANT) {
                dotEquate->argument = new CharArgument(tokenString);
                dotEquate->symbolEntry->setValue( QSharedPointer<SymbolValueNumeric>::create(IsaAsm::charStringToInt(tokenString)));
                state = IsaAsm::PS_CLOSE;
            }
            else {
                errorString = ";ERROR: .EQUATE requires a dec, hex, or string constant argument.";
                return false;
            }
            break;

        case IsaAsm::PS_DOT_WORD:
            if (token == IsaAsm::LT_CHAR_CONSTANT) {
                dotWord->argument = new CharArgument(tokenString);
                byteCount += 2;
                state = IsaAsm::PS_CLOSE;
            }
            else if (token == IsaAsm::LT_DEC_CONSTANT) {
                bool ok;
                int value = tokenString.toInt(&ok, 10);
                if ((-32768 <= value) && (value < 65536)) {

                    if (value < 0) {
                        value += 65536; // Stored as two-byte unsigned.
                        dotWord->argument = new DecArgument(value);
                    }
                    else {
                        dotWord->argument = new UnsignedDecArgument(value);
                    }
                    byteCount += 2;
                    state = IsaAsm::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Decimal constant is out of range (-32768..65535).";
                    return false;
                }
            }
            else if (token == IsaAsm::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                int value = tokenString.toInt(&ok, 16);
                if (value < 65536) {
                    dotWord->argument = new HexArgument(value);
                    byteCount += 2;
                    state = IsaAsm::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
                    return false;
                }
            }
            else if (token == IsaAsm::LT_STRING_CONSTANT) {
                if (IsaAsm::byteStringLength(tokenString) > 2) {
                    errorString = ";ERROR: .WORD string operands must have length at most two.";
                    return false;
                }
                dotWord->argument = new StringArgument(tokenString);
                byteCount += 2;
                state = IsaAsm::PS_CLOSE;
            }
            else {
                errorString = ";ERROR: .WORD requires a char, dec, hex, or string constant argument.";
                return false;
            }
            break;

        case IsaAsm::PS_CLOSE:
            if (token == IsaAsm::LT_EMPTY) {
                code->sourceCodeLine = lineNum;
                state = IsaAsm::PS_FINISH;
            }
            else if (token == IsaAsm::LT_COMMENT) {
                code->comment = tokenString;
                state = IsaAsm::PS_COMMENT;
            }
            else {
                errorString = ";ERROR: Comment expected following instruction.";
                return false;
            }
            break;

        case IsaAsm::PS_COMMENT:
            if (token == IsaAsm::LT_EMPTY) {
                code->sourceCodeLine = lineNum;
                state = IsaAsm::PS_FINISH;
            }
            else {
                // This error should not occur, as all characters are allowed in comments.
                errorString = ";ERROR: Problem detected after comment.";
                return false;
            }
            break;

        default:
            break;
        }
    }
    while (state != IsaAsm::PS_FINISH);
    return true;
}
