/*
	Файл	: dll_plg_kplib_pictures.c

	Описание: Плагин для загрузки изображений через kplib.c

	История	: 05.08.14	Создан

*/

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "../extclib/wcstombsl.h"

#if defined(N_POSIX)
	#define N_API __attribute__ ((visibility("default")))
	#define N_APIENTRY
	#define _wcsicmp wcscasecmp
#else
	#define N_API __declspec(dllexport)
	#define N_APIENTRY __cdecl
#endif

#include "../nyan/nyan_engapi.h"
#include "../nyan/nyan_file_publicapi.h"
#include "../nyan/nyan_plgtypes_publicapi.h"

#include "../forks/kplib_c/kplib_fileio.h"

#include "dll_plg_kplib_files.h"

engapi_type *ea = 0;

bool N_APIENTRY kplib_files_plgInit(void);
void N_APIENTRY kplib_files_plgDestroy(void);
long long int N_APIENTRY kplib_files_plgFileRead(fs_fileinfo_type *file, void *dst, size_t num);
long long int N_APIENTRY kplib_files_plgFileWrite(fs_fileinfo_type *file, void *src, size_t num);
bool N_APIENTRY kplib_files_plgFileCanWrite(fs_fileinfo_type *file);
long long int N_APIENTRY kplib_files_plgFileSeek(fs_fileinfo_type *file,long long int offset,int origin);
bool N_APIENTRY kplib_files_plgFileExisted(const wchar_t *fname);
bool N_APIENTRY kplib_files_plgFileOpen(const wchar_t *fname, fs_fileinfo_type *file);
bool N_APIENTRY kplib_files_plgFileClose(fs_fileinfo_type *file);
bool N_APIENTRY kplib_files_plgMountArchive(const wchar_t *arcname);

fs_fileplg_type plg = {sizeof(fs_fileplg_type), &kplib_files_plgInit, &kplib_files_plgDestroy,
			&kplib_files_plgFileRead, &kplib_files_plgFileWrite, &kplib_files_plgFileCanWrite, &kplib_files_plgFileSeek, &kplib_files_plgFileExisted,
			&kplib_files_plgFileOpen, &kplib_files_plgFileClose, &kplib_files_plgMountArchive};

plg_api_type plg_api = {sizeof(plg_api_type), N_PLG_TYPE_FILESYS, L"Archives via Ken\'s Picture LIBrary (*.zip, *.grp)", &plg};

bool plg_file_opened = false;

/*
	Функция	: plgGetApi

	Описание: Возвращает API плагина

	История	: 10.08.14	Создан

*/
N_API plg_api_type * N_APIENTRY plgGetApi(void)
{
	if(!ea) return 0;

	return &plg_api;
}

/*
	Функция	: plgSetupDll

	Описание: Настройка dll плагина

	История	: 10.08.14	Создан

*/
N_API bool N_APIENTRY plgSetupDll(engapi_type *engapi)
{
	if(engapi) {
		if(engapi->mysize == sizeof(engapi_type)) {
			ea = engapi;

			ea->nlPrint(L"%ls", L"this software using kplib.c:\n\n"
				L"KPLIB.C: Ken\'s Picture LIBrary written by Ken Silverman\n"
				L"Copyright (c) 1998-2008 Ken Silverman\n"
				L"Ken Silverman\'s official web site: http://advsys.net/ken\n\n"
				L"I offer this code to the community for free use - all I ask is that my name be\n"
				L"included in the credits.\n\n"
				L"-Ken S.\n");

			return true;
		}
	}

	return false;
}

/*
	Функция	: kplib_files_plgInit

	Описание: Инициализирует файловый плагин

	История	: 10.08.14	Создан

*/
bool N_APIENTRY kplib_files_plgInit(void)
{
	plg_file_opened = false;

	return true;
}

/*
	Функция	: kplib_files_plgDestroy

	Описание: Выгружает файловый плагин

	История	: 10.08.14	Создан

*/
void N_APIENTRY kplib_files_plgDestroy(void)
{
	if(plg_file_opened)
		kzclose();

	plg_file_opened = false;

	kzuninit();

	return;
}

/*
	Функция	: kplib_files_plgFileRead

	Описание: Читает num байт из файла. Возвращает количество прочитанных байт;
				При чтении за eof возвращает 0; при ошибке возвращает -1

	История	: 10.08.14	Создан

*/
long long int N_APIENTRY kplib_files_plgFileRead(fs_fileinfo_type *file, void *dst, size_t num)
{
	long long int ret;

	if(!plg_file_opened)
		return -1;

	ret = kzread(dst, (int)num);

	if(ret != -1) file->foffset += ret;

	return ret;
}

/*
	Функция	: kplib_files_plgFileWrite

	Описание: Пишет num байт в файл. Возвращает количество записанных байт;
				При ошибке возвращает -1

	История	: 10.08.14	Создан

*/
long long int N_APIENTRY kplib_files_plgFileWrite(fs_fileinfo_type *file, void *src, size_t num)
{
	(void)file; // Неиспользуемая переменная
	(void)src; // Неиспользуемая переменная
	(void)num; // Неиспользуемая переменная

	return -1;
}

/*
	Функция	: kplib_files_plgFileCanWrite

	Описание: Возвращает true если файл существует, иначе false

	История	: 10.08.14	Создан

*/
bool N_APIENTRY kplib_files_plgFileCanWrite(fs_fileinfo_type *file)
{
	(void)file; // Неиспользуемая переменная

	return false;
}

/*
	Функция	: kplib_files_plgFileSeek

	Описание: Устанавливает позицию в файле. Возвращает текущую позицию или -1 при неудаче

	История	: 10.08.14	Создан

*/
long long int N_APIENTRY kplib_files_plgFileSeek(fs_fileinfo_type *file,long long int offset,int origin)
{
	file->foffset = kzseek((int)(file->offset+offset), origin);

	if(file->foffset>-1) file->foffset -= file->offset;

	return file->foffset;
}

/*
	Функция	: kplib_files_plgFileExisted

	Описание: Возвращает true если файл существует, иначе false

	История	: 10.08.14	Создан

*/
bool N_APIENTRY kplib_files_plgFileExisted(const wchar_t *fname)
{
	char *cfname;
	size_t csize;
	int ret;

	csize = wcslen(fname)+1;
	cfname = malloc(csize*sizeof(char));
	if(!cfname) return false;
	wcstombsl(cfname,fname,csize);

	ret = kzaccess(cfname);

	free(cfname);

	return !ret;
}

/*
	Функция	: kplib_files_plgFileOpen

	Описание: Открывает файл. Возвращает false, если файл не был открыт

	История	: 10.08.14	Создан

*/
bool N_APIENTRY kplib_files_plgFileOpen(const wchar_t *fname, fs_fileinfo_type *file)
{
	char *cfname;
	size_t csize;
	intptr_t ret;

	csize = wcslen(fname)+1;
	cfname = malloc(csize*sizeof(char));
	if(!cfname) return false;
	wcstombsl(cfname,fname,csize);

	ret = kzopen(cfname);

	free(cfname);

	if(ret){
		file->offset = 0;
		file->foffset = 0;
		file->fsize = kzfilelength();
		file->fdata = 0;

		plg_file_opened = true;

		return true;
	} else
		return false;
}

/*
	Функция	: kplib_files_plgFileClose

	Описание: Закрывает файл. Возвращает true или false

	История	: 10.08.14	Создан

*/
bool N_APIENTRY kplib_files_plgFileClose(fs_fileinfo_type *file)
{
	(void)file; // Неиспользуемая переменная

	if(plg_file_opened)
		kzclose();

	plg_file_opened = false;

	return true;
}

/*
	Функция	: kplib_files_plgMountArchive

	Описание: Добавляет архив

	История	: 10.08.14	Создан

*/
bool N_APIENTRY kplib_files_plgMountArchive(const wchar_t *arcname)
{
	char *carcname;
	size_t csize;
	int ret;

	csize = wcslen(arcname)+1;
	carcname = malloc(csize*sizeof(char));
	if(!carcname) return false;
	wcstombsl(carcname,arcname,csize);

	ret = kzaddstack2(carcname, 0);

	free(carcname);

	if(!ret)
		return false;
	else
		return true;
}
