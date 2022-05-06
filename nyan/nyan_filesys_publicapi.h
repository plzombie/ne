/*
	Файл	: nyan_filesys_publicapi.h

	Описание: Публичные функции для работы с файлами

	История	: 18.10.15	Создан

*/

#ifndef NYAN_FILESYS_PUBLICAPI_H
#define NYAN_FILESYS_PUBLICAPI_H

#include <stdbool.h>
#include <stdlib.h>
#include <wchar.h>

#include "nyan_decls_publicapi.h"

#include "nyan_file_publicapi.h"

NYAN_FUNC(bool, nFileCreateDir, (const wchar_t *dir, unsigned int relpath))
NYAN_FUNC(bool, nFileDeleteDir, (const wchar_t *dir, unsigned int relpath))
NYAN_FUNC(bool, nFileCreate, (const wchar_t *filename, bool truncate, unsigned int relpath))
NYAN_FUNC(bool, nFileDelete, (const wchar_t *filename, unsigned int relpath))
NYAN_FUNC(long long int, nFileRead, (unsigned int id, void *dst, size_t num))
NYAN_FUNC(long long int, nFileWrite, (unsigned int id, void *src, size_t num))
NYAN_FUNC(bool, nFileCanWrite, (unsigned int id))
NYAN_FUNC(long long int, nFileLength, (unsigned int id))
NYAN_FUNC(long long int, nFileTell, (unsigned int id))
NYAN_FUNC(long long int, nFileSeek, (unsigned int id, long long int offset, int origin))
NYAN_FUNC(unsigned int, nFileOpen, (const wchar_t *fname))
NYAN_FUNC(bool, nFileClose, (unsigned int id))
NYAN_FUNC(bool, nCloseAllFiles, (void))
NYAN_FUNC(bool, nMountDir, (const wchar_t *dirname))
NYAN_FUNC(bool, nMountArchive, (const wchar_t *arcname))

NYAN_FUNC(bool, nAddFilePlugin, (const wchar_t *name, fs_fileplg_type *fs_fileplg))
NYAN_FUNC(void, nDeleteAllFilePlugins, (void))

#endif
