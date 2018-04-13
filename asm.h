// File: asm.h
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
#ifndef ASM_H
#define ASM_H

#include <QRegExp>

class Code; // Forward declaration for argument of processSourceLine.

class Asm
{
public:
    // Lexical tokens
    enum ELexicalToken
    {
        LT_COMMA, LT_COMMENT, LT_DIGIT, LT_EQUALS, LT_EMPTY, LT_IDENTIFIER, LT_PRE_POST, LT_SEMICOLON,
        LT_LEFT_BRACKET, LT_RIGHT_BRACKET, LT_HEX_CONSTANT
    };

    enum ParseState
    {
        PS_COMMENT, PS_CONTINUE_POST_SEMICOLON, PS_CONTINUE_PRE_SEMICOLON, PS_CONTINUE_PRE_SEMICOLON_POST_COMMA,
        PS_DEC_CONTROL, PS_EQUAL_DEC, PS_FINISH, PS_START, PS_START_POST_SEMICOLON, PS_START_SPECIFICATION,
        PS_EXPECT_LEFT_BRACKET, PS_EXPECT_MEM_ADDRESS, PS_EXPECT_RIGHT_BRACKET, PS_EXPECT_MEM_EQUALS, PS_EXPECT_MEM_VALUE,
        PS_EXPECT_SPEC_COMMA, PS_EXPECT_REG_EQUALS, PS_EXPECT_REG_VALUE, PS_EXPECT_STATUS_EQUALS, PS_EXPECT_STATUS_VALUE
    };

    // Regular expressions for lexical analysis
    static QRegExp rxComment;
    static QRegExp rxDigit;
    static QRegExp rxIdentifier;
    static QRegExp rxHexConst;

    static bool getToken(QString &sourceLine, ELexicalToken &token, QString &tokenString);
    // Pre: sourceLine has one line of source code.
    // Post: If the next token is valid, the string of characters representing the next token are deleted from the
    // beginning of sourceLine and returned in tokenString, true is returned, and token is set to the token type.
    // Post: If false is returned, then tokenString is set to the lexical error message.

    static bool processSourceLine(QString sourceLine, Code *&code, QString &errorString);
    // Pre: sourceLine has one line of source code.
    // Pre: lineNum is the line number of the source code.
    // Post: If the source line is valid, true is returned and code is set to the source code for the line.
    // Post: If the source line is not valid, false is returned and errorString is set to the error message.
    // Checks for out of range integer values.
    // The only detected resource conflict checked is for duplicated fields.

    static bool startsWithHexPrefix(QString str);
    // Post: Returns true if str starts with the characters 0x or 0X. Otherwise returns false.

};

#endif // ASM_H
