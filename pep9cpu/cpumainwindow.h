// File: cpumainwindow.h
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
#ifndef CPUMAINWINDOW_H
#define CPUMAINWINDOW_H

#include <QtWidgets/QMainWindow>

#include "enu.h"
#include "pep.h"
#include <QDir>
namespace Ui {
    class CPUMainWindow;
}
class AboutPep;
class ByteConverterBin;
class ByteConverterChar;
class ByteConverterDec;
class ByteConverterHex;
class CpuPane;
class CPUHelpDialog;
class MicrocodePane;
class MicroObjectCodePane;
class UpdateChecker;
class QActionGroup;

//WIP classes
class PartialMicrocodedCPU;
class CPUDataSection;
class MainMemory;

/*
 * The set of possible states for the debugger.
 * For the transitions between debug states, see docs/debugger-fsm-cpu
 */
enum class DebugState
{
    DISABLED, // The simulation is neither being debugged or run.
    DEBUG_MICRO,  // The simulation is being stepped through at the microcode level.
    DEBUG_RESUMED,  // The simulation is being debugged, and resume has been called.
    RUN, // The simulation is being run, not debugged.
};

class CPUMainWindow : public QMainWindow {
    Q_OBJECT
public:
    CPUMainWindow(QWidget *parent = nullptr);
    ~CPUMainWindow();
protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);
    bool eventFilter(QObject *, QEvent *);

private:
    Ui::CPUMainWindow *ui;
    DebugState debugState;
    QString curPath;
    QFont codeFont;
    UpdateChecker *updateChecker;
    bool isInDarkMode;

    // Byte converter
    ByteConverterBin *byteConverterBin;
    ByteConverterChar *byteConverterChar;
    ByteConverterDec *byteConverterDec;
    ByteConverterHex *byteConverterHex;

    // Main Memory
    QSharedPointer<MainMemory> memDevice;
    QSharedPointer<PartialMicrocodedCPU> controlSection;
    QSharedPointer<CPUDataSection> dataSection;

    CPUHelpDialog *helpDialog;
    AboutPep *aboutPepDialog;

    QActionGroup *cpuModesGroup;

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
        static const int RUN = 1<<0, DEBUG = 1<<1,
        INTERRUPT = 1<<2, CONTINUE = 1<<3, STOP = 1<<5,
        SINGLE_STEP_MICRO = 1<<6, BUILD_MICRO = 1<<7,
        OPEN_NEW = 1<<9, SWITCH_BUSES = 1<<10, CLEAR = 1<<20;
    };

    // Which debug buttons to enable, based on integer cracking of the above struct. It is not strongly typed with an enum, because all of the casting
    // would add signifcant code volume, and it would not increase code clarity.
    // To enable only the run and stop buttons one would call "buttonEnableHelper(DebugButtons::RUN | DebugButtons::STOP)".
    void debugButtonEnableHelper(const int which);

    // Coordinates higlighting of memory, microcode pane, micro object code pane.
    void highlightActiveLines();

    // Update the views and initialize the models in a way that can be used for debugging or running.
    bool initializeSimulation();

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

    void on_actionDebug_Continue_triggered();
    void on_actionDebug_Stop_Debugging_triggered();

    // Executes a single line of microcode, which is the behavior of Pep/9CPU
    void on_actionDebug_Single_Step_Microcode_triggered();

    // System
    void on_actionSystem_Clear_CPU_triggered();
    void on_actionSystem_Clear_Memory_triggered();
    void on_actionSystem_One_Byte_triggered();
    void on_actionSystem_Two_Byte_triggered();

    // View
    void onDarkModeChanged();

    // Help
    void on_actionHelp_UsingPep9CPU_triggered();
    void on_actionHelp_InteractiveUse_triggered();
    void on_actionHelp_MicrocodeUse_triggered();
    void on_actionHelp_DebuggingUse_triggered();
    void on_actionHelp_Pep9Reference_triggered();
    void on_actionHelp_One_Byte_Examples_triggered();
    void on_actionHelp_Two_Byte_Examples_triggered();
    void on_actionHelp_triggered();
    void on_actionHelp_About_Pep9CPU_triggered();
    void on_actionHelp_About_Qt_triggered();

    //Pane hiding events
    void on_actionView_CPU_Code_triggered();
    void on_actionView_CPU_Code_Memory_triggered();

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

    void onCopyToMicrocodeClicked();

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
    void darkModeChanged(bool darkMode, QString styleSheet);
};

#endif // MAINWINDOW_H
