#include "fonts.h"
namespace PepCore {

    const QString codeFont = getSystem() == "Windows" ? "Courier" : (getSystem() == "Mac" ? "Courier" : "Ubuntu Mono,Courier New");
    const int codeFontSize = getSystem() == "Windows" ? 9 : (getSystem() == "Mac" ? 11 : 11);
    const int codeFontSizeSmall = getSystem() == "Windows" ? 7 : (getSystem() == "Mac" ? 10 : 10);
    const int codeFontSizeLarge = getSystem() == "Windows" ? 9 : (getSystem() == "Mac" ? 11 : 11);
    const int ioFontSize = getSystem() ==  "Windows" ? 10 : (getSystem() == "Mac" ? 13 : 10);
    const QString labelFont = getSystem() == "Windows" ? "Verdana" : (getSystem() == "Mac" ? "Lucida Grande" : "Ubuntu");
    const int labelFontSize = getSystem() == "Windows" ? 8 : (getSystem() == "Mac" ? 13 : 9);
    const int labelFontSizeCPUPane = getSystem() == "Windows" ? 8 : (getSystem() == "Mac" ? 11 : 9);
    const int labelFontSizeSmall = getSystem() == "Windows" ? 7 : (getSystem() == "Mac" ? 10 : 8);
    const QString cpuFont = getSystem() == "Windows" ? "Verdana" : (getSystem() == "Mac" ? "Lucida Grande" : "Ubuntu");
    const int cpuFontSize = getSystem() == "Windows" ? 8 : (getSystem() == "Mac" ? 11 : 8);

    QString getSystem() {

        #ifdef Q_WS_X11
        return QString("Linux");
        s
        #elif defined Q_OS_OSX
        return QString("Mac");

        #elif defined Q_WS_QWS
        return QString("Embedded Linux");

        #elif defined Q_OS_WIN32
        return QString("Windows");

        #elif defined Q_WS_WIN
        return QString("Windows");

        #else
        return QString("No system");
        #endif
    }
}
