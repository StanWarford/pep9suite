// File: termhelper.h
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
#ifndef TERMHELPER_H
#define TERMHELPER_H

#include <QtCore>

extern const QString errLogOpenErr;
extern const QString hadErr;
extern const QString assemble;

class AsmProgramManager;

// Assemble the default operating system from the help documentation,
// and install it into the program manager.
void buildDefaultOperatingSystem(AsmProgramManager& manager);

// Helper function that turns hexadecimal object code into a vector of
// unsigned characters, which is easier to copy into memory.
QVector<quint8> convertObjectCodeToIntArray(QString program);

#endif // TERMHELPER_H
