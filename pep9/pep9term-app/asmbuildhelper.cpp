// File: asmbuildhelper.cpp
/*
    Pep9Term is a  command line tool utility for assembling Pep/9 programs to
    object code and executing object code programs.

    Copyright (C) 2019  J. Stanley Warford & Matthew McRaevn, Pepperdine University

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
#include "asmbuildhelper.h"

#include "assembler/asmcode.h"
#include "assembler/asmprogram.h"
#include "assembler/asmprogrammanager.h"
#include "pep/pep.h"
#include "symbol/symbolentry.h"
#include "symbol/symboltable.h"

#include "pep9isaasm.h"
#include "termhelper.h"

ASMBuildHelper::ASMBuildHelper(const QString source, QFileInfo objFileInfo,
                         AsmProgramManager &manager, QObject *parent): QObject(parent),
    QRunnable(), source(source), objFileInfo(objFileInfo), manager(manager)
{
    // Default error log name to the base name of the file with an _errLog.txt extension.
    this->error_log = objFileInfo.absoluteDir().absoluteFilePath(objFileInfo.baseName() + "_errLog.txt");
}

ASMBuildHelper::~ASMBuildHelper()
{
    // All of our memory is owned by sharedpointers, so we
    // should not attempt to delete anything ourselves.
}

void ASMBuildHelper::run()
{
    // All set up work is done in build program, so all run needs to do is attempt
    if(buildProgram()) {
       // Placeholder for potential work needing to be done after successful assembly.
    }

    // Application will live forever if we don't signal it to die.
    emit finished();
}

void ASMBuildHelper::set_error_file(QString error_file)
{
    this->error_log = error_file;
}

bool ASMBuildHelper::buildProgram()
{
    // Construct files that will be needed for assembly
    QFile objectFile(objFileInfo.absoluteFilePath());
    QFile errorLog(this->error_log.absoluteFilePath());

    QSharedPointer<AsmProgram> program;
    auto elist = QList<QPair<int, QString>>();
    IsaAsm assmembler(manager);

    // Returns true if object code is successfully generated (i.e. program is non-null).
    bool success = assmembler.assembleUserProgram(source, program, elist);

    // If there were errors, attempt to write all of them to the error file.
    // If the error file can't be opened, log that failure to standard output.
    if(!elist.isEmpty()) {
        if(!errorLog.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            qDebug().noquote() << errLogOpenErr.arg(errorLog.fileName());
        }
        else {
            QTextStream errAsStream(&errorLog);
            auto textList = source.split("\n");
            for(auto errorPair : elist) {
                // The first element of the error pair is the line number which
                // caused the error, allowing us to write the offending line
                // and error message to the console.
                errAsStream << textList[errorPair.first] << errorPair.second << "\n";
            }
            // Error log should be flushed automatically.
            errorLog.close();
        }
    }

    // Only open & write object code file if assembly was successful.
    if(success) {
        // Program assembly can succeed despite the presence of errors in the
        // case of trace tag warnings. Must gaurd against this.
        if(elist.isEmpty()) {
            qDebug() << "Program assembled successfully.";
        }
        else {
            qDebug() << "Warning(s) generated. See error log.";
        }
        // Attempt to open object code file. Write error to standard out if it fails.
        if(!objectFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            qDebug().noquote() << errLogOpenErr.arg(objectFile.fileName());
        }
        else {
            // Below code copeid from object code pane's formatting
            QString objectCodeString = "";
            auto objectCode = program->getObjectCode();
            for (int i = 0; i < objectCode.length(); i++) {
                objectCodeString.append(QString("%1").arg(objectCode[i], 2, 16, QLatin1Char('0')).toUpper());
                objectCodeString.append((i % 16) == 15 ? '\n' : ' ');
            }
            objectCodeString.append("zz");
            QTextStream objStream(&objectFile);
            objStream << objectCodeString << "\n";
            objectFile.close();
        }

        // Also attempt to generate listing file from assembled program as well.
        QFile listingFile(QFileInfo(objectFile).absoluteDir().absoluteFilePath(
                              QFileInfo(objectFile).baseName() + ".pepl"));
        // If listing can't be opened, log an error to the console.
        if(!listingFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            qDebug().noquote() << errLogOpenErr.arg(listingFile.fileName());
        }
        // Otherwise, write out the listing (and symbol table) to the file.
        else {
            QTextStream listingStream(&listingFile);
            listingStream << program->getProgramListing();
            if(!program->getSymbolTable()->getSymbolMap().isEmpty()) {
                listingStream << "\n";
                listingStream << program->getSymbolTable()->getSymbolTableListing();
            }
            listingFile.close();
        }
    }
    else {
        qDebug() << "Error(s) generated. See error log.";
    }
    return success;
}
