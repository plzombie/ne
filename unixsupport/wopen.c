/*
	Файл	: wopen.c

	Описание: Реализация _wopen поверх open

	История	: 04.04.14	Создан

*/

#include "../extclib/wcstombsl.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

int _wopen(const wchar_t *filename, int oflag, int pmode)
{
	char *cfilename;
	int result;
	size_t strsize;

	strsize = wcstombs(NULL, filename, 0)+1;

	cfilename = malloc(strsize);
	if(!cfilename) return -1;

	wcstombsl(cfilename, filename, strsize);

	result = open(cfilename, oflag, pmode);
	
	while(result == -1) {
		if(errno != EINTR)
			break;

		result = open(cfilename, oflag, pmode);
	}

	free(cfilename);

	return result;
}
