/*
	Файл	: nyan_vis_texture_publicapi.h

	Описание: Публичные функции для работы с текстурами

	История	: 05.07.17	Создан

*/

#ifndef NYAN_VIS_TEXTURE_PUBLICAPI_H
#define NYAN_VIS_TEXTURE_PUBLICAPI_H

#include <stdbool.h>
#include <wchar.h>

#include "nyan_decls_publicapi.h"

#include "nyan_texformat_publicapi.h"

// Статус текстуры
#define NV_TEX_STATUS_FREE 0 // id текстуры не существует или не используется
#define NV_TEX_STATUS_EMPTY 1 // Текстура не загружена
#define NV_TEX_STATUS_LOADING 2 // Текстура загружается (!!!в данный момент не используется!!!)
#define NV_TEX_STATUS_LOADED 3 // Текстура загружена
#define NV_TEX_STATUS_UNLOADING 4 // Текстура выгружается

// Способ загрузки текстуры
#define NV_TEX_LTYPE_LOADVOID 0 // Способ загрузки текстуры не определён. Текстура не назначена.
#define NV_TEX_LTYPE_LOADFNAME 1 // Текстура загружается из файла
#define NV_TEX_LTYPE_LOADFNAMEWITHALPHAFROMSUBFNAME 2 // Текстура загружается из файла+отдельный файл для альфа-канала (!!!в данный момент не используется!!!)
#define NV_TEX_LTYPE_LOADFNAMEASRAWDATA 3 // Текстура загружается из файла, сожержащего сырые данные (!!!в данный момент не используется!!!). Параметры текстуры содержатся в структуре nv_texture_type, переданной в качестве параметра
#define NV_TEX_LTYPE_LOADMEMORY 10 // Текстура загружается из структуры nv_texture_type, переданной в качестве параметра

NYAN_FUNC(unsigned int, nvCreateTexture, (int flags))
NYAN_FUNC(unsigned int, nvCreateTextureFromFile, (const wchar_t *fname, int flags))
NYAN_FUNC(unsigned int, nvCreateTextureFromMemory, (nv_texture_type *tex, int flags))
NYAN_FUNC(bool, nvSetTextureLoadTypeVoid, (unsigned int id))
NYAN_FUNC(bool, nvSetTextureLoadTypeFromFile, (unsigned int id, const wchar_t *fname))
NYAN_FUNC(bool, nvSetTextureLoadTypeFromMemory, (unsigned int id, nv_texture_type *tex))
NYAN_FUNC(bool, nvLoadImageToMemory, (const wchar_t *fname, nv_texture_type *tex))
NYAN_FUNC(bool, nvUpdateTextureFromMemory, (unsigned int id, unsigned char *buffer))
NYAN_FUNC(bool, nvLoadTexture, (unsigned int id))
NYAN_FUNC(bool, nvUnloadTexture, (unsigned int id))
NYAN_FUNC(bool, nvDestroyTexture, (unsigned int id))
NYAN_FUNC(void, nvDestroyAllTextures, (void))
NYAN_FUNC(unsigned int, nvGetTextureStatus, (unsigned int id))
NYAN_FUNC(unsigned int, nvGetTextureLType, (unsigned int id))
NYAN_FUNC(bool, nvGetTextureWH, (unsigned int id, unsigned int *wid, unsigned int *hei))
NYAN_FUNC(bool, nvGetTextureFormat, (unsigned int id, unsigned int *colorformat, unsigned int *rowalignment))

NYAN_FUNC(bool, nvAddTexturePlugin, (const wchar_t *name, nv_tex_plugin_type *nv_texplg))
NYAN_FUNC(void, nvDeleteAllTexturePlugins, (void))

#endif
