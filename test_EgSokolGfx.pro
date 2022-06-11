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

SOURCES += test_EgSokolGfx.c
SOURCES += EgSDL.c
SOURCES += EgSDLW.c
SOURCES += EgSokolGfx.c
SOURCES += EgGeometries.c
SOURCES += EgWindows.c
SOURCES += EgEvents.c
SOURCES += EgQuantities.c
SOURCES += sokol_source.c
SOURCES += flecs.c


HEADERS += flecs.h
HEADERS += EgSDL.h
HEADERS += EgSDLW.h
HEADERS += EgGeometries.h
HEADERS += EgWindows.h
HEADERS += EgEvents.h
HEADERS += EgBasics.h
HEADERS += EgQuantities.h


LIBS += -lmingw32 -lSDL2main -lSDL2 -lws2_32 -lgdi32
