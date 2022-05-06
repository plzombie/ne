/*
	Файл	: nyan_vis_spritesheets_publicapi.h

	Описание: Публичные функции для работы с sprite sheets

	История	: 07.11.16	Создан

*/

#ifndef NYAN_VIS_SPRITESHEETS_PUBLICAPI_H
#define NYAN_VIS_SPRITESHEETS_PUBLICAPI_H

#include <wchar.h>
#include <stdbool.h>

#include "nyan_decls_publicapi.h"

NYAN_FUNC(unsigned int, nvCreateSpriteSheetFromTileset, (unsigned int width, unsigned int height, unsigned int cols, unsigned int rows, const wchar_t *basicname))
NYAN_FUNC(unsigned int, nvCreateSpriteSheetFromFile, (const wchar_t *fname))
NYAN_FUNC(unsigned int, nvCreateSpriteSheetFromPreallocMemory, (unsigned int nofsprites, unsigned int spritetype, void *sprites, wchar_t **spritenames))
NYAN_FUNC(const wchar_t *, nvGetSpriteNameById, (unsigned int ssheetid, unsigned int sid))
NYAN_FUNC(unsigned int, nvGetSpriteIdByName, (unsigned int ssheetid, const wchar_t *name))
NYAN_FUNC(bool, nvDrawSprite, (unsigned int ssheetid, unsigned int sid, int x, int y, unsigned int batch_currenttex, float scalex, float scaley, unsigned int color))
NYAN_FUNC(unsigned int, nvGetSpriteSheetSize, (unsigned int ssheetid))
NYAN_FUNC(bool, nvDestroySpriteSheet, (unsigned int ssheetid))
NYAN_FUNC(void, nvDestroyAllSpriteSheets, (void))
NYAN_FUNC(bool, nvGetSpriteWH, (unsigned int ssheetid, unsigned int sid, unsigned int *wid, unsigned int *hei))

#endif
