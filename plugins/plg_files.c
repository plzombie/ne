/*
	Файл	: plg_files.c

	Описание: Плагин для чтения файлов

	История	: 03.01.13	Создан

*/

#ifdef N_POSIX
	#define _LARGEFILE64_SOURCE
#endif

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <errno.h>

#ifdef N_POSIX
	#include <unistd.h>
	#include "../unixsupport/wsopen.h"
	#include "../unixsupport/filelength.h"
	#include "../unixsupport/waccess.h"
	#define _close close
	#define _read read
	#define _write write
#else
	#include <share.h>
	#include <io.h>
#endif

#ifndef O_LARGEFILE
	#define O_LARGEFILE 0
#endif

#include "../nyan/nyan_text.h"
#include "../nyan/nyan_file_publicapi.h"

#include "plg_files.h"

// На будующее - __MINGW__ , __CYGWIN__
// _lseek64, _tell64, _filelength64 - названия ф-й, используемые в Pelles C
// в WatcomC и VisualC++ используются _lseeki64, _telli64, _filelengthi64
// Borland C использует _lseeki64 и _telli64, но не имеет long long версии filelength(во всяком случае, я не нашёл её)
// Для GCC использую lseek64
// Для остальных использую названия из POSIX
#if defined(__GNUC__) && !defined(__FreeBSD__) && !(defined(WIN32) && defined(__clang__))
	#define _lseek64 lseek64
	#define _tell64(handle) lseek64(handle,0,SEEK_CUR);
#elif defined(__WATCOMC__) || (defined(_MSC_VER) && !defined(__POCC__)) || defined(__BORLANDC__)
	#define _lseek64 _lseeki64
	#define _tell64 _telli64
	#ifndef __BORLANDC__
		#define _filelength64 _filelengthi64
	#endif
#elif !defined(__POCC__)
	#define _lseek64 lseek
	#define _tell64(handle) lseek(handle,0,SEEK_CUR);
	#define _filelength64 filelength
#endif

#if (defined(__GNUC__) && !defined(__FreeBSD__) && !(defined(WIN32) && defined(__clang__))) || defined(__BORLANDC__)
static long long _filelength64(int handle)
{
	long long filesize, position;

	position = _lseek64(handle, 0, SEEK_CUR);/*_tell64(handle)*/; // Узнаём текущую позицию

	filesize = _lseek64(handle, 0, SEEK_END); // Переходим в конец файла

	_lseek64(handle, position, SEEK_SET); // Устанавливаем обратно позицию

	return filesize;
}
#endif

bool N_APIENTRY plgFILESInit(void);
void N_APIENTRY plgFILESDestroy(void);
long long int N_APIENTRY plgFILESFileRead(fs_fileinfo_type *file, void *dst, size_t num);
long long int N_APIENTRY plgFILESFileWrite(fs_fileinfo_type *file, void *src, size_t num);
bool N_APIENTRY plgFILESFileCanWrite(fs_fileinfo_type *file);
long long int N_APIENTRY plgFILESFileSeek(fs_fileinfo_type *file,long long int offset,int origin);
bool N_APIENTRY plgFILESFileExisted(const wchar_t *fname);
bool N_APIENTRY plgFILESFileOpen(const wchar_t *fname, fs_fileinfo_type *file);
bool N_APIENTRY plgFILESFileClose(fs_fileinfo_type *file);
bool N_APIENTRY plgFILESMountArchive(const wchar_t *arcname);

fs_fileplg_type plgFILE = {sizeof(fs_fileplg_type), &plgFILESInit, &plgFILESDestroy,
			&plgFILESFileRead, &plgFILESFileWrite, &plgFILESFileCanWrite, &plgFILESFileSeek, &plgFILESFileExisted,
			&plgFILESFileOpen, &plgFILESFileClose, &plgFILESMountArchive};

/*
	Функция	: plgFILESInit

	Описание: Инициализирует файловый плагин

	История	: 27.01.13	Создан

*/
bool N_APIENTRY plgFILESInit(void)
{
	return true;
}

/*
	Функция	: plgFILESDestroy

	Описание: Выгружает файловый плагин

	История	: 27.01.13	Создан

*/
void N_APIENTRY plgFILESDestroy(void)
{
	return;
}

#ifndef N_POSIX
	#define READ_WRITE_MAX INT_MAX
	typedef unsigned int read_write_t;
	typedef int read_write_ret_t;
#else
	#define READ_WRITE_MAX SSIZE_MAX
	typedef size_t read_write_t;
	typedef ssize_t read_write_ret_t;
#endif

/*
	Функция	: plgFILESFileRead

	Описание: Читает num байт из файла. Возвращает количество прочитанных байт;
				При чтении за eof возвращает 0; при ошибке возвращает -1

	История	: 03.01.13	Создан

*/
long long int N_APIENTRY plgFILESFileRead(fs_fileinfo_type *file, void *dst, size_t num)
{
	long long int ret;

	if(num > (unsigned long long)(file->fsize-file->foffset))
		num = (size_t)(file->fsize - file->foffset);

	ret = 0;

	do {
		read_write_ret_t part_read;
		read_write_t part_num;

		if(num > READ_WRITE_MAX)
			part_num = READ_WRITE_MAX;
		else
			part_num = (read_write_t)num;

		part_read = _read((int)(intptr_t)(file->fdata), dst, part_num);

		if(part_read == -1) {
		#ifdef N_POSIX
			if(errno == EINTR)
				continue;
		#endif
		
			file->foffset += ret;

			if(ret)
				return ret;
			else
				return -1;
		} else
			ret += part_read;

		num -= part_num;
		dst = (char *)dst+part_num;
	} while(num > 0);

	file->foffset += ret;

	return ret;
}

/*
	Функция	: plgFILESFileWrite

	Описание: Пишет num байт в файл. Возвращает количество записанных байт;
				При ошибке возвращает -1

	История	: 28.05.13	Создан

*/
long long int N_APIENTRY plgFILESFileWrite(fs_fileinfo_type *file, void *src, size_t num)
{
	long long int ret;

	if(num > (unsigned long long)(LLONG_MAX-file->foffset))
		num = (size_t)(LLONG_MAX-file->foffset);

	ret = 0;

	do {
		read_write_ret_t part_write;
		read_write_t part_num;

		if(num > READ_WRITE_MAX)
			part_num = READ_WRITE_MAX;
		else
			part_num = (read_write_t)num;

		part_write = _write((int)(intptr_t)(file->fdata), src, part_num);

		if(part_write == -1) {
		#ifdef N_POSIX
			if(errno == EINTR)
				continue;
		#endif
		
			file->foffset += ret;
			if(file->fsize < file->foffset)
				file->fsize = file->foffset;

			if(ret)
				return ret;
			else
				return -1;
		} else
			ret += part_write;

		num -= part_num;
		src = (char *)src+part_num;
	} while(num > 0);

	file->foffset += ret;
	if(file->fsize < file->foffset)
		file->fsize = file->foffset;

	return ret;
}

/*
	Функция	: plgFILESFileCanWrite

	Описание: Возвращает true если файл существует, иначе false

	История	: 28.05.13	Создан

*/
bool N_APIENTRY plgFILESFileCanWrite(fs_fileinfo_type *file)
{
	(void)file; // Неиспользуемая переменная

	return true;
}

/*
	Функция	: plgFILESFileSeek

	Описание: Устанавливает позицию в файле. Возвращает текущую позицию или -1 при неудаче

	История	: 03.01.13	Создан

*/
long long int N_APIENTRY plgFILESFileSeek(fs_fileinfo_type *file,long long int offset,int origin)
{
	long long new_offset;

	new_offset = _lseek64((int)(intptr_t)(file->fdata), file->offset+offset, origin);

	if(new_offset>-1) {
		file->foffset = new_offset-file->offset;

		return file->foffset;
	} else
		return -1;
}

/*
	Функция	: plgFILESFileExisted

	Описание: Возвращает true если файл существует, иначе false

	История	: 03.01.13	Создан

*/
bool N_APIENTRY plgFILESFileExisted(const wchar_t *fname)
{
	return !_waccess(fname,0);
}

/*
	Функция	: plgFILESFileOpen

	Описание: Открывает файл. Возвращает false, если файл не был открыт

	История	: 03.01.13	Создан

*/
bool N_APIENTRY plgFILESFileOpen(const wchar_t *fname, fs_fileinfo_type *file)
{
	int fh;

	if(!_waccess(fname,0))
		fh = _wsopen(fname, _O_RDWR | _O_BINARY | O_LARGEFILE, _SH_DENYWR, 0);
	else
		return false;

	if(fh == -1)
		fh = _wsopen(fname, _O_BINARY | O_LARGEFILE, _SH_DENYNO, 0);

	if(fh != -1){
		file->offset = 0;
		file->foffset = 0;
		file->fsize = _filelength64(fh);
		file->fdata = (void *)(intptr_t)fh;
		return true;
	} else
		return false;
}

/*
	Функция	: plgFILESFileClose

	Описание: Закрывает файл. Возвращает true или false

	История	: 03.01.13	Создан

*/
bool N_APIENTRY plgFILESFileClose(fs_fileinfo_type *file)
{
	_close((int)(intptr_t)(file->fdata));

	return true;
}

/*
	Функция	: plgFILESMountArchive

	Описание: Добавляет архив

	История	: 03.01.13	Создан

*/
bool N_APIENTRY plgFILESMountArchive(const wchar_t *arcname)
{
	(void)arcname; // Неиспользуемая переменная

	return false;
}
