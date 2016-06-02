TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../loggerd

LIBS += -lm

SOURCES += main.c \
    ../loggerd/server.c \
    ../loggerd/epollclient.c \
    ../loggerd/mylog.c

HEADERS += \
    ../loggerd/epollclient.h \
    ../loggerd/mylog.h \
    ../loggerd/server.h
