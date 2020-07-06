// File: termhelper.cpp
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
#include "termhelper.h"

#include "assembler/asmcode.h"
#include "assembler/asmprogrammanager.h"
#include "assembler/asmprogram.h"
#include "assembler/isaasm.h"
#include "memory/amemorychip.h"
#include "memory/amemorydevice.h"
#include "memory/mainmemory.h"
#include "memory/memorychips.h"
#include "microassembler/microcode.h"
#include "microassembler/microcodeprogram.h"
#include "pep/pep.h"
#include "symbol/symbolentry.h"
#include "symbol/symboltable.h"

#include "boundexecisacpu.h"
#include "isacpu.h"

// Error messages potentially used in multiple places;
const QString errLogOpenErr = "Could not open file: %1.";
const QString hadErr        = "Errors/warnings encountered while generating output for file: %1.";
const QString assemble      = "About to assemble %1 into object file %2.";

QVector<quint8> convertObjectCodeToIntArray(QString program)
{
    bool ok = false;
    quint8 temp;
    QVector<quint8> output;
    program.replace(QRegExp("\n")," ");
    for(QString byte : program.split(" ")) {
        // toShort(...) should never throw any errors, so there should be no concerns if byte is not a hex constant.
        temp = static_cast<quint8>(byte.toShort(&ok, 16));
        // There could be a loss in precision if given text outside the range of an uchar but in range of a ushort.
        if(ok && byte.length()>0) output.append(temp);
    }
    return output;
}

void buildDefaultOperatingSystem(AsmProgramManager &manager)
{
    // Need to assemble operating system.
    QString defaultOSText = Pep::resToString(":/help-asm/figures/pep9os.pep", false);
    // If there is text, attempt to assemble it
    if(!defaultOSText.isEmpty()) {
        QSharedPointer<AsmProgram> prog;
        auto elist = QList<QPair<int, QString>>();
        IsaAsm assembler(manager);
        if(assembler.assembleOperatingSystem(defaultOSText, true, prog, elist)) {
            manager.setOperatingSystem(prog);
        }
        // If the operating system failed to assembly, we can't progress any further.
        // All application functionality depends on the operating system being defined.
        else {
            qDebug() << "OS failed to assemble.";
            auto textList = defaultOSText.split("\n");
            for(auto errorPair : elist) {
                qDebug() << textList[errorPair.first] << errorPair.second << "\n";
            }
            throw std::logic_error("The default operating system failed to assemble.");
        }
    }
    // If the operating system couldn't be found, we can't progress any further.
    // All application functionality depends on the operating system being defined.
    else {
        throw std::logic_error("Could not find default operating system.");
    }

}

