CONFIG -= qt
TARGET = plgkplib_pictures
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
    ../../plugins/dll_plg_kplib_pictures.c \
    ../../forks/kplib_c/kplib_globalshit.c \
    ../../forks/kplib_c/kplib_globalshit_kzfs.c \
    ../../forks/kplib_c/kplib_pictures.c \
    ../../forks/kplib_c/kplib_pictures_bmp.c \
    ../../forks/kplib_c/kplib_pictures_cel.c \
    ../../forks/kplib_c/kplib_pictures_dds.c \
    ../../forks/kplib_c/kplib_pictures_gif.c \
    ../../forks/kplib_c/kplib_pictures_jpg.c \
    ../../forks/kplib_c/kplib_pictures_pcx.c \
    ../../forks/kplib_c/kplib_pictures_png.c \
    ../../forks/kplib_c/kplib_pictures_tga.c
