#Pep9Micro
Pep9Micro is a fully microcoded implementation of the Pep/9 virtual machine.
In addition to supporting all of assembly programming features of the Pep9 application and graphical CPU interaction of Pep9CPU, it also supports:

* The Micro IDE provides the assembler from Pep9 and the CPU simulator of Pep9CPU so that complete assembly language programs can be executed at the microcode level - spanning two levels of system abstraction.
* Run both memory aligned and nonaligned programs. Assembly language programs that do not use optimal .ALIGN directives still execute correctly but slower.
* Provides performance statistics in the form of statement execution counts at the microcode level and the ISA level. Students can measure the performance differ- ences between aligned and nonaligned programs.
* Retains the unit tests of the original Pep/9 CPU IDE so that students can write mi- crocode fragments with the extended microinstruction format.
* Supports new debugging features like step-into, step-out, and step-over so students can trace assembly programs more efficiently.

#Building from Sources
To sucessfully build from the sources, you must have the Qt libraries installed on their machine. In addition, the WebEngine components must be installed, and if one wishes to package the application with an installer, Qt Installer Framework (QtIFW) 3.0 or higher must be installed.

Qt Creator, while optional, will greatly assist in navigating and exploring the strucutre of the program.

If you do not have a copy of Qt installed, it can be downloaded from [the Qt website](https://www.qt.io/download).

After making a local clone of the repository, navigate to the pep9micro/pep9micro directory, and open the project in Qt Creator. All that's left to do is build and enjoy.

#Help Documentation
The program comes packaged with help documentation to describe the nature & function of the Pep/9 virtual machine. This includes walkthroughs on Pep/9 assembly language programming and debugging tools/tips. It also includes a collection of sample assembly programs.
