#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

#include "pep/constants.h"
#include "style/colors.h"

class ASMHighlighter : public QSyntaxHighlighter
{
public:
    ASMHighlighter(PepColors::Colors colors, QTextDocument *parent=nullptr);
    virtual void rebuildHighlightingRules(PepColors::Colors color) = 0;

protected:
    PepColors::Colors colors;
};

class MicroHighlighter : public QSyntaxHighlighter
{
public:
    MicroHighlighter(PepCore::CPUType type, bool fullCtrlSection, const PepColors::Colors colors, QTextDocument *parent = nullptr);
    void forceAllFeatures(bool features);
    virtual void rebuildHighlightingRules(const PepColors::Colors color) = 0;
    void setCPUType(PepCore::CPUType type);
    void setFullControlSection(bool fullCtrlSection);
protected:
    PepCore::CPUType cpuType;
    bool forcedFeatures, fullCtrlSection;
    PepColors::Colors colors;


};


#endif // HIGHLIGHT_H




