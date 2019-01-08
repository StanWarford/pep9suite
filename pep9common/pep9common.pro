#TEMPLATE = lib
#TARGET = Pep9Common
#CONFIG += staticlib
QT += webenginewidgets widgets printsupport concurrent

# Issues occur when referencing source files from outside the current directory
# In windows, files must use the fully qualified path in order to be found
win32: {
    SRC_PTH = $$PWD\
}
# On Max OS X or Ubuntu, qmake will not function with fully qualified paths
else: {
    SRC_PTH = ""
}

FORMS += \
    $$SRC_PTH aboutpep.ui \
    $$SRC_PTH byteconverterbin.ui \
    $$SRC_PTH byteconverterchar.ui \
    $$SRC_PTH byteconverterdec.ui \
    $$SRC_PTH byteconverterhex.ui \
    $$SRC_PTH byteconverterinstr.ui \
    $$SRC_PTH inputpane.ui \
#   $$SRC_PTH iodialog.ui \ #This widget has yet to be implemented
    $$SRC_PTH iowidget.ui \
    $$SRC_PTH memorydumppane.ui \
    $$SRC_PTH outputpane.ui \
    $$SRC_PTH terminalpane.ui \

HEADERS += \
    $$SRC_PTH aboutpep.h \
    $$SRC_PTH acpumodel.h \
    $$SRC_PTH aisacpumodel.h \
    $$SRC_PTH amemorychip.h \
    $$SRC_PTH amemorydevice.h \
    $$SRC_PTH byteconverterbin.h \
    $$SRC_PTH byteconverterchar.h \
    $$SRC_PTH byteconverterdec.h \
    $$SRC_PTH byteconverterhex.h \
    $$SRC_PTH byteconverterinstr.h \
    $$SRC_PTH colors.h \
    $$SRC_PTH enu.h \
    $$SRC_PTH htmlhighlightermixin.h \
    $$SRC_PTH inputpane.h \
    $$SRC_PTH iowidget.h \
    $$SRC_PTH mainmemory.h \
    $$SRC_PTH memorychips.h \
    $$SRC_PTH memorydumppane.h \
    $$SRC_PTH outputpane.h \
    $$SRC_PTH pep.h \
    $$SRC_PTH symbolentry.h \
    $$SRC_PTH symboltable.h \
    $$SRC_PTH symbolvalue.h \
    $$SRC_PTH terminalpane.h \
    $$SRC_PTH updatechecker.h \


SOURCES += \
    $$SRC_PTH aboutpep.cpp \
    $$SRC_PTH acpumodel.cpp \
    $$SRC_PTH aisacpumodel.cpp \
    $$SRC_PTH amemorychip.cpp \
    $$SRC_PTH amemorydevice.cpp \
    $$SRC_PTH byteconverterbin.cpp \
    $$SRC_PTH byteconverterchar.cpp \
    $$SRC_PTH byteconverterdec.cpp \
    $$SRC_PTH byteconverterhex.cpp \
    $$SRC_PTH byteconverterinstr.cpp \
    $$SRC_PTH colors.cpp \
    $$SRC_PTH htmlhighlightermixin.cpp \
    $$SRC_PTH inputpane.cpp \
    $$SRC_PTH iowidget.cpp \
    $$SRC_PTH mainmemory.cpp \
    $$SRC_PTH memorychips.cpp \
    $$SRC_PTH memorydumppane.cpp \
    $$SRC_PTH outputpane.cpp \
    $$SRC_PTH pep.cpp \
    $$SRC_PTH symbolentry.cpp \
    $$SRC_PTH symboltable.cpp \
    $$SRC_PTH symbolvalue.cpp \
    $$SRC_PTH terminalpane.cpp \
    $$SRC_PTH updatechecker.cpp \





