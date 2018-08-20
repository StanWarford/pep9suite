// File: asm.cpp
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

#include<QStringList>
#include "microasm.h"
#include "pep.h"
#include "microcode.h"
#include "specification.h"
#include "symboltable.h"
#include "symbolvalue.h"
#include "symbolentry.h"

// Regular expressions for lexical analysis
QRegExp MicroAsm::rxComment("^//.*");
QRegExp MicroAsm::rxDigit("^[0-9]+");
QRegExp MicroAsm::rxIdentifier("^((([A-Z|a-z]{1})(\\w*))(:){0,1})");
QRegExp MicroAsm::rxHexConst("^((0(?![x|X]))|((0)([x|X])([0-9|A-F|a-f])+)|((0)([0-9]+)))");

bool MicroAsm::getToken(QString &sourceLine, ELexicalToken &token, QString &tokenString)
{
    sourceLine = sourceLine.trimmed();
    if (sourceLine.length() == 0) {
        token = LT_EMPTY;
        tokenString = "";
        return true;
    }
    QChar firstChar = sourceLine[0];
    if (firstChar == ',') {
        token = LT_COMMA;
        tokenString = ",";
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar == '[') {
        token = LT_LEFT_BRACKET;
        tokenString = "[";
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar == ']') {
        token = LT_RIGHT_BRACKET;
        tokenString = "]";
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar == '/') {
        if (rxComment.indexIn(sourceLine) == -1) {
            tokenString = "// ERROR: Malformed comment"; // Should occur with single "/".
            return false;
        }
        token = LT_COMMENT;
        tokenString = rxComment.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (startsWithHexPrefix(sourceLine)) {
        if (rxHexConst.indexIn(sourceLine) == -1) {
            tokenString = "// ERROR: Malformed hex constant.";
            return false;
        }
        token = LT_HEX_CONSTANT;
        tokenString = rxHexConst.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar.isDigit()) {
        if (rxDigit.indexIn(sourceLine) == -1) {
            tokenString = "// ERROR: Malformed integer"; // Should not occur.
            return false;
        }
        token = LT_DIGIT;
        tokenString = rxDigit.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar == '=') {
        token = LT_EQUALS;
        tokenString = "=";
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar.isLetter()) {
        if (rxIdentifier.indexIn(sourceLine) == -1) {
            tokenString = "// ERROR: Malformed identifier"; // Should not occur
            return false;
        }
        tokenString = rxIdentifier.capturedTexts()[0];
        if(tokenString.endsWith(':'))
        {
                if(tokenString.compare("UnitPre:",Qt::CaseInsensitive)==0||tokenString.compare("UnitPost:",Qt::CaseInsensitive)==0)
                {
                    token = LT_PRE_POST;
                }
                else
                {
                    token = LTE_SYMBOL;
                }
        }

        else if(tokenString.compare("if", Qt::CaseInsensitive) == 0)
        {
            token = LTE_IF;
        }
        else if(tokenString.compare("else", Qt::CaseInsensitive) == 0)
        {
            token = LTE_ELSE;
        }
        else if(tokenString.compare("goto", Qt::CaseInsensitive) == 0)
        {
            token = LTE_GOTO;
        }
        else if(tokenString.compare("stop", Qt::CaseInsensitive) == 0)
        {
            token = LTE_STOP;
        }
        else if(tokenString.compare(Pep::branchFuncToMnemonMap[Enu::AddressingModeDecoder], Qt::CaseInsensitive) == 0)
        {
            token = LTE_AMD;
        }
        else if(tokenString.compare(Pep::branchFuncToMnemonMap[Enu::InstructionSpecifierDecoder], Qt::CaseInsensitive) == 0)
        {
            token = LTE_ISD;
        }
        else
        {
           token = LT_IDENTIFIER;
        }
        //        qDebug() << "tokenString: " << tokenString << "token: " << token;
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar == ';') {
        token = LT_SEMICOLON;
        tokenString = ";";
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    tokenString = "// ERROR: Syntax error starting with " + QString(firstChar);
    return false;
}

bool MicroAsm::processSourceLine(SymbolTable* symTable, QString sourceLine, MicroCodeBase *&code, QString &errorString)
{
    MicroAsm::ELexicalToken token; // Passed to getToken.
    QString tokenString; // Passed to getToken.
    QString localIdentifier = ""; // Saves identifier for processing in the following state.
    int localValue;
    int localAddressValue = 0; // = 0 to suppress compiler warning
    int localEnumMnemonic = Enu::LoadCk; // Key to Pep:: table lookups. = Enu::LoadCk to suppress compiler warning
    bool processingPrecondition = false; // To distinguish between a precondition and a postcondition. = false to suppress compiler warning

    // The concrete code objects asssigned to code.
    MicroCode *microCode = NULL;
    CommentOnlyCode *commentOnlyCode = NULL;
    UnitPreCode *preconditionCode = NULL;
    UnitPostCode *postconditionCode = NULL;
    BlankLineCode *blankLineCode = NULL;
    MicroAsm::ParseState state = MicroAsm::PS_START;
    do {
        if (!getToken(sourceLine, token, tokenString)) {
            errorString = tokenString;
            return false;
        }
        //        qDebug() << "tokenString: " << tokenString;
        switch (state) {
        case MicroAsm::PS_START:
            if (token == MicroAsm::LTE_SYMBOL)
            {
                microCode = new MicroCode();
                code = microCode;
                if(symTable->exists(tokenString.left(tokenString.length()-1)))
                {
                    if(symTable->getValue(tokenString.left(tokenString.length()-1))->isDefined())
                    {
                        errorString = "// ERROR: Multiply defined symbol: " + tokenString.left(tokenString.length()-1);
                        return false;
                    }
                }
                else
                {
                   symTable->insertSymbol(tokenString.left(tokenString.length()-1));
                }
                microCode->setSymbol(symTable->getValue(tokenString.left(tokenString.length()-1)).get());
                state = MicroAsm::PS_CONTINUE_PRE_SEMICOLON_POST_COMMA;
            }
            else if (token == MicroAsm::LT_IDENTIFIER) {
                if (Pep::mnemonToDecControlMap.contains(tokenString.toUpper())) {
                    microCode = new MicroCode();
                    code = microCode;
                    localEnumMnemonic = Pep::mnemonToDecControlMap.value(tokenString.toUpper());
                    localIdentifier = tokenString;
                    state = MicroAsm::PS_EQUAL_DEC;
                }
                else if (Pep::mnemonToMemControlMap.contains(tokenString.toUpper())) {
                    microCode = new MicroCode;
                    code = microCode;
                    localEnumMnemonic = Pep::mnemonToMemControlMap.value(tokenString.toUpper());
                    microCode->setControlSignal((Enu::EControlSignals)localEnumMnemonic, 1);
                    state = MicroAsm::PS_CONTINUE_PRE_SEMICOLON;
                }
                else if (Pep::mnemonToClockControlMap.contains(tokenString.toUpper())) {
                    errorString = "// ERROR: Clock signal " + tokenString + " must appear after semicolon";
                    return false;
                }
                else {
                    errorString = "// ERROR: Unrecognized control signal: " + tokenString;
                    return false;
                }
            }
            else if (token == MicroAsm::LT_SEMICOLON) {
                errorString = "// ERROR: No control signals before semicolon.";
                return false;
            }
            else if (token == MicroAsm::LT_COMMENT) {
                commentOnlyCode = new CommentOnlyCode(tokenString);
                code = commentOnlyCode;
                state = MicroAsm::PS_COMMENT;
            }
            else if (token == MicroAsm::LT_PRE_POST) {
                if (Pep::mnemonToSpecificationMap.contains(tokenString.toUpper())) {
                    if (Pep::mnemonToSpecificationMap.value(tokenString.toUpper()) == Enu::Pre) {
                        processingPrecondition = true;
                        preconditionCode = new UnitPreCode();
                        code = preconditionCode;
                        state = PS_START_SPECIFICATION;
                    }
                    else { // E_Post
                        processingPrecondition = false;
                        postconditionCode = new UnitPostCode();
                        code = postconditionCode;
                        state = PS_START_SPECIFICATION;
                    }
                }
                else {
                    errorString = "// ERROR: Illegal specification symbol " + tokenString;
                    return false;
                }
            }
            else if (token == MicroAsm::LT_EMPTY) {
                blankLineCode = new BlankLineCode();
                code = blankLineCode;
                state = MicroAsm::PS_FINISH;
            }
            else if (token == MicroAsm::LTE_IF)
            {
                state = MicroAsm::PSE_IF;
            }
            else {
                errorString = "// ERROR: Syntax error where control signal or comment expected";
                return false;
            }
            break;

        case MicroAsm::PS_EQUAL_DEC:
            if (token == MicroAsm::LT_EQUALS) {
                state = MicroAsm::PS_DEC_CONTROL;
            }
            else {
                errorString = "// ERROR: Expected = after " + localIdentifier;
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_DEC_CONTROL:
            if (token == MicroAsm::LT_DIGIT) {
                if (microCode->hasControlSignal((Enu::EControlSignals)localEnumMnemonic)) {
                    errorString = "// ERROR: Duplicate control signal, " + localIdentifier;
                    delete code;
                    return false;
                }
                bool ok;
                localValue = tokenString.toInt(&ok);
                if (!microCode->inRange((Enu::EControlSignals)localEnumMnemonic, localValue)) {
                    errorString = "// ERROR: Value " + QString("%1").arg(localValue) + " is out of range for " + localIdentifier;
                    delete code;
                    return false;
                }
                microCode->setControlSignal((Enu::EControlSignals)localEnumMnemonic, localValue);
                state = MicroAsm::PS_CONTINUE_PRE_SEMICOLON;
            }
            else {
                errorString = "// ERROR: Expected decimal number after " + localIdentifier + "=";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_CONTINUE_PRE_SEMICOLON:
            if (token == MicroAsm::LT_COMMA) {
                state = MicroAsm::PS_CONTINUE_PRE_SEMICOLON_POST_COMMA;
            }
            else if (token == MicroAsm::LT_SEMICOLON) {
                state = MicroAsm::PS_START_POST_SEMICOLON;
            }
            else if (token == MicroAsm::LT_COMMENT) {
                microCode->cComment = tokenString;
                state = MicroAsm::PS_COMMENT;
            }
            else if (token == MicroAsm::LT_EMPTY) {
                state = MicroAsm::PS_FINISH;
            }
            else {
                errorString = "// ERROR: Expected ',' or ';' after control signal";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_CONTINUE_PRE_SEMICOLON_POST_COMMA:
            if (token == MicroAsm::LT_IDENTIFIER) {
                if (Pep::mnemonToDecControlMap.contains(tokenString.toUpper())) {
                    localEnumMnemonic = Pep::mnemonToDecControlMap.value(tokenString.toUpper());
                    if (microCode->hasControlSignal((Enu::EControlSignals)localEnumMnemonic)) {
                        errorString = "// ERROR: Duplicate control signal, " + tokenString;
                        delete code;
                        return false;
                    }
                    localIdentifier = tokenString;
                    state = MicroAsm::PS_EQUAL_DEC;
                }
                else if (Pep::mnemonToMemControlMap.contains(tokenString.toUpper())) {
                    localEnumMnemonic = Pep::mnemonToMemControlMap.value(tokenString.toUpper());
                    if (microCode->hasControlSignal((Enu::EControlSignals)localEnumMnemonic)) {
                        errorString = "// ERROR: Duplicate control signal, " + tokenString;
                        delete code;
                        return false;
                    }
                    if (localEnumMnemonic == Enu::MemRead && microCode->hasControlSignal((Enu::EControlSignals)Enu::MemWrite)) {
                        errorString = "// ERROR: MemRead not allowed with MemWrite";
                        delete code;
                        return false;
                    }
                    if (localEnumMnemonic == Enu::MemWrite && microCode->hasControlSignal((Enu::EControlSignals)Enu::MemRead)) {
                        errorString = "// ERROR: MemWrite not allowed with MemRead";
                        delete code;
                        return false;
                    }
                    microCode->setControlSignal((Enu::EControlSignals)localEnumMnemonic, 1);
                    state = MicroAsm::PS_CONTINUE_PRE_SEMICOLON;
                }
                else if (Pep::mnemonToClockControlMap.contains(tokenString.toUpper())) {
                    errorString = "// ERROR: Clock signal (" + tokenString + ") must appear after semicolon";
                    delete code;
                    return false;
                }
                else {
                    errorString = "// ERROR: Unrecognized control signal: " + tokenString;
                    delete code;
                    return false;
                }
            }
            else if (token == MicroAsm::LT_SEMICOLON) {
                errorString = "// ERROR: Control signal expected after comma.";
                delete code;
                return false;
            }
            else if (token == MicroAsm::LT_COMMENT) {
                microCode->cComment = tokenString;
                state = MicroAsm::PS_COMMENT;
            }
            else if (token == MicroAsm::LT_EMPTY) {
                state = MicroAsm::PS_FINISH;
            }
            else if (token == MicroAsm::LTE_IF){
                state = MicroAsm::PSE_IF;
            }
            else if (token == MicroAsm::LTE_STOP){
                microCode->setBranchFunction(Enu::Stop);
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
            }
            else {
                errorString = "// ERROR: Syntax error where control signal or comment expected";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_START_POST_SEMICOLON:
            if (token == MicroAsm::LT_IDENTIFIER) {
                if (Pep::mnemonToClockControlMap.contains(tokenString.toUpper())) {
                    localEnumMnemonic = Pep::mnemonToClockControlMap.value(tokenString.toUpper());
                    if (microCode->hasClockSignal((Enu::EClockSignals)localEnumMnemonic)) {
                        errorString = "// ERROR: Duplicate clock signal, " + tokenString;
                        delete code;
                        return false;
                    }
                    microCode->setClockSingal((Enu::EClockSignals)localEnumMnemonic, 1);
                    state = MicroAsm::PS_CONTINUE_POST_SEMICOLON;
                }
                else if (Pep::mnemonToDecControlMap.contains(tokenString.toUpper())) {
                    errorString = "// ERROR: Control signal " + tokenString + " after ';'";
                    delete code;
                    return false;
                }
                else if (Pep::mnemonToMemControlMap.contains(tokenString.toUpper())) {
                    errorString = "// ERROR: Memory control signal " + tokenString + " after ';'";
                    delete code;
                    return false;
                }
                else {
                    errorString = "// ERROR: Unrecognized clock signal: " + tokenString;
                    delete code;
                    return false;
                }
            }
            else if (token == MicroAsm::LT_SEMICOLON) {
                errorString = "// ERROR: Multiple semicolons.";
                delete code;
                return false;
            }
            else if (token == MicroAsm::LT_COMMENT) {
                microCode->cComment = tokenString;
                state = MicroAsm::PS_COMMENT;
            }
            else if (token == MicroAsm::LT_EMPTY) {
                state = MicroAsm::PS_FINISH;
            }
            else if (token == MicroAsm::LTE_GOTO)
            {
                state = MicroAsm::PSE_LONE_GOTO;
            }
            else if (token == MicroAsm::LTE_IF)
            {
                state = MicroAsm::PSE_IF;
            }
            else if (token == MicroAsm::LTE_STOP)
            {
                microCode->setBranchFunction(Enu::Stop);
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
            }
            else if (token == MicroAsm::LTE_AMD)
            {
                microCode->setBranchFunction(Enu::AddressingModeDecoder);
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
            }
            else if (token == MicroAsm::LTE_ISD)
            {
                microCode->setBranchFunction(Enu::InstructionSpecifierDecoder);
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
            }
            else {
                errorString = "// ERROR: Syntax error where clock signal or comment expected.";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_CONTINUE_POST_SEMICOLON:
            if (token == MicroAsm::LT_COMMA) {
                state = MicroAsm::PS_START_POST_SEMICOLON;
            }
            else if (token == MicroAsm::LT_SEMICOLON) {
                state = MicroAsm::PSE_AFTER_SEMI;
            }
            else if (token == MicroAsm::LT_COMMENT) {
                microCode->cComment = tokenString;
                state = MicroAsm::PS_COMMENT;
            }
            else if (token == MicroAsm::LT_EMPTY) {
                state = MicroAsm::PS_FINISH;
            }
            else {
                errorString = "// ERROR: Expected ',' after clock signal";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_START_SPECIFICATION:
            if (token == MicroAsm::LT_IDENTIFIER) {
                if (Pep::mnemonToMemSpecMap.contains(tokenString.toUpper())) {
                    localEnumMnemonic = Pep::mnemonToMemSpecMap.value(tokenString.toUpper());
                    state = MicroAsm::PS_EXPECT_LEFT_BRACKET;
                }
                else if (Pep::mnemonToRegSpecMap.contains(tokenString.toUpper())) {
                    localEnumMnemonic = Pep::mnemonToRegSpecMap.value(tokenString.toUpper());
                    state = MicroAsm::PS_EXPECT_REG_EQUALS;
                }
                else if (Pep::mnemonToStatusSpecMap.contains(tokenString.toUpper())) {
                    localEnumMnemonic = Pep::mnemonToStatusSpecMap.value(tokenString.toUpper());
                    state = MicroAsm::PS_EXPECT_STATUS_EQUALS;
                }
                else {
                    errorString = "// ERROR: Unrecognized specification symbol: " + tokenString;
                    delete code;
                    return false;
                }
            }
            else if (token == MicroAsm::LT_COMMENT) {
                if (processingPrecondition) {
                    preconditionCode->setComment(tokenString);
                }
                else {
                    postconditionCode->setComment(tokenString);
                }
                state = MicroAsm::PS_COMMENT;
            }
            else if (token == MicroAsm::LT_EMPTY) {
                state = MicroAsm::PS_FINISH;
            }
            else {
                errorString = "// ERROR: Syntax error starting with: " + tokenString;
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_EXPECT_LEFT_BRACKET:
            if (token == MicroAsm::LT_LEFT_BRACKET) {
                state = MicroAsm::PS_EXPECT_MEM_ADDRESS;
            }
            else {
                errorString = "// ERROR: Expected [ after Mem.";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_EXPECT_MEM_ADDRESS:
            if (token == MicroAsm::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                localAddressValue = tokenString.toInt(&ok, 16);
                if (localAddressValue >= 65536) {
                    errorString = "// ERROR: Hexidecimal address is out of range (0x0000..0xFFFF).";
                    delete code;
                    return false;
                }
                state = MicroAsm::PS_EXPECT_RIGHT_BRACKET;
            }
            else {
                errorString = "// ERROR: Expected hex memory address after [.";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_EXPECT_RIGHT_BRACKET:
            if (token == MicroAsm::LT_RIGHT_BRACKET) {
                state = MicroAsm::PS_EXPECT_MEM_EQUALS;
            }
            else {
                errorString = "// ERROR: Expected ] after memory address.";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_EXPECT_MEM_EQUALS:
            if (token == MicroAsm::LT_EQUALS) {
                state = MicroAsm::PS_EXPECT_MEM_VALUE;
            }
            else {
                errorString = "// ERROR: Expected = after ].";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_EXPECT_MEM_VALUE:
            if (token == MicroAsm::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                localValue = tokenString.toInt(&ok, 16);
                if (localValue >= 65536) {
                    errorString = "// ERROR: Hexidecimal memory value is out of range (0x0000..0xFFFF).";
                    delete code;
                    return false;
                }
                if (processingPrecondition) {
                    preconditionCode->appendSpecification(new MemSpecification(localAddressValue, localValue, tokenString.length() > 2 ? 2 : 1));
                }
                else {
                    postconditionCode->appendSpecification(new MemSpecification(localAddressValue, localValue, tokenString.length() > 2 ? 2 : 1));
                }
                state = MicroAsm::PS_EXPECT_SPEC_COMMA;
            }
            else {
                errorString = "// ERROR: Expected hex constant after =.";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_EXPECT_REG_EQUALS:
            if (token == MicroAsm::LT_EQUALS) {
                state = MicroAsm::PS_EXPECT_REG_VALUE;
            }
            else {
                errorString = "// ERROR: Expected = after " + Pep::regSpecToMnemonMap.value((Enu::EKeywords)localEnumMnemonic);
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_EXPECT_REG_VALUE:
            if (token == MicroAsm::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                localValue = tokenString.toInt(&ok, 16);
                if (localEnumMnemonic == Enu::IR && localValue >= 16777216) {
                    errorString = "// ERROR: Hexidecimal register value is out of range (0x000000..0xFFFFFF).";
                    delete code;
                    return false;
                }
                if (localEnumMnemonic == Enu::T1 && localValue >= 256) {
                    errorString = "// ERROR: Hexidecimal register value is out of range (0x00..0xFF).";
                    delete code;
                    return false;
                }
                if (localEnumMnemonic != Enu::IR && localEnumMnemonic != Enu::T1 && localValue >= 65536) {
                    errorString = "// ERROR: Hexidecimal register value is out of range (0x0000..0xFFFF).";
                    delete code;
                    return false;
                }
                if (processingPrecondition) {
                    preconditionCode->appendSpecification(new RegSpecification((Enu::EKeywords)localEnumMnemonic, localValue));
                }
                else {
                    postconditionCode->appendSpecification(new RegSpecification((Enu::EKeywords)localEnumMnemonic, localValue));
                }
                state = MicroAsm::PS_EXPECT_SPEC_COMMA;
            }
            else {
                errorString = "// ERROR: Expected hex constant after =.";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_EXPECT_STATUS_EQUALS:
            if (token == MicroAsm::LT_EQUALS) {
                state = MicroAsm::PS_EXPECT_STATUS_VALUE;
            }
            else {
                errorString = "// ERROR: Expected = after " + Pep::statusSpecToMnemonMap.value((Enu::EKeywords)localEnumMnemonic);
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_EXPECT_STATUS_VALUE:
            if (token == MicroAsm::LT_DIGIT) {
                bool ok;
                localValue = tokenString.toInt(&ok);
                if (localValue > 1) {
                    errorString = "// ERROR: Status bit value is out of range (0..1).";
                    delete code;
                    return false;
                }
                if (processingPrecondition) {
                    preconditionCode->appendSpecification(new StatusBitSpecification((Enu::EKeywords)localEnumMnemonic, localValue == 1));
                }
                else {
                    postconditionCode->appendSpecification(new StatusBitSpecification((Enu::EKeywords)localEnumMnemonic, localValue == 1));
                }
                state = MicroAsm::PS_EXPECT_SPEC_COMMA;
            }
            else {
                errorString = "// ERROR: Expected '1' or '0' after =.";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_EXPECT_SPEC_COMMA:
            if (token == MicroAsm::LT_COMMA) {
                state = MicroAsm::PS_START_SPECIFICATION;
            }
            else if (token == MicroAsm::LT_COMMENT) {
                if (processingPrecondition) {
                    preconditionCode->setComment(tokenString);
                }
                else {
                    postconditionCode->setComment(tokenString);
                }
                state = MicroAsm::PS_COMMENT;
            }
            else if (token == MicroAsm::LT_EMPTY) {
                state = MicroAsm::PS_FINISH;
            }
            else {
                errorString = "// ERROR: Expected ',' comment, or end of line.";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_COMMENT:
            if (token == MicroAsm::LT_EMPTY) {
                state = MicroAsm::PS_FINISH;
            }
            else {
                // This error should not occur, as all characters are allowed in comments.
                errorString = "// ERROR: Problem detected after comment.";
                delete code;
                return false;
            }
            break;
        case MicroAsm::PSE_AFTER_SEMI:
            if(token == MicroAsm::LTE_GOTO)
            {
                state = MicroAsm::PSE_LONE_GOTO;
            }
            else if(token == MicroAsm::LTE_IF)
            {
                state = MicroAsm::PSE_IF;
            }
            else if (token == MicroAsm::LTE_STOP){
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
                microCode->setBranchFunction(Enu::Stop);
            }
            else if(token == MicroAsm::LTE_AMD)
            {
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
                microCode->setBranchFunction(Enu::AddressingModeDecoder);
            }
            else if(token == MicroAsm::LTE_ISD)
            {
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
                microCode->setBranchFunction(Enu::InstructionSpecifierDecoder);
            }
            else
            {
                errorString = "// ERROR: Expected branch after semicolon";
                return false;
            }
            break;
        case MicroAsm::PSE_LONE_GOTO:
            if (token == MicroAsm::LT_IDENTIFIER)
            {
                if(!symTable->exists(tokenString))
                {
                    symTable->insertSymbol(tokenString);
                }
                microCode->setTrueTarget(symTable->getValue(tokenString).get());
                microCode->setBranchFunction(Enu::Unconditional);
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
            }
            else
            {
                errorString="// ERROR: Didn't get symbol for goto";
                return false;
            }
            break;
        case MicroAsm::PSE_IF:
            if(microCode==nullptr)
            {
                microCode = new MicroCode();
                code=microCode;
            }
            if(token == MicroAsm::LT_IDENTIFIER&&Pep::mnemonToBranchFuncMap.contains(tokenString.toUpper()))
            {
                //Switch to conditional branch logic
                microCode->setBranchFunction(Pep::mnemonToBranchFuncMap[tokenString.toUpper()]);
                state = PSE_TRUE_TARGET;
            }
            else
            {
                errorString = "// Error: Expected conditional instruction after 'if'";
                delete code;
                return false;
            }
            break;
        case MicroAsm::PSE_TRUE_TARGET:
            if(token == MicroAsm::LT_IDENTIFIER)
            {
                if(!symTable->exists(tokenString))
                {
                    symTable->insertSymbol(tokenString);
                }
                microCode->setTrueTarget(symTable->getValue(tokenString).get());
                state = MicroAsm::PSE_ELSE;
            }
            else
            {
                errorString ="// Error: Expected a symbol for true target of if";
                delete code;
                return false;
            }
            break;
        case MicroAsm::PSE_ELSE:
            if(token == MicroAsm::LTE_ELSE)
            {
                state = MicroAsm::PSE_FALSE_TARGET;
            }
            else
            {
                errorString = "// Error: Failure";
                delete code;
                return false;
            }
            break;
        case MicroAsm::PSE_FALSE_TARGET:
            if(token == MicroAsm::LT_IDENTIFIER)
            {
                if(!symTable->exists(tokenString))
                {
                    symTable->insertSymbol(tokenString);
                }
                microCode->setFalseTarget(symTable->getValue(tokenString).get());
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
            }
            else
            {
                errorString ="Expected a symbol for false target of if";
                delete code;
                return false;
            }
            break;
        case MicroAsm::PSE_OPTIONAL_COMMENT:
            if (token ==MicroAsm::LT_COMMENT)
            {
                microCode->cComment = tokenString;
                state = MicroAsm::PSE_EXPECT_EMPTY;
            }
            else if (token == MicroAsm::LT_EMPTY) {
                state = MicroAsm::PS_FINISH;
            }
            else
            {
                errorString ="Expected a symbol for false target of if";
                delete code;
                return false;
            }
            break;
        case MicroAsm::PSE_EXPECT_EMPTY:
            if (token == MicroAsm::LT_EMPTY) {
                state = MicroAsm::PS_FINISH;
            }
            break;
        default:
            break;
        }
    }
    while (state != MicroAsm::PS_FINISH);
    return true;
}

bool MicroAsm::startsWithHexPrefix(QString str)
{
    if (str.length() < 2) return false;
    if (str[0] != '0') return false;
    if (str[1] == 'x' || str[1] == 'X') return true;
    return false;
}
