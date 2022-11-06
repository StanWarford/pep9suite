// File: isaasm.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2019  J. Stanley Warford & Matthew McRaven, Pepperdine University

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
#include "pep9isaasm.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>


#include "assembler/asmargument.h"
#include "assembler/asmcode.h"
#include "assembler/asmparserhelper.h"
#include "assembler/asmprogram.h"
#include "assembler/asmprogrammanager.h"
#include "memory/mainmemory.h"
#include "pep/pep.h"
#include "symbol/symboltable.h"
#include "symbol/symbolentry.h"
#include "symbol/symbolvalue.h"
#include "stack/typetags.h"
#include "symbol/symboltypes.h"

#include "pep9asmcode.h"

// Regular expressions for lexical analysis
QRegExp IsaParserHelper::rxAddrMode("^((,)(\\s*)(i|d|x|n|s(?![fx])|sx(?![f])|sf|sfx){1}){1}");
QRegExp IsaParserHelper::rxCharConst("^((\')(?![\'])(([^\'\\\\]){1}|((\\\\)([\'|b|f|n|r|t|v|\"|\\\\]))|((\\\\)(([x|X])([0-9|A-F|a-f]{2}))))(\'))");
QRegExp IsaParserHelper::rxComment("^((;{1})(.)*)");
QRegExp IsaParserHelper::rxDecConst("^((([+|-]{0,1})([0-9]+))|^(([1-9])([0-9]*)))");
QRegExp IsaParserHelper::rxDotCommand("^((.)(([A-Z|a-z]{1})(\\w)*))");
QRegExp IsaParserHelper::rxHexConst("^((0(?![x|X]))|((0)([x|X])([0-9|A-F|a-f])+)|((0)([0-9]+)))");
QRegExp IsaParserHelper::rxIdentifier("^((([A-Z|a-z|_]{1})(\\w*))(:){0,1})");
QRegExp IsaParserHelper::rxStringConst("^((\")((([^\"\\\\])|((\\\\)([\'|b|f|n|r|t|v|\"|\\\\]))|((\\\\)(([x|X])([0-9|A-F|a-f]{2}))))*)(\"))");

// Regular expressions for trace tag analysis
QRegExp IsaParserHelper::rxFormatTag("(#((1c)|(1d)|(1h)|(2d)|(2h))((\\d)+a)?(\\s|$))");
QRegExp IsaParserHelper::rxArrayTag("(#((1c)|(1d)|(1h)|(2d)|(2h))(\\d)+a)(\\s|$)?");
QRegExp IsaParserHelper::rxSymbolTag("#[a-zA-Z][a-zA-Z0-9]{0,7}");
QRegExp IsaParserHelper::rxArrayMultiplier("((\\d)+)a");

// Formats for trace tag error messages
const QString bytesAllocMismatch = ";WARNING: Number of bytes allocated (%1) not equal to number of bytes listed in trace tag (%2).";
const QString badTag = ";WARNING: %1 not specified in .EQUATE";
const QString neSymbol(";WARNING: Looked up a symbol that does not exist: %1");
const QString noEquate(";WARNING: Looked for existing symbol not defined in .EQUATE: %1");
const QString noSymbol(";WARNING: Trace tag with no symbol declaration");
const QString illegalAddrMode(";WARNING: Stack trace not possible unless immediate addressing is specified.");

IsaAsm::IsaAsm(AsmProgramManager &manager): manager(manager)
{

}

IsaAsm::~IsaAsm()
{

}

bool IsaAsm::assembleUserProgram(const QString &progText, QSharedPointer<AsmProgram> &progOut, QList<QPair<int, QString> > &errList)
{
    bool dotEndDetected = false, success = true;
    QString sourceLine, errorString;
    QStringList sourceCodeList = progText.split("\n");
    AsmCode *code;
    int lineNum = 0;
    QList<QSharedPointer<AsmCode>> programList;

    QSharedPointer<SymbolTable> symTable = QSharedPointer<SymbolTable>::create();

    int byteCount = 0;
    // Contains information about the address of a .BURN, and the size of memory burned.
    BURNInfo info;
    QSharedPointer<StaticTraceInfo> traceInfo = QSharedPointer<StaticTraceInfo>::create();
    while (lineNum < sourceCodeList.size() && !dotEndDetected) {
        sourceLine = sourceCodeList[lineNum];
        if (!IsaAsm::processSourceLine(symTable.data(), info, *traceInfo, byteCount,
                                       sourceLine, lineNum, code,
                                       errorString, dotEndDetected)) {
            errList.append(QPair<int,QString>{lineNum, errorString});
            return false;
        }
        // If a non-fatal error occured, log it to the error list.
        else if(!errorString.isEmpty()) {
            errList.append(QPair<int,QString>{lineNum, errorString});
            errorString.clear();
        }
        programList.append(QSharedPointer<AsmCode>(code));
        lineNum++;
    }

    // Insert charIn, charOut symbols if they have not been previously defined.
    quint16 chin, chout;
    if(symTable->exists("charIn") && symTable->getValue("charIn")->isUndefined()) {
        // According to the OS memory map vector, the location of chicharIn is
        // stored in the 6th and 7th bytes from the end of the operating system.
        // quint16 chinOffset = manager.getOperatingSystem()->getBurnValue() - 0x7;
        // memDevice->getWord(chinOffset, chin);
        // No longer use above approach, as it requires the operating system be
        // loaded in memory. Instead, diretly querry the operating system's symbol
        // table for the value of the symbol.
        chin = static_cast<quint16>(
                    manager.getOperatingSystem()->getSymbolTable()->getValue("charIn")->getValue());
        symTable->setValue("charIn", QSharedPointer<SymbolValueNumeric>::create(chin));
    }
    if(symTable->exists("charOut") && symTable->getValue("charOut")->isUndefined()) {
        // According to the OS memory map vector, the location of charOut is
        // stored in the 4th and 5th bytes from the end of the operating system.
        // quint16 choutOffset = manager.getOperatingSystem()->getBurnValue() - 0x5;
        // memDevice->getWord(choutOffset, chout);
        // No longer use above approach, as it requires the operating system be
        // loaded in memory. Instead, diretly querry the operating system's symbol
        // table for the value of the symbol.
        chout = static_cast<quint16>(
                    manager.getOperatingSystem()->getSymbolTable()->getValue("charOut")->getValue());
        symTable->setValue("charOut", QSharedPointer<SymbolValueNumeric>::create(chout));
    }

    // Perform whole program error checking.

    // Error where no .END occured.
    if (!dotEndDetected) {
        errorString = ";ERROR: Missing .END sentinel.";
        errList.append(QPair<int,QString>{0, errorString});
        success = false;
    }
    // Error where there the program is larger than memory.
    else if (byteCount > 65535) {
        errorString = ";ERROR: Object code size too large to fit into memory.";
        errList.append(QPair<int,QString>{0, errorString});
        success = false;
    }
    else if(info.burnCount != 0) {
        errorString = ";ERROR: Only operating systems may contain a .BURN.";
        errList.append(QPair<int,QString>{0, errorString});
        success = false;
    }
    // Error where an insturction use an undefined symbolic operand.
    else if(symTable->numUndefinedSymbols() > 0) {
        // Search through the program to find which instructions' operand references an undefined symbol
        for(int it = 0; it < programList.length(); it++) {
            if(programList[it]->hasSymbolicOperand() && programList[it]->getSymbolicOperand()->isUndefined()) {
                QString errorString =  QString(";ERROR: Symbol \"%1\" is undefined.").arg(programList[it]->getSymbolicOperand()->getName());
                errList.append(QPair<int,QString>{it, errorString});
            }
        }
        success = false;
    }
    // Finds remaining trace tags (structs, add/subsp) and handles malloc/heap allocation.
    handleTraceTags(*symTable.get(), *traceInfo.get(), programList, errList);
    progOut = QSharedPointer<AsmProgram>::create(programList, symTable, traceInfo);

    return success;
}

bool IsaAsm::assembleOperatingSystem(const QString &progText, bool forceBurnAt0xFFFF,
                                     QSharedPointer<AsmProgram> &progOut, QList<QPair<int, QString> > &errList)
{
    QStringList fileLines = progText.split("\n");
    QString sourceLine;
    QString errorString;
    AsmCode *code;
    int lineNum = 0;
    bool dotEndDetected = false, success = true;

    QSharedPointer<SymbolTable> symTable = QSharedPointer<SymbolTable>::create();
    QList<QSharedPointer<AsmCode>> programList;
    int byteCount = 0;
    BURNInfo info;
    QSharedPointer<StaticTraceInfo> traceInfo = QSharedPointer<StaticTraceInfo>::create();
    while (lineNum < fileLines.size() && !dotEndDetected) {
        sourceLine = fileLines[lineNum];
        if (!IsaAsm::processSourceLine(symTable.data(), info, *traceInfo.get(),
                                       byteCount, sourceLine, lineNum,
                                       code, errorString, dotEndDetected)) {
            return false;
        }
        // If a non-fatal error occured, log it to the error list.
        else if(!errorString.isEmpty()) {
            errList.append(QPair<int,QString>{lineNum, errorString});
            errorString.clear();
        }
        programList.append(QSharedPointer<AsmCode>(code));
        lineNum++;
    }

    // Perform whole program error checking.

    // Error where no .END occured.
    if (!dotEndDetected) {
        errorString = ";ERROR: Missing .END sentinel.";
        errList.append(QPair<int,QString>{0, errorString});
        success = false;
    }
    // Error where there the program is larger than memory.
    else if (byteCount > 65535) {
        errorString = ";ERROR: Object code size too large to fit into memory.";
        errList.append(QPair<int,QString>{0, errorString});
        success = false;
    }
    // Error where an insturction use an undefined symbolic operand.
    else if(symTable->numUndefinedSymbols() > 0) {
        // Search through the program to find which instructions' operand references an undefined symbol.
        for(int it = 0; it < programList.length(); it++) {
            if(programList[it]->hasSymbolicOperand() && programList[it]->getSymbolicOperand()->isUndefined()) {
                QString errorString =  QString(";ERROR: Symbol \"%1\" is undefined.").arg(programList[it]->getSymbolicOperand()->getName());
                errList.append(QPair<int,QString>{it, errorString});
            }
        }
        success = false;
    }
    else if (info.burnCount != 1) {
        errorString = ";ERROR: Operating systems must contain exactly 1 .BURN.";
        errList.append(QPair<int,QString>{0, errorString});
        success = false;
    }
    else if(forceBurnAt0xFFFF && info.burnValue != 0xFFFF) {
        success = false;
#pragma message("TODO: Insert error on correct line")
        errList.append({0,";ERROR: .BURN must have an argument of 0xFFFF."});
    }
    if(!success) return false;
    for(int it = 0; it < programList.size(); it++) {
        if(programList[it]->getMemoryAddress() < info.burnAddress) {
            programList[it]->setEmitObjectCode(false);
        }
    }

    // Adjust for .BURN.
    quint16 addressDelta = static_cast<quint16>(info.burnValue - byteCount + 1);
    info.startROMAddress = info.burnValue - (byteCount - info.burnAddress) +1;
    relocateCode(programList, addressDelta);
    symTable->setOffset(addressDelta);

    /*
     * Adjust for .ALIGNs before .BURN
     *
     * For any .ALIGNs before a .BURN, they should align in the opposite direction.
     * That is, instead of spanning from an arbitray starting address to the byte
     * before the desired alignment, they should end at the address of the next
     * instruction, and continue upwards in memory until the first byte of the
     * .ALIGN is on the proper byte boundary.
     *
     * This feature is needed so that arbitrary changes may be made to the
     * operating system without the user having to manually align the IO ports.
     *
     */
    // Find the .BURN directive
    int indexOfBurn = 0;
    for(int it = 0; it < programList.size(); it++) {
        if(dynamic_cast<DotBurn*>(programList[it].get()) != nullptr) {
            indexOfBurn = it;
            break;
        }
    }

    // Total number of bytes that each instruction needs to be shifted.
    int rollingOffset = 0;

    // Begin aligning every instruction from the .BURN upwards.
    for(int it = indexOfBurn; it > 0; it--) {
        // Every line of assembly code before a .BURN must be moved upward by the size of all preceding .ALIGN
        // directives, starting from the .BURN and moving upward.
        programList[it]->adjustMemAddress(rollingOffset);
        // Symbols that represent memory locations must also be adjusted.
        if(programList[it]->hasSymbolEntry()) {
            auto sym = symTable->getValue(programList[it]->getSymbolEntry()->getSymbolID());
            // Don't allow symbols with an .EQUATE, .ADDRSS to be re-adjusted.
            if(sym->getRawValue()->canRelocate()) {
                sym->setValue(QSharedPointer<SymbolValueLocation>::create(programList[it]->getMemoryAddress()));
            }
        }

        // If the instruction is a .ALIGN, then we must re-calculate the rolling offset.
        if(dynamic_cast<DotAlign*>(programList[it].get()) != nullptr) {
            // The instruction is known to be an ALIGN directive, so just cast it.
            DotAlign* asAlign = static_cast<DotAlign*>(programList[it].get());
            // The address of the .ALIGN.
            int startAddr = asAlign->getMemoryAddress();
            // The address of the last byte of the .ALIGN.
            int endAddr = startAddr + asAlign->getNumBytesGenerated();
            // Based on the ending byte, calculate where the first byte needs to be for proper alignment.
            int blockStart = endAddr - endAddr % asAlign->getArgument()->getArgumentValue();
            // The align must still reach down to endAddr, but now must span up to blockStart.
            asAlign->setNumBytesGenerated(blockStart - endAddr);
            // Other instructions will be shifter by the change in starting address.
            rollingOffset += blockStart - startAddr;
            asAlign->setMemoryAddress(blockStart);
        }
    }

    // Don't trace tags in the OS. We don't want to support this behavior, since
    // the operating system is not a translation of a C program.
    // handleTraceTags(*symTable.get(), *traceInfo, codeList, errList);
    traceInfo->hadTraceTags = false;
    progOut = QSharedPointer<AsmProgram>::create(programList, symTable, traceInfo, info.startROMAddress, info.burnValue);
    return true;
}

QPair<QSharedPointer<StructType>,QString> IsaAsm::parseStruct(const SymbolTable& symTable,QString name,
                                               QStringList symbols, StaticTraceInfo &traceInfo)
{
    // If there is no symbol associated with the struct-to-be, then assembly should fail.
    if(!symTable.exists(name)) return {nullptr, neSymbol.arg(name)};
    else {
        auto namePtr = symTable.getValue(name);
        QList<QSharedPointer<AType>> structList;
        // For every symbol tag in the tag list:
        for(auto string: symbols) {
            // If there is no symbol by that name, error.
            if(!symTable.exists(string)) {
                traceInfo.staticTraceError = true;
                return {nullptr, neSymbol.arg(string)};
            }
            // If there is no non-static storage space associated with that symbol
            // then it cannot appear in a struct. So error.
            else if(!traceInfo.dynamicAllocSymbolTypes.contains(symTable.getValue(string))) {
                traceInfo.staticTraceError = true;
                return {nullptr, noEquate.arg(string)};
            }
            else structList.append(traceInfo.dynamicAllocSymbolTypes[symTable.getValue(string)]);
        }
        // Otherwise create the struct, and return no error message.
        return {QSharedPointer<StructType>::create(namePtr, structList),""};
    }
}
template<typename T>
bool instanceof (AsmCode* inst) {
    return dynamic_cast<T*>(inst) != nullptr;

}
void IsaAsm::handleTraceTags(const SymbolTable& symTable, StaticTraceInfo& traceInfo,
                             QList<QSharedPointer<AsmCode>>& programList, QList<QPair<int, QString>> &errList)
{
    using namespace Pep9::ISA;
    // Extract the list of lines that have remaing trace tags.
    QList<QPair<int,QSharedPointer<AsmCode>>> structs, allocs;
    int lineIt = 0;
    for(auto line : programList) {
        // If a line is a non-unary instruction & the program has trace tags.
        if(traceInfo.hadTraceTags && dynamic_cast<NonUnaryInstruction*>(line.get()) != nullptr) {
            NonUnaryInstruction* instr = dynamic_cast<NonUnaryInstruction*>(line.get());
            switch(instr->getMnemonic()) {
            case EMnemonic::CALL:
                [[fallthrough]];
            case EMnemonic::ADDSP:
                [[fallthrough]];
            case EMnemonic::SUBSP:
                allocs.append({lineIt, line});
                break;
            default:
                break;
            }
        }
        // If a line doesn't have a comment or a #, it can't be a trace tag.
        if(!line->hasComment() || !hasSymbolTag(line->getComment()));
        else {
            // If a line has symbol tags and allocates storage
            // (local or global) for a struct.
            if(hasSymbolTag(line->getComment())
                    && (instanceof<DotBlock>(line.get()) ||
                        instanceof<DotEquate>(line.get()) ||
                        instanceof<DotByte>(line.get()) ||
                        instanceof<DotWord>(line.get())
                        )
                    ) {
                structs.append({lineIt, line});
            }
        }
        lineIt++;
    }

    // Construct struct definitions
    /*
     * Every iteration, attempt to define all presumed structs in the struct list.
     * If a struct can be defined this iteration, remove it from struct. If it can't be parsed,
     * leave it in the list.
     *
     * Since structs camn be definde in terms of each other, it is not sufficent to loop through the list 1 time.
     * To avoid getting stuck in the infinite loop of struct "A{ B item;}; struct B{ A item;};",
     * check that the length of structs shortens every pass through.
     * If there is ever a pass where it does not shorten, then there is a recursively defined struct.
     * Looping should be stopped, and errors should be emited.
     *
     */
    int lastLen;
    do {
        lastLen = structs.length();
        for(QMutableListIterator<QPair<int,QSharedPointer<AsmCode>>> it(structs); it.hasNext();) {
            auto line = it.next();
            // If a line with symbol tags listed does not contain a symbol definition, there is an error.
            if(!line.second->hasSymbolEntry()) {
                traceInfo.staticTraceError = true;
                break;
            }
            auto strDefs = extractTagList(line.second->getComment());
            // Attempt to parse the struct definition given previously defined primitives,
            // arrays, and structs. Will return non-null first if successful, or nullptr and
            // error if unsucessful.
            auto rVal = parseStruct(symTable, line.second->getSymbolEntry()->getName(), strDefs, traceInfo);
            // If parseStruct didn't null, it could sucessfully parse the tag,
            // so do remove it from the list of things todo.
            if(!rVal.first.isNull()) {
                it.remove();
            }
            // Otherwise continue working on the rest of the entries in the list.
            else {
                // Don't do anything with the error message now, since additional repetitions of the
                // loop might successfully define complicated structs.
                continue;
            }
            // Global structs - static allocated.
            if(dynamic_cast<DotBlock*>(line.second.get()) != nullptr) {
                DotBlock* instr = dynamic_cast<DotBlock*>(line.second.get());
                // Check that the allocated size of a global struct is the same size as the type tag.
                if(instr->getArgument()->getArgumentValue() != rVal.first->size()) {
                    traceInfo.staticTraceError = true;
                    errList.append({line.first, bytesAllocMismatch
                                    .arg(instr->getArgument()->getArgumentValue())
                                    .arg(rVal.first->size())});
                }
                else {
                    traceInfo.staticAllocSymbolTypes.insert(line.second->getSymbolEntry(), rVal.first);
                }
            }
            // Dynamic structs - stack or heap allocated.
            else {
                traceInfo.dynamicAllocSymbolTypes.insert(line.second->getSymbolEntry(), rVal.first);
            }
        }
    } while(structs.length() != lastLen);

    /* If a struct definition could not be parsed, there was an error.
    * So, use the parseStruct method to return the exact error message, and append it to the
    * offending line.
    */
    if(structs.length() != 0) {
        for(auto line : structs) {
            if(!line.second->hasSymbolEntry()) {
                traceInfo.staticTraceError = true;
                errList.append({line.first, noSymbol});
            }
            else {
                auto strDefs = extractTagList(line.second->getComment());
                auto ret = parseStruct(symTable, line.second->getSymbolEntry()->getName(), strDefs, traceInfo);
                errList.append({line.first, ret.second});
            }
        }
    }

    // Handle lines with instructions such as ADDSP, SUBSP.
    for(auto line: allocs) {
        // Used to force a continue on the outer loop from an inner loop.
        bool forceContinue = false;
        if(dynamic_cast<NonUnaryInstruction*>(line.second.get()) != nullptr) {
            NonUnaryInstruction *instr = static_cast<NonUnaryInstruction*>(line.second.get());
            QList<QSharedPointer<AType>> lineTypes;
            if(hasSymbolTag(line.second->getComment())) {
                QStringList texts = extractTagList(line.second->getComment());
                for(auto tag : texts) {
                // We allow empty tags (can occur to calls to malloc),
                // with a primitive type, but we disallow empty symbols.
                if(!symTable.exists(tag)) {
                    errList.append({line.first, badTag.arg(tag)});
                    // The rest of the checks on a ADDSP or SUBSP are meaningless
                    // if the tag list contains bad entries, so give up.
                    traceInfo.staticTraceError = true;
                    forceContinue = true;
                    continue;
                }
                auto symPtr = symTable.getValue(tag);
                if(!traceInfo.dynamicAllocSymbolTypes.contains(symPtr)) {
                    errList.append({line.first, badTag.arg(tag)});
                    // The rest of the checks on a ADDSP or SUBSP are meaningless
                    // if the tag list contains bad entries, so give up.
                    traceInfo.staticTraceError = true;
                    forceContinue = true;
                    continue;
                }
                else {
                    lineTypes.append(traceInfo.dynamicAllocSymbolTypes[symPtr]);
                }
            }
            }
            else if(hasArrayType(line.second->getComment())) {
                auto array = arrayType(extractTypeTags(line.second->getComment()));
                for(int it = 0; it < array.first; it++) {
                    lineTypes.append(QSharedPointer<LiteralPrimitiveType>::create("", array.second));
                }
            }
            else if(hasPrimitiveType(line.second->getComment())) {
                auto type = extractTypeTags(line.second->getComment());
                lineTypes.append(QSharedPointer<LiteralPrimitiveType>::create("", primitiveType(type)));

            }
            if(forceContinue) continue;
            // Calculate the number of bytes listed in the trace tags.
            quint16 size  = 0;
            quint16 address = static_cast<quint16>(instr->getMemoryAddress());
            for(auto tag : lineTypes) {
                size += tag->size();
            }
            // On lines where trace tags have significance, verify that the value of the argument
            // is equal to the number of bytes listed in the trace tags.
            if(instr->getAddressingMode() != EAddrMode::I) {
                traceInfo.staticTraceError = true;
                errList.append({line.first, illegalAddrMode});
                continue;
            }
            switch(instr->getMnemonic()) {
            case EMnemonic::ADDSP:
                if(instr->getArgument()->getArgumentValue() != size) {
                    traceInfo.staticTraceError = true;
                    errList.append({line.first, bytesAllocMismatch.arg(instr->getArgument()->getArgumentValue()).arg(size)});
                }
                else {
                    traceInfo.instrToSymlist[address] = lineTypes;
                }
                break;
            case EMnemonic::SUBSP:
                if(instr->getArgument()->getArgumentValue() != size) {
                    traceInfo.staticTraceError = true;
                    errList.append({line.first, bytesAllocMismatch.arg(instr->getArgument()->getArgumentValue()).arg(size)});
                }
                else {
                    for (auto type : lineTypes) type->reversed = true;
                    traceInfo.instrToSymlist[address] = lineTypes;
                }
                break;
            case EMnemonic::CALL:
                if(instr->hasSymbolicOperand()
                        && instr->getSymbolicOperand()->getName() == "malloc") {
                    traceInfo.instrToSymlist[address] = lineTypes;
                }
                break;
            default:
                qDebug() << "Should never get here";
                break;
            }
        }
    }

    // Detect if heap, malloc are present.
    // And if they are present, are they both locations?
    // E.G. malloc: .equate 0 would be wrong
    if(symTable.exists("malloc")
            && symTable.exists("heap")
            && symTable.getValue("malloc")->getRawValue()->getSymbolType() == SymbolType::ADDRESS
            && symTable.getValue("heap")->getRawValue()->getSymbolType() == SymbolType::ADDRESS) {
        traceInfo.hasHeapMalloc = true;
        traceInfo.heapPtr = symTable.getValue("heap");
        traceInfo.mallocPtr = symTable.getValue("malloc");
    }

    // Since model works, no need to print debug info, but retain code for future debugging.
    /*qDebug().noquote().nospace() << "Stack / Heap allocated types:";
    for(auto sym : traceInfo.dynamicAllocSymbolTypes) {
        qDebug().noquote().nospace() << sym->toPrimitives();
    }

    qDebug().noquote().nospace() << "Static allocated types:";
    for(auto sym : traceInfo.staticAllocSymbolTypes) {
        qDebug().noquote().nospace() << sym->toPrimitives();
    }

    qDebug().noquote().nospace() << "Instructions modifying stack / heap:";
    for(auto sym : traceInfo.instrToSymlist.keys()) {
        qDebug().noquote().nospace() << QString("(%1,").arg(sym) << traceInfo.instrToSymlist[sym] <<")";
    }*/

}

void IsaAsm::relocateCode(QList<QSharedPointer<AsmCode> > &codeList, quint16 addressDelta)
{
    for (int i = 0; i < codeList.length(); i++) {
        codeList[i]->adjustMemAddress(addressDelta);
    }
}

bool IsaAsm::processSourceLine(SymbolTable* symTable, BURNInfo& info, StaticTraceInfo& traceInfo,
                               int& byteCount, QString sourceLine, int lineNum, AsmCode *&code,
                               QString &errorString, bool &dotEndDetected, bool hasBreakpoint)
{
    using namespace Pep9::ISA;

    IsaParserHelper::ELexicalToken token; // Passed to getToken.
    QString tokenString; // Passed to getToken.
    QString localSymbolDef = ""; // Saves symbol definition for processing in the following state.
    Pep9::ISA::EMnemonic localEnumMnemonic; // Key to Pep:: table lookups.

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
    IsaParserHelper::ParseState state = IsaParserHelper::PS_START;
    do {
        if (!getToken(sourceLine, token, tokenString)) {
            errorString = tokenString;
            return false;
        }
        switch (state) {
        case IsaParserHelper::PS_START:
            if (token == IsaParserHelper::LT_IDENTIFIER) {
                if (mnemonToEnumMap.contains(tokenString.toUpper())) {
                    localEnumMnemonic = mnemonToEnumMap.value(tokenString.toUpper());
                    if (isUnaryMap.value(localEnumMnemonic)) {
                        unaryInstruction = new UnaryInstruction;
                        unaryInstruction->setMnemonic(localEnumMnemonic);
                        unaryInstruction->setBreakpoint(hasBreakpoint);
                        code = unaryInstruction;
                        code->setMemoryAddress(byteCount);
                        byteCount += 1; // One byte generated for unary instruction.
                        state = IsaParserHelper::PS_CLOSE;
                    }
                    else {
                        nonUnaryInstruction = new NonUnaryInstruction;
                        nonUnaryInstruction->setMnemonic(localEnumMnemonic);
                        nonUnaryInstruction->setBreakpoint(hasBreakpoint);
                        code = nonUnaryInstruction;
                        code->setMemoryAddress(byteCount);
                        byteCount += 3; // Three bytes generated for nonunary instruction.
                        state = IsaParserHelper::PS_INSTRUCTION;
                    }
                }
                else {
                    errorString = ";ERROR: Invalid mnemonic.";
                    return false;
                }
            }
            else if (token == IsaParserHelper::LT_DOT_COMMAND) {
                tokenString.remove(0, 1); // Remove the period
                tokenString = tokenString.toUpper();
                if (tokenString == "ADDRSS") {
                    dotAddrss = new DotAddrss;
                    code = dotAddrss;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_ADDRSS;
                }
                else if (tokenString == "ALIGN") {
                    dotAlign = new DotAlign;
                    code = dotAlign;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_ALIGN;
                }
                else if (tokenString == "ASCII") {
                    dotAscii = new DotAscii;
                    code = dotAscii;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_ASCII;
                }
                else if (tokenString == "BLOCK") {
                    dotBlock = new DotBlock;
                    code = dotBlock;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_BLOCK;
                }
                else if (tokenString == "BURN") {
                    dotBurn = new DotBurn;
                    code = dotBurn;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_BURN;
                }
                else if (tokenString == "BYTE") {
                    dotByte = new DotByte;
                    code = dotByte;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_BYTE;
                }
                else if (tokenString == "END") {
                    dotEnd = new DotEnd;
                    code = dotEnd;
                    // End symbol does not have a memory address
                    code->setMemoryAddress(byteCount);
                    dotEndDetected = true;
                    state = IsaParserHelper::PS_DOT_END;
                }
                else if (tokenString == "EQUATE") {
                    dotEquate = new DotEquate;
                    code = dotEquate;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_EQUATE;
                }
                else if (tokenString == "WORD") {
                    dotWord = new DotWord;
                    code = dotWord;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_WORD;
                }
                else {
                    errorString = ";ERROR: Invalid dot command.";
                    return false;
                }
            }
            else if (token == IsaParserHelper::LT_SYMBOL_DEF) {
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
                state = IsaParserHelper::PS_SYMBOL_DEF;
            }
            else if (token == IsaParserHelper::LT_COMMENT) {
                commentOnly = new CommentOnly;
                // Assignmed implicitly in setComment.
                // commentOnly->hasCom = true;
                commentOnly->setComment(tokenString);
                code = commentOnly;
                // Comments don't have a memory address
                code->setMemoryAddress(-1);
                state = IsaParserHelper::PS_COMMENT;
            }
            else if (token == IsaParserHelper::LT_EMPTY) {
                blankLine = new BlankLine;
                code = blankLine;
                // Neither do empty lines
                code->setMemoryAddress(-1);
                code->setSourceLineNumber(lineNum);
                state = IsaParserHelper::PS_FINISH;
            }
            else {
                errorString = ";ERROR: Line must start with symbol definition, mnemonic, dot command, or comment.";
                return false;
            }
            break;

        case IsaParserHelper::PS_SYMBOL_DEF:
            if (token == IsaParserHelper::LT_IDENTIFIER){
                if (mnemonToEnumMap.contains(tokenString.toUpper())) {
                    localEnumMnemonic = mnemonToEnumMap.value(tokenString.toUpper());
                    if (isUnaryMap.value(localEnumMnemonic)) {
                        unaryInstruction = new UnaryInstruction;
                        unaryInstruction->setSymbolEntry(symTable->getValue(localSymbolDef));
                        unaryInstruction->setMnemonic(localEnumMnemonic);
                        unaryInstruction->setBreakpoint(hasBreakpoint);
                        code = unaryInstruction;
                        code->setMemoryAddress(byteCount);
                        byteCount += 1; // One byte generated for unary instruction.
                        state = IsaParserHelper::PS_CLOSE;
                    }
                    else {
                        nonUnaryInstruction = new NonUnaryInstruction;
                        nonUnaryInstruction->setSymbolEntry(symTable->getValue(localSymbolDef));
                        nonUnaryInstruction->setMnemonic(localEnumMnemonic);
                        nonUnaryInstruction->setBreakpoint(hasBreakpoint);
                        code = nonUnaryInstruction;
                        code->setMemoryAddress(byteCount);
                        byteCount += 3; // Three bytes generated for unary instruction.
                        state = IsaParserHelper::PS_INSTRUCTION;
                    }
                }
                else {
                    errorString = ";ERROR: Invalid mnemonic.";
                    return false;
                }
            }
            else if (token == IsaParserHelper::LT_DOT_COMMAND) {
                tokenString.remove(0, 1); // Remove the period
                tokenString = tokenString.toUpper();
                if (tokenString == "ADDRSS") {
                    dotAddrss = new DotAddrss;
                    dotAddrss->setSymbolEntry(symTable->getValue(localSymbolDef));
                    code = dotAddrss;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_ADDRSS;
                }
                else if (tokenString == "ASCII") {
                    dotAscii = new DotAscii;
                    dotAscii->setSymbolEntry(symTable->getValue(localSymbolDef));
                    code = dotAscii;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_ASCII;
                }
                else if (tokenString == "BLOCK") {
                    dotBlock = new DotBlock;
                    dotBlock->setSymbolEntry(symTable->getValue(localSymbolDef));
                    code = dotBlock;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_BLOCK;
                }
                else if (tokenString == "BURN") {
                    dotBurn = new DotBurn;
                    dotBurn->setSymbolEntry(symTable->getValue(localSymbolDef));
                    code = dotBurn;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_BURN;
                }
                else if (tokenString == "BYTE") {
                    dotByte = new DotByte;
                    dotByte->setSymbolEntry(symTable->getValue(localSymbolDef));
                    code = dotByte;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_BYTE;
                }
                else if (tokenString == "END") {
                    dotEnd = new DotEnd;
                    dotEnd->setSymbolEntry(symTable->getValue(localSymbolDef));
                    code = dotEnd;
                    code->setMemoryAddress(byteCount);
                    dotEndDetected = true;
                    state = IsaParserHelper::PS_DOT_END;
                }
                else if (tokenString == "EQUATE") {
                    dotEquate = new DotEquate;
                    dotEquate->setSymbolEntry(symTable->getValue(localSymbolDef));
                    code = dotEquate;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_EQUATE;
                }
                else if (tokenString == "WORD") {
                    dotWord = new DotWord;
                    dotWord->setSymbolEntry(symTable->getValue(localSymbolDef));
                    code = dotWord;
                    code->setMemoryAddress(byteCount);
                    state = IsaParserHelper::PS_DOT_WORD;
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

        case IsaParserHelper::PS_INSTRUCTION:
            if (token == IsaParserHelper::LT_IDENTIFIER) {
                if (tokenString.length() > 8) {
                    errorString = ";ERROR: Symbol " + tokenString + " cannot have more than eight characters.";
                    return false;
                }
                if(!symTable->exists(tokenString)) symTable->insertSymbol(tokenString);
                nonUnaryInstruction->setArgument(QSharedPointer<SymbolRefArgument>::create(symTable->getValue(tokenString)));
                state = IsaParserHelper::PS_ADDRESSING_MODE;
            }
            else if (token == IsaParserHelper::LT_STRING_CONSTANT) {
                if (byteStringLength(tokenString) > 2) {
                    errorString = ";ERROR: String operands must have length at most two.";
                    return false;
                }
                nonUnaryInstruction->setArgument(QSharedPointer<StringArgument>::create(tokenString));
                state = IsaParserHelper::PS_ADDRESSING_MODE;
            }
            else if (token == IsaParserHelper::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                int value = tokenString.toInt(&ok, 16);
                if (value < 65536) {
                    nonUnaryInstruction->setArgument(QSharedPointer<HexArgument>::create(value));
                    state = IsaParserHelper::PS_ADDRESSING_MODE;
                }
                else {
                    errorString = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
                    return false;
                }
            }
            else if (token == IsaParserHelper::LT_DEC_CONSTANT) {
                bool ok;
                int value = tokenString.toInt(&ok, 10);
                if ((-32768 <= value) && (value <= 65535)) {
                    if (value < 0) {
                        value += 65536; // Stored as two-byte unsigned.
                        nonUnaryInstruction->setArgument(QSharedPointer<DecArgument>::create(value));
                    }
                    else {
                        nonUnaryInstruction->setArgument(QSharedPointer<UnsignedDecArgument>::create(value));
                    }
                    state = IsaParserHelper::PS_ADDRESSING_MODE;
                }
                else {
                    errorString = ";ERROR: Decimal constant is out of range (-32768..65535).";
                    return false;
                }
            }
            else if (token == IsaParserHelper::LT_CHAR_CONSTANT) {
                nonUnaryInstruction->setArgument(QSharedPointer<CharArgument>::create(tokenString));
                state = IsaParserHelper::PS_ADDRESSING_MODE;
            }
            else {
                errorString = ";ERROR: Operand specifier expected after mnemonic.";
                return false;
            }
            break;

        case IsaParserHelper::PS_ADDRESSING_MODE:
            if (token == IsaParserHelper::LT_ADDRESSING_MODE) {
                EAddrMode addrMode = IsaParserHelper::stringToAddrMode(tokenString);
                if ((static_cast<int>(addrMode) & addrModesMap.value(localEnumMnemonic)) == 0) { // Nested parens required.
                    errorString = ";ERROR: Illegal addressing mode for this instruction.";
                    return false;
                }
                nonUnaryInstruction->setAddressingMode(addrMode);
                state = IsaParserHelper::PS_CLOSE;
            }
            else if (addrModeRequiredMap.value(localEnumMnemonic)) {
                errorString = ";ERROR: Addressing mode required for this instruction.";
                return false;
            }
            else { // Must be branch type instruction with no addressing mode. Assign default addressing mode.
                nonUnaryInstruction->setAddressingMode(EAddrMode::I);
                if (token == IsaParserHelper::LT_COMMENT) {
                    code->setComment(tokenString);
                    state = IsaParserHelper::PS_COMMENT;
                }
                else if (token == IsaParserHelper::LT_EMPTY) {
                    code->setSourceLineNumber(lineNum);
                    state = IsaParserHelper::PS_FINISH;
                }
                else {
                    errorString = ";ERROR: Comment expected following instruction.";
                    return false;
                }
            }
            break;

        case IsaParserHelper::PS_DOT_ADDRSS:
            if (token == IsaParserHelper::LT_IDENTIFIER) {
                if (tokenString.length() > 8) {
                    errorString = ";ERROR: Symbol " + tokenString + " cannot have more than eight characters.";
                    return false;
                }
                if(!symTable->exists(tokenString)) symTable->insertSymbol(tokenString);
                dotAddrss->setArgument(QSharedPointer<SymbolRefArgument>::create(symTable->getValue(tokenString)));
                byteCount += 2;
                state = IsaParserHelper::PS_CLOSE;
            }
            else {
                errorString = ";ERROR: .ADDRSS requires a symbol argument.";
                return false;
            }
            break;

        case IsaParserHelper::PS_DOT_ALIGN:
            if (token == IsaParserHelper::LT_DEC_CONSTANT) {
                bool ok;
                int value = tokenString.toInt(&ok, 10);
                if (value == 2 || value == 4 || value == 8) {
                    int numBytes = (value - byteCount % value) % value;
                    dotAlign->setArgument(QSharedPointer<UnsignedDecArgument>::create(value));
                    dotAlign->setNumBytesGenerated(numBytes);
                    byteCount += numBytes;
                    state = IsaParserHelper::PS_CLOSE;
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

        case IsaParserHelper::PS_DOT_ASCII:
            if (token == IsaParserHelper::LT_STRING_CONSTANT) {
                dotAscii->setArgument(QSharedPointer<StringArgument>::create(tokenString));
                byteCount += byteStringLength(tokenString);
                state = IsaParserHelper::PS_CLOSE;
            }
            else {
                errorString = ";ERROR: .ASCII requires a string constant argument.";
                return false;
            }
            break;

        case IsaParserHelper::PS_DOT_BLOCK:
            if (token == IsaParserHelper::LT_DEC_CONSTANT) {
                bool ok;
                int value = tokenString.toInt(&ok, 10);
                if ((0 <= value) && (value <= 65535)) {
                    if (value < 0) {
                        value += 65536; // Stored as two-byte unsigned.
                        dotBlock->setArgument(QSharedPointer<DecArgument>::create(value));
                    }
                    else {
                        dotBlock->setArgument(QSharedPointer<UnsignedDecArgument>::create(value));
                    }
                    byteCount += value;
                    state = IsaParserHelper::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Decimal constant is out of range (0..65535).";
                    return false;
                }
            }
            else if (token == IsaParserHelper::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                int value = tokenString.toInt(&ok, 16);
                if (value < 65536) {
                    dotBlock->setArgument(QSharedPointer<HexArgument>::create(value));
                    byteCount += value;
                    state = IsaParserHelper::PS_CLOSE;
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

        case IsaParserHelper::PS_DOT_BURN:
            if (token == IsaParserHelper::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                int value = tokenString.toInt(&ok, 16);
                if (value < 65536) {
                    dotBurn->setArgument(QSharedPointer<HexArgument>::create(value));
                    info.burnCount++;
                    info.burnValue = value;
                    info.burnAddress = byteCount;
                    // The strating rom address cannot be calculated until the length of the program is known
                    // info.startROMAddress = ???;
                    state = IsaParserHelper::PS_CLOSE;
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

        case IsaParserHelper::PS_DOT_BYTE:
            if (token == IsaParserHelper::LT_CHAR_CONSTANT) {
                dotByte->setArgument(QSharedPointer<CharArgument>::create(tokenString));
                byteCount += 1;
                state = IsaParserHelper::PS_CLOSE;
            }
            else if (token == IsaParserHelper::LT_DEC_CONSTANT) {
                bool ok;
                int value = tokenString.toInt(&ok, 10);
                if ((-128 <= value) && (value <= 255)) {
                    if (value < 0) {
                        value += 256; // value stored as one-byte unsigned.
                    }
                    dotByte->setArgument(QSharedPointer<DecArgument>::create(value));
                    byteCount += 1;
                    state = IsaParserHelper::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Decimal constant is out of byte range (-128..255).";
                    return false;
                }
            }
            else if (token == IsaParserHelper::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                int value = tokenString.toInt(&ok, 16);
                if (value < 256) {
                    dotByte->setArgument(QSharedPointer<HexArgument>::create(value));
                    byteCount += 1;
                    state = IsaParserHelper::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Hex constant is out of byte range (0x00..0xFF).";
                    return false;
                }
            }
            else if (token == IsaParserHelper::LT_STRING_CONSTANT) {
                if (byteStringLength(tokenString) > 1) {
                    errorString = ";ERROR: .BYTE string operands must have length one.";
                    return false;
                }
                dotByte->setArgument(QSharedPointer<StringArgument>::create(tokenString));
                byteCount += 1;
                state = IsaParserHelper::PS_CLOSE;
            }
            else {
                errorString = ";ERROR: .BYTE requires a char, dec, hex, or string constant argument.";
                return false;
            }
            break;

        case IsaParserHelper::PS_DOT_END:
            if (token == IsaParserHelper::LT_COMMENT) {
                dotEnd->setComment(tokenString);
                code->setSourceLineNumber(lineNum);
                state = IsaParserHelper::PS_FINISH;
            }
            else if (token == IsaParserHelper::LT_EMPTY) {
                dotEnd->setComment(QString());
                code->setSourceLineNumber(lineNum);
                state = IsaParserHelper::PS_FINISH;
            }
            else {
                errorString = ";ERROR: Only a comment can follow .END.";
                return false;
            }
            break;

        case IsaParserHelper::PS_DOT_EQUATE:
            if (dotEquate->getSymbolEntry() == nullptr) {
                errorString = ";ERROR: .EQUATE must have a symbol definition.";
                return false;
            }
            else if (token == IsaParserHelper::LT_DEC_CONSTANT) {
                bool ok;
                int value = tokenString.toInt(&ok, 10);
                if ((-32768 <= value) && (value <= 65535)) {

                    if (value < 0) {
                        value += 65536; // Stored as two-byte unsigned.
                        dotEquate->setArgument(QSharedPointer<DecArgument>::create(value));
                    }
                    else {
                        dotEquate->setArgument(QSharedPointer<UnsignedDecArgument>::create(value));
                    }
                    dotEquate->getSymbolEntry()->setValue( QSharedPointer<SymbolValueNumeric>::create(value));
                    state = IsaParserHelper::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Decimal constant is out of range (-32768..65535).";
                    return false;
                }
            }
            else if (token == IsaParserHelper::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                int value = tokenString.toInt(&ok, 16);
                if (value < 65536) {
                    dotEquate->setArgument(QSharedPointer<HexArgument>::create(value));
                    dotEquate->getSymbolEntry()->setValue( QSharedPointer<SymbolValueNumeric>::create(value));
                    state = IsaParserHelper::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
                    return false;
                }
            }
            else if (token == IsaParserHelper::LT_STRING_CONSTANT) {
                if (byteStringLength(tokenString) > 2) {
                    errorString = ";ERROR: .EQUATE string operand must have length at most two.";
                    return false;
                }
                dotEquate->setArgument(QSharedPointer<StringArgument>::create(tokenString));
                dotEquate->getSymbolEntry()->setValue( QSharedPointer<SymbolValueNumeric>::create(string2ArgumentToInt(tokenString)));
                state = IsaParserHelper::PS_CLOSE;
            }
            else if (token == IsaParserHelper::LT_CHAR_CONSTANT) {
                dotEquate->setArgument(QSharedPointer<CharArgument>::create(tokenString));
                dotEquate->getSymbolEntry()->setValue( QSharedPointer<SymbolValueNumeric>::create(charStringToInt(tokenString)));
                state = IsaParserHelper::PS_CLOSE;
            }
            else {
                errorString = ";ERROR: .EQUATE requires a dec, hex, or string constant argument.";
                return false;
            }
            break;

        case IsaParserHelper::PS_DOT_WORD:
            if (token == IsaParserHelper::LT_CHAR_CONSTANT) {
                dotWord->setArgument(QSharedPointer<CharArgument>::create(tokenString));
                byteCount += 2;
                state = IsaParserHelper::PS_CLOSE;
            }
            else if (token == IsaParserHelper::LT_DEC_CONSTANT) {
                bool ok;
                int value = tokenString.toInt(&ok, 10);
                if ((-32768 <= value) && (value < 65536)) {

                    if (value < 0) {
                        value += 65536; // Stored as two-byte unsigned.
                        dotWord->setArgument(QSharedPointer<DecArgument>::create(value));
                    }
                    else {
                        dotWord->setArgument(QSharedPointer<UnsignedDecArgument>::create(value));
                    }
                    byteCount += 2;
                    state = IsaParserHelper::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Decimal constant is out of range (-32768..65535).";
                    return false;
                }
            }
            else if (token == IsaParserHelper::LT_HEX_CONSTANT) {
                tokenString.remove(0, 2); // Remove "0x" prefix.
                bool ok;
                int value = tokenString.toInt(&ok, 16);
                if (value < 65536) {
                    dotWord->setArgument(QSharedPointer<HexArgument>::create(value));
                    byteCount += 2;
                    state = IsaParserHelper::PS_CLOSE;
                }
                else {
                    errorString = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
                    return false;
                }
            }
            else if (token == IsaParserHelper::LT_STRING_CONSTANT) {
                if (byteStringLength(tokenString) > 2) {
                    errorString = ";ERROR: .WORD string operands must have length at most two.";
                    return false;
                }
                dotWord->setArgument(QSharedPointer<StringArgument>::create(tokenString));
                byteCount += 2;
                state = IsaParserHelper::PS_CLOSE;
            }
            else {
                errorString = ";ERROR: .WORD requires a char, dec, hex, or string constant argument.";
                return false;
            }
            break;

        case IsaParserHelper::PS_CLOSE:
            if (token == IsaParserHelper::LT_EMPTY) {
                code->setSourceLineNumber(lineNum);
                state = IsaParserHelper::PS_FINISH;
            }
            else if (token == IsaParserHelper::LT_COMMENT) {
                code->setComment(tokenString);
                state = IsaParserHelper::PS_COMMENT;
            }
            else {
                errorString = ";ERROR: Comment expected following instruction.";
                return false;
            }
            break;

        case IsaParserHelper::PS_COMMENT:
            if (token == IsaParserHelper::LT_EMPTY) {
                code->setSourceLineNumber(lineNum);
                state = IsaParserHelper::PS_FINISH;
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
    while (state != IsaParserHelper::PS_FINISH);
    // Parse trace tags
    // If a line has a symbolic or format tag, we must perform additional parsing.
    // Failure to check for symbol tags led to bug #80, where programs
    // only containing symbolic trace tags would assemble with no warnings.
    if(!code->hasComment() ||
            !(hasTypeTag(code->getComment()) ||
              hasSymbolTag(code->getComment()) )) {
        return true;
    }
    QString comment = code->getComment(), tag = extractTypeTags(comment);

    // If the line of code is a nonunary instruction, but not a stack modifying instruction,
    // then don't attempt any further parsing.
    auto nui = dynamic_cast<NonUnaryInstruction*>(code);
    if(     dynamic_cast<DotBlock*>(code) == nullptr &&
            dynamic_cast<DotWord*>(code) == nullptr &&
            dynamic_cast<DotEquate*>(code) == nullptr &&
            dynamic_cast<DotByte*>(code) == nullptr &&
            (nui == nullptr || (
            nui->getMnemonic() != EMnemonic::CALL &&
            nui->getMnemonic() != EMnemonic::SUBSP &&
            nui->getMnemonic() != EMnemonic::ADDSP))) {
    }
    else if(hasArrayType(tag)) {
        traceInfo.hadTraceTags = true;
        auto aTag = arrayType(tag);
        if(nui != nullptr &&
           nui->getMnemonic() == EMnemonic::CALL &&
           nui->getArgument()->getArgumentString() == "malloc") {
            auto item = QSharedPointer<LiteralArrayType>::create(aTag.second, aTag.first);
            if( nui != nullptr && nui->getMnemonic() == EMnemonic::SUBSP) item->reversed = true;
            traceInfo.dynamicAllocSymbolTypes.insert(code->getSymbolEntry(), item);
            return true;
        }
        if(!code->hasSymbolEntry()) {
            errorString = ";WARNING: given trace tag, but no symbol";
            traceInfo.staticTraceError = true;
            return true;
        }
        auto item = QSharedPointer<ArrayType>::create(code->getSymbolEntry(), aTag.second, aTag.first);

        // Global arrays - static alloacation
        if(dotBlock != nullptr) {
            // Check that size of allocated memory matches trace tag size
            if(dotBlock->getArgument()->getArgumentValue() != item->size()) {
                traceInfo.staticTraceError = true;
                errorString =   bytesAllocMismatch
                                .arg(dotBlock->getArgument()->getArgumentValue())
                                .arg(item->size());
               return true;
            }
            else {
                traceInfo.staticAllocSymbolTypes.insert(code->getSymbolEntry(), item);
            }
        }
        // Dynamic arrays - stack or heap allocation
        else {
            if( nui != nullptr && nui->getMnemonic() == EMnemonic::SUBSP) item->reversed = true;
            traceInfo.dynamicAllocSymbolTypes.insert(code->getSymbolEntry(), item);

        }
    }
    else if(hasPrimitiveType(tag)) {
        traceInfo.hadTraceTags = true;
        auto pTag = primitiveType(tag);
        if(nui != nullptr &&
           nui->getMnemonic() == EMnemonic::CALL &&
           nui->getArgument()->getArgumentString() == "malloc") {
            // It's alright for a call to malloc to have a primitive type tag.
            auto item = QSharedPointer<LiteralPrimitiveType>::create("", pTag);
            traceInfo.dynamicAllocSymbolTypes.insert(code->getSymbolEntry(), item);
            return true;
        }
        else if(!code->hasSymbolEntry()) {
            errorString = ";WARNING: given trace tag, but no symbol";
            traceInfo.staticTraceError = true;
            return true;
        }
        auto item = QSharedPointer<PrimitiveType>::create(code->getSymbolEntry(), pTag);


        // Global Primitives - static allocation
        if(dotBlock != nullptr) {
            // Check that size of allocated memory matches trace tag size
            if(dotBlock->getArgument()->getArgumentValue() != item->size()) {
                traceInfo.staticTraceError = true;
                errorString =   bytesAllocMismatch
                                .arg(dotBlock->getArgument()->getArgumentValue())
                                .arg(item->size());
               return true;
            }
            else {
                traceInfo.staticAllocSymbolTypes.insert(code->getSymbolEntry(), item);
            }
        }
        else if(dotWord != nullptr) {
            // Check that size of allocated memory matches trace tag size
            if(2 != item->size()) {
                traceInfo.staticTraceError = true;
                errorString =   bytesAllocMismatch
                                .arg(dotWord->getArgument()->getArgumentValue())
                                .arg(item->size());
               return true;
            }
            else {
                traceInfo.staticAllocSymbolTypes.insert(code->getSymbolEntry(), item);
            }
        }
        else if(dotByte != nullptr) {
            // Check that size of allocated memory matches trace tag size
            if(1 != item->size()) {
                traceInfo.staticTraceError = true;
                errorString =   bytesAllocMismatch
                                .arg(dotByte->getArgument()->getArgumentValue())
                                .arg(item->size());
               return true;
            }
            else {
                traceInfo.staticAllocSymbolTypes.insert(code->getSymbolEntry(), item);
            }
        }
        // Dynamic primitives - stack or heap allocation
        else {
            traceInfo.dynamicAllocSymbolTypes.insert(code->getSymbolEntry(), item);
        }
    }
    // Some (malformed) programs may only symbolic trace tags, and no format tags.
    // Without this line, only format tags trigger set the hadTraceTags field.
    // This led to issue #80.
    // So, if a symbolic trace tag is detected, we must note that trace tags
    // are present in the program, allowing handleTraceTags(...) to
    // detect the cases where a symbolic tag is undefined.
    else if(hasSymbolTag(tag)) {
        traceInfo.hadTraceTags = true;
    }
    return true;
}

bool IsaAsm::hasTypeTag(QString comment)
{
    // A comment can't be a type tag if it is empty or lacks a "#".
    if(comment.isEmpty() || !comment.contains(IsaParserHelper::rxFormatTag)) {
        return false;
    }
    // If the comment contains a primitive type (e.g. #2d, #2da), then it has a type.
    else if(primitiveType(comment.mid(comment.indexOf(IsaParserHelper::rxFormatTag))) != ESymbolFormat::F_NONE) {
        return true;
    }
    else {
        return false;
    }
}

QString IsaAsm::extractTypeTags(QString comment)
{
    return comment.mid(comment.indexOf(IsaParserHelper::rxFormatTag));
}

bool IsaAsm::hasPrimitiveType(QString typeTag)
{
    return typeTag.contains(IsaParserHelper::rxFormatTag);
}

bool IsaAsm::hasArrayType(QString formatTag)
{
    return hasPrimitiveType(formatTag)
            && formatTag.contains(IsaParserHelper::rxArrayTag);
}

bool IsaAsm::getToken(QString &sourceLine, IsaParserHelper::ELexicalToken &token, QString &tokenString)
{
    sourceLine = sourceLine.trimmed();
    if (sourceLine.length() == 0) {
        token = IsaParserHelper::LT_EMPTY;
        tokenString = "";
        return true;
    }
    QChar firstChar = sourceLine[0];
    IsaParserHelper::rxAddrMode.setCaseSensitivity (Qt::CaseInsensitive);  // Make rxAddrMode not case sensitive
    if (firstChar == ',') {
        if (IsaParserHelper::rxAddrMode.indexIn(sourceLine) == -1) {
            tokenString = ";ERROR: Malformed addressing mode.";
            return false;
        }
        token = IsaParserHelper::LT_ADDRESSING_MODE;
        tokenString = IsaParserHelper::rxAddrMode.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar == '\'') {
        if (IsaParserHelper::rxCharConst.indexIn(sourceLine) == -1) {
            tokenString = ";ERROR: Malformed character constant.";
            return false;
        }
        token = IsaParserHelper::LT_CHAR_CONSTANT;
        tokenString = IsaParserHelper::rxCharConst.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar == ';') {
        if (IsaParserHelper::rxComment.indexIn(sourceLine) == -1) {
            // This error should not occur, as any characters are allowed in a comment.
            tokenString = ";ERROR: Malformed comment";
            return false;
        }
        token = IsaParserHelper::LT_COMMENT;
        tokenString = IsaParserHelper::rxComment.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (startsWithHexPrefix(sourceLine)) {
        if (IsaParserHelper::rxHexConst.indexIn(sourceLine) == -1) {
            tokenString = ";ERROR: Malformed hex constant.";
            return false;
        }
        token = IsaParserHelper::LT_HEX_CONSTANT;
        tokenString = IsaParserHelper::rxHexConst.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if ((firstChar.isDigit() || firstChar == '+' || firstChar == '-')) {
        if (IsaParserHelper::rxDecConst.indexIn(sourceLine) == -1) {
            tokenString = ";ERROR: Malformed decimal constant.";
            return false;
        }
        token = IsaParserHelper::LT_DEC_CONSTANT;
        tokenString = IsaParserHelper::rxDecConst.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar == '.') {
        if (IsaParserHelper::rxDotCommand.indexIn(sourceLine) == -1) {
            tokenString = ";ERROR: Malformed dot command.";
            return false;
        }
        token = IsaParserHelper::LT_DOT_COMMAND;
        tokenString = IsaParserHelper::rxDotCommand.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar.isLetter() || firstChar == '_') {
        if (IsaParserHelper::rxIdentifier.indexIn(sourceLine) == -1) {
            // This error should not occur, as one-character identifiers are valid.
            tokenString = ";ERROR: Malformed identifier.";
            return false;
        }
        tokenString = IsaParserHelper::rxIdentifier.capturedTexts()[0];
        token = tokenString.endsWith(':') ?
                    IsaParserHelper::LT_SYMBOL_DEF :
                    IsaParserHelper::LT_IDENTIFIER;
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    if (firstChar == '\"') {
        if (IsaParserHelper::rxStringConst.indexIn(sourceLine) == -1) {
            tokenString = ";ERROR: Malformed string constant.";
            return false;
        }
        token = IsaParserHelper::LT_STRING_CONSTANT;
        tokenString = IsaParserHelper::rxStringConst.capturedTexts()[0];
        sourceLine.remove(0, tokenString.length());
        return true;
    }
    tokenString = ";ERROR: Syntax error.";
    return false;
}

ESymbolFormat IsaAsm::primitiveType(QString formatTag) {
    if (formatTag.startsWith("#1c")) return ESymbolFormat::F_1C;
    if (formatTag.startsWith("#1d")) return ESymbolFormat::F_1D;
    if (formatTag.startsWith("#2d")) return ESymbolFormat::F_2D;
    if (formatTag.startsWith("#1h")) return ESymbolFormat::F_1H;
    if (formatTag.startsWith("#2h")) return ESymbolFormat::F_2H;
    return ESymbolFormat::F_NONE; // Should not occur
}

QPair<quint8, ESymbolFormat> IsaAsm::arrayType(QString formatTag)
{
    auto type = primitiveType(formatTag);
    QRegularExpression matcher(IsaParserHelper::rxArrayMultiplier.pattern());
    auto match = matcher.match(formatTag);
    QString text = match.captured(0);
    text.chop(1);
    int size = text.toInt();
    return {static_cast<quint8>(size), type};

}

bool IsaAsm::hasSymbolTag(QString comment)
{
    return comment.contains(IsaParserHelper::rxSymbolTag);
}

QStringList IsaAsm::extractTagList(QString comment)
{
    QRegularExpression reg(IsaParserHelper::rxSymbolTag.pattern());
    auto items = reg.globalMatch(comment);
    QStringList out;
    while(items.hasNext()) {
        QString match = items.next().captured(0);
        match = match.mid(1);
        out.append(match);
    }
    return out;
}

Pep9::ISA::EAddrMode IsaParserHelper::stringToAddrMode(QString str)
{
    using namespace Pep9::ISA;
    str.remove(0, 1); // Remove the comma.
    str = str.trimmed().toUpper();
    if (str == "I") return EAddrMode::I;
    if (str == "D") return EAddrMode::D;
    if (str == "N") return EAddrMode::N;
    if (str == "S") return EAddrMode::S;
    if (str == "SF") return EAddrMode::SF;
    if (str == "X") return EAddrMode::X;
    if (str == "SX") return EAddrMode::SX;
    if (str == "SFX") return EAddrMode::SFX;
    return EAddrMode::NONE;
}
