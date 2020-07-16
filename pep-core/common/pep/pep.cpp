// File: pep.cpp
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

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

#include <QFile>
#include <QString>
#include <QStringList>
#include <QtCore>

#include "pep.h"


QString Pep::resToString(QString fileName, bool removeLineNumbers) {
    QFile file(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    QString inString = "";
    while (!in.atEnd()) {
        QString line = in.readLine();
        inString.append(line + "\n");
    }
    if(removeLineNumbers) {
        inString = removeCycleNumbers(inString);
    }
    return inString;
}

QString Pep::addCycleNumbers(QString codeString) {
    int lineNumber = 1;
    QStringList microcodeList = codeString.split("\n");
    for (int i = 0; i < microcodeList.size(); i++) {
        // Skip any lines that start are empty, have a comment, unitpre, or unitpost.
        if (QRegExp("^\\s*(//|$|unitpre|unitpost)", Qt::CaseInsensitive).indexIn(microcodeList.at(i)) != 0) {
            microcodeList[i].prepend(QString("%1. ").arg(lineNumber++));
        }
    }
    return microcodeList.join("\n");
}

QString Pep::removeCycleNumbers(QString codeString)
{
    QStringList microcodeList;
    microcodeList = codeString.split('\n');
    for (int i = 0; i < microcodeList.size(); i++) {
        microcodeList[i].remove(QRegExp("^[0-9]+\\.?\\s*"));
    }
    return microcodeList.join("\n");
}

/*
 * Begin Pep9 Sources
 */
