/*
	Файл	: nyan_vis_fonts_publicapi.h

	Описание: Публичные функции для вывода текста, работы со шрифтами

	История	: 16.01.16	Создан

*/

#ifndef NYAN_VIS_JUSTDRAW_PUBLICAPI_H
#define NYAN_VIS_JUSTDRAW_PUBLICAPI_H

#include <wchar.h>
#include <stdbool.h>

#include "nyan_decls_publicapi.h"

NYAN_FUNC(void, nvJustDraw2dTriangle, (float x1, float y1, float x2, float y2, float x3, float y3, unsigned int color1, unsigned int color2, unsigned int color3))
NYAN_FUNC(void, nvJustDraw2dLine, (float x1, float y1, float x2, float y2, unsigned int color1, unsigned int color2))
NYAN_FUNC(void, nvJustDraw2dPoint, (float x, float y, unsigned int color))

#endif
