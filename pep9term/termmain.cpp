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
#include <QDebug>
#include <QThreadPool>

#include <functional>
#include <optional>

#include "asmbuildhelper.h"
#include "asmrunhelper.h"
#include "asmprogrammanager.h"
#include "boundexecisacpu.h"
#include "boundexecmicrocpu.h"
#include "CLI11.hpp"
#include "cpubuildhelper.h"
#include "cpurunhelper.h"
#include "termhelper.h"
#include "mainmemory.h"
#include "memorychips.h"
#include "microstephelper.h"
#include "pep.h"

const std::string asm_description = "Assemble a Pep/9 assembler source code program to object code.";
const std::string run_description = "Run a Pep/9 object code program.";
const std::string cpuasm_description = "Check a Pep/9 microcode program for syntax errors.\
 Supports 1- and 2-byte data buses as well as the extended control section detailed in Pep9Micro.";
const std::string cpurun_description = "Run a Pep/9 microcode program with an optional list of preconditions.";

const std::string asm_input_file_text = "Input Pep/9 source program for assembler.";
const std::string asm_output_file_text = "Output object code generated from source.";
const std::string asm_run_log = "Override the default error log file, which contains assembly errors. \
If no errors are present, no data will be written to the file.";
const std::string obj_input_file_text = "Input Pep/9 object code program for simulator.";
const std::string charin_file_text = "File which will be buffered behind the charIn.";
const std::string charout_file_text = "File which charOut will be written to.";
const std::string charout_echo_text = "Echo data written to charOut to the terminal.";
const std::string isaMaxStepText = "The maximum number of assembly instructions executed before aborting. Defaults to %1";
const std::string microMaxStepText = "The maximum number of CPU cycles executed before aborting. Defaults to %1";
const std::string cpuasm_input_file_text = "Input Pep/9 microcode source program for microassembler.";
const std::string cpu_asm_log = "Override the default error log file, which contains assembly errors. \
If no errors are present, no data will be written to the file.";
const std::string cpu_1byte = "Assemble the microcode program with a 1-byte data bus (default is 1-byte).";
const std::string cpu_2byte = "Assemble the microcode program with a 2-byte data bus (default is 1-byte).";
const std::string cpu_full_control = "Assemble the microprogram with the full control section (default is partial control section).";

const std::string cpu_preconditions = "Pep/9 Microcode file containg pre- and post-conditions.\
 Using this flag overrides and skipps any pre- and post-conditions in the microcode source file provided with (--mc).";
const std::string cpu_run_log = "Override the default error log file, which contains assembly or postcondition errors. \
If no errors are present, no data will be written to the file.";

struct command_line_values {
    bool had_version{false}, had_about{false}, had_d2{false}, had_full_control{false}, had_echo_output{false};
    std::string e{}, s{}, o{}, i{}, mc{}, p{};
    uint64_t m{2500};
};

void handle_full_control(command_line_values&, bool use_full_control);
void handle_databus_size(command_line_values&, bool two_byte);
void handle_version(command_line_values&, int64_t);
void handle_about(command_line_values&, int64_t);
void handle_asm(command_line_values&, QRunnable**);
void handle_run(command_line_values&, QRunnable**);
void handle_cpuasm(command_line_values&, QRunnable**);
void handle_cpurun(command_line_values&, QRunnable**);

int main(int argc, char *argv[])
{
    command_line_values values;
    // Initialize global state maps.
    Pep::initEnumMnemonMaps();
    Pep::initMnemonicMaps(true);
    Pep::initAddrModesMap();
    Pep::initDecoderTables();
    Pep::initMicroDecoderTables();
    // Can't initialize Pep9CPU controls tables, since these depend
    // on the mode data bus size of the CPU.
    // Therefore, they must be initialized within the handlers for the CPU
    // related assembly instructions.

    // Construct an application so that we have a Qt main event loop.
    QCoreApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Pepperdine Computer Science Lab");
    QCoreApplication::setOrganizationDomain("cslab.pepperdine.edu");
    QCoreApplication::setApplicationName("Pep9Term");
    QCoreApplication::setApplicationVersion("9.3.0");

    // Runnable into which the executable program will be loaded by the below subcommands.
    QRunnable* run = nullptr;

    CLI::App parser{"Pep9Term is a  command line tool utility for interacting with the Pep9 virtual machine.\
 It assembles Pep/9 programs to object code and executes object code programs.\
 Additionally, it checks microcode programs for errors and executes microcode programs with optional preconditions."};

    // Top level option flags
    auto help = parser.set_help_flag("--help,-h", "Show this help information and exit.");
    auto help_all = parser.set_help_all_flag("--help-all", "Show help information for all subcommands and exit.");
    std::string version_string =  "Display program version number.";
    auto version_flag = parser.add_flag("-v,--version", [&](int64_t flag){handle_version(values,flag);}, version_string);

    std::string about_string =  "Display information about licensing, Qt, and developers.";
    auto about_flag = parser.add_flag("--about", [&](int64_t flag){handle_about(values,flag);}, about_string);

    // Subcommands for ASSEMBLE
    auto asm_subcommand = parser.add_subcommand("asm", asm_description);
    // File where errors will be written. By default, will be written to a file based on object code file name.
    asm_subcommand->add_option("-e", values.e, asm_run_log)->expected(1);
    // File from which Pep/9 assembly code will be loaded.
    asm_subcommand->add_option("-s", values.s, asm_input_file_text)->expected(1)->required(1);
    // File to which object code will be written.
    asm_subcommand->add_option("-o", values.o, asm_output_file_text)->expected(1)->required(1);
    // Create a runnable application from command line arguments
    asm_subcommand->callback(std::function<void()>([&](){handle_asm(values, &run);}));

    // Subcommands for RUN
    auto run_subcommand = parser.add_subcommand("run", run_description);
    // File from which object code will be loaded.
    run_subcommand->add_option("-s", values.s, obj_input_file_text)->expected(1)->required(true);
    // Batch input that will be loaded into charIn.
    run_subcommand->add_option("-i", values.i, charin_file_text)->expected(1);
    // File where values written to charOut will be stored.
    run_subcommand->add_option("-o", values.o, charout_file_text)->expected(1);
    run_subcommand->add_flag("--echo-output", values.had_echo_output, charout_echo_text);
    //run_subcommand->add_option("-e", obj_input_file_text);
    // Maximum number of instructions to be executed.
    std::string max_steps_text = QString::fromStdString(isaMaxStepText).arg(BoundExecIsaCpu::getDefaultMaxSteps()).toStdString();
    run_subcommand->add_option("-m", values.m, max_steps_text)->expected(1)->check(CLI::PositiveNumber)
            ->default_val(std::to_string(BoundExecIsaCpu::getDefaultMaxSteps()));
    // Create a runnable application from command line arguments
    run_subcommand->callback(std::function<void()>([&](){handle_run(values, &run);}));

    // Subcommands for CPUASM
    auto cpuasm_subcommand = parser.add_subcommand("cpuasm", cpuasm_description);
    // File where errors will be written. By default, will be written to a file based on the mc name.
    cpuasm_subcommand->add_option("-e", values.e, cpu_asm_log)->expected(1);
    // Microcode input file.
    cpuasm_subcommand->add_option("--mc", values.mc, cpuasm_input_file_text)->expected(1)->required(true);
    // Add flags to select 1-byte or 2-byte CPU data bus.
    auto cpuasm_d1_flag = cpuasm_subcommand->add_flag("--dbus-1-byte,--d1", [&](int64_t){handle_databus_size(values, false);}, cpu_1byte);
    auto cpuasm_d2_flag = cpuasm_subcommand->add_flag("--dbus-2-byte,--d2", [&](int64_t){handle_databus_size(values, true);}, cpu_2byte);
    // Only allow 1-byte or 2-byte to be selected, not both at once.
    cpuasm_d1_flag->excludes(cpuasm_d2_flag);
    cpuasm_d2_flag->excludes(cpuasm_d1_flag);
    // Allow full control section to be enabled iff 2-byte data bus is enabled.
    // auto cpuasm_full_ctrl_flag = cpuasm_subcommand->add_flag("--full-control", [&](int64_t){handle_full_control(values, true);}, cpu_full_control);
    // cpuasm_full_ctrl_flag->needs(cpuasm_d2_flag);
    // Create a runnable application from command line arguments
    cpuasm_subcommand->callback(std::function<void()>([&](){handle_cpuasm(values, &run);}));

    // Subcommands for CPURUN
    auto cpurun_subcommand = parser.add_subcommand("cpurun", cpurun_description);
    // File where errors will be written. By default, will be written to a file based on the mc name.
    cpurun_subcommand->add_option("-e", values.e, cpu_run_log)->expected(1);
    cpurun_subcommand->add_option("--mc", values.mc, cpuasm_input_file_text)->expected(1)->required(true);
    // Add flags to select 1-byte or 2-byte CPU data bus.
    auto cpurun_d1_flag = cpurun_subcommand->add_flag("--dbus-1-byte,--d1", [&](int64_t){handle_databus_size(values, false);}, cpu_1byte);
    auto cpurun_d2_flag = cpurun_subcommand->add_flag("--dbus-2-byte,--d2", [&](int64_t){handle_databus_size(values, true);}, cpu_2byte);
    cpurun_d1_flag->excludes(cpurun_d2_flag);
    cpurun_d2_flag->excludes(cpurun_d1_flag);
    // Allow full control section to be enabled iff 2-byte data bus is enabled.
    //auto cpurun_full_ctrl_flag = cpurun_subcommand->add_flag("--full-control",[&](int64_t){handle_full_control(values, true);}, cpu_full_control);
    //cpurun_full_ctrl_flag->needs(cpurun_d2_flag);
    // Maximum number of cycles to be executed.
    //std::string max_cycles_text = QString::fromStdString(microMaxStepText).arg(BoundExecMicroCpu::getDefaultMaxCycles()).toStdString();
    //cpurun_subcommand->add_option("-m", values.m, max_cycles_text)->expected(1)->needs(cpurun_full_ctrl_flag)->check(CLI::PositiveNumber)
            //->default_val(std::to_string(BoundExecMicroCpu::getDefaultMaxCycles()));
    cpurun_subcommand->add_option("-p", values.p, cpu_preconditions)->expected(1);
    // Create a runnable application from command line arguments
    cpurun_subcommand->callback(std::function<void()>([&](){handle_cpurun(values, &run);}));

    // Require that one of the modes be used.
    parser.require_subcommand();

    try {
        parser.parse(argc, argv);
    } catch(const CLI::ValidationError &e){
        std::cout <<e.what() << std::endl;
        return e.get_exit_code();
    } catch (const CLI::ParseError &e) {
        if(values.had_about || values.had_version) return 0;
        if(!help || !help_all) {
            std::cout << parser.help();
        }
        return parser.exit(e);
    }

    // Assemble the default operating system from this thread, so that
    // no worker threads have to check for the presence of an operating system.
    buildDefaultOperatingSystem(*AsmProgramManager::getInstance());

    /*
     * This asynchronous approach must be used, because if quit() is called
     * before a.exec() happens, then the application will not actually quit.
     * So, by causing the task to be scheduled via the event loop, we allow
     * the task to be scheduled via the main event loop.
     *
     */
    QThreadPool pool;
    // If the optional does not have a value, we must not run it.
    if(run == nullptr) {
        return -1;
    }
    else {
        pool.start(run);
    }

    return a.exec();
}

void handle_full_control(command_line_values &values, bool use_full_control)
{
    values.had_full_control = use_full_control;
}

void handle_databus_size(command_line_values &values, bool two_byte)
{
    values.had_d2 = two_byte;
}

void handle_version(command_line_values &values, int64_t)
{
    values.had_version = true;
    std::cout << QString("%1 %2").arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion()).toStdString() << std::endl;
}

void handle_about(command_line_values &values, int64_t)
{
    values.had_about = true;
    QFile aboutFile(":/help-term/about.txt");
    aboutFile.open(QIODevice::ReadOnly | QIODevice::Text);
    std::cout<< aboutFile.readAll().toStdString() << std::endl;
    aboutFile.close();
}

void handle_asm(command_line_values &values, QRunnable **runnable)
{
    // Needs a assembler source program to be well defined.
    if(values.s.empty()) {
        //qDebug() << "Must set assembler input (-s).";
        throw CLI::ValidationError("Must set assembler input (-s).", -1);
    }
    // Needs an object code output to be well defined.
    else if(values.o.empty()) {
        //qDebug() << "Must set object code output (-o).";
        throw CLI::ValidationError("Must set object code output (-o).", -1);
    }

    // File names associated with cli parameters.
    QString sourceFileString = QString::fromStdString(values.s);
    QString objectFileString = QString::fromStdString(values.o);

    QFile sourceFile(sourceFileString);
    QString sourceText;
    // Read to assembler source, or return error that it could not be opened.
    if(!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //qDebug().noquote() << errLogOpenErr.arg(sourceFile.fileName());
        throw CLI::ValidationError(errLogOpenErr.arg(sourceFile.fileName()).toStdString(), -1);
    }
    else {
        QTextStream sourceStream(&sourceFile);
        sourceText = sourceStream.readAll();
        sourceFile.close();

        ASMBuildHelper *helper = new ASMBuildHelper(sourceText, objectFileString,
                                                    *AsmProgramManager::getInstance());
        if(!values.e.empty()) {
            helper->set_error_file(QString::fromStdString(values.e));
        }

        QObject::connect(helper, &ASMBuildHelper::finished, QCoreApplication::instance(), &QCoreApplication::quit);

        (*runnable) = helper;
    }
}

void handle_run(command_line_values &values, QRunnable **runnable)
{
    // Needs a source object code program to be well defined.
    if(values.s.empty()) {
        //qDebug() << "Must set object code input input (-s).";
        throw CLI::ValidationError("Must set object code input input (-s).", -1);
    }
    // Needs an output log.
    else if(values.o.empty()) {
        //qDebug() << "Must set output log file (-o).";
        throw CLI::ValidationError("Must set output log file (-o).", -1);
    }

    // File names associated with cli parameters.
    QString objCodeFileName = QString::fromStdString(values.s);
    QString textInputFileName = QString::fromStdString(values.i);
    QString textOutputFileName = QString::fromStdString(values.o);
    // Attempt to parse stepMax string as an integer.
    quint64 stepMaxValue = values.m;

    // Load object code string from file if possible, else print error log.
    QFile objFile(objCodeFileName);
    if(!objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //qDebug().noquote() << errLogOpenErr.arg(objFile.fileName());
        throw CLI::ValidationError(errLogOpenErr.arg(objFile.fileName()).toStdString(), -1);
    }

    QTextStream objStream(&objFile);
    QString objText = objStream.readAll();
    objFile.close();

    ASMRunHelper *helper = new ASMRunHelper(objText, stepMaxValue, textOutputFileName,
                                      textInputFileName, *AsmProgramManager::getInstance());
    helper->set_echo_charout(values.had_echo_output);
    QObject::connect(helper, &ASMRunHelper::finished, QCoreApplication::instance(), &QCoreApplication::quit);

    (*runnable) = helper;
}

void handle_cpuasm(command_line_values &values, QRunnable **runnable)
{
    // Needs a microcode source program to be well defined.
    if(values.mc.empty()) {
        //qDebug() << "Must set microcode input (--mc).";
        throw CLI::ValidationError("Must set microcode input (--mc).", -1);
    }

    // Determine CPU type.
    Enu::CPUType type = Enu::CPUType::OneByteDataBus;
    if(values.had_d2) {
       type = Enu::CPUType::TwoByteDataBus;
    }

    // Enable or disable full control section features depending on passed flags.
    Pep::initMicroEnumMnemonMaps(type, values.had_full_control);

    // Setup parsing of microcode program.
    QString sourceFileString = QString::fromStdString(values.mc);
    QFile sourceFile(sourceFileString);
    QString sourceText;

    // Check that the file can be opened, otherwise log an error to the console.
    if(!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //qDebug().noquote() << errLogOpenErr.arg(sourceFile.fileName());
        throw CLI::ValidationError(errLogOpenErr.arg(sourceFile.fileName()).toStdString(), -1);
    }
    else {
        // Otherwise read the file.
        QTextStream sourceStream(&sourceFile);
        // Must remove line numbers, on microassembler will raise spurious errors.
        sourceText = Pep::removeCycleNumbers(sourceStream.readAll());
        sourceFile.close();

        auto helper = new CPUBuildHelper(type, values.had_full_control, sourceText, sourceFile);
        if(!values.e.empty()) {
            helper->set_error_file(QString::fromStdString(values.e));
        }
        QObject::connect(helper, &CPUBuildHelper::finished, QCoreApplication::instance(), &QCoreApplication::quit);
        (*runnable) = helper;
    }

}

void handle_cpurun(command_line_values &values, QRunnable **run)
{
    // Needs a microcode source program to be well defined.
    if(values.mc.empty()) {
        //qDebug() << "Must set microcode input (--mc).";
        throw CLI::ValidationError("Must set microcode input (--mc).", -1);
    }

    // Determine CPU type.
    Enu::CPUType type = Enu::CPUType::OneByteDataBus;
    if(values.had_d2) {
       type = Enu::CPUType::TwoByteDataBus;
    }

    // Enable or disable full control section features depending on passed flags.
    Pep::initMicroEnumMnemonMaps(type, values.had_full_control);

    // Microcode input file and unit test output file.
    QString microcodeFileName = QString::fromStdString(values.mc);

    // Load object code string from file if possible, else print error log.
    QFile microcodeFile(microcodeFileName);
    if(!microcodeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //qDebug().noquote() << errLogOpenErr.arg(microcodeFile.fileName());
        throw CLI::ValidationError(errLogOpenErr.arg(microcodeFile.fileName()).toStdString(), -1);
    }

    QTextStream microprogramStream(&microcodeFile);
    QString microprogramText = Pep::removeCycleNumbers(microprogramStream.readAll());
    microcodeFile.close();

    // Load overriding preconditions if present.
    QString preconditionText;
    if(!values.mc.empty()) {
        QString preconditionFileName = QString::fromStdString(values.mc);
        QFile preconditionFile(preconditionFileName);
        // If passed precondition file that can't be opened, raise an error.
        if(!preconditionFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            //qDebug().noquote() << errLogOpenErr.arg(preconditionFile.fileName());
            throw CLI::ValidationError(errLogOpenErr.arg(preconditionFile.fileName()).toStdString(), -1);
        }

        QTextStream preconditionStream(&preconditionFile);
        preconditionText = Pep::removeCycleNumbers(preconditionStream.readAll());
        preconditionFile.close();
    }

    if(values.had_full_control) {
        CPURunHelper *helper = new CPURunHelper(type, microprogramText,
                                               QFileInfo(microcodeFile), preconditionText,
                                               nullptr);
        if(!values.e.empty()) {
            helper->set_error_file(QString::fromStdString(values.e));
        }

        QObject::connect(helper, &CPURunHelper::finished, QCoreApplication::instance(), &QCoreApplication::quit);
        (*run) = helper;
    }
    else {
        // Attempt to parse stepMax string as an integer.
        quint64 stepMaxValue = values.m;

        MicroStepHelper *helper = new MicroStepHelper(stepMaxValue,
                                                      microprogramText,
                                                      QFileInfo(microcodeFile),
                                                      preconditionText,
                                                      nullptr);
        if(!values.e.empty()) {
            helper->set_error_file(QString::fromStdString(values.e));
        }

        QObject::connect(helper, &MicroStepHelper::finished, QCoreApplication::instance(), &QCoreApplication::quit);
        (*run) = helper;

    }
}
