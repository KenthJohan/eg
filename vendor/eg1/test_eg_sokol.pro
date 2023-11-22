TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += __USE_MINGW_ANSI_STDIO=1

QMAKE_CFLAGS += -Wno-unused-function
QMAKE_CFLAGS += -Wno-unused-parameter
QMAKE_CFLAGS += -Wno-missing-braces
QMAKE_CFLAGS += -Wno-missing-field-initializers

INCLUDEPATH += ck/include

SOURCES += test_eg_sokol.c
SOURCES += eg_sokol.c
SOURCES += eg_geometry.c
SOURCES += eg_window.c
SOURCES += eg_userevent.c
SOURCES += eg_quantity.c
SOURCES += flecs.c


HEADERS += flecs.h
HEADERS += eg_sokol.h
HEADERS += eg_geometry.h
HEADERS += eg_window.h
HEADERS += eg_userevent.h
HEADERS += eg_base.h
HEADERS += eg_quantity.h
HEADERS += eg_window_sokol.h


LIBS += -lmingw32 -lgdi32 -lws2_32
