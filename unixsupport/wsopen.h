/*
	Файл	: wsopen.h

	Описание: Заголовок для wsopen

	История	: 27.06.18	Создан

*/

#ifndef WSOPEN_H
#define WSOPEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wopen.h"

#include "share.h"

extern int _wsopen(const wchar_t *filename, int oflag, int shflag, int pmode);

#ifdef __cplusplus
}
#endif

#endif
