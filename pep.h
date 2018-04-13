// File: pep.h
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
#ifndef PEP_H
#define PEP_H

#include <QString>
#include <QMap>
#include <QColor>

#include "enu.h"

class Pep
{
public:
    // Fonts:
    static const QString codeFont;
    static const int codeFontSize;
    static const int codeFontSizeSmall;
    static const int codeFontSizeLarge;
    static const QString labelFont;
    static const int labelFontSize;
    static const int labelFontSizeSmall;
    static const QString cpuFont;
    static const int cpuFontSize;
    static QString getSystem();

    // Function to read text from a resource file
    static QString resToString(QString fileName);
    static QString addCycleNumbers(QString codeString);

    // Maps between mnemonic enums and strings
    static QMap<Enu::EControlSignals, QString> decControlToMnemonMap;
    static QMap<Enu::EControlSignals, QString> memControlToMnemonMap;
    static QMap<Enu::EClockSignals, QString> clockControlToMnemonMap;
    static QMap<Enu::EKeywords, QString> specificationToMnemonMap;
    static QMap<Enu::EKeywords, QString> memSpecToMnemonMap;
    static QMap<Enu::EKeywords, QString> regSpecToMnemonMap;
    static QMap<Enu::EKeywords, QString> statusSpecToMnemonMap;
    static QMap<QString, Enu::EControlSignals> mnemonToDecControlMap;
    static QMap<QString, Enu::EControlSignals> mnemonToMemControlMap;
    static QMap<QString, Enu::EClockSignals> mnemonToClockControlMap;
    static QMap<QString, Enu::EKeywords> mnemonToSpecificationMap;
    static QMap<QString, Enu::EKeywords> mnemonToMemSpecMap;
    static QMap<QString, Enu::EKeywords> mnemonToRegSpecMap;
    static QMap<QString, Enu::EKeywords> mnemonToStatusSpecMap;
    static void initEnumMnemonMaps();

};
#endif // PEP_H
