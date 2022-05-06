/*
	Файл	: nyan_plgloader_publicapi.h

	Описание: Публичные функции для загрузки плагинов

	История	: 05.07.17	Создан

*/

#ifndef NYAN_PLGLOADER_PUBLICAPI_H
#define NYAN_PLGLOADER_PUBLICAPI_H

#include <stdbool.h>
#include <wchar.h>

#include "nyan_decls_publicapi.h"

NYAN_FUNC(bool, nAddPlugin, (const wchar_t *dllname))
NYAN_FUNC(void, nDeleteAllPlugins, (void))

#endif
