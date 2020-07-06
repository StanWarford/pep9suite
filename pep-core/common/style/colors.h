// File: colors.h
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

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
#ifndef COLORS_H
#define COLORS_H
#include <QColor>
#include <QImage>
namespace PepColors{
    struct Colors
    {
        QColor comment; //Used for normal comments in micro & asm.
        QColor rightOfExpression; //Used for right hand side of expressions in micro, and mnemonics and pseudo-ops in asm
        QColor leftOfExpression; //Used for numbers in micro, and symbols in asm.
        QColor symbolHighlight; //Used for symbols in micro & unused in asm.
        QColor conditionalHighlight;
        QColor branchFunctionHighlight;
        QColor warningHighlight; //Unused in micro, and used as warnings in asm.
        QColor errorHighlight; //Used by errors in micro & asm, and strings in asm.
        QColor altTextHighlight; //Text color opposite of the default text color (e.g. white when the default is black).

        QColor seqCircuitColor;
        QColor combCircuitRed;
        QColor muxCircuitRed; // A sightly lighter shade of combCircuitRed that is a better background for text
        QColor combCircuitBlue;
        QColor muxCircuitBlue; // A sightly lighter shade of combCircuitBlue that is a better background for text
        QColor aluColor;
        QColor aluOutline;
        QColor combCircuitYellow;
        QColor muxCircuitYellow; // A sightly lighter shade of combCircuitYellow that is a better background for text
        QColor combCircuitGreen;
        QColor muxCircuitGreen; // A sightly lighter shade of combCircuitGreen that is a better background for text
        QColor arrowColorOn;
        QColor arrowColorOff;
        QColor textColor;
        QColor backgroundFill;
        QString arrowImageOn;
        QString arrowImageOff;

        QColor memoryHighlightPC;
        QColor memoryHighlightSP;
        QColor memoryHighlightChanged;
        QColor lineAreaBackground;
        QColor lineAreaText;
        QColor lineAreaHighlight;
    };
    static const QColor transparent = QColor(255,255,255,0);
    const Colors initLight();
    const Colors initDark();
    static const Colors lightMode = {initLight()};
    static const Colors darkMode = {initDark()};
    QColor invert(QColor input);
}
#endif // COLORS_H
