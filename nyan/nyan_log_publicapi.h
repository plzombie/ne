/*
	Файл	: nyan_log_publicapi.h

	Описание: Публичные функции для работы с журналом сообщений

	История	: 18.10.15	Создан

*/

#ifndef NYAN_LOG_PUBLICAPI_H
#define NYAN_LOG_PUBLICAPI_H

#include <stdbool.h>
#include <wchar.h>

#include "nyan_decls_publicapi.h"

NYAN_FUNC(void, nlEnable, (bool enable))
NYAN_FUNC(void, nlAddTab, (int tabs))
NYAN_FUNC(void, nlPrint, (const wchar_t *format, ...))

#endif
