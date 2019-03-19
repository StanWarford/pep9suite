// File: pephighlighter.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.
    
    Copyright (C) 2009  J. Stanley Warford, Pepperdine University

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

#ifndef PEPASMHIGHLIGHTER_H
#define PEPASMHIGHLIGHTER_H

#include <QSyntaxHighlighter>

#include <QHash>
#include <QTextCharFormat>
#include "colors.h"
QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class PepASMHighlighter : public QSyntaxHighlighter
{
public:
    PepASMHighlighter(PepColors::Colors colors, QTextDocument *parent = nullptr);
    void rebuildHighlightingRules(PepColors::Colors color);
protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    PepColors::Colors colors;
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat oprndFormat;
    QTextCharFormat dotFormat;
    QTextCharFormat symbolFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat errorCommentFormat;
    QTextCharFormat singleQuotationFormat;
    QTextCharFormat doubleQuotationFormat;
    QTextCharFormat warningFormat;
    QTextCharFormat errorFormat;

};

#endif // PEPHIGHLIGHTER_H
