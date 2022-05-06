/*
Файл	: strlcpy.c

Описание: Реализация strlcpy поверх strncpy_s или strncpy. Описание strlcpy: http://www.freebsd.org/cgi/man.cgi?query=strlcpy

История	: 19.04.15	Создан

*/

#include <string.h>

#if defined(__linux__)

size_t strlcpy(char *dst, const char *src, size_t dsize)
{
	strncpy(dst, src, dsize);

	if(dsize)
		dst[dsize-1] = 0;

	return strlen(src);
}

#elif (defined(_MSC_VER) && !defined(__POCC__))

size_t strlcpy(char *dst, const char *src, size_t dsize)
{
	strncpy_s(dst, dsize, src, _TRUNCATE);

	return strlen(src);
}

#endif


