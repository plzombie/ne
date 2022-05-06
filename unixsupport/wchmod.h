/*
	Файл	: wchmod.h

	Описание: Заголовок для wchmod

	История	: 12.06.18	Создан

*/

#ifndef WCHMOD_H
#define WCHMOD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "permissions.h"

extern int _wchmod(const wchar_t *path, int permission);

#ifdef __cplusplus
}
#endif

#endif
