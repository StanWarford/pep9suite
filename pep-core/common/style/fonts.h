#ifndef FONTS_H
#define FONTS_H

#include <QFont>
namespace PepCore {
    QString getSystem();
    extern const QString codeFont;
    extern const int codeFontSize;
    extern const int codeFontSizeSmall;
    extern const int codeFontSizeLarge;
    extern const int ioFontSize;
    extern const QString labelFont;
    extern const int labelFontSize;
    extern const int labelFontSizeCPUPane;
    extern const int labelFontSizeSmall;
    extern const QString cpuFont;
    extern const int cpuFontSize;
}
#endif // FONTS_H
