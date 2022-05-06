/*
	Файл	: nyan_sys_getproc.c

	Описание: Работа с dll. Обёртки над системными функциями.

	История	: 02.01.18	Создан

*/

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>
//#include "nyan_log_publicapi.h"

#include "../nyan_sys_getproc.h"

#ifdef __FreeBSD__
	#define PROC_SELF_EXE "/proc/curproc/file"
#else
	#define PROC_SELF_EXE "/proc/self/exe"
#endif

/*
	Функция	: nSysLoadLib

	Описание: Загружает библиотеку

	История	: 02.01.18	Создан

*/
void *nSysLoadLib(const wchar_t *filename)
{
	void *result = 0;
	char *cfilename;
	int csize;
	int flags_for_dlopen = RTLD_LAZY;
	//wchar_t *tempws;

	if(!filename) return 0;

	csize = wcstombs(NULL, filename, 0)+1;
	cfilename = malloc(csize*sizeof(char));
	if(!cfilename) return 0;
	wcstombs(cfilename, filename, csize);

	if(!result) { // Пытаемся подставить путь к программе к имени библиотеки
		char *path, *temp, *newcfilename;
		struct stat sb;
		int pathlen;


		// Читаю полное имя программы
		if(lstat(PROC_SELF_EXE, &sb) == -1)
			goto TRYDEFPATHS;

		if(sb.st_size == 0)
			sb.st_size = 1024;

		path = malloc(sb.st_size);

		if(!path)
			goto TRYDEFPATHS;

		memset(path, 0, sb.st_size);

		if(readlink(PROC_SELF_EXE, path, sb.st_size) == -1) {
			free(path);

			goto TRYDEFPATHS;
		}

		// Удаляю часть с именем, оставляю только путь
		temp = strrchr(path, '/');
		if(temp)
			*temp = 0;

		pathlen = strlen(path);

		newcfilename = malloc(pathlen+strlen(cfilename)+2);

		if(!newcfilename) {
			free(path);

			goto TRYDEFPATHS;
		}

		strcpy(newcfilename, path);
		if(cfilename[0] != '/') { // path возвращается без '/' на конце, а cfilename может быть как с '/' в начале, так и без
			// Если без, то необходимо добавить '/' в newcfilename между path и cfilename
			newcfilename[pathlen] = '/';
			newcfilename[pathlen+1] = 0;
		}
		strcat(newcfilename, cfilename);

		/*tempws = malloc((strlen(newcfilename)+1)*sizeof(wchar_t));
		mbstowcs(tempws, newcfilename, strlen(newcfilename)+1);
		nlPrint(L"Try to open %ls", tempws);
		free(tempws);*/
		result = dlopen(newcfilename, flags_for_dlopen);


		free(path);
		free(newcfilename);
	}

TRYDEFPATHS:

	if(!result) { // Попробуем стандартные пути
		/*tempws = malloc((strlen(cfilename)+1)*sizeof(wchar_t));
		mbstowcs(tempws, cfilename, strlen(cfilename)+1);
		nlPrint(L"Try to open %ls", tempws);
		free(tempws);*/
		result = dlopen(cfilename, flags_for_dlopen);
	}

//TRYCURDIR:

	if(!result) { // Ещё одна попытка, только теперь подставляем текущую папку
		char *path, *newcfilename;
		int pathlen;

		path = malloc(1024);
		
		if(!path)
			goto EXIT;

		if(!getcwd(path, 1024)) {
			free(path);
			
			goto EXIT;
		}

		pathlen = strlen(path);

		newcfilename = malloc(pathlen+strlen(cfilename)+2);

		if(!newcfilename) {
			free(path);
			
			goto EXIT;
		}

		strcpy(newcfilename, path);
		free(path);
		if(cfilename[0] != '/') { // path возвращается без '/' на конце, а cfilename может быть как с '/' в начале, так и без
			// Если без, то необходимо добавить '/' в newcfilename между path и cfilename
			newcfilename[pathlen] = '/';
			newcfilename[pathlen+1] = 0;
		}
		strcat(newcfilename, cfilename);

		/*tempws = malloc((strlen(newcfilename)+1)*sizeof(wchar_t));
		mbstowcs(tempws, newcfilename, strlen(newcfilename)+1);
		nlPrint(L"Try to open %ls", tempws);
		free(tempws);*/
		result = dlopen(newcfilename, flags_for_dlopen);

		free(newcfilename);
	}

EXIT:

	free(cfilename);

	return result;
}

/*
	Функция	: nGetProcAddress

	Описание: Получает указатель на функцию из библиотеки

	История	: 12.08.12	Создан

*/
n_getproc_func_type nSysGetProcAddress(void *handle, const char *fname)
{
#if defined(__FreeBSD__)
	return (n_getproc_func_type)dlfunc(handle, fname);
#else
	union {
		n_getproc_func_type funcptr;
		void *dataptr;
	} result;
	/*wchar_t *tempws;
	tempws = malloc((strlen(fname)+1)*sizeof(wchar_t));
	mbstowcs(tempws, fname, strlen(fname)+1);
	nlPrint(L"Try to get %ls", tempws);
	free(tempws);*/

	result.dataptr = dlsym(handle, fname);

	return result.funcptr;
#endif
}

/*
	Функция	: nFreeLib

	Описание: Закрывает библиотеку

	История	: 12.08.12	Создан

*/
void nSysFreeLib(void *handle)
{
	dlclose(handle);
}

