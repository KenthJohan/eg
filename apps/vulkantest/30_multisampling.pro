TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES = ""
DEFINES += __USE_MINGW_ANSI_STDIO=1

QMAKE_CFLAGS = ""
# pacman -S mingw64/mingw-w64-x86_64-glfw
# pacman -S mingw64/mingw-w64-x86_64-glm

#QMAKE_CFLAGS += -Wno-unused-function
QMAKE_CFLAGS += -Wno-unused-parameter
QMAKE_CFLAGS += -Wno-missing-braces
QMAKE_CFLAGS += -Wno-missing-field-initializers

SOURCES += 30_multisampling.cpp
SOURCES += vk_assert.cpp
SOURCES += tiny_obj_loader.cc
SOURCES += stb_image.c
SOURCES += readfile.c
SOURCES += load_model.cpp
SOURCES += flecs.c
SOURCES += EgTypes.c
SOURCES += EgVk_types.c
SOURCES += EgVk_systems.c
SOURCES += renderer.cpp
SOURCES += platform_glfw.c
SOURCES += eg_util.c

INCLUDEPATH += C:/VulkanSDK/1.3.216.0/Include
LIBS += -LC:/VulkanSDK/1.3.216.0/Lib
LIBS += -lglfw3 -lgdi32 -lvulkan-1 -lws2_32
