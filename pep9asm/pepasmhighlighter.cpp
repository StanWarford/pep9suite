//File: pephighlighter.cpp
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
#include "pepasmhighlighter.h"
#include "enu.h"
#include "pep.h"
PepASMHighlighter::PepASMHighlighter(PepColors::Colors colors, QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    rebuildHighlightingRules(colors);
}

void PepASMHighlighter::rebuildHighlightingRules(PepColors::Colors color)
{
    colors = color;
    HighlightingRule rule;

    // For all highlighting rules, set the case sensitivity now.
    // Doing so in the highlighting function caused a huge perfomance
    // hit for large blocks of highlighted text.
    oprndFormat.setForeground(colors.leftOfExpression);
    oprndFormat.setFontWeight(QFont::Bold);
    QStringList oprndPatterns;
    highlightingRules.clear();
    for(QString text : Pep::enumToMnemonMap) {
        oprndPatterns << "\\b" + text + "\\b";
    }

    foreach (const QString &pattern, oprndPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.pattern.setCaseSensitivity(Qt::CaseInsensitive);
        rule.format = oprndFormat;
        highlightingRules.append(rule);
    }

    dotFormat.setForeground(colors.leftOfExpression);
    dotFormat.setFontItalic(true);
    QStringList dotPatterns;
    dotPatterns << "[\\.]\\bEQUATE\\b" << "[\\.]\\bASCII\\b" << "[\\.]\\bBLOCK\\b"
            << "[\\.]\\bBURN\\b" << "[\\.]\\bBYTE\\b" << "[\\.]\\bEND\\b"
            << "[\\.]\\bALIGN\\b" << "[\\.]\\bWORD\\b" << "[\\.]\\bADDRSS\\b";
    foreach (const QString &pattern, dotPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.pattern.setCaseSensitivity(Qt::CaseInsensitive);
        rule.format = dotFormat;
        highlightingRules.append(rule);
    }

    symbolFormat.setFontWeight(QFont::Bold);
    symbolFormat.setForeground(colors.rightOfExpression);
    // Selects most accented unicode characters, based on answer:
    // https://stackoverflow.com/a/26900132
    rule.pattern = QRegExp("([A-zÀ-ÖØ-öø-ÿ_][0-9A-zÀ-ÖØ-öø-ÿ_]*)(?=:)");
    rule.pattern.setCaseSensitivity(Qt::CaseInsensitive);
    rule.format = symbolFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(colors.comment);
    rule.pattern = QRegExp(";.*");
    rule.pattern.setCaseSensitivity(Qt::CaseInsensitive);
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    singleQuotationFormat.setForeground(colors.errorHighlight);
    rule.pattern = QRegExp("((\')(?![\'])(([^\'|\\\\]){1}|((\\\\)([\'|b|f|n|r|t|v|\"|\\\\]))|((\\\\)(([x|X])([0-9|A-F|a-f]{2}))))(\'))");
    rule.pattern.setCaseSensitivity(Qt::CaseInsensitive);
    rule.format = singleQuotationFormat;
    highlightingRules.append(rule);

    doubleQuotationFormat.setForeground(colors.errorHighlight);
    rule.pattern = QRegExp("((\")((([^\"|\\\\])|((\\\\)([\'|b|f|n|r|t|v|\"|\\\\]))|((\\\\)(([x|X])([0-9|A-F|a-f]{2}))))*)(\"))");
    rule.pattern.setCaseSensitivity(Qt::CaseInsensitive);
    rule.format = doubleQuotationFormat;
    highlightingRules.append(rule);

    warningFormat.setForeground(colors.altTextHighlight);
    warningFormat.setBackground(colors.warningHighlight);
    rule.pattern = QRegExp(";WARNING:[\\s].*$");
    rule.pattern.setCaseSensitivity(Qt::CaseInsensitive);
    rule.format = warningFormat;
    highlightingRules.append(rule);

    errorCommentFormat.setForeground(colors.altTextHighlight);
    errorCommentFormat.setBackground(colors.errorHighlight);
    rule.pattern = QRegExp(";ERROR:[\\s].*$");
    rule.pattern.setCaseSensitivity(Qt::CaseInsensitive);
    rule.format = errorCommentFormat;
    highlightingRules.append(rule);
}

void PepASMHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }

}
