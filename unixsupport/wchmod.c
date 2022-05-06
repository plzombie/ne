/*
	Файл	: wchmod.c

	Описание: Реализация _wchmod поверх chmod

	История	: 12.06.18	Создан

*/

#include "../extclib/wcstombsl.h"

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <sys/stat.h>

int _wchmod(const wchar_t *path, int permission)
{
	char *cpath;
	int result;
	size_t strsize;

	strsize = wcstombs(NULL, path, 0)+1;

	cpath = malloc(strsize);
	if(!cpath) return -1;

	wcstombsl(cpath, path, strsize);

	result = chmod(cpath, permission);

	free(cpath);

	return(result);
}
