// File: micromain.cpp
/*
    Pep9Micro is a complete CPU simulator for the Pep/9 instruction set,
    and is capable of assembling programs to object code, executing
    object code programs, and executing microcode fragments.

    Copyright (C) 2018  J. Stanley Warford & Matthew McRaven, Pepperdine University

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

#include <QtWidgets/QApplication>
#include "micromainwindow.h"
#ifdef WIN32
#include <string.h>
#include <qvector.h>
#endif
#include <iostream>
int main(int argc, char *argv[])
{
#ifdef WIN32 //Always inject -platform windows:dpiawareness=0 flag to disable hi-dpi support.
    //Hi-dpi support makes all of the pixel arithmatic break.
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling,true);
    std::vector<char*> new_argv(argv, argv + argc);
    char* string = new char[10];
    strncpy(string, "-platform",10);
    new_argv.push_back(string);
    string = new char[23];
    strncpy(string, "windows:dpiawareness=0",23);
    new_argv.push_back(string);
    new_argv.push_back(nullptr);
    argv = &new_argv.data()[0];
    argc+=2;
#endif

    // Initialize all global maps.
    Pep::initMicroEnumMnemonMaps(Enu::CPUType::TwoByteDataBus, true);
    Pep::initEnumMnemonMaps();
    Pep::initMnemonicMaps(false);
    Pep::initAddrModesMap();
    Pep::initDecoderTables();
    Pep::initMicroDecoderTables();
    qInstallMessageHandler(nullptr);


    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Pepperdine Computer Science Lab");
    QCoreApplication::setOrganizationDomain("cslab.pepperdine.edu");
    QCoreApplication::setApplicationName("Pep9Micro");
    QCoreApplication::setApplicationVersion("9.3.0");

    MicroMainWindow w;
    w.show();
    return a.exec();
}
