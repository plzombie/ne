CONFIG -= qt
TARGET = nullgl
TEMPLATE = lib

CONFIG += dll

unix {
    target.path = /usr/lib
    CONFIG += plugin no_plugin_name_prefix
    INSTALLS += target
    DEFINES += N_POSIX N_WCHAR32
    QMAKE_CFLAGS += -fvisibility=hidden
    QMAKE_LFLAGS = -s
}
windows {
    DEFINES += N_WINDOWS _CRT_SECURE_NO_WARNINGS
}

CONFIG(debug, debug|release) {
    DESTDIR = ../output/debug
    LIBS += -L../output/debug
} else {
    DESTDIR = ../output/release
    LIBS += -L../output/release
}

SOURCES += \
    ../../null_render/ngl_batch.c \
    ../../null_render/ngl_init.c \
    ../../null_render/ngl_texture.c \
    ../../null_render/null_ngl_init.c
