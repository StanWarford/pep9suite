RESOURCES += \
    $$PWD/dark_style.qrc

DISTFILES += \
    $$PWD/dark_style.qss

HEADERS += \
    $$PWD/colors.h \
    $$PWD/darkhelper.h \
    $$PWD/fonts.h

SOURCES += \
    $$PWD/colors.cpp \
    $$PWD/fonts.cpp

macx {
    QT += macextras
    LIBS += -framework Foundation -framework AppKit
    OBJECTIVE_SOURCES = $$PWD/darkhelper_mac.mm
}
else {
    SOURCES += $$PWD/darkhelper_other.cpp
}
