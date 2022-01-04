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

SOURCES += test_eg_log.c
SOURCES += flecs.c
SOURCES += eg_log.c
SOURCES += eg_memory_pool.c


HEADERS += flecs.h
HEADERS += eg_log.h
HEADERS += eg_memory_pool.h


LIBS += -lmingw32 -lws2_32
