// File: microasm.cpp
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
#include "microasm.h"

#include<QStringList>


#include "pep/pep.h"
#include "microassembler/microcode.h"
#include "microassembler/microcodeprogram.h"
#include "microassembler/specification.h"
#include "symbol/symboltable.h"
#include "symbol/symbolvalue.h"
#include "symbol/symbolentry.h"

#include "pep9microcode.h"
#include "pep9specification.h"

// Regular expressions for lexical analysis
const QSet<MicroAsm::ELexicalToken> MicroAsm::extendedTokens = {LTE_SYMBOL,LTE_GOTO,LTE_IF,LTE_ELSE,LTE_STOP, LTE_AMD, LTE_ISD};
const QSet<MicroAsm::ParseState> MicroAsm::extendedParseStates = {        PSE_SYMBOL,
                                                                          PSE_LONE_GOTO,PSE_OPTIONAL_COMMENT,PSE_EXPECT_EMPTY,
                                                                          PSE_AFTER_SEMI,PSE_IF,PSE_CONDITIONAL_BRANCH,PSE_TRUE_TARGET,PSE_ELSE,PSE_FALSE_TARGET,
                                                                          PSE_JT_JUMP};
QRegExp MicroAsm::rxComment("^//.*");
QRegExp MicroAsm::rxDigit("^[0-9]+");
QRegExp MicroAsm::rxIdentifier("^((([A-Z|a-z]{1})(\\w*))(:){0,1})");
QRegExp MicroAsm::rxHexConst("^((0(?![x|X]))|((0)([x|X])([0-9|A-F|a-f])+)|((0)([0-9]+)))");
MicroAsm::MicroAsm(Enu::CPUType type, bool useExtendedFeatures): cpuType(type),
    useExt(useExtendedFeatures)
{

}

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
        if(tokenString.endsWith(':')) {
                if(tokenString.compare("UnitPre:",Qt::CaseInsensitive) == 0
                        || tokenString.compare("UnitPost:",Qt::CaseInsensitive) == 0) {
                    token = LT_PRE_POST;
                }
                else {
                    token = LTE_SYMBOL;
                }
        }

        else if(tokenString.compare("if", Qt::CaseInsensitive) == 0) {
            token = LTE_IF;
        }
        else if(tokenString.compare("else", Qt::CaseInsensitive) == 0) {
            token = LTE_ELSE;
        }
        else if(tokenString.compare("goto", Qt::CaseInsensitive) == 0) {
            token = LTE_GOTO;
        }
        else if(tokenString.compare(Pep::branchFuncToMnemonMap[Enu::Stop], Qt::CaseInsensitive) == 0) {
            token = LTE_STOP;
        }
        else if(tokenString.compare(Pep::branchFuncToMnemonMap[Enu::AddressingModeDecoder], Qt::CaseInsensitive) == 0) {
            token = LTE_AMD;
        }
        else if(tokenString.compare(Pep::branchFuncToMnemonMap[Enu::InstructionSpecifierDecoder], Qt::CaseInsensitive) == 0) {
            token = LTE_ISD;
        }
        else {
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

MicroAsm::MicroAsmResult MicroAsm::assembleProgram(QString program)
{
    int lineNum = 0;
    MicroAsmResult result;
    result.symTable = QSharedPointer<SymbolTable>::create();
    result.success = true;

    QString errorMessage;
    QVector<AMicroCode*> codeList;
    AMicroCode *code;
    auto sourceCodeList = program.split('\n');
    while (lineNum < sourceCodeList.size()) {
        auto sourceLine = sourceCodeList[lineNum];
        if (!processSourceLine(&*result.symTable,sourceLine, code, errorMessage)) {
            result.errorMessages.append({lineNum, errorMessage});
            // Create a dummy program that will delete all asm code entries
            QSharedPointer<MicrocodeProgram>::create(codeList, result.symTable);
            result.success = false;
            break;
        }
        if(code->isMicrocode() && static_cast<MicroCode*>(code)->hasControlSignal(Enu::EControlSignals::MemRead) &&
                static_cast<MicroCode*>(code)->hasControlSignal(Enu::EControlSignals::MemWrite)) {
            result.errorMessages.append({lineNum, "\\ ERROR: Can't have memread and memwrite"});
            // Create a dummy program that will delete all asm code entries
            QSharedPointer<MicrocodeProgram>::create(codeList, result.symTable);
            result.success = false;
            break;
        }
        codeList.append(code);
        lineNum++;
    }

    if(!result.success) return result;
    // Assign all branch targets for ASSEMBLER_ASSIGNED addressing mode.
    assignImplicitAddresses(codeList, *result.symTable);

    for(auto sym : result.symTable->getSymbolEntries()) {
            if(sym->isUndefined()){
                result.errorMessages.append({-1, "// ERROR: Undefined symbol "+sym->getName()});
                result.success = false;
                break;
            }
            else if(sym->isMultiplyDefined()) {
                result.errorMessages.append({-1, "// ERROR: Multiply defined symbol "+sym->getName()});
                result.success = false;
                break;
            }
    }

    result.program =  QSharedPointer<MicrocodeProgram>::create(codeList, result.symTable);
    return result;
}

bool MicroAsm::processSourceLine(SymbolTable* symTable, QString sourceLine, AMicroCode *&code, QString &errorString)
{
    MicroAsm::ELexicalToken token; // Passed to getToken.
    QString tokenString; // Passed to getToken.
    QString localIdentifier = ""; // Saves identifier for processing in the following state.
    int localValue;
    int localAddressValue = 0; // = 0 to suppress compiler warning
    int localEnumMnemonic = 0; // Key to Pep:: table lookups. = 0 to suppress compiler warning
    bool processingPrecondition = false; // To distinguish between a precondition and a postcondition. = false to suppress compiler warning

    // The concrete code objects asssigned to code.
    MicroCode *microCode = nullptr;
    CommentOnlyCode *commentOnlyCode = nullptr;
    UnitPreCode *preconditionCode = nullptr;
    UnitPostCode *postconditionCode = nullptr;
    BlankLineCode *blankLineCode = nullptr;
    MicroAsm::ParseState state = MicroAsm::PS_START;
    QString temp_str;
    do {
        if (!getToken(sourceLine, token, tokenString)) {
            errorString = tokenString;
            return false;
        }
#pragma message("TODO: make error messages more informative.")
        if(!useExt) {
            if(extendedTokens.contains(token)) {
                errorString = "// ERROR: A control flow / symbol token was hit. These have been disabled.";
                return false;
            }
            else if(extendedParseStates.contains(state)) {
                errorString = "// ERROR: The default microassembler was used, but an unexpected parser state was encountered.";
                return false;
            }
        }
        //        qDebug() << "tokenString: " << tokenString;
        switch (state) {
        case MicroAsm::PS_START:
            if (token == MicroAsm::LTE_SYMBOL) {
                microCode = new MicroCode(cpuType, useExt);
                code = microCode;
                temp_str = tokenString.left(tokenString.length() - 1);
                if(symTable->exists(temp_str)) {
                    if(symTable->getValue(temp_str)->isDefined()) {
                        errorString = "// ERROR: Multiply defined symbol: " + temp_str + ".";
                        return false;
                    }
                }
                else {
                   symTable->insertSymbol(temp_str);
                }

                microCode->setSymbol(symTable->getValue(temp_str).data());
                state = MicroAsm::PS_CONTINUE_PRE_SEMICOLON_POST_COMMA;
            }
            else if (token == MicroAsm::LT_IDENTIFIER) {
                if (Pep::mnemonToDecControlMap.contains(tokenString.toUpper())) {
                    microCode = new MicroCode(cpuType, useExt);
                    code = microCode;
                    localEnumMnemonic = Pep::mnemonToDecControlMap.value(tokenString.toUpper());
                    localIdentifier = tokenString;
                    state = MicroAsm::PS_EQUAL_DEC;
                }
                else if (Pep::mnemonToMemControlMap.contains(tokenString.toUpper())) {
                    microCode = new MicroCode(cpuType, useExt);
                    code = microCode;
                    localEnumMnemonic = Pep::mnemonToMemControlMap.value(tokenString.toUpper());
                    microCode->setControlSignal(static_cast<Enu::EControlSignals>(localEnumMnemonic), 1);
                    state = MicroAsm::PS_CONTINUE_PRE_SEMICOLON;
                }
                else if (Pep::mnemonToClockControlMap.contains(tokenString.toUpper())) {
                    errorString = "// ERROR: Clock signal " + tokenString + " must appear after semicolon.";
                    return false;
                }
                else {
                    errorString = "// ERROR: Unrecognized control signal: " + tokenString + ".";
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
                    errorString = "// ERROR: Illegal specification symbol " + tokenString + ".";
                    return false;
                }
            }
            else if (token == MicroAsm::LT_EMPTY) {
                blankLineCode = new BlankLineCode();
                code = blankLineCode;
                state = MicroAsm::PS_FINISH;
            }
            else if (token == MicroAsm::LTE_IF)  {
                microCode = new MicroCode(cpuType, useExt);
                code = microCode;
                state = MicroAsm::PSE_IF;
            }
            else if(token == LTE_STOP) {
                microCode = new MicroCode(cpuType, useExt);
                microCode->setBranchFunction(Enu::EBranchFunctions::Stop);
                code = microCode;
                state = PSE_OPTIONAL_COMMENT;
            }
            else if(token == LTE_GOTO) {
                microCode = new MicroCode(cpuType, useExt);
                code = microCode;
                state = PSE_LONE_GOTO;
            }
            else if (token == LTE_AMD) {
                microCode = new MicroCode(cpuType, useExt);
                microCode->setBranchFunction(Enu::EBranchFunctions::AddressingModeDecoder);
                code = microCode;
                state = PSE_OPTIONAL_COMMENT;
            }
            else if (token == LTE_ISD) {
                microCode = new MicroCode(cpuType, useExt);
                microCode->setBranchFunction(Enu::EBranchFunctions::InstructionSpecifierDecoder);
                code = microCode;
                state = PSE_OPTIONAL_COMMENT;
            }
            else {
                errorString = "// ERROR: Syntax error where control signal or comment expected.";
                return false;
            }
            break;

        case MicroAsm::PS_EQUAL_DEC:
            if (token == MicroAsm::LT_EQUALS) {
                state = MicroAsm::PS_DEC_CONTROL;
            }
            else {
                errorString = "// ERROR: Expected = after " + localIdentifier + ".";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_DEC_CONTROL:
            if (token == MicroAsm::LT_DIGIT) {
                if (microCode->hasControlSignal(static_cast<Enu::EControlSignals>(localEnumMnemonic))) {
                    errorString = "// ERROR: Duplicate control signal, " + localIdentifier + ".";
                    delete code;
                    return false;
                }
                bool ok;
                localValue = tokenString.toInt(&ok);
                if (!microCode->inRange(static_cast<Enu::EControlSignals>(localEnumMnemonic), localValue)) {
                    errorString = "// ERROR: Value " + QString("%1").arg(localValue)
                            + " is out of range for " + localIdentifier  + ".";
                    delete code;
                    return false;
                }
                microCode->setControlSignal(static_cast<Enu::EControlSignals>(localEnumMnemonic),
                                            static_cast<quint8>(localValue));
                state = MicroAsm::PS_CONTINUE_PRE_SEMICOLON;
            }
            else {
                errorString = "// ERROR: Expected decimal number after " + localIdentifier + "=.";
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
                microCode->setComment(tokenString);
                state = MicroAsm::PS_COMMENT;
            }
            else if (token == MicroAsm::LT_EMPTY) {
                state = MicroAsm::PS_FINISH;
            }
            else {
                errorString = "// ERROR: Expected ',' or ';' after control signal.";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_CONTINUE_PRE_SEMICOLON_POST_COMMA:
            if (token == MicroAsm::LT_IDENTIFIER) {
                if (Pep::mnemonToDecControlMap.contains(tokenString.toUpper())) {
                    localEnumMnemonic = Pep::mnemonToDecControlMap.value(tokenString.toUpper());
                    if (microCode->hasControlSignal(static_cast<Enu::EControlSignals>(localEnumMnemonic))) {
                        errorString = "// ERROR: Duplicate control signal, " + tokenString + ".";
                        delete code;
                        return false;
                    }
                    localIdentifier = tokenString;
                    state = MicroAsm::PS_EQUAL_DEC;
                }
                else if (Pep::mnemonToMemControlMap.contains(tokenString.toUpper())) {
                    localEnumMnemonic = Pep::mnemonToMemControlMap.value(tokenString.toUpper());
                    if (microCode->hasControlSignal(static_cast<Enu::EControlSignals>(localEnumMnemonic))) {
                        errorString = "// ERROR: Duplicate control signal, " + tokenString + ".";
                        delete code;
                        return false;
                    }
                    if (localEnumMnemonic == Enu::MemRead
                            && microCode->hasControlSignal(static_cast<Enu::EControlSignals>(localEnumMnemonic))) {
                        errorString = "// ERROR: MemRead not allowed with MemWrite.";
                        delete code;
                        return false;
                    }
                    if (localEnumMnemonic == Enu::MemWrite
                            && microCode->hasControlSignal(static_cast<Enu::EControlSignals>(localEnumMnemonic))) {
                        errorString = "// ERROR: MemWrite not allowed with MemRead.";
                        delete code;
                        return false;
                    }
                    microCode->setControlSignal(static_cast<Enu::EControlSignals>(localEnumMnemonic), 1);
                    state = MicroAsm::PS_CONTINUE_PRE_SEMICOLON;
                }
                else if (Pep::mnemonToClockControlMap.contains(tokenString.toUpper())) {
                    errorString = "// ERROR: Clock signal (" + tokenString + ") must appear after semicolon.";
                    delete code;
                    return false;
                }
                else {
                    errorString = "// ERROR: Unrecognized control signal: " + tokenString + ".";
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
                microCode->setComment(tokenString);
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
            else if(token == LTE_GOTO) {
                state = PSE_LONE_GOTO;
            }
            else if (token == LTE_AMD) {
                microCode->setBranchFunction(Enu::EBranchFunctions::AddressingModeDecoder);
                state = PSE_OPTIONAL_COMMENT;
            }
            else if (token == LTE_ISD) {
                microCode->setBranchFunction(Enu::EBranchFunctions::InstructionSpecifierDecoder);
                state = PSE_OPTIONAL_COMMENT;
            }
            else {
                errorString = "// ERROR: Syntax error where control signal or comment expected.";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PS_START_POST_SEMICOLON:
            if (token == MicroAsm::LT_IDENTIFIER) {
                if (Pep::mnemonToClockControlMap.contains(tokenString.toUpper())) {
                    localEnumMnemonic = Pep::mnemonToClockControlMap.value(tokenString.toUpper());
                    if (microCode->hasClockSignal(static_cast<Enu::EClockSignals>(localEnumMnemonic))) {
                        errorString = "// ERROR: Duplicate clock signal, " + tokenString + ".";
                        delete code;
                        return false;
                    }
                    microCode->setClockSingal(static_cast<Enu::EClockSignals>(localEnumMnemonic), 1);
                    state = MicroAsm::PS_CONTINUE_POST_SEMICOLON;
                }
                else if (Pep::mnemonToDecControlMap.contains(tokenString.toUpper())) {
                    errorString = "// ERROR: Control signal " + tokenString + " after ';'.";
                    delete code;
                    return false;
                }
                else if (Pep::mnemonToMemControlMap.contains(tokenString.toUpper())) {
                    errorString = "// ERROR: Memory control signal " + tokenString + " after ';'.";
                    delete code;
                    return false;
                }
                else {
                    errorString = "// ERROR: Unrecognized clock signal: " + tokenString + ".";
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
                microCode->setComment(tokenString);
                state = MicroAsm::PS_COMMENT;
            }
            else if (token == MicroAsm::LT_EMPTY) {
                state = MicroAsm::PS_FINISH;
            }
            else if (token == MicroAsm::LTE_GOTO) {
                state = MicroAsm::PSE_LONE_GOTO;
            }
            else if (token == MicroAsm::LTE_IF) {
                state = MicroAsm::PSE_IF;
            }
            else if (token == MicroAsm::LTE_STOP) {
                microCode->setBranchFunction(Enu::Stop);
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
            }
            else if (token == MicroAsm::LTE_AMD) {
                microCode->setBranchFunction(Enu::AddressingModeDecoder);
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
            }
            else if (token == MicroAsm::LTE_ISD) {
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
                // When using the unextended assembler, multiple semicolons cannot occur
                if(!useExt) {
                    errorString = "// ERROR: Multiple semicolons.";
                    delete code;
                    return false;
                }
                state = MicroAsm::PSE_AFTER_SEMI;
            }
            else if (token == MicroAsm::LT_COMMENT) {
                microCode->setComment(tokenString);
                state = MicroAsm::PS_COMMENT;
            }
            else if (token == MicroAsm::LT_EMPTY) {
                state = MicroAsm::PS_FINISH;
            }
            else {
                errorString = "// ERROR: Expected ',' after clock signal.";
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
                    errorString = "// ERROR: Unrecognized specification symbol: " + tokenString + ".";
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
                errorString = "// ERROR: Syntax error starting with: " + tokenString + ".";
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
                    preconditionCode->appendSpecification(new MemSpecification(localAddressValue,
                                                                               localValue,
                                                                               tokenString.length() > 2 ? 2 : 1));
                }
                else {
                    postconditionCode->appendSpecification(new MemSpecification(localAddressValue,
                                                                                localValue,
                                                                                tokenString.length() > 2 ? 2 : 1));
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
                errorString = "// ERROR: Expected = after " +
                        Pep::regSpecToMnemonMap.value(static_cast<Enu::ECPUKeywords>(localEnumMnemonic))
                         + ".";
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
                    preconditionCode->appendSpecification(
                                new RegSpecification(static_cast<Enu::ECPUKeywords>(localEnumMnemonic), localValue));
                }
                else {
                    postconditionCode->appendSpecification(
                                new RegSpecification(static_cast<Enu::ECPUKeywords>(localEnumMnemonic), localValue));
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
                errorString = "// ERROR: Expected = after "
                        + Pep::statusSpecToMnemonMap.value(static_cast<Enu::ECPUKeywords>(localEnumMnemonic))
                        + ".";
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
                    preconditionCode->appendSpecification(
                                new StatusBitSpecification(static_cast<Enu::ECPUKeywords>(localEnumMnemonic),
                                                           localValue == 1));
                }
                else {
                    postconditionCode->appendSpecification(
                                new StatusBitSpecification(static_cast<Enu::ECPUKeywords>(localEnumMnemonic),
                                                           localValue == 1));
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
            if(token == MicroAsm::LTE_GOTO) {
                state = MicroAsm::PSE_LONE_GOTO;
            }
            else if(token == MicroAsm::LTE_IF) {
                state = MicroAsm::PSE_IF;
            }
            else if (token == MicroAsm::LTE_STOP){
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
                microCode->setBranchFunction(Enu::Stop);
            }
            else if(token == MicroAsm::LTE_AMD) {
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
                microCode->setBranchFunction(Enu::AddressingModeDecoder);
            }
            else if(token == MicroAsm::LTE_ISD) {
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
                microCode->setBranchFunction(Enu::InstructionSpecifierDecoder);
            }
            else {
                errorString = "// ERROR: Expected branch after semicolon.";
                return false;
            }
            break;

        case MicroAsm::PSE_LONE_GOTO:
            if (token == MicroAsm::LT_IDENTIFIER) {
                if(!symTable->exists(tokenString)) {
                    symTable->insertSymbol(tokenString);
                }
                microCode->setTrueTarget(symTable->getValue(tokenString).data());
                microCode->setBranchFunction(Enu::Unconditional);
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
            }
            else {
                errorString = "// ERROR: No symbol after goto.";
                return false;
            }
            break;

        case MicroAsm::PSE_IF:
            if(microCode == nullptr) {
                microCode = new MicroCode(cpuType, useExt);
                code = microCode;
            }
            if(token == MicroAsm::LT_IDENTIFIER && Pep::mnemonToBranchFuncMap.contains(tokenString.toUpper())) {
                //Switch to conditional branch logic
                microCode->setBranchFunction(Pep::mnemonToBranchFuncMap[tokenString.toUpper()]);
                state = PSE_TRUE_TARGET;
            }
            else {
                errorString = "// Error: Expected conditional instruction after \"if\".";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PSE_TRUE_TARGET:
            if(token == MicroAsm::LT_IDENTIFIER) {
                if(!symTable->exists(tokenString)) {
                    symTable->insertSymbol(tokenString);
                }
                microCode->setTrueTarget(symTable->getValue(tokenString).data());
                state = MicroAsm::PSE_ELSE;
            }
            else {
                errorString = "// Error: Expected a symbol for true target of \"if\".";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PSE_ELSE:
            if(token == MicroAsm::LTE_ELSE) {
                state = MicroAsm::PSE_FALSE_TARGET;
            }
            else {
                errorString = "// Error: expected \"else\" after \"if\".";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PSE_FALSE_TARGET:
            if(token == MicroAsm::LT_IDENTIFIER) {
                if(!symTable->exists(tokenString)) {
                    symTable->insertSymbol(tokenString);
                }
                microCode->setFalseTarget(symTable->getValue(tokenString).data());
                state = MicroAsm::PSE_OPTIONAL_COMMENT;
            }
            else {
                errorString = "// Error: Expected a symbol for false target of \"if\".";
                delete code;
                return false;
            }
            break;

        case MicroAsm::PSE_OPTIONAL_COMMENT:
            if (token == MicroAsm::LT_COMMENT) {
                microCode->setComment(tokenString);
                state = MicroAsm::PSE_EXPECT_EMPTY;
            }
            else if (token == MicroAsm::LT_EMPTY) {
                state = MicroAsm::PS_FINISH;
            }
            else {
                // This error should not occur, as all characters are allowed in comments.
                errorString = "// Error: Expected a symbol for false target of \"if\".";
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

void MicroAsm::setCPUType(Enu::CPUType type)
{
    cpuType = type;
}

void MicroAsm::useExtendedAssembler(bool useExt)
{
    this->useExt = useExt;
}

void MicroAsm::assignImplicitAddresses(QVector<AMicroCode *> &codeList, SymbolTable & symTable)
{
    // Gather the indexes of all microcode lines.
    QList<size_t> microcodeOffsets;
    for(int it=0; it<codeList.length(); it++) {
        if(codeList[it]->isMicrocode()) microcodeOffsets.append(it);
    }

    // Must iterate over offset list rather than code list.
    // Using indecies in code list as "memory addresses" will cause
    // there to be gaps in address space, and crash the simulator.
    for(int it = 0; it < microcodeOffsets.length(); it++) {
        auto line = codeList[microcodeOffsets[it]];
        if(!line->isMicrocode()) continue;
        MicroCode* as_mc = static_cast<MicroCode*>(line);
        // If the line doesn't already have a symbol, create an assembler assigned one.
        if((as_mc->getSymbol()) == nullptr) {
            as_mc->setSymbol(symTable.insertSymbol("_as" + QString::number(it)).data());
        }
        symTable.setValue(as_mc->getSymbol()->getSymbolID(), QSharedPointer<SymbolValueNumeric>::create(it));
    }

    // For each microcode line with an Assembler_Assigned branch function,
    // replace it with the appropriate Uncoditional branch to the next line of microcode.
    for(int it=0; it<microcodeOffsets.length(); it++) {
        auto line = codeList[microcodeOffsets[it]];
        if(!line->isMicrocode()) continue;
        MicroCode* as_mc = static_cast<MicroCode*>(line);
        // If the line of microcode has no branch function, assume an uncoditional branch to the following line of microcode.
        if((as_mc->getBranchFunction() == Enu::Assembler_Assigned) && (it + 1 <microcodeOffsets.length())) {
            as_mc->setBranchFunction(Enu::Unconditional);
            as_mc->setTrueTarget(static_cast<MicroCode*>(codeList[microcodeOffsets[it + 1]])->getSymbol());
            as_mc->setFalseTarget(static_cast<MicroCode*>(codeList[microcodeOffsets[it + 1]])->getSymbol());
        }
        // If the final instruction has no explicit addressing mode, set the line's branch function to stop.
        else if((as_mc->getBranchFunction() == Enu::Assembler_Assigned) && (it + 1 == microcodeOffsets.length())) {
            as_mc->setBranchFunction(Enu::Stop);
            as_mc->setTrueTarget(static_cast<MicroCode*>(codeList[microcodeOffsets[it]])->getSymbol());
            as_mc->setFalseTarget(static_cast<MicroCode*>(codeList[microcodeOffsets[it]])->getSymbol());
        }
        // If either the true are false targets are null pointers, set the target to self.
        // This could prevent crashes if someone attempt to operate on either target without an explit null check.
        if(as_mc->getTrueTarget() == nullptr) {
            as_mc->setTrueTarget(static_cast<MicroCode*>(codeList[microcodeOffsets[it]])->getSymbol());
        }
        if(as_mc->getFalseTarget() == nullptr) {
            as_mc->setFalseTarget(static_cast<MicroCode*>(codeList[microcodeOffsets[it]])->getSymbol());
        }
    }
}
