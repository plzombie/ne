/*
	Файл	: nyan_vis_fonts_publicapi.h

	Описание: Публичные функции для вывода текста, работы со шрифтами

	История	: 16.01.16	Создан

*/

#ifndef NYAN_VIS_FONTS_PUBLICAPI_H
#define NYAN_VIS_FONTS_PUBLICAPI_H

#include <wchar.h>
#include <stdbool.h>

#include "nyan_decls_publicapi.h"

NYAN_FUNC(bool, nvCreateFont, (const wchar_t *fname))
NYAN_FUNC(bool, nvDestroyFont, (unsigned int id))
NYAN_FUNC(void, nvDestroyAllFonts, (void))
NYAN_FUNC(bool, nvSetFontDefaultSym, (unsigned int id, unsigned int sym))
NYAN_FUNC(bool, nvSetFontEmptySymFromDefault, (unsigned int id));
NYAN_FUNC(bool, nvDraw2dText, (const wchar_t *text, int posx, int posy, unsigned int fontid, unsigned int texid, float scalex, float scaley, unsigned int color))
NYAN_FUNC(bool, nvDraw2dTextbox, (wchar_t *textbuf, int *status, unsigned int maxsize, wchar_t csym, unsigned int catime, int posx, int posy, unsigned int fontid, unsigned int texid, float scalex, float scaley, unsigned int color, unsigned int color_focused))


#endif
