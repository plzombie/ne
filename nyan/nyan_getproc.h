/*
	Файл	: nyan_getproc.h

	Описание: Заголовок для nyan_getproc.c

	История	: 04.08.12	Создан

*/

#include "nyan_getproc_func.h"

extern void *nLoadModule(const wchar_t *modname);
extern void *nLoadLib(const wchar_t *filename);
extern n_getproc_func_type nGetProcAddress(void *handle, const char *fname, const char *altfname);
extern void nFreeLib(void *handle);
