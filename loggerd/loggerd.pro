TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += /usr/include/glib-2.0

LIBS += -L/usr/lib -lm -lglib-2.0
LIBS += -lpthread

SOURCES += \
    client.c \
    mylog.c \
    parserthread.c \
    server.c \
    types.c \
    utils.c \
    main.c

HEADERS += \
    client.h \
    mylog.h \
    parserthread.h \
    types.h \
    utils.h \
    server.h

