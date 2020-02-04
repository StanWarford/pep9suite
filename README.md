# What is Pep/9?

The Pep/9 computer is a 16-bit complex instruction set computer (CISC). It is designed to teach computer architecture, assembly language programming, and computer organization principles as described in the text [_Computer Systems_, J. Stanley Warford, 5th edition](http://computersystemsbook.com/5th-edition/). Pep/9 instructions are based on an expanding opcode and are either unary (one byte) or nonunary (three bytes). The eight addressing modes and eight dot commands are designed for straightforward translation from C to assembly language.

# Pep9Suite
Pep9Suite is a suite of software for the Pep/9 virtual machine.
It consists of four applications:
* [Pep9](#pep9)
* [Pep9CPU](#pep9cpu)
* [Pep9Micro](#pep9micro)
* [Pep9Term](#pep9term)

## Pep9
Pep9 is a simulator allowing users to interact with the Pep/9 virtual machine at the assembly, operating system, and ISA levels.

The Pep9 assembler features an integrated text editor, error messages in red type that are inserted within the source code at the place where the error is detected, student-friendly machine language object code in hexadecimal format, the ability to code directly in machine language, bypassing the assembler, and the ability to redefine the mnemonics for the unimplemented opcodes that trigger synchronous traps.

The simulator features simulated ROM that is not altered by store instructions, a small operating system burned into simulated ROM that includes a loader and a trap handler system, an integrated debugger that allows for break points, single and multi step execution, CPU tracing, and memory tracing, the option to trace an application, the loader, or the operating system, the ability to recover from endless loops, and the ability to modify the operating system by designing new trap handlers for the unimplemented opcodes.

## Pep9CPU
Pep9CPU is a simulator allowing users to interact with the data section of the Pep/9 CPU.

It contains two versions of the Pep/9 CPU data section &ndash; one with a one-byte wide data bus and another with a two-byte wide data bus. Using a GUI, students are able to set the control signals to direct the flow of data and change the state of the CPU. Alternatively, the Microcode IDE allows students to write microprogram code fragments to perform useful computations. An integrated unit test facility allows users to write pre- and post-conditions to verify correct behavior of arbitrary microprograms.

While debugging a microprogram fragment, the CPU simulator performs graphical tracing of data paths through the CPU. Using breakpoints, students may skip over previously debugged microstatments and resume debugging at a later point in the program.

## Pep9Micro
Pep9Micro is a fully microcoded implementation of the Pep/9 virtual machine.
It adds a control section, missing in Pep9CPU, and extends the microcode language to allow conditional microcode branches.
It integrates all the programming features of Pep9 and the graphical CPU interaction of Pep9CPU to simulate the complete execution of assembly language programs.

The Pep9Micro IDE:

* Provides the assembler from Pep9 and the CPU simulator from Pep9CPU so that complete assembly language programs can be executed at the microcode level spanning four levels of system abstraction &ndash; the assembly level, the operating system level, the ISA level, and the microcode level.
* Runs both memory aligned and nonaligned programs. Assembly language programs that do not use optimal .ALIGN directives still execute correctly but slower.
* Provides performance statistics in the form of statement execution counts at the microcode level and the ISA level. Students can measure the performance differences between aligned and nonaligned programs.
* Retains the unit tests of the original Pep/9 CPU IDE so that students can write microcode fragments with the extended microinstruction format.
* Supports new debugging features like step-into, step-out, and step-over so students can trace assembly programs more efficiently.

## Pep9Term
Pep9Term is a command-line version of the Pep/9 virtual machine.
It uses the assembler from the Pep9 application to create a .pepo file, and the simulator to execute the .pepo file.
Teachers can script Pep9Term to batch test assembly language homework submissions.

# Building from Sources
To sucessfully build from the sources, you must have Qt Creator and the Qt libraries installed on your machine, including the WebEngine components for the integrated Help systems. Qt can be downloaded from [the Qt website](https://www.qt.io/download).

If you want to package the application with an installer, you must also install the Qt Installer Framework (QtIFW) 3.0 or higher.

# Help Documentation
The programs come packaged with help documentation to describe the nature and function of the Pep/9 virtual machine including walkthroughs on Pep/9 assembly language programming and debugging tools/tips. They also have collections of sample assembly programs from the text [_Computer Systems_, J. Stanley Warford, 5th edition](http://computersystemsbook.com/5th-edition/), on which Pep/9 is based.

# Executable Downloads
For analytic reasons, we prefer that you download executables from the above book web site instead of GitHub.

# Developers

Contribute to Pep/9 development. Join the Discord chat at [https://discord.gg/Qza7QH8](https://discord.gg/Qza7QH8).
