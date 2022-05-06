/*
	Файл	: nyan_vis_fonts_publicapi.h

	Описание: Публичные функции для инициализации рендера

	История	: 16.01.16	Создан

*/

#ifndef NYAN_VIS_INIT_PUBLICAPI_H
#define NYAN_VIS_INIT_PUBLICAPI_H

#include <stdbool.h>
#include <wchar.h>

#include "nyan_decls_publicapi.h"

// Прикрепление рендера
NYAN_FUNC(bool, nvAttachRender, (const wchar_t *dllname))

// Очистка экрана
NYAN_FUNC(void, nvClear, (unsigned int mask))

// Снятие скриншота
NYAN_FUNC(void, nvMakeScreenshot, (const wchar_t *filename, unsigned int relpath))

// Получение информации о графической подсистеме
NYAN_FUNC(int, nvGetStatusi, (int status))
NYAN_FUNC(void, nvSetStatusi, (int status, int param))
NYAN_FUNC(float, nvGetStatusf, (int status))
NYAN_FUNC(void, nvSetStatusf, (int status, float param))
NYAN_FUNC(const wchar_t *, nvGetStatusw, (int status))
NYAN_FUNC(void, nvSetStatusw, (int status, const wchar_t *param))
NYAN_FUNC(char, nvGetKey, (unsigned char keyid))
NYAN_FUNC(void, nvSetScreen, (int winx, int winy, int winbpp, int winmode, int vsync))
NYAN_FUNC(void, nvSetClippingRegion, (int sx, int sy, int ex, int ey))

#endif
