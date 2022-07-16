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

SOURCES += 30_multisampling.cpp
SOURCES += vk_assert.cpp


LIBS += -LC:/VulkanSDK/1.3.216.0/Lib
LIBS += -lglfw3 -lgdi32 -lvulkan-1
