CONFIG -= qt
TARGET = ngl
TEMPLATE = lib

CONFIG += dll

unix {
    target.path = /usr/lib
    CONFIG += plugin no_plugin_name_prefix
    INSTALLS += target
    DEFINES += N_POSIX N_WCHAR32
    SOURCES += \
	../../opengl_render/unix_ngl_init.c  \
	../../unixsupport/keysym2ucs.c
    LIBS += -lGL -lGLU -lX11 -lXxf86vm -lrt
    QMAKE_CFLAGS += -fvisibility=hidden
    QMAKE_LFLAGS = -s
}
windows {
    DEFINES += N_WINDOWS _CRT_SECURE_NO_WARNINGS
    SOURCES += ../../commonsrc/render/win_ngl_init_common.c \
	../../opengl_render/win_ngl_init.c
    LIBS += -lopengl32 -lglu32 -lkernel32 -luser32 -lgdi32
}

CONFIG(debug, debug|release) {
    DESTDIR = ../output/debug
    LIBS += -L../output/debug
} else {
    DESTDIR = ../output/release
    LIBS += -L../output/release
}

SOURCES += \
    ../../extclib/mbstowcsl.c \
    ../../extclib/wcstombsl.c \
    ../../opengl_render/ngl_batch.c \
    ../../opengl_render/ngl_init.c \
    ../../opengl_render/ngl_shaders.c \
    ../../opengl_render/ngl_texture.c
