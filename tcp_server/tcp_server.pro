TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../loggerd

SOURCES += main.c \
    ../loggerd/server.c \
    ../loggerd/client.c \
    ../loggerd/mylog.c

HEADERS += \
    ../loggerd/client.h \
    ../loggerd/mylog.h \
    ../loggerd/server.h
