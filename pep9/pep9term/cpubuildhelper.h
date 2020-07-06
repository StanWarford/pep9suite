// File: cpubuildhelper.h
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
#ifndef CPUBUILDHELPER_H
#define CPUBUILDHELPER_H

#include <QFileInfo>
#include <QObject>
#include <QRunnable>

#include "microassembler/microcodeprogram.h"
#include "pep/enu.h"


// Result of a microcode assembler invocation.
struct MicrocodeAssemblyResult
{
    bool success;
    QList<QPair<int, QString>> elist;
    QSharedPointer<MicrocodeProgram> program;

};

// Helper function that assemble a microcode source program given a parameter list
MicrocodeAssemblyResult buildMicroprogramHelper(Enu::CPUType type,
                                                bool useExtendedFeatures,
                                                const QString source);
/*
 * This class is responsible for assembling a single microcode language source file.
 * Since there are many combinations of CPU data bus sizes and control section
 * configurations, these settings must be passed to the assembler.
 *
 * The input microcode source program must have its line numbers removed.
 * The logFileInfo will have "success" written to it if all compile check pass,
 * or if there are warnings or errors, a error log will be written to a file
 * of the same name as logFileInfo with the .extension replaced by _errLog.txt.
 *
 * When the microassembler finishes running, or is terminated, finished() will be emitted
 * so that the application may shut down safely.
 */
class CPUBuildHelper: public QObject, public QRunnable {
    Q_OBJECT
public:
    explicit CPUBuildHelper(Enu::CPUType type, bool useExtendedFeatures,
                            const QString source, QFileInfo source_file_info,
                            QObject *parent = nullptr);
    ~CPUBuildHelper() override;


signals:
    // Signals fired when the computation completes (either successfully or due to an error),
    // or the simulation terminates due to exceeding the maximum number of allowed steps.
    void finished();

    // QRunnable interface
public:
    void run() override;
    // Pre: The CPU type is one or two bytes.
    // Pre: If extended features are enabled, then the CPU type is two byte.
    // Pre: The Pep9 mnemonic maps have been initizialized correctly.
    // Pre: Microcode source program does not contain line numbers.
    // Pre: logFileInfo's directory exists.
    // Post:If assembly succeeded, "success" is written to logFileInfo.

    // Instead of using the output file as a base file name, manually specify
    // error file path.
    void set_error_file(QString error_file);
private:
    const Enu::CPUType type;
    bool useExtendedFeatures;
    const QString source;
    QFileInfo error_log;
    // Helper method responsible for triggering program assembly.
    bool buildMicroprogram();
};

#endif // CPUBUILDHELPER_H
