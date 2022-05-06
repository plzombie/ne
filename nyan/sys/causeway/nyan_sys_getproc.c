/*
	Файл	: nyan_sys_getproc.c

	Описание: Работа с dll. Обёртки над системными функциями.

	История	: 02.01.18	Создан

*/

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <cwdll.h>

#include "../nyan_sys_getproc.h"

/*
	Функция	: nSysLoadLib

	Описание: Загружает библиотеку

	История	: 02.01.18	Создан

*/
void *nSysLoadLib(const wchar_t *filename)
{
	void *result = 0;
	char *cfilename;
	int csize;

	if(!filename) return 0;

	csize = wcstombs(NULL, filename, 0)+1;
	cfilename = malloc(csize*sizeof(char));
	if(!cfilename) return 0;
	wcstombs(cfilename, filename, csize);

	result = LoadLibrary(cfilename);

	free(cfilename);

	return result;
}

/*
	Функция	: nGetProcAddress

	Описание: Получает указатель на функцию из библиотеки

	История	: 12.08.12	Создан

*/
n_getproc_func_type nSysGetProcAddress(void *handle, const char *fname)
{
	return (n_getproc_func_type)GetProcAddress(handle, fname);
}

/*
	Функция	: nFreeLib

	Описание: Закрывает библиотеку

	История	: 12.08.12	Создан

*/
void nSysFreeLib(void *handle)
{
	FreeLibrary(handle);
}

