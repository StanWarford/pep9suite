// File: main.cpp
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

#include <QtWidgets/QApplication>
#include "mainwindow.h"
#ifdef WIN32
#include <string.h>
#include <qvector.h>
#include <iostream>
#endif
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        std::cerr << "QtDebugMsg " << msg.toStdString();
        break;
    case QtInfoMsg:
        std::cerr << "QtInfoMsg " << msg.toStdString();
        break;
    case QtWarningMsg:
        std::cerr << "QtWarningMsg " << msg.toStdString();
        break;
    case QtCriticalMsg:
        std::cerr << "QtCriticalMsg " << msg.toStdString();
        break;
    case QtFatalMsg:
        std::cerr << "QtFatalMsg " << msg.toStdString();
        break;
    default:
        std::cerr <<"Idk wtf this is " << msg.toStdString();
    }
}

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
    //qInstallMessageHandler(myMessageOutput);
    qInstallMessageHandler(0);
    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}
