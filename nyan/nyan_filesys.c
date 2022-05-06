/*
	Файл	: nyan_filesys.c

	Описание: Функции для работы с файлами

	История	: 15.07.12	Создан

*/

#ifdef N_POSIX
	#define _LARGEFILE64_SOURCE
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#if defined(N_WINDOWS) || defined(N_DOS) || defined(N_CAUSEWAY)
	#include <direct.h>
	#include <share.h>
#endif

#ifdef N_WINDOWS
	#include <windows.h>
#endif

#if defined(N_POSIX) || defined(N_ANDROID)
	#include <unistd.h>
	#include "../unixsupport/wmkdir.h"
	#include "../unixsupport/wrmdir.h"
	#include "../unixsupport/wremove.h"
	#include "../unixsupport/wsopen.h"
	#define _close close
#else
	#include <io.h>
#endif

#ifndef O_LARGEFILE
	#define O_LARGEFILE 0
#endif

#include "nyan_log_publicapi.h"
#include "nyan_mem_publicapi.h"
#include "nyan_filesys_publicapi.h"
#include "nyan_filesys_dirpaths_publicapi.h"
#include "nyan_text.h"

#include "nyan_filesys.h"


typedef struct {
	fs_fileinfo_type info;
	int plgid; // Id плагина, используемого для открытия файла
	bool used; // TRUE, если файл открыт
} fs_file_type;

static fs_file_type *fs_files;
static unsigned int fs_maxfiles = 0; // Количество файлов

static fs_fileplg_type *fs_fileplgs;
static unsigned int fs_maxfileplgs = 0; // Количество файловых плагинов
static unsigned int fs_allocfileplgs = 0;

static wchar_t **fs_mounteddirs = 0;
static unsigned int fs_maxmounteddirs = 0; // Количество смонтированных корневых папок
static unsigned int fs_allocmounteddirs = 0; // Количество смонтированных корневых папок, для которых выделена память (fs_maxmounteddirs<=fs_allocmounteddirs)

static n_sysmutex_type fs_fsmutex; // Мьютекс, используемый для синхронизации файловой системы
static bool fs_isfsmutexinit = false;

// Инициализация fs_fsmutex происходит в nInit() сразу перед вызовом nInitThreadsLib()
// Деинициализация - после вызова nDestroyThreadsLib()
#define NLOCKFSMUTEX if(fs_isfsmutexinit){nLockSystemMutex(&fs_fsmutex);}
#define NUNLOCKFSMUTEX if(fs_isfsmutexinit){nUnlockSystemMutex(&fs_fsmutex);}

/*
	Функция	: nFileInitMutex

	Описание: Инициализирует мьютекс, используемый для синхронизации файловой системы

	История	: 18.10.15	Создан

*/
bool nFileInitMutex(void)
{
	if(fs_isfsmutexinit)
		return false;

	if(!nCreateSystemMutex(&fs_fsmutex))
		return false;
	
	fs_isfsmutexinit = true;

	return true;
}

/*
	Функция	: nFileDestroyMutex

	Описание: Уничтожает мьютекс, используемый для синхронизации файловой системы

	История	: 18.10.15	Создан

*/
bool nFileDestroyMutex(void)
{
	if(!fs_isfsmutexinit)
		return false;

	fs_isfsmutexinit = false;

	nDestroySystemMutex(&fs_fsmutex);

	return true;
}

/*
	Функция	: nFileLockMutex

	Описание: Забирает мьютекс, используемый для синхронизации файловой системы

	История	: 20.02.18	Создан

*/
void nFileLockMutex(void)
{
	NLOCKFSMUTEX
}

/*
	Функция	: nFileUnlockMutex

	Описание: Освобождает мьютекс, используемый для синхронизации файловой системы

	История	: 20.02.18	Создан

*/
void nFileUnlockMutex(void)
{
	NUNLOCKFSMUTEX
}

/*
	Функция	: nFileCreateDir

	Описание: Создаёт подпапку в папке, обозначенной в переменной relpath

	История	: 17.03.17	Создан

*/
N_API bool N_APIENTRY_EXPORT nFileCreateDir(const wchar_t *dir, unsigned int relpath)
{
	wchar_t *newdir;
	int result;
	bool success;

	// Получаю имя для файла filename в папке, обозначенной в relpath
	newdir = nFileGetAbsoluteFilename(dir, relpath);
	if(!newdir) return false;

	//wprintf(L"trying to create dir '%ls'\n", newdir);

#if defined(N_DOS) || defined(N_CAUSEWAY)
	// Под досом _wmkdir возвращает -1 и errno == 6
	if(_waccess(newdir, 0) == 0) {
		nFreeMemory(newdir);

		return true;
	}
#endif

	result = _wmkdir(newdir);

	//wprintf(L"result %d errno %d\n", result, (int)errno);

	if(result) {
		if(errno == EEXIST)
			success = true;
		else
			success = false;
	} else
		success = true;

	nFreeMemory(newdir);

	return success;
}

/*
	Функция	: nFileDeleteDir

	Описание: Удаляет подпапку в папке, обозначенной в переменной relpath

	История	: 17.03.17	Создан

*/
N_API bool N_APIENTRY_EXPORT nFileDeleteDir(const wchar_t *dir, unsigned int relpath)
{
	wchar_t *newdir;
	int result;

	// Получаю имя для файла filename в папке, обозначенной в relpath
	newdir = nFileGetAbsoluteFilename(dir, relpath);
	if(!newdir) return false;

	result = _wrmdir(newdir);

	nFreeMemory(newdir);

	if(result)
		return false;
	else
		return true;
}

/*
	Функция	: nFileCreate

	Описание: Создаёт файл в папке, обозначенной в переменной relpath

	История	: 27.10.13	Создан

*/
N_API bool N_APIENTRY_EXPORT nFileCreate(const wchar_t *filename, bool truncate, unsigned int relpath)
{
	int f;
	wchar_t *newfilename;

	// Получаю имя для файла filename в папке, обозначенной в relpath
	newfilename = nFileGetAbsoluteFilename(filename, relpath);
	if(!newfilename) return false;

	f = _wsopen(newfilename, _O_WRONLY | _O_CREAT | O_LARGEFILE | ((truncate)?(_O_TRUNC):0), _SH_DENYRW, _S_IREAD  | _S_IWRITE);

	nFreeMemory(newfilename);

	if(f == -1)
		return false;
	else {
		_close(f);
		return true;
	}
}

/*
	Функция	: nFileDelete

	Описание: Удаляет файл в папке nFileGetAppDataDir()

	История	: 27.10.13	Создан

*/
N_API bool N_APIENTRY_EXPORT nFileDelete(const wchar_t *filename, unsigned int relpath)
{
	wchar_t *newfilename;
	int result;

	// Получаю имя для файла filename в папке, обозначенной в relpath
	newfilename = nFileGetAbsoluteFilename(filename, relpath);
	if(!newfilename) return false;

	result = _wremove(newfilename);

	nFreeMemory(newfilename);

	if(result)
		return false;
	else
		return true;
}

/*
	Функция	: nFileRead

	Описание: Читает num байт из файла. Возвращает количество прочитанных байт;
				При чтении за eof возвращает 0; при ошибке возвращает -1

	История	: 24.06.12	Создан

*/
N_API long long int N_APIENTRY_EXPORT nFileRead(unsigned int id, void *dst, size_t num)
{
	long long int result;

	NLOCKFSMUTEX

	if((id > fs_maxfiles) || (id == 0)) { NUNLOCKFSMUTEX return -1; }
	if(!fs_files[id-1].used) { NUNLOCKFSMUTEX return -1; }

	result = fs_fileplgs[fs_files[id-1].plgid].plgFileRead(&fs_files[id-1].info, dst, num);

	NUNLOCKFSMUTEX

	return result;
}

/*
	Функция	: nFileWrite

	Описание: Пишет num байт в файл. Возвращает количество записанных байт;
				При ошибке возвращает -1

	История	: 28.05.13	Создан

*/
N_API long long int N_APIENTRY_EXPORT nFileWrite(unsigned int id, void *src, size_t num)
{
	long long int result;

	NLOCKFSMUTEX

	if((id > fs_maxfiles) || (id == 0)) { NUNLOCKFSMUTEX return -1; }
	if(!fs_files[id-1].used) { NUNLOCKFSMUTEX return -1; }

	result = fs_fileplgs[fs_files[id-1].plgid].plgFileWrite(&fs_files[id-1].info, src, num);

	NUNLOCKFSMUTEX

	return result;
}

/*
	Функция	: nFileCanWrite

	Описание: Возвращает true, если в файл можно записывать данные.

	История	: 24.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nFileCanWrite(unsigned int id)
{
	bool result;

	NLOCKFSMUTEX

	if((id > fs_maxfiles) || (id == 0)) { NUNLOCKFSMUTEX return false; }
	if(!fs_files[id-1].used) { NUNLOCKFSMUTEX return false; }

	result = fs_fileplgs[fs_files[id-1].plgid].plgFileCanWrite(&fs_files[id-1].info);

	NUNLOCKFSMUTEX

	return result;

	
}

/*
	Функция	: nFileLength

	Описание: Возвращает размер файла.

	История	: 24.06.12	Создан

*/
N_API long long int N_APIENTRY_EXPORT nFileLength(unsigned int id)
{
	long long int result;

	NLOCKFSMUTEX

	if((id > fs_maxfiles) || (id == 0)) { NUNLOCKFSMUTEX return -1; }
	if(!fs_files[id-1].used) { NUNLOCKFSMUTEX return -1; }

	result = fs_files[id-1].info.fsize;

	NUNLOCKFSMUTEX

	return result;
}

/*
	Функция	: nFileTell

	Описание: Возвращает позицию в файле. Возвращает текущую позицию или -1 при неудаче

	История	: 24.06.12	Создан

*/
N_API long long int N_APIENTRY_EXPORT nFileTell(unsigned int id)
{
	long long int result;

	NLOCKFSMUTEX

	if((id > fs_maxfiles) || (id == 0)) { NUNLOCKFSMUTEX return -1; }
	if(!fs_files[id-1].used) { NUNLOCKFSMUTEX return -1; }

	result = fs_files[id-1].info.foffset;

	NUNLOCKFSMUTEX

	return result;
}

/*
	Функция	: nFileSeek

	Описание: Устанавливает позицию в файле. Возвращает текущую позицию или -1 при неудаче

	История	: 24.06.12	Создан

*/
N_API long long int N_APIENTRY_EXPORT nFileSeek(unsigned int id, long long int offset, int origin)
{
	long long int result;

	NLOCKFSMUTEX

	if((id > fs_maxfiles) || (id == 0)) { NUNLOCKFSMUTEX return -1; }
	if(!fs_files[id-1].used) { NUNLOCKFSMUTEX return -1; }

	if(offset < 0 || offset+fs_files[id-1].info.offset<0) { NUNLOCKFSMUTEX return -1; }

	result = fs_fileplgs[fs_files[id-1].plgid].plgFileSeek(&fs_files[id-1].info, offset, origin);

	NUNLOCKFSMUTEX

	return result;
}

/*
	Функция	: nFileOpen

	Описание: Открывает файл. Возвращает 0, если файл не был открыт

	История	: 24.06.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nFileOpen(const wchar_t *fname)
{
	unsigned int i, id; // i - номер файла, id - номер файлового плагина
	wchar_t *ffname = 0; bool ffname_mustfree = false; // Final file name
	bool success = false;

	NLOCKFSMUTEX

	for(i = 0;i < fs_maxfiles;i++)
		if(!fs_files[i].used)
			break;

	if(i == fs_maxfiles) {
		fs_file_type *_fs_files;
		_fs_files = nReallocMemory(fs_files,(fs_maxfiles+1024)*sizeof(fs_file_type));
		if(_fs_files)
			fs_files = _fs_files;
		else {
			NUNLOCKFSMUTEX

			return 0;
		}
		for(i=fs_maxfiles;i<fs_maxfiles+1024;i++)
			fs_files[i].used = false;
		i = fs_maxfiles;
		fs_maxfiles += 1024;
	}

	for(id = 0; id < fs_maxfileplgs; id++)
		if(fs_fileplgs[id].plgFileExisted(fname)) {
			success = true;
			ffname = (wchar_t *)fname;
			break;
		}

	if(!success) {
		size_t fname_len, ffname_len;
		unsigned int j;
		bool check_mounted_dirs = true;

		fname_len = wcslen(fname);
		ffname_len = 0;
		ffname = 0;

		if(fname_len >= 2) {
			// Если файл уже имеет абсолютный путь
			if((fname[1] == ':') || (fname[0] == '.' && (fname[1] == '\\' || fname[1] == '/')) || (fname[0] == '\\' || fname[0] == '/'))
				check_mounted_dirs = false;
		}

		if(check_mounted_dirs) {
			for(j=0;j<fs_maxmounteddirs;j++) {
				if( ffname_len < (fname_len+wcslen(fs_mounteddirs[j])+1) ) {
					wchar_t* _ffname;
					size_t _ffname_len;
					_ffname_len = fname_len+wcslen(fs_mounteddirs[j])+1;
					_ffname = nReallocMemory(ffname, _ffname_len*sizeof(wchar_t));
					if(_ffname) {
						ffname = _ffname;
						ffname_len = _ffname_len;
					} else
						break;
					ffname_mustfree = true;
				}
				wcscpy(ffname,fs_mounteddirs[j]);
				wcscat(ffname,fname);
				for(id = 0; id < fs_maxfileplgs; id++)
					if(fs_fileplgs[id].plgFileExisted(ffname)){
						success = true;
						break;
					}
				if(success) break;
			}
		}
	}

	if(success)
		success = fs_fileplgs[id].plgFileOpen(ffname, &fs_files[i].info);

	if(ffname_mustfree) nFreeMemory(ffname);

	if(!success) {
		fs_files[i].used = false;
		i = 0;
	} else {
		fs_files[i].used = true;
		fs_files[i].plgid = id;
		i++;
	}

	NUNLOCKFSMUTEX

	return i;
}

/*
	Функция	: nFileClose

	Описание: Закрывает файл. Возвращает true или false

	История	: 24.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nFileClose(unsigned int id)
{
	NLOCKFSMUTEX

	if((id > fs_maxfiles) || (id == 0)) { NUNLOCKFSMUTEX return false; }
	if(!fs_files[id-1].used) { NUNLOCKFSMUTEX return false; }

	if(fs_fileplgs[fs_files[id-1].plgid].plgFileClose(&fs_files[id-1].info)) {
		fs_files[id-1].used = false;
		NUNLOCKFSMUTEX

		return true;
	} else {
		NUNLOCKFSMUTEX

		return false;
	}
}

/*
	Функция	: nCloseAllFiles

	Описание: Закрывает все файлы, папки, архивы...

	История	: 24.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nCloseAllFiles(void)
{
	bool gsuccess = true;

	nlPrint(F_NCLOSEALLFILES); nlAddTab(1);

	NLOCKFSMUTEX

	if(fs_maxfiles) {
		unsigned int i;
		bool success = true;

		for(i = 1; i <= fs_maxfiles; i++)
			if(fs_files[i-1].used)
				if(!nFileClose(i))
					success = false;

		if(success) {
			fs_maxfiles = 0;
			nFreeMemory(fs_files);
			fs_files = 0;
		} else
			gsuccess = false;
	}

	if(fs_maxmounteddirs) {
		unsigned int i;
		for(i=0;i<fs_maxmounteddirs;i++)
			nFreeMemory(fs_mounteddirs[i]);
		nFreeMemory(fs_mounteddirs);
		fs_mounteddirs = 0;
		fs_maxmounteddirs = 0;
		fs_allocmounteddirs = 0;
	}

	NUNLOCKFSMUTEX

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NCLOSEALLFILES, (gsuccess)?N_OK:N_FALSE);

	return gsuccess;
}

/*
	Функция	: nMountDir

	Описание: Добавляет папку

	История	: 13.07.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nMountDir(const wchar_t *dirname)
{
	size_t dirname_len;

	dirname_len = wcslen(dirname);
	
	if(!dirname_len) // Пустой путь
		return true;

	NLOCKFSMUTEX

	if(fs_maxmounteddirs == fs_allocmounteddirs) {
		wchar_t **_fs_mounteddirs;
		_fs_mounteddirs = nReallocMemory(fs_mounteddirs,(fs_allocmounteddirs+1024)*sizeof(wchar_t *));
		if(_fs_mounteddirs)
			fs_mounteddirs = _fs_mounteddirs;
		else {
			NUNLOCKFSMUTEX
			nlPrint(LOG_FDEBUGFORMAT,F_NMOUNTDIR,N_FALSE);

			return false;
		}
		fs_allocmounteddirs += 1024;
	}

	if((dirname[dirname_len-1] == L'\\') || (dirname[dirname_len-1] == L'/')) {
		fs_mounteddirs[fs_maxmounteddirs] = nAllocMemory((dirname_len+1)*sizeof(wchar_t));
		if(!fs_mounteddirs[fs_maxmounteddirs]) {
			NUNLOCKFSMUTEX
			nlPrint(LOG_FDEBUGFORMAT,F_NMOUNTDIR,N_FALSE);

			return false;
		}
		wmemcpy(fs_mounteddirs[fs_maxmounteddirs],dirname,dirname_len+1);
		//wcscpy(fs_mounteddirs[fs_maxmounteddirs],dirname);
	} else { // Если нет слэша, то надо его добавить
		fs_mounteddirs[fs_maxmounteddirs] = nAllocMemory((dirname_len+2)*sizeof(wchar_t));
		if(!fs_mounteddirs[fs_maxmounteddirs]) {
			NUNLOCKFSMUTEX
			nlPrint(LOG_FDEBUGFORMAT,F_NMOUNTDIR,N_FALSE);

			return false;
		}
		//wcscpy(fs_mounteddirs[fs_maxmounteddirs],dirname);
		wmemcpy(fs_mounteddirs[fs_maxmounteddirs],dirname,dirname_len);
		fs_mounteddirs[fs_maxmounteddirs][dirname_len] = L'/';
		fs_mounteddirs[fs_maxmounteddirs][dirname_len+1] = 0;
	}

	nlPrint(LOG_FDEBUGFORMAT7, F_NMOUNTDIR, N_MOUNTDIR, fs_mounteddirs[fs_maxmounteddirs]);

	fs_maxmounteddirs++;

	NUNLOCKFSMUTEX

	return true;
}

/*
	Функция	: nMountArchive

	Описание: Добавляет архив

	История	: 01.08.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nMountArchive(const wchar_t *arcname)
{
	unsigned int i;
	bool success = false;

	NLOCKFSMUTEX

	for(i = 0; i < fs_maxfileplgs; i++)
		if(fs_fileplgs[i].plgMountArchive(arcname))
			success = true;

	NUNLOCKFSMUTEX

	nlPrint(LOG_FDEBUGFORMAT7, F_NMOUNTARCHIVE, N_MOUNTARCHIVE, arcname);
	return success;
}

/*
	Функция	: nAddFilePlugin

	Описание: Добавляет плагин для чтения файлов

	История	: 27.01.13	Создан

*/
N_API bool N_APIENTRY_EXPORT nAddFilePlugin(const wchar_t *name, fs_fileplg_type *fs_fileplg)
{
	if(fs_fileplg->size != sizeof(fs_fileplg_type)) {
		nlPrint(LOG_FDEBUGFORMAT, F_NADDFILEPLUGIN, N_FALSE);

		return false;
	}

	NLOCKFSMUTEX

	if(!fs_allocfileplgs) {
		fs_fileplgs = nAllocMemory(1024*sizeof(fs_fileplg_type));
		if(!fs_fileplgs) {
			NUNLOCKFSMUTEX
			nlPrint(LOG_FDEBUGFORMAT, F_NADDFILEPLUGIN, N_FALSE);

			return false;
		}
		fs_allocfileplgs = 1024;
	} else if(fs_maxfileplgs == fs_allocfileplgs) {
		fs_fileplg_type *_fs_fileplgs;
		_fs_fileplgs = nReallocMemory(fs_fileplgs, (fs_allocfileplgs+1024)*sizeof(fs_fileplg_type));
		if(_fs_fileplgs)
			fs_fileplgs = _fs_fileplgs;
		else {
			NUNLOCKFSMUTEX
			nlPrint(LOG_FDEBUGFORMAT, F_NADDFILEPLUGIN, N_FALSE);

			return false;
		}
		fs_allocfileplgs += 1024;
	}

	fs_fileplgs[fs_maxfileplgs] = *fs_fileplg;

	if(!fs_fileplg->plgInit()) {
		NUNLOCKFSMUTEX
		nlPrint(LOG_FDEBUGFORMAT, F_NADDFILEPLUGIN, N_FALSE);

		return false;
	}

	fs_maxfileplgs++;

	NUNLOCKFSMUTEX

	nlPrint(LOG_FDEBUGFORMAT7, F_NADDFILEPLUGIN, N_ADDFILEPLUGIN, name);

	return true;
}

/*
	Функция	: nDeleteAllFilePlugins

	Описание: Удаляет все плагины для чтения файлов

	История	: 27.01.13	Создан

*/
N_API void N_APIENTRY_EXPORT nDeleteAllFilePlugins(void)
{
	unsigned int i;

	NLOCKFSMUTEX

	if(fs_maxfiles)
		nCloseAllFiles();

	for(i = 0; i < fs_maxfileplgs; i++)
		fs_fileplgs[i].plgDestroy();

	if(fs_allocfileplgs) nFreeMemory(fs_fileplgs);

	fs_fileplgs = 0;
	fs_maxfileplgs = 0;
	fs_allocfileplgs = 0;

	NUNLOCKFSMUTEX

	nlPrint(LOG_FDEBUGFORMAT, F_NDELETEALLFILEPLUGINS, N_OK);
}
