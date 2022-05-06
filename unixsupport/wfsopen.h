/*
	Файл	: wfsopen.h

	Описание: Заголовок для wfsopen

	История	: 27.06.18	Создан

*/

#ifndef WFSOPEN_H
#define WFSOPEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wfopen.h"

#include "share.h"

extern FILE *_wfsopen(const wchar_t *filename, const wchar_t *mode, int shflag);

#ifdef __cplusplus
}
#endif

#endif
