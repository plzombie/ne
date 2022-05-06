/*
	Файл	: nyan_filesys_dirpaths_publicapi.h

	Описание: Публичные функции для работы с файлами. Получение пути папок и файлов.

	История	: 12.04.17	Создан

*/

#ifndef NYAN_FILESYS_DIRPATHS_PUBLICAPI_H
#define NYAN_FILESYS_DIRPATHS_PUBLICAPI_H

#include <stdbool.h>
#include <wchar.h>

#include "nyan_decls_publicapi.h"

#define NF_PATH_CURRENTDIR 0
#define NF_PATH_USERDIR 1
#define NF_PATH_DATA_ROAMING 2
#define NF_PATH_DATA_LOCAL 3
#define NF_PATH_DOCUMENTS 4
#define NF_PATH_GAMESAVES 5
#define NF_PATH_PICTURES 6
#define NF_PATH_MUSIC 7
#define NF_PATH_VIDEOS 8
#define NF_PATH_DOWNLOADS 9

NYAN_FUNC(const wchar_t *, nFileGetDir, (unsigned int relpath))
NYAN_FUNC(wchar_t *, nFileAppendFilenameToDir, (const wchar_t *filename, const wchar_t *dir))
NYAN_FUNC(wchar_t *, nFileGetAbsoluteFilename, (const wchar_t *filename, unsigned int relpath))

#endif
