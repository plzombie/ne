CONFIG -= qt
TARGET = plgwavpack
TEMPLATE = lib

CONFIG += dll

unix {
    target.path = /usr/lib
    CONFIG += plugin no_plugin_name_prefix
    INSTALLS += target
    DEFINES += N_POSIX N_WCHAR32
    LIBS += -lwavpack
    QMAKE_CFLAGS += -fvisibility=hidden
    QMAKE_LFLAGS = -s
}
windows {
    DEFINES += N_WINDOWS _CRT_SECURE_NO_WARNINGS
    win32-msvc*:contains(QMAKE_TARGET.arch, x86_64) {
        LIBS += ../../forks/wavpack/x64/wavpackdll.lib
    } else {
        LIBS += ../../forks/wavpack/wavpackdll.lib
    }
}

CONFIG(debug, debug|release) {
    DESTDIR = ../output/debug
    LIBS += -L../output/debug
} else {
    DESTDIR = ../output/release
    LIBS += -L../output/release
}

SOURCES += \
    ../../plugins/dll_plg_wv.c
