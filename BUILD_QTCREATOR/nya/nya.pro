
CONFIG -= qt
TARGET = nya
TEMPLATE = lib

unix {
    CONFIG += staticlib
    DEFINES += N_STATIC N_POSIX N_WCHAR32
    SOURCES += \
	../../unixsupport/filelength.c  \
	../../unixsupport/waccess.c \
	../../unixsupport/wchmod.c \
	../../unixsupport/wfopen.c \
	../../unixsupport/wfsopen.c \
	../../unixsupport/wopen.c \
	../../unixsupport/wsopen.c \
	../../unixsupport/wremove.c \
	../../unixsupport/wmkdir.c \
	../../unixsupport/wrmdir.c \
        ../../nyan/sys/unix/nyan_sys_getproc.c
    # Don't want it to be viewed from dll's (even after I fixed name collisions)
    QMAKE_CFLAGS += -fvisibility=hidden -pthread
}
windows {
    CONFIG += dll
    DEFINES += N_EXPORT N_WINDOWS _CRT_SECURE_NO_WARNINGS __STDC_NO_THREADS__
    SOURCES += ../../nyan/sys/win/nyan_sys_getproc.c
    LIBS += -lole32 -luuid
}

CONFIG(debug, debug|release) {
    DESTDIR = ../output/debug
    LIBS += -L../output/debug
} else {
    DESTDIR = ../output/release
    LIBS += -L../output/release
}

SOURCES += \
    ../../nyan/nyan.c \
    ../../nyan/nyan_apifordlls.c \
    ../../nyan/nyan_au_init.c \
    ../../nyan/nyan_au_main.c \
    ../../nyan/nyan_filesys.c \
    ../../nyan/nyan_filesys_dirpaths.c \
    ../../nyan/nyan_fps.c \
    ../../nyan/nyan_getproc.c \
    ../../nyan/nyan_init.c \
    ../../nyan/nyan_log.c \
    ../../nyan/nyan_mem.c \
    ../../nyan/nyan_plgloader.c \
    ../../nyan/nyan_threads.c \
    ../../nyan/nyan_vis_3dlevel.c \
    ../../nyan/nyan_vis_3dmodel.c \
    ../../nyan/nyan_vis_draw.c \
    ../../nyan/nyan_vis_fonts.c \
    ../../nyan/nyan_vis_init.c \
    ../../nyan/nyan_vis_justdraw.c \
    ../../nyan/nyan_vis_matrixes.c \
    ../../nyan/nyan_vis_spritesheets.c \
    ../../nyan/nyan_vis_texture.c \
    ../../plugins/plg_bmp.c \
    ../../plugins/plg_files.c \
    ../../plugins/plg_nek0.c \
    ../../plugins/plg_pcx.c \
    ../../plugins/plg_tga.c \
    ../../plugins/plg_wav.c \
    ../../extclib/mbstowcsl.c \
    ../../extclib/strlcpy.c \
    ../../extclib/wcstombsl.c \
    ../../nyan_container/nyan_container.c \
    ../../nyan_container/nyan_container_strings.c \
    ../../nyan_container/nyan_container_ne_helpers.c

HEADERS += \
    ../../nyan/nyan_apifordlls.h \
    ../../nyan/nyan_au_init.h \
    ../../nyan/nyan_au_main.h \
    ../../nyan/nyan_audiofile_publicapi.h \
    ../../nyan/nyan_decls_publicapi.h \
    ../../nyan/nyan_draw_publicapi.h \
    ../../nyan/nyan_engapi.h \
    ../../nyan/nyan_file_publicapi.h \
    ../../nyan/nyan_filesys.h \
    ../../nyan/nyan_filesys_dirpaths.h \
    ../../nyan/nyan_filesys_dirpaths_publicapi.h \
    ../../nyan/nyan_filesys_publicapi.h \
    ../../nyan/nyan_fps.h \
    ../../nyan/nyan_fps_publicapi.h \
    ../../nyan/nyan_getproc.h \
    ../../nyan/nyan_init.h \
    ../../nyan/nyan_keycodes_publicapi.h \
    ../../nyan/nyan_log.h \
    ../../nyan/nyan_log_publicapi.h \
    ../../nyan/nyan_mem.h \
    ../../nyan/nyan_mem_publicapi.h \
    ../../nyan/nyan_nalapi.h \
    ../../nyan/nyan_nglapi.h \
    ../../nyan/nyan_plgloader.h \
    ../../nyan/nyan_plgtypes_publicapi.h \
    ../../nyan/nyan_publicapi.h \
    ../../nyan/nyan_texformat_publicapi.h \
    ../../nyan/nyan_text.h \
    ../../nyan/nyan_threads.h \
    ../../nyan/nyan_threads_publicapi.h \
    ../../nyan/nyan_vis_3dlevel.h \
    ../../nyan/nyan_vis_3dmodel.h \
    ../../nyan/nyan_vis_draw.h \
    ../../nyan/nyan_vis_fonts.h \
    ../../nyan/nyan_vis_fonts_publicapi.h \
    ../../nyan/nyan_vis_init.h \
    ../../nyan/nyan_vis_init_publicapi.h \
    ../../nyan/nyan_vis_justdraw.h \
    ../../nyan/nyan_vis_justdraw_publicapi.h \
    ../../nyan/nyan_vis_matrixes.h \
    ../../nyan/nyan_vis_matrixes_publicapi.h \
    ../../nyan/nyan_vis_spritesheets.h \
    ../../nyan/nyan_vis_spritesheets_publicapi.h \
    ../../nyan/nyan_vis_texture.h \
    ../../nyan/nyan_vismodes_publicapi.h \
    ../../extclib/mbstowcsl.h \
    ../../extclib/strlcpy.h \
    ../../extclib/wcstombsl.h \
    ../../nyan_container/nyan_container.h \
    ../../nyan_container/nyan_container_ne_helpers.h
