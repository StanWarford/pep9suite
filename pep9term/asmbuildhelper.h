#ifndef ASMBUILDHELPER_H
#define ASMBUILDHELPER_H

#include <QtCore>
#include <QRunnable>

class AsmProgramManager;

/*
 * This class is responsible for assembling a single assembly language source file.
 * Takes an assembly language program's text as input, in addition to a program manager
 * which already has the default operating system installed
 *
 * If there are warnings or errors, a error log will be written to a file
 * of the same name as objName with the .extension replaced by -errLog.txt.
 *
 * If program assembly was successful (or the only warnings were trace tag issues),
 * then the object code text will be written to objFile. If objFile doesn't exist,
 * it will bre created, and if it does, it will be truncated.
 *
 * If objFile doesn't exist at the end of the execution of this script, then
 * the file failed to assemble, and as such there must be an error log.
 *
 * When the assembler finishes running, or is terminated, finished() will be emitted
 * so that the application may shut down safely.
 */
class AsmBuildHelper: public QObject, public QRunnable {
    Q_OBJECT
public:
    explicit AsmBuildHelper(const QString source, QFileInfo objFileInfo, AsmProgramManager& manager,
                        QObject *parent = nullptr);
    ~AsmBuildHelper() override;


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
private:
    const QString source;
    QFileInfo objFileInfo;
    AsmProgramManager& manager;
    // Helper method responsible for triggering program assembly.
    bool buildProgram();
};


#endif // ASMBUILDHELPER_H
