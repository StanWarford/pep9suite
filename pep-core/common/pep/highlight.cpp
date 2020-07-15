#include "highlight.h"

ASMHighlighter::ASMHighlighter(PepColors::Colors colors, QTextDocument *parent): QSyntaxHighlighter(parent), colors(colors)
{

}

MicroHighlighter::MicroHighlighter(PepCore::CPUType type, bool fullCtrlSection, const PepColors::Colors colors, QTextDocument *parent):
    QSyntaxHighlighter(parent), cpuType(type), forcedFeatures(false), fullCtrlSection(fullCtrlSection), colors(colors)
{

}

void MicroHighlighter::forceAllFeatures(bool features)
{
    forcedFeatures = features;
}

void MicroHighlighter::setCPUType(PepCore::CPUType type)
{
    cpuType = type;
}

void MicroHighlighter::setFullControlSection(bool fullCtrlSection)
{
    this->fullCtrlSection = fullCtrlSection;
}


