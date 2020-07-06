// File: asmprogrammanager.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2018 J. Stanley Warford & Matthew McRaven, Pepperdine University

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


#ifndef ASMPROGRAMMANAGER_H
#define ASMPROGRAMMANAGER_H
#include <QObject>
#include <QSharedPointer>
#include <QSet>
#include "isaasm.h"
class AsmProgram;
class SymbolTable;
/*
 * A class to manage the lifetime of user programs & the operating system.
 * Also helps communicate changes in breakpoints between assembler source &
 * listing panes, and the CPU Control Section.
 * It can also translate from a memory address to the assembly code program which contains that address.
 */
class AsmProgramManager: public QObject
{
    Q_OBJECT
public:
    struct AsmOutput {
        QSharedPointer<AsmProgram> prog;
        QList<QPair<int, QString>> errors;
        bool success;
    };
    QSharedPointer<AsmOutput> assembleOS(QString sourceCode, bool forceBurnAt0xFFFF);
    QSharedPointer<AsmOutput> assembleProgram(QString sourceCode);
    /*
     * The Pep/9 virtual machine specifies multiple address at the bottom of memory
     * that contain useful addresses.
     */
    enum MemoryVectors{
        UserStack, SystemStack, CharIn, CharOut, Loader, Trap
    };
    /* Number of bytes to subtract from the end of memory to get a particular memory
     * vector.
     */
    static quint16 getMemoryVectorOffset(MemoryVectors which);

    static AsmProgramManager* getInstance();
    // Get or set operating system code
    QSharedPointer<AsmProgram> getOperatingSystem();
    QSharedPointer<const AsmProgram> getOperatingSystem() const;
    void setOperatingSystem(QSharedPointer<AsmProgram> prog);
    quint16 getMemoryVectorValue(MemoryVectors vector) const;

    // Get or set user program
    QSharedPointer<AsmProgram> getUserProgram();
    QSharedPointer<const AsmProgram> getUserProgram() const;
    void setUserProgram(QSharedPointer<AsmProgram> prog);

    // Return the program that contains the address
    const AsmProgram* getProgramAt(quint16 address) const;
    AsmProgram* getProgramAt(quint16 address);

    // Return all breakpoints for the program counter
    QSet<quint16> getBreakpoints() const;
public slots:
    void onBreakpointAdded(quint16 address);
    void onBreakpointRemoved(quint16 address);
    void onRemoveAllBreakpoints();

signals:
    void breakpointAdded(quint16 address);
    void breakpointRemoved(quint16 address);
    void removeAllBreakpoints();
    void setBreakpoints(QSet<quint16> addresses);

private:
    AsmProgramManager(QObject* parent = nullptr);
    static AsmProgramManager* instance;
    QSharedPointer<AsmProgram> operatingSystem;
    QSharedPointer<AsmProgram> userProgram;

};

#endif // ASMPROGRAMMANAGER_H
