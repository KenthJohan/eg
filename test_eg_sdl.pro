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

SOURCES += test_eg_sdl.c
SOURCES += eg_sdl.c
SOURCES += eg_geometry.c
SOURCES += eg_window.c
SOURCES += eg_userevent.c
SOURCES += eg_quantity.c
SOURCES += flecs.c


HEADERS += flecs.h
HEADERS += eg_sdl.h
HEADERS += eg_geometry.h
HEADERS += eg_window.h
HEADERS += eg_userevent.h
HEADERS += eg_base.h
HEADERS += eg_quantity.h


LIBS += -lmingw32 -lSDL2main -lSDL2 -lws2_32
