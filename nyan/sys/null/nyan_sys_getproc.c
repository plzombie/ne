/*
	Файл	: nyan_sys_getproc.c

	Описание: Работа с dll. Обёртки над системными функциями.

	История	: 02.01.18	Создан

*/

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "../nyan_sys_getproc.h"

/*
	Функция	: nSysLoadLib

	Описание: Загружает библиотеку

	История	: 02.01.18	Создан

*/
void *nSysLoadLib(const wchar_t *filename)
{
	void *result = 0;
	
	(void)filename; // Unused

	return result;
}

/*
	Функция	: nGetProcAddress

	Описание: Получает указатель на функцию из библиотеки

	История	: 12.08.12	Создан

*/
n_getproc_func_type nSysGetProcAddress(void *handle, const char *fname)
{
	n_getproc_func_type result;
	
	(void)handle; // Unused
	(void)fname; // Unused

	result = 0;

	return result;
}

/*
	Функция	: nFreeLib

	Описание: Закрывает библиотеку

	История	: 12.08.12	Создан

*/
void nSysFreeLib(void *handle)
{
	(void)handle; // Unused
	
	return;
}

