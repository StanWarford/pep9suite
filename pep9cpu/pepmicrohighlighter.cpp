//File: pephighlighter.cpp
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

#include "pepmicrohighlighter.h"
#include "pep.h"
#include <QTextDocument>
PepMicroHighlighter::PepMicroHighlighter(Enu::CPUType type, bool fullCtrlSection, const PepColors::Colors color, QTextDocument *parent)
    : QSyntaxHighlighter(parent), cpuType(type),
      /*RestyleableItem(color, parent),*/ forcedFeatures(false), fullCtrlSection(fullCtrlSection)
{
    // Trigger an update whenever a style is changed.
    // connect(this, &RestyleableItem::styleChanged, this, &PepMicroHighlighter::onStyleChange);
    rebuildHighlightingRules(color);
}



void PepMicroHighlighter::forceAllFeatures(bool features)
{
    forcedFeatures = features;
}

void PepMicroHighlighter::rebuildHighlightingRules(const PepColors::Colors color)
{
    colors = color;
    HighlightingRule rule;

    highlightingRulesOne.clear();
    highlightingRulesTwo.clear();
    highlightingRulesAll.clear();
    numFormat.setForeground(color.rightOfExpression);
    rule.pattern = QRegExp("(0x)?[0-9a-fA-F]+(?=(,|;|(\\s)*$|\\]|(\\s)*//))");
    rule.format = numFormat;
    highlightingRulesOne.append(rule);
    highlightingRulesTwo.append(rule);

    // Only enable highlighting conditionals if the full control section is used
    if(fullCtrlSection) {
        symbolFormat.setForeground(color.symbolHighlight);
        // A symbol is an text from the start of the line up to, but not including a ':'
        // Selects most accented unicode characters, based on answer:
        // https://stackoverflow.com/a/26900132
        rule.pattern = QRegExp("^([A-zÀ-ÖØ-öø-ÿ][0-9A-zÀ-ÖØ-öø-ÿ]*)(?=:)\\b");
        rule.format = symbolFormat;
        highlightingRulesOne.append(rule);
        highlightingRulesTwo.append(rule);
        identFormat.setForeground(color.seqCircuitColor);

        // Treat anything following an if, else, or a goto as a valid identifier
        // and highlight it in blue
        QStringList symLoc;
        symLoc << ("else \\w+")
               << ("if \\w+ \\w+")
               << ("goto \\w+");
        foreach (const QString &pattern, symLoc) {
            rule.pattern = QRegExp(pattern);
            rule.format = identFormat;
            highlightingRulesOne.append(rule);
            highlightingRulesTwo.append(rule);
        }

        // Highlight the special conditional branching keywords
        conditionalFormat.setForeground(color.conditionalHighlight);
        QStringList keywords;
        keywords << "if" << "else" << "goto" << "stop";
        foreach (const QString &pattern, keywords) {
            rule.pattern = QRegExp(pattern);
            rule.format = conditionalFormat;
            highlightingRulesOne.append(rule);
            highlightingRulesTwo.append(rule);
        }

        // Highlight the branch functions
        branchFunctionFormat.setForeground(color.branchFunctionHighlight);
        for(QString function : Pep::branchFuncToMnemonMap.values())
        {
            rule.pattern = QRegExp(function,Qt::CaseInsensitive);
            rule.format = branchFunctionFormat;
            highlightingRulesOne.append(rule);
            highlightingRulesTwo.append(rule);
        }
    }
    oprndFormat.setForeground(color.leftOfExpression);
    oprndFormat.setFontWeight(QFont::Bold);
    QStringList oprndPatterns;
        oprndPatterns << "\\bLoadCk\\b" << "\\bC\\b" << "\\bB\\b"
                << "\\bA\\b" << "\\bMARCk\\b" << "\\bMDRCk\\b"
                << "\\bAMux\\b" << "\\bMDRMux\\b" << "\\bCMux\\b"
                << "\\bALU\\b" << "\\bCSMux\\b" << "\\bSCk\\b" << "\\bCCk\\b" << "\\bVCk\\b"
                << "\\bAndZ\\b" << "\\bZCk\\b" << "\\bNCk\\b"
                << "\\bMemRead\\b" << "\\bMemWrite\\b" << "^(\\s)*UnitPre(?=:)\\b" << "^(\\s)*UnitPost(?=:)\\b"
                   // pre/post symbols:
                << "\\bN\\b" << "\\bZ\\b" << "\\bV\\b" << "\\bS\\b"
                << "\\bX\\b" << "\\bSP\\b" << "\\bPC\\b" << "\\bIR\\b"
                << "\\bT1\\b" << "\\bT2\\b" << "\\bT3\\b" << "\\bT4\\b"
                << "\\bT5\\b" << "\\bT6\\b" << "\\bMem\\b";
        foreach (const QString &pattern, oprndPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = oprndFormat;
            highlightingRulesOne.append(rule);
        }
        oprndPatterns.clear();
        oprndPatterns << "\\bLoadCk\\b" << "\\bC\\b" << "\\bB\\b"
                << "\\bA\\b" << "\\bMARCk\\b" << "\\bMARMux\\b"
                << "\\bMDROCk\\b" << "\\bMDRECk\\b" << "\\bMDROMux\\b" << "\\bMDREMux\\b" << "\\bEOMux\\b" << "\\bCMux\\b"
                << "\\bAMux\\b"<< "\\bALU\\b" << "\\bCSMux\\b" << "\\bSCk\\b" << "\\bCCk\\b" << "\\bVCk\\b"
                << "\\bAndZ\\b" << "\\bZCk\\b" << "\\bNCk\\b"
                << "\\bMemRead\\b" << "\\bMemWrite\\b" << "^(\\s)*UnitPre(?=:)\\b" << "^(\\s)*UnitPost(?=:)\\b"
                   // pre/post symbols:
                << "\\bN\\b" << "\\bZ\\b" << "\\bV\\b" << "\\bS\\b"
                << "\\bX\\b" << "\\bSP\\b" << "\\bPC\\b" << "\\bIR\\b"
                << "\\bT1\\b" << "\\bT2\\b" << "\\bT3\\b" << "\\bT4\\b"
                << "\\bT5\\b" << "\\bT6\\b" << "\\bMem\\b"
                << "\\bPValid\\b"<< "\\bPValidCk\\b";
        foreach (const QString &pattern, oprndPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = oprndFormat;
            highlightingRulesTwo.append(rule);
        }


    singleLineCommentFormat.setForeground(color.comment);
    rule.pattern = QRegExp("//.*");
    rule.format = singleLineCommentFormat;
    highlightingRulesOne.append(rule);
    highlightingRulesTwo.append(rule);

    errorCommentFormat.setForeground(color.altTextHighlight);
    errorCommentFormat.setBackground(color.errorHighlight);
    rule.pattern = QRegExp("//\\sERROR:[\\s].*");
    rule.format = errorCommentFormat;
    highlightingRulesOne.append(rule);
    highlightingRulesTwo.append(rule);

    highlightingRulesAll.append(highlightingRulesOne);
    highlightingRulesAll.append(highlightingRulesTwo);
}

void PepMicroHighlighter::setCPUType(Enu::CPUType type)
{
    cpuType = type;
}

void PepMicroHighlighter::highlightBlock(const QString &text)
{
    QVector<HighlightingRule> highlightingRules;
    if(forcedFeatures){
        highlightingRules=highlightingRulesAll;
    }
    else{
        highlightingRules = (cpuType == Enu::CPUType::OneByteDataBus
                ? highlightingRulesOne : highlightingRulesTwo);
    }

    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        expression.setCaseSensitivity(Qt::CaseInsensitive);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    /*
    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0) {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, errorCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
    */
}

/*void PepMicroHighlighter::onStyleChange()
{
    rebuildHighlightingRules(*getStyle());
}*/

