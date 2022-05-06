/*
	Файл	: mbstowcsl.c

	Описание: Версия mbstowcs, где max задаёт размер буфера dest (в словах), а не макс. количество записанных туда символов. В случае ошибки, в dest[0] также записывается 0. Если dest == NULL или src == NULL, возвращает (size_t)-1

	История	: 19.04.15	Создан

*/

#include "mbstowcsl.h"

#include <stdlib.h>

size_t mbstowcsl(wchar_t *dest, const char *src, size_t max)
{
	size_t ret;

	if(max == 0) return 0;
	if(src == NULL || dest == NULL) return (size_t)-1;

	ret = mbstowcs(dest, src, max-1);

	if(ret == max-1)
		dest[max-1] = 0;

	if(ret == (size_t)(-1))
		dest[0] = 0;

	return ret;
}
