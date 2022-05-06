/*
	Файл	: wsopen.c

	Описание: Реализация _wsopen поверх _wopen и flock. Обрабатывает _SH_DENYWR и _SH_DENYRD как _SH_DENYRW

	История	: 27.06.18	Создан

*/

#include "wsopen.h"

#include <errno.h>

#include <sys/file.h>
#include <unistd.h>

int _wsopen(const wchar_t *filename, int oflag, int shflag, int pmode)
{
	int ret, oflag_temp;
	
	switch(shflag) {
		case _SH_COMPAT:
			if(oflag & _O_WRONLY || oflag & _O_RDWR)
				shflag = _SH_DENYWR;
			else
				shflag = _SH_DENYNO;
			break;
		case _SH_DENYRW:
		case _SH_DENYWR:
		case _SH_DENYRD:
		case _SH_DENYNO:
			break;
		default:
			shflag = _SH_DENYNO;
	}
	
	if(oflag & O_TRUNC)
		oflag_temp = oflag ^ O_TRUNC;
	else
		oflag_temp = oflag;
	
	ret = _wopen(filename, oflag_temp, pmode);
	
	if((ret != -1) && (shflag != _SH_DENYNO)) {
		while(flock(ret, LOCK_EX | LOCK_NB)) {
			if(errno != EINTR) {
				close(ret);
				errno = EACCES;
				return -1;
			}
		}
		
		if(oflag & O_TRUNC) {
			while(ftruncate(ret, 0)) {
				if(errno != EINTR) {
					close(ret);
					errno = EACCES;
					return -1;
				}
			}
		}
	}
	
	return ret;
}
