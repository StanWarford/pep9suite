#include <QCoreApplication>
#include <QtCore>
#include <QDebug>
#include <QThreadPool>
#include "termhelper.h"
#include "mainmemory.h"
#include "memorychips.h"
#include "asmprogrammanager.h"
#include "pep.h"
#include <QCommandLineOption>
#include <QCommandLineParser>
QString asmInputFileText = "Input Pep/9 source program for assembler";
QString asmOutputFileText = "Output object code generated from source.";
int main(int argc, char *argv[])
{
    // Construct an application so that we have a Qt main event loop.
    QCoreApplication a(argc, argv);

    // Initialize global state maps.
    Pep::initEnumMnemonMaps();
    Pep::initMnemonicMaps();
    Pep::initAddrModesMap();
    Pep::initDecoderTables();

    QCoreApplication::setApplicationName("pep9term");
    QCoreApplication::setApplicationVersion("9.2");

    // Create a parser
    QCommandLineParser parser;
    parser.addHelpOption();
    // Placeholder for one of the execution modes. Ideally, the parser would
    // have different options and positional arguments depending on the value of
    // the leading positional argument. However, the parser lacks this functionality.
    parser.addPositionalArgument("mode", "The mode Pep/ to be executed: Options are \"asm\" and \"run\".  \
Run pep9term 'mode' --help for more options.");
    parser.parse(QCoreApplication::arguments());

    // Fetch the positional argument list, the first of which is the "mode"
    // Pep9term is to be run in.
    const QStringList args = parser.positionalArguments();
    const QString command = args.isEmpty() ? QString() : args.first();
    // Reconstruct the parser options based on the application's mode.
    if (command == "asm") {
        parser.clearPositionalArguments();
        parser.addPositionalArgument("asm", "Assemble a Pep/9 source code program", "pep9term asm -i source.pep -o assembled.pepo");
        parser.addOption(QCommandLineOption("i", asmInputFileText, "source_file"));
        parser.addOption(QCommandLineOption("o", asmOutputFileText, "object_file"));
    }
    else if (command == "run") {
        parser.clearPositionalArguments();
        parser.addPositionalArgument("run", "Run an object code program.",
                                     "pep9term run -s asm.pepo -i charInInput.txt, -o charOut.txt");
        parser.addOption(QCommandLineOption("s", asmInputFileText, "object_files"));
        // Batch input that will be loaded into charIn.
        parser.addOption(QCommandLineOption("i", asmOutputFileText, "text_input"));
        // File where values written to charOut will be stored.
        parser.addOption(QCommandLineOption("o", asmOutputFileText, "text_output"));
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
    QRunnable *run;

    // Assembly the default operating system from this thread, so that
    // no worker threads have to check for the presence of an operating system.
    buildDefaultOperatingSystem(*AsmProgramManager::getInstance());

    // Make sure when the task finishes that the application quits.
    // This can be bound as the receiver of a signal from the worker threads.
    auto quitLambda = [&](){a.quit();};

    if (command == "asm") {
        // Needs both an input and output source to be a well-defined command.
        if(!parser.isSet("i") || !parser.isSet("o")) {
            qDebug() << "Invalid option combination";
            parser.showHelp(-1);
        }

        // File names associated with cli parameters.
        QString inputFileString = parser.value("i");
        QString outputFileString = parser.value("o");

        QFile inputFile(inputFileString);
        QString inputText;
        if(!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug().noquote() << errLogOpenErr.arg(inputFile.fileName());
            parser.showHelp(-1);
        }
        else {
            QTextStream inputStream(&inputFile);
            inputText = inputStream.readAll();
            inputFile.close();

            BuildHelper *helper = new BuildHelper(inputText, outputFileString, *AsmProgramManager::getInstance());
            QObject::connect(helper, &BuildHelper::finished, quitLambda);

            run = helper;
        }
    }
    else if(command == "run") {
        // Needs both an source program, input & output to be a well-defined command.
        if(!parser.isSet("s") || !parser.isSet("i") || !parser.isSet("o")) {
            qDebug() << "Invalid option combination";
            parser.showHelp(-1);
        }

        // File names associated with cli parameters.
        QString objCodeFileName = parser.value("s");
        QString textInputFileName = parser.value("i");
        QString textOutputFileName = parser.value("o");

        QFile objFile(objCodeFileName);
        if(!objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug().noquote() << errLogOpenErr.arg(objFile.fileName());
            parser.showHelp(-1);
        }

        QTextStream inputStream(&objFile);
        QString objText = inputStream.readAll();
        objFile.close();

        RunHelper *helper = new RunHelper(objText, textOutputFileName, textInputFileName,
                                          *AsmProgramManager::getInstance());
        QObject::connect(helper, &RunHelper::finished, quitLambda);

        run = helper;

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
    pool.start(run);
    return a.exec();
}
