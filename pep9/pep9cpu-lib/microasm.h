// File: microasm.h
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
#ifndef MICROASM_H
#define MICROASM_H

#include <QRegExp>
#include <QSharedPointer>

#include "pep/constants.h"
#include "symbol/symboltable.h"

class AMicroCode; // Forward declaration for argument of processSourceLine.
class MicrocodeProgram;
class SymbolTable;

class MicroAsm
{
    // Lexical tokens
    enum ELexicalToken
    {
        LT_COMMA, LT_COMMENT, LT_DIGIT, LT_EQUALS, LT_EMPTY, LT_IDENTIFIER, LT_PRE_POST, LT_SEMICOLON,
        LT_LEFT_BRACKET, LT_RIGHT_BRACKET, LT_HEX_CONSTANT,
        LTE_SYMBOL,LTE_GOTO,LTE_IF,LTE_ELSE,LTE_STOP, LTE_AMD, LTE_ISD,
    };

    //Parser states
    enum ParseState
    {
        PS_COMMENT, PS_CONTINUE_POST_SEMICOLON, PS_CONTINUE_PRE_SEMICOLON, PS_CONTINUE_PRE_SEMICOLON_POST_COMMA,
        PS_DEC_CONTROL, PS_EQUAL_DEC, PS_FINISH, PS_START, PS_START_POST_SEMICOLON, PS_START_SPECIFICATION,
        PS_EXPECT_LEFT_BRACKET, PS_EXPECT_MEM_ADDRESS, PS_EXPECT_RIGHT_BRACKET, PS_EXPECT_MEM_EQUALS, PS_EXPECT_MEM_VALUE,
        PS_EXPECT_SPEC_COMMA, PS_EXPECT_REG_EQUALS, PS_EXPECT_REG_VALUE, PS_EXPECT_STATUS_EQUALS, PS_EXPECT_STATUS_VALUE,
        PSE_SYMBOL,
        PSE_LONE_GOTO,PSE_OPTIONAL_COMMENT,PSE_EXPECT_EMPTY,
        PSE_AFTER_SEMI,PSE_IF,PSE_CONDITIONAL_BRANCH,PSE_TRUE_TARGET,PSE_ELSE,PSE_FALSE_TARGET,
        PSE_JT_JUMP
    };

    PepCore::CPUType cpuType;
    bool useExt;
    static const QSet<ELexicalToken> extendedTokens;
    static const QSet<ParseState> extendedParseStates;
public:
    struct MicroAsmResult
    {
        bool success;
        QSharedPointer<SymbolTable> symTable;
        QList<QPair<int, QString>> errorMessages;
        QSharedPointer<MicrocodeProgram> program;

    };
    // Regular expressions for lexical analysis
    static QRegExp rxComment;
    static QRegExp rxDigit;
    static QRegExp rxIdentifier;
    static QRegExp rxHexConst;

    static bool startsWithHexPrefix(QString str);
    // Post: Returns true if str starts with the characters 0x or 0X. Otherwise returns false.

    explicit MicroAsm(PepCore::CPUType type, bool useExtendedFeatures = true);

    bool getToken(QString &sourceLine, ELexicalToken &token, QString &tokenString);
    // Pre: sourceLine has one line of source code.
    // Post: If the next token is valid, the string of characters representing the next token are deleted from the
    // beginning of sourceLine and returned in tokenString, true is returned, and token is set to the token type.
    // Post: If false is returned, then tokenString is set to the lexical error message.

    MicroAsmResult assembleProgram(QString program);

    void setCPUType(PepCore::CPUType type);

    // If true, symbols, gotos, if-elses will be allowed.
    // Otherwise, the default Pep9CPU is used.
    void useExtendedAssembler(bool useExt);
protected:
    // Replace assembler_assigned addressing modes with correct uncoditional branches.
    void assignImplicitAddresses(QVector<AMicroCode *> &codeList, SymbolTable& symTable);

    bool processSourceLine(SymbolTable* symTable, QString sourceLine, AMicroCode *&code, QString &errorString);
    // Pre: CPU type is set
    // Pre: sourceLine has one line of source code.
    // Pre: lineNum is the line number of the source code.
    // Post: If the source line is valid, true is returned and code is set to the source code for the line.
    // Post: If the source line is not valid, false is returned and errorString is set to the error message.
    // Checks for out of range integer values.
    // The only detected resource conflict checked is for duplicated fields.
};

#endif // ASM_H
