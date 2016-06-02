TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../loggerd

#DEFINES += MACHINE_AMD64
DEFINES += MACHINE_X86

LIBS += -lgtest -lgtest_main -lpthread
SOURCES += \
    Tester.cpp \
    ../loggerd/mylog.c \
    ../loggerd/memtypes.c \
    ../loggerd/utils.c

HEADERS += \
    ../loggerd/mylog.h \
    ../loggerd/memtypes.h \
    ../loggerd/utils.h

