TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG(debug, debug|release) {
    DESTDIR = ../output/debug
    LIBS += -L../output/debug
    CONFIG(msvc) {
        PRE_TARGETDEPS += ../output/debug/nya.lib
    } else {
        PRE_TARGETDEPS += ../output/debug/libnya.a
    }
} else {
    DESTDIR = ../output/release
    LIBS += -L../output/release
    CONFIG(msvc) {
        PRE_TARGETDEPS += ../output/release/nya.lib
    } else {
        PRE_TARGETDEPS += ../output/release/libnya.a
    }
    unix {
        QMAKE_LFLAGS += -s
    }
}

LIBS += -lnya

SOURCES += \
    ../../games/colorlines2013/cl_game.c \
    ../../games/colorlines2013/cl_main.c \
    ../../games/colorlines2013/cl_menu.c \
    ../../games/colorlines2013/cl_score.c \
    ../../games/colorlines2013/cl_sdifficulty.c

unix {
    DEFINES += N_POSIX N_WCHAR32 N_STATIC
    LIBS += -lrt
    !freebsd {
        LIBS += -ldl
    }
    QMAKE_CFLAGS += -pthread
    QMAKE_LFLAGS += -pthread
}
freebsd {
    LIBS += -lstdthreads
}
windows {
    DEFINES += N_WINDOWS _CRT_SECURE_NO_WARNINGS
}
