TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES = ""
DEFINES += __USE_MINGW_ANSI_STDIO=1

QMAKE_CFLAGS = ""
QMAKE_CFLAGS += -Wno-unused-function
QMAKE_CFLAGS += -Wno-unused-parameter
QMAKE_CFLAGS += -Wno-missing-braces
QMAKE_CFLAGS += -Wno-missing-field-initializers

INCLUDEPATH += ck/include

SOURCES += test_EgSDLR.c
SOURCES += EgSDL.c
SOURCES += EgSDLR.c
SOURCES += EgSDLW.c
SOURCES += EgGeometries.c
SOURCES += EgWindows.c
SOURCES += EgEvents.c
SOURCES += EgQuantities.c
SOURCES += flecs.c


HEADERS += flecs.h
HEADERS += EgSDL.h
HEADERS += EgSDLR.h
HEADERS += EgSDLW.h
HEADERS += EgGeometries.h
HEADERS += EgWindows.h
HEADERS += EgEvents.h
HEADERS += EgBasics.h
HEADERS += EgQuantities.h
HEADERS += EgWindows.h


LIBS += -lmingw32 -lSDL2main -lSDL2 -lws2_32
