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


SOURCES += test_eg_jobs.c
SOURCES += flecs.c
SOURCES += eg_jobs.c
HEADERS += eg_jobs.h


LIBS += -lmingw32 -lws2_32
