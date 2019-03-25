# Pep9Suite
Pep9Suite is a suite of software for the Pep/9 virtual machine.
It consists of three applications:
* Pep9
* Pep9CPU
* Pep9Micro

# Pep9

The Pep/9 computer is a 16-bit complex instruction set computer (CISC). It is designed to teach computer architecture and assembly language programming principles. Its instructions are based on an expanding opcode and are either unary (one byte) or nonunary (three bytes). The eight addressing modes and eight dot commands are designed for straightforward translation between C/C++ and assembly language.

The assembler features an integrated text editor, error messages in red type that are inserted within the source code at the place where the error is detected, student-friendly machine language object code in hexadecimal format, the ability to code directly in machine language, bypassing the assembler, and the ability to redefine the mnemonics for the unimplemented opcodes that trigger synchronous traps.

The simulator features simulated ROM that is not altered by store instructions, a small operating system burned into simulated ROM that includes a loader and a trap handler system, an integrated debugger that allows for break points, single step execution, CPU tracing, and memory tracing, the option to trace an application, the loader, or the operating system, the ability to recover from endless loops, and the ability to modify the operating system by designing new trap handlers for the unimplemented opcodes.

# Pep9CPU

# Pep9Micro
Pep9Micro is a fully microcoded implementation of the Pep/9 virtual machine.
In addition to supporting all of assembly programming features of the Pep9 application and graphical CPU interaction of Pep9CPU, it also supports:

* The Micro IDE provides the assembler from Pep9 and the CPU simulator of Pep9CPU so that complete assembly language programs can be executed at the microcode level - spanning two levels of system abstraction.
* Run both memory aligned and nonaligned programs. Assembly language programs that do not use optimal .ALIGN directives still execute correctly but slower.
* Provides performance statistics in the form of statement execution counts at the microcode level and the ISA level. Students can measure the performance differences between aligned and nonaligned programs.
* Retains the unit tests of the original Pep/9 CPU IDE so that students can write microcode fragments with the extended microinstruction format.
* Supports new debugging features like step-into, step-out, and step-over so students can trace assembly programs more efficiently.

# Building from Sources
To sucessfully build from the sources, you must have the Qt libraries installed on their machine. In addition, the WebEngine components must be installed, and if one wishes to package the application with an installer, Qt Installer Framework (QtIFW) 3.0 or higher must be installed.

Qt Creator, while optional, will greatly assist in navigating and exploring the strucutre of the program.

If you do not have a copy of Qt installed, it can be downloaded from [the Qt website](https://www.qt.io/download).

After making a local clone of the repository, navigate to the pep9micro/pep9micro directory, and open the project in Qt Creator. All that's left to do is build and enjoy.

# Help Documentation
The program comes packaged with help documentation to describe the nature and function of the Pep/9 virtual machine including walkthroughs on Pep/9 assembly language programming and debugging tools/tips. It also has a collection of sample assembly programs from the text _Computer Systems_, J. Stanley Warford, 5th edition, on which Pep/9 is based.
