// File: mainwindow.h
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2010  J. Stanley Warford, Pepperdine University

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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>

#include "enu.h"
#include "pep.h"
#include <QDir>
namespace Ui {
    class MainWindow;
}
class AboutPep;
class ByteConverterBin;
class ByteConverterChar;
class ByteConverterDec;
class ByteConverterHex;
class CpuPane;
//class HelpDialog;
class MicrocodePane;
class MicroObjectCodePane;
class UpdateChecker;
class QActionGroup;

//WIP classes
class PartialMicrocodedCPU;
class NewCPUDataSection;
class MainMemory;
#pragma message("TODO: Type up debugger FSM")
/*
 * The set of possible states for the debugger.
 * For the transitions between debug states, see docs/debuggerFSM
 */
enum class DebugState
{
    DISABLED, // The simulation is neither being debugged or run.
    DEBUG_MICRO,  // The simulation is being stepped through at the microcode level.
    DEBUG_RESUMED,  // The simulation is being debugged, and resume has been called.
    RUN, // The simulation is being run, not debugged.
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);
    bool eventFilter(QObject *, QEvent *);

private:
    Ui::MainWindow *ui;
    DebugState debugState;
    QString curPath;
    QString lightStyle, darkStyle;
    QFont codeFont;
    UpdateChecker *updateChecker;
    bool inDarkMode;

    // Byte converter
    ByteConverterBin *byteConverterBin;
    ByteConverterChar *byteConverterChar;
    ByteConverterDec *byteConverterDec;
    ByteConverterHex *byteConverterHex;
#pragma message ("TODO: Remove instr converter")

    // Main Memory
    QSharedPointer<MainMemory> memDevice;
    QSharedPointer<PartialMicrocodedCPU> controlSection;
    QSharedPointer<NewCPUDataSection> dataSection;

    //HelpDialog *helpDialog;
    AboutPep *aboutPepDialog;

    QActionGroup* statisticsLevelsGroup;

    // Disconnect or reconnect events that notify views of changes in model,
    // Disconnecting these events allow for faster execution when running or continuing.
    void connectViewUpdate();
    void disconnectViewUpdate();

    // Methods to persist & restore class to file.
    void readSettings();
    void writeSettings();

    // Save methods
    // Save a pane if it is associated with a file. If not, it switches to saveAsFile(which)
    bool save();
    // Try to save all modified panes.
    bool maybeSave();
    void loadFile(const QString &fileName);
    bool saveFile();
    bool saveFile(const QString& fileName);
    bool saveAsFile();
    QString strippedName(const QString &fullFileName);

    //Print a pane to file using a print dialog
    void print();

    // Debug helper functions

    // This struct contains bit masks for each button that needs to be enabled or disabled deppending on the current state of the debugging FSM
    // See buttonEnableHelper(int)
    struct DebugButtons
    {
        static const int RUN = 1<<0, RUN_OBJECT = 1<<1, DEBUG = 1<<2, DEBUG_OBJECT = 1<<3, DEBUG_LOADER = 1<<4,
        INTERRUPT = 1<<5, CONTINUE = 1<<6, RESTART = 1<<7, STOP = 1<<8, STEP_OVER_ASM = 1<<9, STEP_INTO_ASM = 1<<10,
        STEP_OUT_ASM = 1<<11, SINGLE_STEP_MICRO = 1<<12, SINGLE_STEP_ASM = 1<<13, BUILD_ASM = 1<<14, BUILD_MICRO = 1<<15,
        STATS_LEVELS = 1<<16, OPEN_NEW = 1<<17;
    };

    // Which debug buttons to enable, based on integer cracking of the above struct. It is not strongly typed with an enum, because all of the casting
    // would add signifcant code volume, and it would not increase code clarity.
    // To enable only the run and stop buttons one would call "buttonEnableHelper(DebugButtons::RUN | DebugButtons::STOP)".
    void debugButtonEnableHelper(const int which);

    // Coordinates higlighting of memory, microcode pane, micro object code pane, and assembler listings.
    // Parameter forces re-highlight at the ISA level
    void highlightActiveLines(bool forceISA = false);
    // Overload for signal-slot connections. Calls highlightActiveLines(false).
    void highlightActiveLines();

    // Update the views and initialize the models in a way that can be used for debugging or running.
    bool initializeSimulation();

    // Set the appropriate checkboxes in teh statistics tab based on a debug level
    void setCheckedFromDebugLevel(Enu::DebugLevels level);
private slots:
    // Update Check
    void onUpdateCheck(int val);

    // File
    void on_actionFile_New_Microcode_triggered();
    void on_actionFile_Open_triggered();

    bool on_actionFile_Save_Microcode_triggered();

    bool on_actionFile_Save_Microcode_As_triggered();

    void on_actionFile_Print_Microcode_triggered();

    // Edit
    void on_actionEdit_Undo_triggered();
    void on_actionEdit_Redo_triggered();
    void on_actionEdit_Cut_triggered();
    void on_actionEdit_Copy_triggered();
    void on_actionEdit_Paste_triggered();
    void on_actionEdit_UnComment_Line_triggered();
    void on_actionEdit_Format_Microcode_triggered();
    void on_actionEdit_Remove_Error_Microcode_triggered();
    void on_actionEdit_Font_triggered();
    void on_actionEdit_Reset_font_to_Default_triggered();

    // Build
    bool on_actionBuild_Microcode_triggered();
    void on_actionBuild_Run_triggered();


    //Debug Events
    void handleDebugButtons();
    bool on_actionDebug_Start_Debugging_triggered();

    void on_actionDebug_Interupt_Execution_triggered();
    void on_actionDebug_Continue_triggered();
    void on_actionDebug_Restart_Debugging_triggered();
    void on_actionDebug_Stop_Debugging_triggered();

    // Executes a single line of microcode, which is the behavior of Pep/9CPU
    void on_actionDebug_Single_Step_Microcode_triggered();

    // System
    void on_actionSystem_Clear_CPU_triggered();
    void on_actionSystem_Clear_Memory_triggered();

    // Statistics Events
    void on_actionStatistics_Level_All_triggered();
    void on_actionStatistics_Level_Minimal_triggered();
    void on_actionStatistics_Level_None_triggered();

    // View
    void on_actionDark_Mode_triggered();

    // Help
    void on_actionHelp_UsingPep9CPU_triggered();
    void on_actionHelp_InteractiveUse_triggered();
    void on_actionHelp_MicrocodeUse_triggered();
    void on_actionHelp_DebuggingUse_triggered();
    void on_actionHelp_Pep9Reference_triggered();
    void on_actionHelp_Examples_triggered();
    void on_actionHelp_triggered();
    void on_actionHelp_About_Pep9CPU_triggered();
    void on_actionHelp_About_Qt_triggered();

    //Run events
    void onSimulationFinished();

    // Byte converter
    void slotByteConverterDecEdited(const QString &);
    void slotByteConverterHexEdited(const QString &);
    void slotByteConverterBinEdited(const QString &);
    void slotByteConverterCharEdited(const QString &);

    // Focus coloring Undo/Redo/Cut/Copy/Paste activate/deactivate
    void focusChanged(QWidget *, QWidget *);
    void setUndoability(bool b);
    void setRedoability(bool b);

    void appendMicrocodeLine(QString string);

    void helpCopyToSourceClicked();

    // Handle a breakpoint in the model, and determine the correct handler based
    // on breakpoint type.
    void onBreakpointHit(Enu::BreakpointTypes type);

private:
    // Helpers to seperate breakpoint logic
    void onMicroBreakpointHit();
signals:
    void beginUpdateCheck();
    // Emitted once when a simulation is begun
    void simulationStarted();
    // Emitted whenever a step has occured in the simulation.
    void simulationUpdate();
    // Emitted once when a simulation is finished(). Rebroadcasted from CPUControlSection::simulationFinished
    void simulationFinished();
    //If a sub-compnent wants to be notified that fonts should be restored to their default values, connect to this signal.
    void fontChanged(QFont font);
    void darkModeChanged(bool);
};

#endif // MAINWINDOW_H
