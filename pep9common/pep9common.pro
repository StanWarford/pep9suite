#TEMPLATE = lib
#TARGET = Pep9Common
#CONFIG += staticlib
QT += webenginewidgets widgets printsupport concurrent

# Always prefix files with their fully qualified paths.
# Otherwise, including projects can't find the files.

FORMS += \
    $$PWD\byteconverterbin.ui \
    $$PWD\byteconverterchar.ui \
    $$PWD\byteconverterdec.ui \
    $$PWD\byteconverterhex.ui \
    $$PWD\byteconverterinstr.ui \
    $$PWD\inputpane.ui \
 #   $$PWD\iodialog.ui \ This widget has yet to be implemented
    $$PWD\iowidget.ui \
    $$PWD\outputpane.ui \
    $$PWD\terminalpane.ui \

HEADERS += \
    $$PWD\byteconverterbin.h \
    $$PWD\byteconverterchar.h \
    $$PWD\byteconverterdec.h \
    $$PWD\byteconverterhex.h \
    $$PWD\byteconverterinstr.h \
    $$PWD\colors.h \
    $$PWD\enu.h \
    $$PWD\htmlhighlightermixin.h \
    $$PWD\inputpane.h \
    $$PWD\iowidget.h \
    $$PWD\memorysection.h \
    $$PWD\outputpane.h \
    $$PWD\pep.h \
    $$PWD\symbolentry.h \
    $$PWD\symboltable.h \
    $$PWD\symbolvalue.h \
    $$PWD\terminalpane.h \
    $$PWD\updatechecker.h \
    $$PWD\acpumodel.h \
    $$PWD\amccpumodel.h \
    $$PWD\aisacpumodel.h \
    $$PWD\amemorydevice.h \
    $$PWD\amemorychip.h \
    $$PWD\memorychips.h \
    $$PWD\mainmemory.h \


SOURCES += \
    $$PWD\byteconverterbin.cpp \
    $$PWD\byteconverterchar.cpp \
    $$PWD\byteconverterdec.cpp \
    $$PWD\byteconverterhex.cpp \
    $$PWD\byteconverterinstr.cpp \
    $$PWD\colors.cpp \
    $$PWD\htmlhighlightermixin.cpp \
    $$PWD\inputpane.cpp \
    $$PWD\iowidget.cpp \
    $$PWD\memorysection.cpp \
    $$PWD\outputpane.cpp \
    $$PWD\pep.cpp \
    $$PWD\symbolentry.cpp \
    $$PWD\symboltable.cpp \
    $$PWD\symbolvalue.cpp \
    $$PWD\terminalpane.cpp \
    $$PWD\updatechecker.cpp \
    $$PWD\acpumodel.cpp \
    $$PWD\amccpumodel.cpp \
    $$PWD\aisacpumodel.cpp \
    $$PWD\amemorydevice.cpp \
    $$PWD\amemorychip.cpp \
    $$PWD\memorychips.cpp \
    $$PWD\mainmemory.cpp

