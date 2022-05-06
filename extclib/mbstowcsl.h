/*
	Файл	: mbstowcsl.h

	Описание: Заголовок для mbstowcsl

	История	: 19.04.15	Создан

*/

#ifndef MBSTOWCSL_H
#define MBSTOWCSL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

extern size_t mbstowcsl(wchar_t *dest, const char *src, size_t max);

#ifdef __cplusplus
}
#endif

#endif
