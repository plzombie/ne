/*
	Файл	: nyan_file_publicapi.h

	Описание: Параметры для файла

	История	: 28.05.13	Создан

*/

#ifndef NYAN_FILE_PUBLICAPI_H
#define NYAN_FILE_PUBLICAPI_H

#include <stdbool.h>
#include <wchar.h>

#ifndef N_APIENTRY
#include "nyan_decls_publicapi.h"
#endif

#define FILE_SEEK_SET  0
#define FILE_SEEK_CUR  1
#define FILE_SEEK_END  2

typedef struct {
	long long int offset; // Оффсет от начала файла-родителя (например файл находится внутри "duke.grp". offset - смещение от начала "duke.grp".)
	long long int foffset; // Оффсет от начала файла
	long long int fsize; // Размер файла
	void *fdata; // Данные, необходимые плагину для работы с файлом
} fs_fileinfo_type;

typedef bool (N_APIENTRY *fs_plg_init_type)(void);
typedef void (N_APIENTRY *fs_plg_destroy_type)(void);
typedef long long int (N_APIENTRY *fs_plg_fileread_type)(fs_fileinfo_type *file, void *dst, size_t num);
typedef long long int (N_APIENTRY *fs_plg_filewrite_type)(fs_fileinfo_type *file, void *src, size_t num);
typedef bool (N_APIENTRY *fs_plg_filecanwrite_type)(fs_fileinfo_type *file);
typedef long long int (N_APIENTRY *fs_plg_fileseek_type)(fs_fileinfo_type *file, long long int offset, int origin);
typedef bool (N_APIENTRY *fs_plg_fileexisted_type)(const wchar_t *fname);
typedef bool (N_APIENTRY *fs_plg_fileopen_type)(const wchar_t *fname, fs_fileinfo_type *file);
typedef bool (N_APIENTRY *fs_plg_fileclose_type)(fs_fileinfo_type *file);
typedef bool (N_APIENTRY *fs_plg_mountarchive_type)(const wchar_t *arcname);
typedef struct {
	size_t size; // Размер структуры fs_fileplg_type
	fs_plg_init_type plgInit;
	fs_plg_destroy_type plgDestroy;
	fs_plg_fileread_type plgFileRead;
	fs_plg_filewrite_type plgFileWrite;
	fs_plg_filecanwrite_type plgFileCanWrite;
	fs_plg_fileseek_type plgFileSeek;
	fs_plg_fileexisted_type plgFileExisted;
	fs_plg_fileopen_type plgFileOpen;
	fs_plg_fileclose_type plgFileClose;
	fs_plg_mountarchive_type plgMountArchive;
} fs_fileplg_type;

#endif
