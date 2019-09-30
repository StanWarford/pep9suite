#include "cpubuildhelper.h"
#include <QSharedPointer>
#include "microasm.h"
#include "microcode.h"
#include "microcodeprogram.h"
#include "symboltable.h"
#include "symbolentry.h"
#include "termhelper.h"

bool CPUBuildHelper::buildMicroprogram()
{
    // Construct files that will be needed for assembly
    QFile errorLog(QFileInfo(sourceFileInfo).absoluteDir().absoluteFilePath(
                       QFileInfo(sourceFileInfo).baseName() + "_errLog.txt"));

    auto result = buildMicroprogramHelper(type, useExtendedFeatures,
                                          source);
    // If there were errors, attempt to write all of them to the error file.
    // If the error file can't be opened, log that failure to standard output.
    if(!result.elist.isEmpty()) {
        if(!errorLog.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            qDebug().noquote() << errLogOpenErr.arg(errorLog.fileName());
        }
        else {
            QTextStream errAsStream(&errorLog);
            auto textList = source.split("\n");
            for(auto errorPair : result.elist) {
                errAsStream << textList[errorPair.first] << errorPair.second << endl;
            }
            // Error log should be flushed automatically.
            errorLog.close();
        }
    }

    // Only open & write object code file if assembly was successful.
    if(result.success) {
        // Program assembly can succeed despite the presence of errors in the
        // case of trace tag warnings. Must gaurd against this.
        if(result.elist.isEmpty()) {
            qDebug() << "Program assembled successfully.";
        }
    }
    else {
        qDebug() << "Error(s) generated. See error log.";
    }
    return result.success;
}

CPUBuildHelper::CPUBuildHelper(Enu::CPUType type,bool useExtendedFeatures,
                               const QString source, QFileInfo sourceFileInfo,
                               QObject *parent):
    QObject(parent), QRunnable(), type(type), useExtendedFeatures(useExtendedFeatures),
    source(source), sourceFileInfo(sourceFileInfo)
{

}

CPUBuildHelper::~CPUBuildHelper()
{

}

void CPUBuildHelper::run()
{
    // All set up work is done in build program, so all run needs to do is attempt
    if(buildMicroprogram()) {
       // Placeholder for potential work needing to be done after successful assembly.
    }

    // Application will live forever if we don't signal it to die.
    emit finished();
}

BuildResult buildMicroprogramHelper(Enu::CPUType type, bool useExtendedFeatures,
                                    const QString source)
{
    BuildResult result;
    // Returns true if object code is successfully generated (i.e. program is non-null).
    result.success = true;


    MicroAsm assembler(type, useExtendedFeatures);
    QSharedPointer<SymbolTable> symbolTable = QSharedPointer<SymbolTable>::create();
    QStringList sourceCodeList = source.split('\n');
    QString sourceLine;
    QString errorString;
    AMicroCode* code;
    QVector<AMicroCode*> codeList;
    int lineNumber = 0;
    while (lineNumber < sourceCodeList.size()) {
        sourceLine = sourceCodeList[lineNumber];
        if (!assembler.processSourceLine(symbolTable.data(), sourceLine, code, errorString)) {
            result.success = false;
            result.elist.append({lineNumber, errorString});
            // Create a dummy program that will delete all asm code entries
            QSharedPointer<MicrocodeProgram>::create(codeList, symbolTable);
            break;
        }
        if(code->isMicrocode() && static_cast<MicroCode*>(code)->hasControlSignal(Enu::EControlSignals::MemRead) &&
                static_cast<MicroCode*>(code)->hasControlSignal(Enu::EControlSignals::MemWrite)) {
            result.success = false;
            result.elist.append({0, "\\ ERROR: Can't have memread and memwrite"});
            // Create a dummy program that will delete all asm code entries
            QSharedPointer<MicrocodeProgram>::create(codeList, symbolTable);
            break;
        }
        codeList.append(code);
        lineNumber++;
    }
    if(!result.success) {
        return result;
    }
    result.program =  QSharedPointer<MicrocodeProgram>::create(codeList, symbolTable);
    for(auto sym : symbolTable->getSymbolEntries()) {
        if(sym->isUndefined()){
            result.elist.append({0,"// ERROR: Undefined symbol "+sym->getName()});
            result.success = false;
        }
        else if(sym->isMultiplyDefined()) {
            result.elist.append({0,"// ERROR: Multiply defined symbol "+sym->getName()});
            result.success = false;
        }
    }
    return result;
}
