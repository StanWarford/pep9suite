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
#ifndef MICROMAINWINDOW_H
#define MICROMAINWINDOW_H

#include <QtWidgets/QMainWindow>

#include "pep.h"
#include <QDir>
#include "isaasm.h"
namespace Ui {
    class MicroMainWindow;
}
class AboutPep;
class AsmProgramManager;
class ByteConverterBin;
class ByteConverterChar;
class ByteConverterDec;
class ByteConverterHex;
class ByteConverterInstr;
class CpuPane;
class DecoderTableDialog;
class FullMicrocodedCPU;
class MicroHelpDialog;
class MainMemory;
class MicrocodePane;
class MicroObjectCodePane;
class CPUDataSection;
class UpdateChecker;
class RedefineMnemonicsDialog;

/*
 * The set of possible states for the debugger.
 * For the transitions between debug states, see docs/debugger-fsm-micro
 */
enum class DebugState
{
    DISABLED, // The simulation is neither being debugged or run.
    DEBUG_ISA,  // The simulation is being stepped through at the assembly level.
    DEBUG_MICRO,  // The simulation is being stepped through at the microcode level.
    DEBUG_RESUMED,  // The simulation is being debugged, and resume has been called.
    RUN, // The simulation is being run, not debugged.
};

class MicroMainWindow : public QMainWindow {
    Q_OBJECT
public:
    MicroMainWindow(QWidget *parent = nullptr);
    ~MicroMainWindow();
protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);
    bool eventFilter(QObject *, QEvent *);

private:
    Ui::MicroMainWindow *ui;
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
    ByteConverterInstr *byteConverterInstr;
    // Main Memory
    QSharedPointer<MainMemory> memDevice;
    QSharedPointer<FullMicrocodedCPU> controlSection;
    QSharedPointer<CPUDataSection> dataSection;

    // Dialogues
    MicroHelpDialog *helpDialog;
    AboutPep *aboutPepDialog;
    RedefineMnemonicsDialog *redefineMnemonicsDialog;
    DecoderTableDialog *decoderTableDialog;

    AsmProgramManager* programManager;

    // Disconnect or reconnect events that notify views of changes in model,
    // Disconnecting these events allow for faster execution when running or continuing.
    void connectViewUpdate();
    void disconnectViewUpdate();

    // Methods to persist & restore class to file.
    void readSettings();
    void writeSettings();

    // Save methods
    // Save a pane if it is associated with a file. If not, it switches to saveAsFile(which)
    bool save(Enu::EPane which);
    // Try to save all modified panes.
    bool maybeSave();
    bool maybeSave(Enu::EPane which);
    void loadFile(const QString &fileName, Enu::EPane which);
    bool saveFile(Enu::EPane which);
    bool saveFile(const QString& fileName, Enu::EPane which);
    bool saveAsFile(Enu::EPane which);
    QString strippedName(const QString &fullFileName);

    //Print a pane to file using a print dialog
    void print(Enu::EPane which);

    //Methods to load user compiled code
    void assembleDefaultOperatingSystem();
    void loadOperatingSystem();
    bool loadObjectCodeProgram();

    //
    void set_Obj_Listing_filenames_from_Source();
    void doubleClickedCodeLabel(Enu::EPane which);

    // Debug helper functions

    // This struct contains bit masks for each button that needs to be enabled or disabled deppending on the current state of the debugging FSM
    // See buttonEnableHelper(int)
    struct DebugButtons
    {
        static const int RUN = 1<<0, RUN_OBJECT = 1<<1, DEBUG = 1<<2, DEBUG_OBJECT = 1<<3, DEBUG_LOADER = 1<<4,
        INTERRUPT = 1<<5, CONTINUE = 1<<6, RESTART = 1<<7, STOP = 1<<8, STEP_OVER_ASM = 1<<9, STEP_INTO_ASM = 1<<10,
        STEP_OUT_ASM = 1<<11, SINGLE_STEP_MICRO = 1<<12/*, SINGLE_STEP_ASM = 1<<13*/, BUILD_ASM = 1<<14, BUILD_MICRO = 1<<15,
        OPEN_NEW = 1<<17, INSTALL_OS = 1<<18;
    };

    // Which debug buttons to enable, based on integer cracking of the above struct. It is not strongly typed with an enum, because the casting
    // would add signifcant code volume, and would not increase code clarity.
    // To enable only the run and stop buttons one would call "buttonEnableHelper(DebugButtons::RUN | DebugButtons::STOP)".
    void debugButtonEnableHelper(const int which);

    // Coordinates higlighting of memory, microcode pane, micro object code pane, and assembler listings.
    void highlightActiveLines();

    // Update the views and initialize the models in a way that can be used for debugging or running.
    bool initializeSimulation();

private slots:
    // Update Check
    void onUpdateCheck(int val);
    // File
    void on_actionFile_New_Asm_triggered();
    void on_actionFile_New_Microcode_triggered();
    void on_actionFile_Open_triggered();

    bool on_actionFile_Save_Microcode_triggered();
    bool on_actionFile_Save_Asm_triggered();

    bool on_actionFile_Save_Asm_Source_As_triggered();
    bool on_actionFile_Save_Object_Code_As_triggered();
    bool on_actionFile_Save_Assembler_Listing_As_triggered();
    bool on_actionFile_Save_Microcode_As_triggered();

    void on_actionFile_Print_Assembler_Source_triggered();
    void on_actionFile_Print_Object_Code_triggered();
    void on_actionFile_Print_Assembler_Listing_triggered();
    void on_actionFile_Print_Microcode_triggered();

    // Edit
    void on_actionEdit_Undo_triggered();
    void on_actionEdit_Redo_triggered();
    void on_actionEdit_Cut_triggered();
    void on_actionEdit_Copy_triggered();
    void on_actionEdit_Paste_triggered();
    void on_actionEdit_UnComment_Line_triggered();
    void on_actionEdit_Format_Assembler_triggered();
    void on_actionEdit_Format_Microcode_triggered();
    void on_actionEdit_Remove_Error_Assembler_triggered();
    void on_actionEdit_Remove_Error_Microcode_triggered();
    void on_actionEdit_Font_triggered();
    void on_actionEdit_Reset_font_to_Default_triggered();

    // Build
    void on_actionBuild_Microcode_triggered();
    bool on_actionBuild_Assemble_triggered(); //Returns true if assembly succeded.
    void on_actionBuild_Load_Object_triggered();
    void on_actionBuild_Execute_triggered();
    void on_actionBuild_Run_triggered();
    void on_actionBuild_Run_Object_triggered();


    //Debug Events
    void handleDebugButtons();
    bool on_actionDebug_Start_Debugging_triggered();
    bool on_actionDebug_Start_Debugging_Object_triggered();
    bool on_actionDebug_Start_Debugging_Loader_triggered();

    void on_actionDebug_Interupt_Execution_triggered();
    void on_actionDebug_Continue_triggered();
    void on_actionDebug_Restart_Debugging_triggered();
    void on_actionDebug_Stop_Debugging_triggered();

    void on_actionDebug_Single_Step_Assembler_triggered();
    // Stores the call depth, and continues to execute ISA instructions until the new call depth equals the old call depth.
    void on_actionDebug_Step_Over_Assembler_triggered();
    // Uncoditionally executes the next ISA instruction, including going into function calls and traps.
    void on_actionDebug_Step_Into_Assembler_triggered();
    // Executes the next ISA instructions until the call depth is decreased by 1.
    void on_actionDebug_Step_Out_Assembler_triggered();
    // Executes a single line of microcode, which is the behavior of Pep/9CPU
    void on_actionDebug_Single_Step_Microcode_triggered();

    // System
    void on_actionSystem_Clear_CPU_triggered();
    void on_actionSystem_Clear_Memory_triggered();
    void on_actionSystem_Assemble_Install_New_OS_triggered();
    void on_actionSystem_Reinstall_Default_OS_triggered();
    void on_actionSystem_Redefine_Mnemonics_triggered();
    void on_actionSystem_Redefine_Decoder_Tables_triggered();
    // Allow main window to update highlighting rules after
    // changes to the mnemonics have been finished.
    void redefine_Mnemonics_closed();

    // View
    void onDarkModeChanged();

    // Help
    void on_actionHelp_triggered();
    void on_actionHelp_Writing_Assembly_Programs_triggered();
    void on_actionHelp_Machine_Language_triggered();
    void on_actionHelp_Assembly_Language_triggered();
    void on_actionHelp_Debugging_Assembly_triggered();
    void on_actionHelp_Writing_Trap_Handlers_triggered();
    void on_actionHelp_Using_Pep_9_CPU_triggered();
    void on_actionHelp_Interactive_Use_triggered();
    void on_actionHelp_Microcode_Use_triggered();
    void on_actionHelp_Debugging_Use_triggered();
    void on_actionHelp_Pep9Reference_triggered();
    void on_actionHelp_Examples_triggered();
    void on_actionHelp_Pep9_Operating_System_triggered();
    void on_actionHelp_Pep9_Microcode_Implementation_triggered();

    void on_actionHelp_About_Pep9Micro_triggered();
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

    //Disable UI elements for IO
    void onInputRequested(quint16 address);
    void onOutputReceived(quint16 address, quint8 value);

    // Handle a breakpoint in the model, and determine the correct handler based
    // on breakpoint type.
    void onBreakpointHit(Enu::BreakpointTypes type);

private:
    // Helper function for onInputReceived(...) that
    // reenables any disabled window components after IO completion,
    // with no other side effects.
    void reenableUIAfterInput();
    // Helpers to seperate breakpoint logic
    void onMicroBreakpointHit();
    void onASMBreakpointHit();
    void onPaletteChanged(const QPalette &palette);

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
    void darkModeChanged(bool, QString styleSgeet);
};

#endif // MAINWINDOW_H
