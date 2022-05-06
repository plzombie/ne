/*
	Файл	: strlcpy.h

	Описание: Заголовок для strlcpy. Описание strlcpy: http://www.freebsd.org/cgi/man.cgi?query=strlcpy

	История	: 19.04.15	Создан

*/

// Ставим здесь список компиляторов и платформ, в которых нет данной функции
#if (defined(_MSC_VER) && !defined(__POCC__)) || defined(__linux__)

#ifdef __cplusplus
extern "C" {
#endif

extern size_t strlcpy(char *dst, const char *src, size_t dsize);

#ifdef __cplusplus
}
#endif

#else
#include <string.h>
#endif
