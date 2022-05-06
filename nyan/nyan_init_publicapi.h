/*
	Файл	: nyan_init_publicapi.h

	Описание: Публичные функции для инициализации движка

	История	: 04.07.17	Создан

*/

#ifndef NYAN_INIT_PUBLICAPI_H
#define NYAN_INIT_PUBLICAPI_H

#include <stdbool.h>

#include "nyan_decls_publicapi.h"

NYAN_FUNC(bool, nIsInit, (void))
NYAN_FUNC(bool, nInit, (void))
NYAN_FUNC(bool, nClose, (void))
NYAN_FUNC(void, nUpdate, (void))

#endif
