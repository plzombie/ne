/*
	Файл	: wfsopen.c

	Описание: Реализация _wfsopen поверх _wsopen и fdopen. Обрабатывает _SH_DENYWR и _SH_DENYRD как _SH_DENYRW

	История	: 27.06.18	Создан

*/

#include "wfsopen.h"
#include "wsopen.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>

FILE *_wfsopen(const wchar_t *filename, const wchar_t *mode, int shflag)
{
	int iret, oflag = 0, pmode = 0;
	char cmode[4] = {0};
	FILE *ret;
	
	if(wcschr(mode, 'w')) {
		pmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
		oflag = O_CREAT;
	}
	
	if(wcschr(mode, '+') || wcschr(mode, 'a'))
		oflag |= O_RDWR;
	else if(wcschr(mode, 'r'))
		oflag |= O_RDONLY;
	else if(wcschr(mode, 'w'))
		oflag |= O_WRONLY;
		
	iret = _wsopen(filename, oflag, shflag, pmode);
	
	if(iret == -1)
		return 0; // сохраняется errno из _wsopen
		
	if(wcschr(mode, 'r'))
		strcpy(cmode, "r");
	else if(wcschr(mode, 'w'))
		strcpy(cmode, "w");
	else if(wcschr(mode, 'a'))
		strcpy(cmode, "a");
	
	if(wcschr(mode, '+'))
		strcpy(cmode, "+");
	
	if(wcschr(mode, 'b'))
		strcpy(cmode, "b");
	
	do {
		ret = fdopen(iret, cmode);
		
		if(!ret)
			if(errno != EINTR)
				break;
	} while(!ret);
	
	if(!ret) {
		int old_errno;
		
		old_errno = errno;
		close(iret);
		errno = old_errno;
		
		return 0; // сохраняется errno из fdopen
	}
	
	return ret;
}
