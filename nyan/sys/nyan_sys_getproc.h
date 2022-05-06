/*
	Файл	: nyan_sys_getproc.h

	Описание: Заголовок для nyan_sys_getproc.c

	История	: 02.01.18	Создан

*/

#include "../nyan_getproc_func.h"

extern void *nSysLoadLib(const wchar_t *filename);
extern n_getproc_func_type nSysGetProcAddress(void *handle, const char *fname);
extern void nSysFreeLib(void *handle);
