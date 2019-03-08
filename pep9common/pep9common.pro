#TEMPLATE = lib
#TARGET = Pep9Common
#CONFIG += staticlib
QT += webenginewidgets widgets printsupport concurrent

FORMS += \
    aboutpep.ui \
    byteconverterbin.ui \
    byteconverterchar.ui \
    byteconverterdec.ui \
    byteconverterhex.ui \
    byteconverterinstr.ui \
    inputpane.ui \
#   iodialog.ui \ #This widget has yet to be implemented
    iowidget.ui \
    memorydumppane.ui \
    outputpane.ui \
    terminalpane.ui \

HEADERS += \
    aboutpep.h \
    acpumodel.h \
    amemorychip.h \
    amemorydevice.h \
    byteconverterbin.h \
    byteconverterchar.h \
    byteconverterdec.h \
    byteconverterhex.h \
    byteconverterinstr.h \
    colors.h \
    enu.h \
    inputpane.h \
    iowidget.h \
    mainmemory.h \
    memorychips.h \
    memorydumppane.h \
    outputpane.h \
    pep.h \
    symbolentry.h \
    symboltable.h \
    symbolvalue.h \
    terminalpane.h \
    updatechecker.h \
    registerfile.h


SOURCES += \
    aboutpep.cpp \
    acpumodel.cpp \
    amemorychip.cpp \
    amemorydevice.cpp \
    byteconverterbin.cpp \
    byteconverterchar.cpp \
    byteconverterdec.cpp \
    byteconverterhex.cpp \
    byteconverterinstr.cpp \
    colors.cpp \
    inputpane.cpp \
    iowidget.cpp \
    mainmemory.cpp \
    memorychips.cpp \
    memorydumppane.cpp \
    outputpane.cpp \
    pep.cpp \
    symbolentry.cpp \
    symboltable.cpp \
    symbolvalue.cpp \
    terminalpane.cpp \
    updatechecker.cpp \
    enu.cpp \
    registerfile.cpp





