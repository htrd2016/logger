TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += /usr/include/glib-2.0

LIBS += -L/usr/lib -lm -lglib-2.0
LIBS += -lpthread

SOURCES += \
    mylog.c \
    parserthread.c \
    server.c \
    utils.c \
    main.c \
    memtypes.c \
    epollclient.c

HEADERS += \
    mylog.h \
    parserthread.h \
    utils.h \
    server.h \
    memtypes.h \
    epollclient.h

