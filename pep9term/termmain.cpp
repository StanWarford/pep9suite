// File: main.cpp
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
#include <QCoreApplication>
#include <QtCore>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QThreadPool>

#include <optional>

#include "asmbuildhelper.h"
#include "asmrunhelper.h"
#include "asmprogrammanager.h"
#include "boundexecisacpu.h"
#include "cpubuildhelper.h"
#include "cpurunhelper.h"
#include "termhelper.h"
#include "mainmemory.h"
#include "memorychips.h"
#include "pep.h"

const QString asmInputFileText = "Input Pep/9 source program for assembler.";
const QString asmOutputFileText = "Output object code generated from source.";
const QString objInputFileText = "Input Pep/9 object code program for simulator";
const QString charinFileText = "File which will be buffered behind the charIn";
const QString charoutFileText = "File which charOut will be written to.";
const QString maxStepText = "The maximum number of steps executed before aborting. Defaults to %1";
const QString cpuasmInputFileText = "Input Pep/9 CPU source program for microassembler.";
const QString cpu1or2 = "Assemble the program with assuming a 2-byte data bus (default is 1).";
const QString cpuPreconditions = "Pep9 Microcode file containg pre and post conditions, overriding and skipping any preconditions and postconditions in the main microcode source file. ";
const QString cpuRunLog = "Text file that will contain \"Succes\" if the program passes unit tests, or will contain the violated unit tests.";

std::optional<QRunnable*> handle_asm(QCommandLineParser& parser, QCoreApplication &app);
std::optional<QRunnable*> handle_run(QCommandLineParser& parser, QCoreApplication &app);
std::optional<QRunnable*> handle_cpuasm(QCommandLineParser& parser, QCoreApplication &app);
std::optional<QRunnable*> handle_cpurun(QCommandLineParser& parser, QCoreApplication &app);

int main(int argc, char *argv[])
{
    // Initialize global state maps.
    Pep::initEnumMnemonMaps();
    Pep::initMnemonicMaps(true);
    Pep::initAddrModesMap();
    Pep::initDecoderTables();
    Pep::initMicroDecoderTables();

    // Construct an application so that we have a Qt main event loop.
    QCoreApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Pepperdine Computer Science Lab");
    QCoreApplication::setOrganizationDomain("cslab.pepperdine.edu");
    QCoreApplication::setApplicationName("Pep9Term");
    QCoreApplication::setApplicationVersion("9.2");

    // Create a parser
    QCommandLineParser parser;
    parser.addHelpOption();
    // Placeholder for one of the execution modes. Ideally, the parser would
    // have different options and positional arguments depending on the value of
    // the leading positional argument. However, the parser lacks this functionality.
    parser.addPositionalArgument("mode", "The mode Pep/ to be executed: Options are \"asm\" and \"run\".  \
Run pep9term 'mode' --help for more options.");
    parser.addOption(QCommandLineOption("about",
                                        "Display information about Qt & developers."));
    parser.parse(QCoreApplication::arguments());

    // Fetch the positional argument list, the first of which is the "mode"
    // Pep9term is to be run in.
    const QStringList args = parser.positionalArguments();
    const QString command = args.isEmpty() ? QString() : args.first();
    // Reconstruct the parser options based on the application's mode.
    if(parser.isSet("about")) {
        QFile aboutFile(":/help-term/about.txt");
        aboutFile.open(QIODevice::ReadOnly | QIODevice::Text);
        qDebug().noquote() << aboutFile.readAll();
        aboutFile.close();
        return 0;
    }
    else if (command == "asm") {
        parser.clearPositionalArguments();
        parser.addPositionalArgument("asm", "Assemble a Pep/9 source code program", "pep9term asm -i source.pep -o assembled.pepo");
        parser.addOption(QCommandLineOption("s", asmInputFileText, "source_file"));
        parser.addOption(QCommandLineOption("o", asmOutputFileText, "object_file"));
    }
    else if (command == "run") {
        parser.clearPositionalArguments();
        parser.addPositionalArgument("run", "Run an object code program.",
                                     "pep9term run -s asm.pepo -i charIn.txt, -o charOut.txt");
        parser.addOption(QCommandLineOption("s", objInputFileText, "object_source_file"));
        // Batch input that will be loaded into charIn.
        parser.addOption(QCommandLineOption("i", charinFileText, "char_input"));
        // File where values written to charOut will be stored.
        parser.addOption(QCommandLineOption("o", charoutFileText, "char_output"));
        // Maximum number of steps.
        parser.addOption(QCommandLineOption("m",
                                            maxStepText.arg(BoundExecIsaCpu::getDefaultMaxSteps()),
                                            "max_Steps"));
    }
    else if(command == "cpuasm") {
        parser.clearPositionalArguments();
        parser.addPositionalArgument("cpuasm", "Assemble a Pep/9 CPU source code program. \
It will write any errors to output if assembly fails or \"success\" if assembly succeeds.",
                                     "pep9term cpuasm -i source.pepcpu -o output.txt");
        parser.addOption(QCommandLineOption("m", cpuasmInputFileText, "source_file"));
        parser.addOption(QCommandLineOption("o", cpuasmOutputFileText, "output_log_file"));
        parser.addOption(QCommandLineOption("d2", cpu1or2));
    }
    else if (command == "cpurun") {
        parser.clearPositionalArguments();
        parser.addPositionalArgument("cpurun", "Run a microcode program with an optional list of preconditions.",
                                     "pep9term cpurun -m microcode.pepcpu -p pre.pepcpu -o log.txt");
        parser.addOption(QCommandLineOption("m", cpuasmInputFileText, "micro_source_file"));
        parser.addOption(QCommandLineOption("d2", cpu1or2));
        parser.addOption(QCommandLineOption("p", cpuPreconditions, "precondition_source_file"));
        parser.addOption(QCommandLineOption("o", cpuRunLog, "output_log_file"));
    }
    // Otherwise it's an invalid mode, return an error and have the help
    // documentation appear
    else {
        parser.showHelp(-1);
    }

    // Must re-add help documentation to ensure -h is still a valid option.
    auto helpOption = parser.addHelpOption();
    parser.process(a);

    // Task that will be
    std::optional<QRunnable *> run;

    // Assembly the default operating system from this thread, so that
    // no worker threads have to check for the presence of an operating system.
    buildDefaultOperatingSystem(*AsmProgramManager::getInstance());

    if (command == "asm") {
        run = handle_asm(parser, a);
    }
    else if(command == "run") {
        run = handle_run(parser, a);
    }
    else if (command == "cpuasm") {
        run = handle_cpuasm(parser, a);
    }
    else if (command == "cpurun") {
        run = handle_cpurun(parser, a);
    }
    else {
        parser.showHelp(0);
    }

    /*
     * This asynchronous approach must be used, because if quit() is called
     * before a.exec() happens, then the application will not actually quit.
     * So, by causing the task to be scheduled via the event loop, we allow
     * the task to be scheduled via the main event loop.
     *
     */
    QThreadPool pool;
    if(!run.has_value()) {
        return -1;
    }
    else {
        pool.start(*run);
    }

    return a.exec();
}

std::optional<QRunnable*> handle_asm(QCommandLineParser& parser, QCoreApplication& app)
{
    // Needs both an input and output source to be a well-defined command.
    if(!parser.isSet("s") || !parser.isSet("o")) {
        qDebug() << "Invalid option combination";
        parser.showHelp(-1);
    }

    // File names associated with cli parameters.
    QString sourceFileString = parser.value("s");
    QString objectFileString = parser.value("o");

    QFile sourceFile(sourceFileString);
    QString sourceText;
    if(!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug().noquote() << errLogOpenErr.arg(sourceFile.fileName());
        parser.showHelp(-1);
    }
    else {
        QTextStream sourceStream(&sourceFile);
        sourceText = sourceStream.readAll();
        sourceFile.close();

        AsmBuildHelper *helper = new AsmBuildHelper(sourceText, objectFileString, *AsmProgramManager::getInstance());
        QObject::connect(helper, &AsmBuildHelper::finished, &app, &QCoreApplication::quit);

        return helper;
    }
}

std::optional<QRunnable*> handle_run(QCommandLineParser& parser, QCoreApplication &app)
{
    // Needs both an source program, input & output to be a well-defined command.
    if(!parser.isSet("s") || !parser.isSet("o")) {
        qDebug() << "Invalid option combination";
        parser.showHelp(-1);
    }

    // File names associated with cli parameters.
    QString objCodeFileName = parser.value("s");
    QString textInputFileName = parser.value("i");
    QString textOutputFileName = parser.value("o");
    QString stepMaxValue = parser.value("m");
    // Load object code string from file if possible, else print error log.
    QFile objFile(objCodeFileName);
    if(!objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug().noquote() << errLogOpenErr.arg(objFile.fileName());
        parser.showHelp(-1);
    }

    QTextStream objStream(&objFile);
    QString objText = objStream.readAll();
    objFile.close();

    // Attempt to parse stepMax string as an integer.
    bool stepConvWorked;
    quint64 maxSimSteps = stepMaxValue.toULong(&stepConvWorked);
    if(!stepConvWorked || maxSimSteps == 0) {
        maxSimSteps = BoundExecIsaCpu::getDefaultMaxSteps();
    }
    ASMRunHelper *helper = new ASMRunHelper(objText,maxSimSteps, textOutputFileName,
                                      textInputFileName, *AsmProgramManager::getInstance());
    QObject::connect(helper, &ASMRunHelper::finished, &app, &QCoreApplication::quit);

    return helper;
}

std::optional<QRunnable*> handle_cpuasm(QCommandLineParser& parser, QCoreApplication &app)
{
    // Needs a source program to be well defined.
    if(!parser.isSet("m")) {
        qDebug() << "Must set microcode program.";
        parser.showHelp(-1);
    }
    // Needs an output log to be well defined.
    else if(!parser.isSet("o")) {
        qDebug() << "Must set output log.";
        parser.showHelp(-1);
    }
    QString outputFileName = parser.value("o");

    // Determine CPU type.
    Enu::CPUType type = Enu::CPUType::OneByteDataBus;
    if(parser.isSet("d2")) {
       type = Enu::CPUType::TwoByteDataBus;
    }

    Pep::initMicroEnumMnemonMaps(type, false);

    // Setup parsing of microcode program.
    QString sourceFileString = parser.value("m");
    QFile sourceFile(sourceFileString);
    QString sourceText;

    // Check that the file can be opened, otherwise log an error to the console.
    if(!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug().noquote() << errLogOpenErr.arg(sourceFile.fileName());
        parser.showHelp(-1);
    }
    else {
        // Otherwise read the file.
        QTextStream sourceStream(&sourceFile);
        // Must remove line numbers, on microassembler will raise spurious errors.
        sourceText = Pep::removeCycleNumbers(sourceStream.readAll());
        sourceFile.close();

        CPUBuildHelper *helper = new CPUBuildHelper(type, false, sourceText,
                                              QFileInfo(outputFileName));
        QObject::connect(helper, &CPUBuildHelper::finished, &app, &QCoreApplication::quit);

        return helper;
    }

}

std::optional<QRunnable*> handle_cpurun(QCommandLineParser& parser, QCoreApplication &app)
{

    // Needs a source program, output file to be well defined
    if(!parser.isSet("m") || !parser.isSet("o")) {
        qDebug() << "Need to set the (m)icrocode program and (o)utput file.";
        parser.showHelp(-1);
    }
    // Determine CPU type.
    Enu::CPUType type = Enu::CPUType::OneByteDataBus;
    if(parser.isSet("d2")) {
       type = Enu::CPUType::TwoByteDataBus;
    }

    Pep::initMicroEnumMnemonMaps(type, false);

    // Microcode input file and unit test output file.
    QString microcodeFileName = parser.value("m");
    QString textOutputFileName = parser.value("o");

    // Load object code string from file if possible, else print error log.
    QFile microcodeFile(microcodeFileName);
    if(!microcodeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug().noquote() << errLogOpenErr.arg(microcodeFile.fileName());
        parser.showHelp(-1);
    }

    QTextStream microprogramStream(&microcodeFile);
    QString microprogramText = Pep::removeCycleNumbers(microprogramStream.readAll());
    microcodeFile.close();

    // Load overriding preconditions if present.
    QString preconditionText;
    if(parser.isSet("p")) {
        QString preconditionFileName = parser.value("p");
        QFile preconditionFile(preconditionFileName);
        // If passed precondition file that can't be opened, raise an error.
        if(!preconditionFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug().noquote() << errLogOpenErr.arg(microcodeFile.fileName());
            parser.showHelp(-1);
        }

        QTextStream preconditionStream(&preconditionFile);
        preconditionText = Pep::removeCycleNumbers(preconditionStream.readAll());
        preconditionFile.close();
    }


    CPURunHelper *helper = new CPURunHelper(type, microprogramText,
                                           QFileInfo(microcodeFile), preconditionText,
                                           textOutputFileName, nullptr);
    QObject::connect(helper, &CPURunHelper::finished, &app, &QCoreApplication::quit);

    return helper;
}
