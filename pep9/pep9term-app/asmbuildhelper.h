// File: asmbuildhelper.h
/*
    Pep9Term is a  command line tool utility for assembling Pep/9 programs to
    object code and executing object code programs.

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

#ifndef ASMBUILDHELPER_H
#define ASMBUILDHELPER_H

#include <QtCore>
#include <QRunnable>

class AsmProgramManager;

/*
 * This class is responsible for assembling a single assembly language source file.
 * It takes an assembly language program's text as input, in addition to a
 * program manager which already has the default operating system installed
 *
 * If there are warnings or errors, a error log will be written to a file
 * of the same name as objName with the .extension replaced by _errLog.txt.
 *
 * If program assembly was successful (or the only warnings were trace tag issues),
 * then the object code text will be written to objFile. If objFile doesn't exist,
 * it will be created, and if it does, it will be truncated.
 *
 * If objFile doesn't exist at the end of the execution of this script, then
 * the file failed to assemble, and as such there must be an error log.
 *
 * When the assembler finishes running, or is terminated, finished() will be emitted
 * so that the application may shut down safely.
 */
class ASMBuildHelper: public QObject, public QRunnable {
    Q_OBJECT
public:
    explicit ASMBuildHelper(const QString source, QFileInfo objFileInfo, AsmProgramManager& manager,
                        QObject *parent = nullptr);
    ~ASMBuildHelper() override;


signals:
    // Signals fired when the computation completes (either successfully or due to an error),
    // or the simulation terminates due to exceeding the maximum number of allowed steps.
    void finished();

    // QRunnable interface
public:
    void run() override;
    // Pre: The operating system has been built and installed.
    // Pre: The Pep9 mnemonic maps have been initizialized correctly.
    // Pre: objFile's directory exists.

    // Instead of using the output file as a base file name, manually specify
    // error file path.
    void set_error_file(QString error_file);
private:
    const QString source;
    QFileInfo objFileInfo;
    AsmProgramManager& manager;
    QFileInfo error_log;
    // Helper method responsible for triggering program assembly.
    bool buildProgram();
};


#endif // ASMBUILDHELPER_H
