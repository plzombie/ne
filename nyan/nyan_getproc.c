/*
	Файл	: nyan_getproc.c

	Описание: Работа с dll

	История	: 12.08.12	Создан

*/

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#if defined(N_WINDOWS)
	#ifndef UNICODE
		#define UNICODE
	#endif
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#elif defined(N_CAUSEWAY)
	#include <cwdll.h>
#elif defined(N_POSIX)
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <dlfcn.h>
	//#include "nyan_log_publicapi.h"
#endif

#include "sys/nyan_sys_getproc.h"
#include "nyan_getproc.h"

#if defined(N_WINDOWS) || defined(N_CAUSEWAY)
	#define N_MODULE_EXT L".dll"
#elif defined(N_POSIX)
	#define N_MODULE_EXT L".so"
#else
	#define N_MODULE_EXT L""
#endif

/*
	Функция	: nLoadModule

	Описание: Загружает библиотеку. modname обозначает имя файла модуля, без расширения.

	История	: 10.04.17	Создан

*/
void *nLoadModule(const wchar_t *modname)
{
	size_t extlen;
	wchar_t *substr;
	void *result;
	wchar_t *dllname;

	if(!modname) return 0;

	extlen = wcslen(N_MODULE_EXT);

	if(extlen == 0) return nLoadLib(modname);

	substr = wcsrchr(modname, N_MODULE_EXT[0]);

	if(substr) {
		if(wcscmp(substr, N_MODULE_EXT) == 0)
			return nLoadLib(modname);
	}

	dllname = malloc((wcslen(modname)+extlen+1)*sizeof(wchar_t));
	if(!dllname) return 0;

	wcscpy(dllname, modname);
	wcscat(dllname, N_MODULE_EXT);

	result = nLoadLib(dllname);

	free(dllname);

	return result;
}

/*
	Функция	: nLoadLib

	Описание: Загружает библиотеку

	История	: 12.08.12	Создан

*/
void *nLoadLib(const wchar_t *filename)
{
	return nSysLoadLib(filename);
}

/*
	Функция	: nGetProcAddress

	Описание: Получает указатель на функцию из библиотеки

	История	: 12.08.12	Создан

*/
n_getproc_func_type nGetProcAddress(void *handle, const char *fname, const char *altfname)
{
	n_getproc_func_type result;

	result = nSysGetProcAddress(handle, fname);

	if(!result)
		result = nSysGetProcAddress(handle, altfname);

	return result;
}

/*
	Функция	: nFreeLib

	Описание: Закрывает библиотеку

	История	: 12.08.12	Создан

*/
void nFreeLib(void *handle)
{
	nSysFreeLib(handle);
}

