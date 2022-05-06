/*
	Файл	: wopen.h

	Описание: Заголовок для wopen

	История	: 18.03.17	Создан

*/

#ifndef WOPEN_H
#define WOPEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "permissions.h"

extern int _wopen(const wchar_t *filename, int oflag, int pmode);

#ifndef _O_WRONLY
	#define _O_WRONLY O_WRONLY
#endif
#ifndef _O_CREAT
	#define _O_CREAT O_CREAT
#endif
#ifndef _O_TRUNC
	#define _O_TRUNC O_TRUNC
#endif
#ifndef _O_RDWR
	#define _O_RDWR O_RDWR
#endif
#ifndef _O_BINARY
	#define _O_BINARY 0
#endif

#ifdef __cplusplus
}
#endif

#endif
