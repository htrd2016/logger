TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../loggerd

#for clang compiler, disable this line when using non-clang, like gcc
#DEFINES += __CLANG__

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

