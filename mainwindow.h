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

#include "pep.h"
#include <QDir>
#include "isaasm.h"
namespace Ui {
    class MainWindow;
}
class AboutPep;
class ByteConverterBin;
class ByteConverterChar;
class ByteConverterDec;
class ByteConverterHex;
class ByteConverterInstr;
class CPUControlSection;
class CPUDataSection;
class CpuPane;
class HelpDialog;
class MemorySection;
class MicrocodePane;
class MicroObjectCodePane;
class UpdateChecker;
enum class DebugState
{
    DISABLED, DEBUG_ISA, DEBUG_MICRO, DEBUG_RESUMED, RUN
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
    QString darkStyle;
    QString lightStyle;
    QFont codeFont;
    UpdateChecker *updateChecker;

    // Byte converter
    ByteConverterBin *byteConverterBin;
    ByteConverterChar *byteConverterChar;
    ByteConverterDec *byteConverterDec;
    ByteConverterHex *byteConverterHex;
    ByteConverterInstr *byteConverterInstr;
    // Main Memory

    HelpDialog *helpDialog;
    AboutPep *aboutPepDialog;

    MemorySection* memorySection;
    CPUDataSection* dataSection;
    CPUControlSection* controlSection;

    //Disconnect or reconnect draw events from the model.
    //These allow the screen to not be updated when the "run" option is picked
    void connectMicroDraw();
    void disconnectMicroDraw();

    void readSettings();
    void writeSettings();

    // Save methods
    bool save(Enu::EPane which);
    bool maybeSave();
    bool maybeSave(Enu::EPane which);
    void loadFile(const QString &fileName, Enu::EPane which);
    bool saveFile(Enu::EPane which);
    bool saveFile(const QString& fileName, Enu::EPane which);
    bool saveAsFile(Enu::EPane which);
    QString strippedName(const QString &fullFileName);

    void print(Enu::EPane which);

    void loadOperatingSystem();
    void loadObjectCodeProgram();

    void set_Obj_Listing_filenames_from_Source();
    void doubleClickedCodeLabel(Enu::EPane which);

    // Debug helper functions

    // This struct contains bit masks for each button that needs to be enabled or disabled deppending on the current state of the debugging FSM
    // See buttonEnableHelper(int)
    struct DebugButtons
    {
        static const int RUN = 1<<0, RUN_OBJECT = 1<<1, DEBUG = 1<<2, DEBUG_OBJECT = 1<<3, DEBUG_LOADER = 1<<4,
        INTERRUPT = 1<<5, CONTINUE = 1<<6, RESTART = 1<<7, STOP = 1<<8, STEP_OVER_ASM = 1<<9, STEP_INTO_ASM = 1<<10,
        STEP_OUT_ASM = 1<<11, SINGLE_STEP_MICRO = 1<<12, SINGLE_STEP_ASM = 1<<13, BUILD_ASM = 1<<14, BUILD_MICRO = 1<<15;
    };
    // Which debug buttons to enable, based on integer cracking of the above struct. It is not strongly typed with an enum, because all of the casting
    // would add signifcant code volume, and it would not increase code clarity.
    // To enable only the run and stop buttons one would call "buttonEnableHelper(DebugButtons::RUN | DebugButtons::STOP)".
    void buttonEnableHelper(const int which);

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
    void on_actionFile_Open_triggered(); //Todo

    bool on_actionFile_Save_Microcode_triggered();
    bool on_actionFile_Save_Asm_triggered();

    bool on_actionFile_Save_Asm_Source_As_triggered(); //Todo
    bool on_actionFile_Save_Object_Code_As_triggered(); //Todo
    bool on_actionFile_Save_Assembler_Listing_As_triggered(); //Todo
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
    void on_ActionBuild_Assemble_triggered();
    void on_actionBuild_Load_Object_triggered();
    void on_actionBuild_Run_Object_triggered();
    void on_actionBuild_Run_triggered();


    //Debug Events
    void handleDebugButtons();
    bool on_actionDebug_Start_Debugging_triggered();
    void on_actionDebug_Stop_Debugging_triggered();

    void on_actionDebug_Interupt_Execution_triggered();
    void on_actionDebug_Continue_triggered();
    void on_actionDebug_Restart_Debugging_triggered();
    //Merges the behavior of Step Into and Step Over.
    //If it it a trap & we are tracing traps, then step into the trap.
    //If it is a trap & we are not tracing trpas, the step over the trap.
    //If it is a call, step into the call.
    //Else, execute a single ISA instruction
    void on_actionDebug_Single_Step_Assembler_triggered();
    //Stores the call depth, and continues to execute ISA instructions until the new call depth equals the old call depth.
    void on_actionDebug_Step_Over_Assembler_triggered();
    //Uncoditionally executes the next ISA instruction, including going into function calls and traps.
    void on_actionDebug_Step_Into_Assembler_triggered();
    //Executes the next ISA instructions until the call depth is decreased by 1.
    void on_actionDebug_Step_Out_Assembler_triggered();
    void on_actionDebug_Single_Step_Microcode_triggered();

    // System

    void on_actionSystem_Clear_CPU_triggered();
    void on_actionSystem_Clear_Memory_triggered();

    //Run events
    void onSimulationFinished();

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

    void helpCopyToMicrocodeButtonClicked();

    //Disable UI elements for IO
    void onInputRequested();
    void onInputReceived();
signals:
    void beginUpdateCheck();
    void simulationStarted();
    void simulationUpdate();
    void simulationFinished();
    //If a sub-compnent wants to be notified that fonts should be restored to their default values, connect to this signal.
    void fontChanged(QFont font);
    void darkModeChanged(bool);
};

#endif // MAINWINDOW_H
