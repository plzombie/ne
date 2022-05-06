/*
	Файл	: nyan_mem_publicapi.h

	Описание: Публичные функции для работы с памятью

	История	: 18.10.15	Создан

*/

#ifndef NYAN_MEM_PUBLICAPI_H
#define NYAN_MEM_PUBLICAPI_H

#include <stdlib.h>

#include "nyan_decls_publicapi.h"

NYAN_FUNC(void *, nAllocMemory, (size_t size))
NYAN_FUNC(void, nFreeMemory, (void *ptr))
NYAN_FUNC(void *, nReallocMemory, (void *ptr, size_t size))

#endif
