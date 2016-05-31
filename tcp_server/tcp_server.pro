TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../loggerd
INCLUDEPATH += /usr/include/glib-2.0/
INCLUDEPATH += /usr/include/glib-2.0/glib/
INCLUDEPATH += /usr/lib/x86_64-linux-gnu/glib-2.0/include/

LIBS += -L/usr/lib -lm -lglib-2.0

SOURCES += main.c \
    ../loggerd/server.c \
    ../loggerd/client.c \
    ../loggerd/mylog.c

HEADERS += \
    ../loggerd/client.h \
    ../loggerd/mylog.h \
    ../loggerd/server.h
