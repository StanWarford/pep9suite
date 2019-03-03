// File: asm.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.
    
    Copyright (C) 2019  Matthew McRaven, Pepperdine University

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

#ifndef ASM_H
#define ASM_H

#include <QRegExp>
#include "enu.h"

class AsmCode; // Forward declaration for argument of processSourceLine.
class AsmProgram;
class AsmProgramManager;
class MainMemory;
struct StaticTraceInfo;
class SymbolTable;

struct BURNInfo
{
    int startROMAddress, burnValue, burnAddress, burnCount = 0;
};

namespace IsaParserHelper
{
    // Lexical tokens
    enum ELexicalToken
    {
        LT_ADDRESSING_MODE, LT_CHAR_CONSTANT, LT_COMMENT, LT_DEC_CONSTANT, LT_DOT_COMMAND,
        LT_EMPTY, LT_HEX_CONSTANT, LT_IDENTIFIER, LT_STRING_CONSTANT, LT_SYMBOL_DEF
    };

    // Parser state for the assembler FSM
    enum ParseState
    {
        PS_ADDRESSING_MODE, PS_CLOSE, PS_COMMENT, PS_DOT_ADDRSS, PS_DOT_ALIGN, PS_DOT_ASCII,
        PS_DOT_BLOCK, PS_DOT_BURN, PS_DOT_BYTE, PS_DOT_END, PS_DOT_EQUATE, PS_DOT_WORD,
        PS_FINISH, PS_INSTRUCTION, PS_START, PS_STRING, PS_SYMBOL_DEF
    };

    // Regular expressions for lexical analysis
    extern QRegExp rxAddrMode;
    extern QRegExp rxCharConst;
    extern QRegExp rxComment;
    extern QRegExp rxDecConst;
    extern QRegExp rxDotCommand;
    extern QRegExp rxHexConst;
    extern QRegExp rxIdentifier;
    extern QRegExp rxStringConst;

    // Regular expressions for trace tag analysis
    extern QRegExp rxFormatTag;
    extern QRegExp rxSymbolTag;
    extern QRegExp rxArrayMultiplier;
    extern QRegExp rxArrayTag;

    bool startsWithHexPrefix(QString str);
    // Post: Returns true if str starts with the characters 0x or 0X. Otherwise returns false.

    Enu::EAddrMode stringToAddrMode(QString str);
    // Post: Returns the addressing mode integer defined in Pep from its string representation.

    int charStringToInt(QString str);
    // Pre: str is enclosed in single quotes.
    // Post: Returns the ASCII integer value of the character accounting for \ quoted characters.

    int string2ArgumentToInt(QString str);
    // Pre: str is enclosed in double quotes and contains at most two possibly quoted characters.
    // Post: Returns the two-byte ASCII integer value for the string.

    void unquotedStringToInt(QString &str, int &value);
    // Pre: str is a character or string stripped of its single or double quotes.
    // Post: The sequence of characters representing the first possibly \ quoted character
    // is stripped from the beginning of str.
    // Post: value is the ASCII integer value of the first possibly \ quoted character.

    int byteStringLength(QString str);
    // Pre: str is a double quoted string.
    // Post: Returns the byte length of str accounting for possibly \ quoted characters.

}

class StructType;
class IsaAsm
{
    QSharedPointer<MainMemory> memDevice;
    AsmProgramManager& manager;
public:
    IsaAsm(QSharedPointer<MainMemory> memDevice, AsmProgramManager& manager);
    ~IsaAsm();

    bool assembleUserProgram(const QString& progText, QSharedPointer<AsmProgram> &progOut,
                             QList<QPair<int, QString>> &errList);
    // Pre: Operating system has been succesfully assembled, and
    // is already burned into memory.


    bool assembleOperatingSystem(const QString& progText, bool forceBurnAt0xFFFF,
                                 QSharedPointer<AsmProgram> &progOut, QList<QPair<int, QString>> &errList);
    // Attempt to assemble progText as the operating system.
    // The flag forceBurnAt0xFFFF requires the the argument of the .BURN have a value of 0xFFFF.
    // this is needed in Pep9Micro, as there is not enough flexibility in the static registers
    // to allow a relocatable operating system.

private:

    QPair<QSharedPointer<StructType>,QString> parseStruct(const SymbolTable& symTable, QString name, QStringList symbols,
                                           StaticTraceInfo& traceInfo);
    // Attempt to create a type definition for a struct from a list of symbol tags.
    // Returns a pointer (possibly null) and an error message.
    // In the case of successful parsing, the pointer will be non-non, and the string will be empty.
    // If the pointer is null, then there exists an error message.

    void handleTraceTags(const SymbolTable& symTable, StaticTraceInfo& traceInfo, QList<QSharedPointer<AsmCode>>& progList, QList<QPair<int, QString>> &errList);
    void relocateCode(QList<QSharedPointer<AsmCode>>& codeList, quint16 addressDelta);
    // Pre: codeList contains a valid program
    // Post: The address of every line of code in codeList is increase by addressDelta

    bool getToken(QString &sourceLine, IsaParserHelper::ELexicalToken &token, QString &tokenString);
    // Pre: sourceLine has one line of source code.
    // Post: If the next token is valid, the string of characters representing the next token are deleted from the
    // beginning of sourceLine and returned in tokenString, true is returned, and token is set to the token type.
    // Post: If false is returned, then tokenString is set to the lexical error message.

    bool processSourceLine(SymbolTable* symTable, BURNInfo& info, StaticTraceInfo& traceInfo, int& byteCount, QString sourceLine, int lineNum, AsmCode *&code, QString &errorString, bool &dotEndDetected, bool hasBreakpoint = false);
    // Pre: sourceLine has one line of source code.
    // Pre: lineNum is the line number of the source code.
    // Post: If the source line is valid, true is returned and code is set to the source code for the line.
    // Post: dotEndDetected is set to true if .END is processed. Otherwise it is set to false.
    // Post: Pep::byteCount is incremented by the number of bytes generated.
    // Post: If the source line is not valid, false is returned and errorString is set to the error message.
    // Post: Symbol format tags have been detected

    // Returns true if a primitive / array type tag is present. Does not look for symbol tags
    static bool hasTypeTag(QString comment);
    // Return the string representation of a present primitive / array type tag.
    static QString extractTypeTags(QString comment);
    // Returns if formatTag contains a valid primitive tag (e.g. #2d => true, #6f => false).
    static bool hasPrimitiveType(QString formatTag);
    // Returns if formatTag contains a valid array tag (e.g. #2d4a => true, #2d4 => false,
    // #6f6a => false).
    static bool hasArrayType(QString formatTag);
    // Returns if formatTag could be a valid struct type tag,
    // or is a symbol list for ADDSP/SUBSP
    static bool hasSymbolTag(QString formatTag);
    static Enu::ESymbolFormat primitiveType(QString formatTag);
    // Pre: formatTag is a valid format trace tag.
    // Post: Returns the enumerated trace tag format type.
    static QPair<quint8,  Enu::ESymbolFormat> arrayType(QString formatTag);
    // Pre: formatTag is a valid format array trace tag.
    // Post: Returns a pair of the array's length and the enumerated trace tag format type.

    // If the line contains a list of symbol tags, return all present symbol tags
    static QStringList extractTagList(QString comment);

};

#endif // ASM_H
