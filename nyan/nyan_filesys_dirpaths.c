/*
	Файл	: nyan_filesys_dirpaths.c

	Описание: Функции для работы с файлами. Получение пути папок и файлов.

	История	: 12.04.17	Создан

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>

#ifdef N_WINDOWS
	#include <windows.h>
	#include <ShlObj.h>
#endif

#if defined(N_POSIX) || defined(N_ANDROID)
	#include "../extclib/mbstowcsl.h"
	#include "../unixsupport/waccess.h"
	#include "../unixsupport/wmkdir.h"
#else
	#include <io.h>
#endif

#include "nyan_text.h"

#include "nyan_mem_publicapi.h"

#include "nyan_filesys.h"
#include "nyan_filesys_dirpaths.h"
#include "nyan_filesys_dirpaths_publicapi.h"


static wchar_t fs_userdir[512] = {0};
static bool fs_isuserdirget = false;

static wchar_t fs_roamingdatadir[512] = {0};
static bool fs_isroamingdatadirget = false;

static wchar_t fs_localdatadir[512] = {0};
static bool fs_islocaldatadirget = false;

static wchar_t fs_documentsdir[512] = {0};
static bool fs_isdocumentsdirget = false;

static wchar_t fs_savedgamesdir[512] = {0};
static bool fs_issavedgamesdirget = false;

static wchar_t fs_picturesdir[512] = {0};
static bool fs_ispicturesdirget = false;

static wchar_t fs_musicdir[512] = {0};
static bool fs_ismusicdirget = false;

static wchar_t fs_videosdir[512] = {0};
static bool fs_isvideosdirget = false;

static wchar_t fs_downloadsdir[512] = {0};
static bool fs_isdownloadsdirget = false;

#if defined(N_WINDOWS)

// Watcom не очень работает с NTDDI_VERSION висты
#define N_KF_FLAG_DEFAULT 0x00000000
#define N_KF_FLAG_CREATE 0x00008000

typedef HRESULT (WINAPI *SHGetFolderPathW_type)(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);
typedef HRESULT (WINAPI *SHGetKnownFolderPath_type)(REFKNOWNFOLDERID rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath);

static SHGetFolderPathW_type funcptr_SHGetFolderPathW = 0;
static SHGetKnownFolderPath_type funcptr_SHGetKnownFolderPath = 0;

static bool n_is_shfolderfunctions_initialized = false;

static void nFileGetShFolderFunctions(void)
{
	HMODULE hmodule_shell32;
	HMODULE hmodule_shfolder;

	if(n_is_shfolderfunctions_initialized)
		return;

	hmodule_shell32 = LoadLibraryW(L"shell32.dll");
	hmodule_shfolder = LoadLibraryW(L"shfolder.dll");

	if(hmodule_shell32) {
		funcptr_SHGetFolderPathW = (SHGetFolderPathW_type)GetProcAddress(hmodule_shell32, "SHGetFolderPathW");
		funcptr_SHGetKnownFolderPath = (SHGetKnownFolderPath_type)GetProcAddress(hmodule_shell32, "SHGetKnownFolderPath");
	}
	if((!funcptr_SHGetFolderPathW) && hmodule_shfolder)
		funcptr_SHGetFolderPathW = (SHGetFolderPathW_type)GetProcAddress(hmodule_shfolder, "SHGetFolderPathW");

	n_is_shfolderfunctions_initialized = true;
}

#endif

/*
	Функция	: nFileGetUserDir

	Описание: Возвращает путь к папке пользователя

	История	: 18.04.17	Создан

*/
static const wchar_t *nFileGetUserDir(void)
{
	nFileLockMutex();

#if defined(N_WINDOWS)
	if(!fs_isuserdirget) {
		DWORD size;

		size = GetEnvironmentVariableW(L"USERPROFILE", fs_userdir, 512);
		if(size == 0 || size >= 512) {
			size = ExpandEnvironmentStringsW(L"%HOMEDRIVE%%HOMEPATH%", fs_userdir, 512);

			if(size == 0 || size > 512)
				*fs_userdir = 0;
			else if(wcsstr(fs_userdir, L"%HOMEDRIVE%") || wcsstr(fs_userdir, L"%HOMEPATH%"))
				*fs_userdir = 0;
		}

		if(_waccess(fs_userdir, 0))
			*fs_userdir = 0;

		fs_isuserdirget = true;
	}
#elif  defined(N_POSIX) || defined(N_ANDROID)
	if(!fs_isuserdirget) {
		char *path; int pathlen;
		path = getenv("HOME");

		if(path) {
			pathlen = strlen(path);

			if(pathlen < 512)
				mbstowcsl(fs_userdir, path, pathlen+1);
			else
				*fs_userdir = 0;
			
		}else
			*fs_userdir = 0;

		fs_isuserdirget = true;
	}
#else
	fs_isuserdirget = true;
#endif

	nFileUnlockMutex();

	return fs_userdir;
}

/*
	Функция	: nFileGetRoamingDataDir

	Описание: Возвращает путь к папке, в которой будут находиться общие между компьютерами данные приложения

	История	: 27.10.13	Создан

*/
static const wchar_t *nFileGetRoamingDataDir(void)
{
	nFileLockMutex();

#if defined(N_WINDOWS)
	nFileGetShFolderFunctions();

	if(funcptr_SHGetFolderPathW) {
		if(funcptr_SHGetFolderPathW(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, fs_roamingdatadir) == S_OK)
			fs_isroamingdatadirget = true;
	}

	if(!fs_isroamingdatadirget) {
		DWORD size;

		size = GetEnvironmentVariableW(L"APPDATA", fs_roamingdatadir, 512);
		if(size == 0 || size >= 512)
			wcscpy(fs_roamingdatadir, nFileGetUserDir());

		fs_isroamingdatadirget = true;
	}
#elif  defined(N_POSIX) || defined(N_ANDROID)
	if(!fs_isroamingdatadirget) { // $XDG_DATA_HOME или $HOME/.local/share
		char *path; int pathlen;
		path = getenv("XDG_DATA_HOME");

		if(path) {
			pathlen = strlen(path);

			if(pathlen < 512)
				mbstowcsl(fs_roamingdatadir, path, pathlen+1);
			else
				*fs_roamingdatadir = 0;
		} else {
			if(wcslen(nFileGetUserDir())+wcslen(L"/.local/share") < 512) {
				wcscpy(fs_roamingdatadir, nFileGetUserDir());
				wcscat(fs_roamingdatadir, L"/.local");

				if(_waccess(fs_roamingdatadir, 0)) {
					if(_wmkdir(fs_roamingdatadir)) {
						wcscpy(fs_roamingdatadir, nFileGetUserDir());
						goto EXIT;
					}
				}

				wcscat(fs_roamingdatadir, L"/share");

				if(_waccess(fs_roamingdatadir, 0)) {
					if(_wmkdir(fs_roamingdatadir)) {
						wcscpy(fs_roamingdatadir, nFileGetUserDir());
						goto EXIT;
					}
				}
			} else
				wcscpy(fs_roamingdatadir, nFileGetUserDir());
		}

EXIT:

		fs_isroamingdatadirget = true;
	}
#else
	fs_isroamingdatadirget = true;
#endif

	nFileUnlockMutex();

	return fs_roamingdatadir;
}

/*
	Функция	: nFileGetLocalDataDir

	Описание: Возвращает путь к папке, в которой будут находиться локальные для этого компьютера данные приложения

	История	: 18.04.17	Создан

*/
static const wchar_t *nFileGetLocalDataDir(void)
{
	nFileLockMutex();

#if defined(N_WINDOWS)
	nFileGetShFolderFunctions();

	if(funcptr_SHGetFolderPathW) {
		if(funcptr_SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, fs_localdatadir) == S_OK)
			fs_islocaldatadirget = true;
	}

	if(!fs_islocaldatadirget) {
		DWORD size;

		size = GetEnvironmentVariableW(L"LOCALAPPDATA", fs_localdatadir, 512);
		if(size == 0 || size >= 512) {
			size = ExpandEnvironmentStringsW(L"%USERPROFILE%\\Local Settings\\Application Data", fs_localdatadir, 512);
			
			if(size == 0 || size > 512) *fs_localdatadir = 0;

			if(!wcscmp(L"%USERPROFILE%\\Local Settings\\Application Data", fs_localdatadir))
				wcscpy(fs_localdatadir, nFileGetRoamingDataDir());
			else if(_waccess(fs_localdatadir, 0))
				wcscpy(fs_localdatadir, nFileGetRoamingDataDir());
		}

		fs_islocaldatadirget = true;
	}
#elif  defined(N_POSIX) || defined(N_ANDROID)
	if(!fs_islocaldatadirget) { // $XDG_CACHE_HOME или "$HOME/.cache"
		char *path; int pathlen;
		path = getenv("XDG_CACHE_HOME");

		if(path) {
			pathlen = strlen(path);

			if(pathlen < 512)
				mbstowcsl(fs_localdatadir, path, pathlen+1);
			else
				wcscpy(fs_localdatadir, nFileGetRoamingDataDir());
		} else {
			if(wcslen(nFileGetUserDir())+wcslen(L"/.cache") < 512) {
				wcscpy(fs_localdatadir, nFileGetUserDir());
				wcscat(fs_localdatadir, L"/.cache");

				if(_waccess(fs_localdatadir, 0)) {
					if(_wmkdir(fs_localdatadir))
						wcscpy(fs_localdatadir, nFileGetRoamingDataDir());
				}
			} else
				wcscpy(fs_localdatadir, nFileGetRoamingDataDir());
		}

		fs_islocaldatadirget = true;
	}
#else
	fs_islocaldatadirget = true;
#endif

	nFileUnlockMutex();

	return fs_localdatadir;
}

/*
	Функция	: nFileGetDocumentsDir

	Описание: Возвращает путь к папке, в которой находятся документы пользователя

	История	: 18.04.17	Создан

*/
static const wchar_t *nFileGetDocumentsDir(void)
{
	nFileLockMutex();

#if defined(N_WINDOWS)
	nFileGetShFolderFunctions();

	if(funcptr_SHGetFolderPathW) {
		if(funcptr_SHGetFolderPathW(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, fs_documentsdir) == S_OK)
			fs_isdocumentsdirget = true;
	}
#endif

	if(!fs_isdocumentsdirget) {
		if(wcslen(nFileGetUserDir())+wcslen(L"/Documents") < 512) {
			wcscpy(fs_documentsdir, nFileGetUserDir());
			wcscat(fs_documentsdir, L"/Documents");

			if(_waccess(fs_documentsdir, 0))
				wcscpy(fs_documentsdir, nFileGetUserDir());
		} else
			wcscpy(fs_documentsdir, nFileGetUserDir());

		fs_isdocumentsdirget = true;
	}

	nFileUnlockMutex();

	return fs_documentsdir;
}

/*
	Функция	: nFileGetSavedGamesDir

	Описание: Возвращает путь к папке, в которой находятся сохранения игр пользователя

	История	: 18.04.17	Создан

*/
static const wchar_t *nFileGetSavedGamesDir(void)
{
	nFileLockMutex();

#if !(defined(N_POSIX) || defined(N_ANDROID))

#if defined(N_WINDOWS)
	nFileGetShFolderFunctions();

	if(funcptr_SHGetKnownFolderPath) {
		PWSTR path;

		if(funcptr_SHGetKnownFolderPath(&FOLDERID_SavedGames, N_KF_FLAG_DEFAULT|N_KF_FLAG_CREATE, NULL, &path) == S_OK) {
			if(wcslen(path) < 512) {
				wcscpy(fs_savedgamesdir, path);
				fs_issavedgamesdirget = true;
			}
			CoTaskMemFree(path);
		}
	}
#endif

	if(!fs_issavedgamesdirget) {
		if(wcslen(nFileGetUserDir())+wcslen(L"/Saved Games") < 512) {
			wcscpy(fs_savedgamesdir, nFileGetUserDir());
			wcscat(fs_savedgamesdir, L"/Saved Games");

			if(_waccess(fs_savedgamesdir, 0))
				wcscpy(fs_savedgamesdir, nFileGetUserDir());
		} else
			wcscpy(fs_savedgamesdir, nFileGetUserDir());

		fs_issavedgamesdirget = true;
	}
#else
	if(!fs_issavedgamesdirget) { // $XDG_CONFIG_HOME или $HOME/.config
		char *path; int pathlen;
		path = getenv("XDG_CONFIG_HOME");

		if(path) {
			pathlen = strlen(path);

			if(pathlen < 512)
				mbstowcsl(fs_savedgamesdir, path, pathlen+1);
			else
				wcscpy(fs_savedgamesdir, nFileGetUserDir());
		} else {
			if(wcslen(nFileGetUserDir())+wcslen(L"/.config") < 512) {
				wcscpy(fs_savedgamesdir, nFileGetUserDir());
				wcscat(fs_savedgamesdir, L"/.config");

				if(_waccess(fs_savedgamesdir, 0)) {
					if(_wmkdir(fs_savedgamesdir))
						wcscpy(fs_savedgamesdir, nFileGetUserDir());
				}
			} else
				wcscpy(fs_savedgamesdir, nFileGetUserDir());
		}

		fs_issavedgamesdirget = true;
	}
#endif

	nFileUnlockMutex();

	return fs_savedgamesdir;
}

/*
	Функция	: nFileGetPicturesDir

	Описание: Возвращает путь к папке, в которой находятся изображения пользователя

	История	: 18.04.17	Создан

*/
static const wchar_t *nFileGetPicturesDir(void)
{
	nFileLockMutex();

#if defined(N_WINDOWS)
	nFileGetShFolderFunctions();

	if(funcptr_SHGetFolderPathW) {
		if(funcptr_SHGetFolderPathW(NULL, CSIDL_MYPICTURES|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, fs_picturesdir) == S_OK)
			fs_ispicturesdirget = true;
	}
#endif

	if(!fs_ispicturesdirget) {
		if(wcslen(nFileGetUserDir())+wcslen(L"/Pictures") < 512) {
			wcscpy(fs_picturesdir, nFileGetUserDir());
			wcscat(fs_picturesdir, L"/Pictures");

			if(_waccess(fs_picturesdir, 0))
				wcscpy(fs_picturesdir, nFileGetUserDir());
		} else
			wcscpy(fs_picturesdir, nFileGetUserDir());

		fs_ispicturesdirget = true;
	}

	nFileUnlockMutex();

	return fs_picturesdir;
}

/*
	Функция	: nFileGetMusicDir

	Описание: Возвращает путь к папке, в которой находится музыка пользователя

	История	: 18.04.17	Создан

*/
static const wchar_t *nFileGetMusicDir(void)
{
	nFileLockMutex();

#if defined(N_WINDOWS)
	nFileGetShFolderFunctions();

	if(funcptr_SHGetFolderPathW) {
		if(funcptr_SHGetFolderPathW(NULL, CSIDL_MYMUSIC|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, fs_musicdir) == S_OK)
			fs_ismusicdirget = true;
	}
#endif

	if(!fs_ismusicdirget) {
		if(wcslen(nFileGetUserDir())+wcslen(L"/Music") < 512) {
			wcscpy(fs_musicdir, nFileGetUserDir());
			wcscat(fs_musicdir, L"/Music");

			if(_waccess(fs_musicdir, 0))
				wcscpy(fs_musicdir, nFileGetUserDir());
		} else
			wcscpy(fs_musicdir, nFileGetUserDir());

		fs_ismusicdirget = true;
	}

	nFileUnlockMutex();

	return fs_musicdir;
}

/*
	Функция	: nFileGetVideosDir

	Описание: Возвращает путь к папке, в которой находятся видеофайлы пользователя

	История	: 18.04.17	Создан

*/
static const wchar_t *nFileGetVideosDir(void)
{
	nFileLockMutex();

#if defined(N_WINDOWS)
	nFileGetShFolderFunctions();

	if(funcptr_SHGetFolderPathW) {
		if(funcptr_SHGetFolderPathW(NULL, CSIDL_MYVIDEO|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, fs_videosdir) == S_OK)
			fs_isvideosdirget = true;
	}
#endif

	if(!fs_isvideosdirget) {
		if(wcslen(nFileGetUserDir())+wcslen(L"/Videos") < 512) {
			wcscpy(fs_videosdir, nFileGetUserDir());
			wcscat(fs_videosdir, L"/Videos");

			if(_waccess(fs_videosdir, 0))
				wcscpy(fs_videosdir, nFileGetUserDir());
		} else
			wcscpy(fs_videosdir, nFileGetUserDir());

		fs_isvideosdirget = true;
	}

	nFileUnlockMutex();

	return fs_videosdir;
}

/*
	Функция	: nFileGetDownloadsDir

	Описание: Возвращает путь к папке, в которой находятся загруженные файлы пользователя

	История	: 18.04.17	Создан

*/
static const wchar_t *nFileGetDownloadsDir(void)
{
	nFileLockMutex();

#if defined(N_WINDOWS)
	nFileGetShFolderFunctions();

	if(funcptr_SHGetKnownFolderPath) {
		PWSTR path;

		if(funcptr_SHGetKnownFolderPath(&FOLDERID_Downloads, N_KF_FLAG_DEFAULT|N_KF_FLAG_CREATE, NULL, &path) == S_OK) {
			if(wcslen(path) < 512) {
				wcscpy(fs_downloadsdir, path);
				fs_isdownloadsdirget = true;
			}
			CoTaskMemFree(path);
		}
	}
#endif

	if(!fs_isdownloadsdirget) {
		if(wcslen(nFileGetUserDir())+wcslen(L"/Downloads") < 512) {
			wcscpy(fs_downloadsdir, nFileGetUserDir());
			wcscat(fs_downloadsdir, L"/Downloads");

			if(_waccess(fs_downloadsdir, 0))
				wcscpy(fs_downloadsdir, nFileGetUserDir());
		} else
			wcscpy(fs_downloadsdir, nFileGetUserDir());

		fs_isdownloadsdirget = true;
	}

	nFileUnlockMutex();

	return fs_downloadsdir;
}

/*
	Функция	: nFileGetDir

	Описание: Возвращает путь к папке, обозначенной в переменной relpath

	История	: 17.03.17	Создан
*/
N_API const wchar_t * N_APIENTRY_EXPORT nFileGetDir(unsigned int relpath)
{
	switch(relpath) {
		case NF_PATH_CURRENTDIR:
			return L"./";
		case NF_PATH_USERDIR:
			return nFileGetUserDir();
		case NF_PATH_DATA_ROAMING:
			return nFileGetRoamingDataDir();
		case NF_PATH_DATA_LOCAL:
			return nFileGetLocalDataDir();
		case NF_PATH_DOCUMENTS:
			return nFileGetDocumentsDir();
		case NF_PATH_GAMESAVES:
			return nFileGetSavedGamesDir();
		case NF_PATH_PICTURES:
			return nFileGetPicturesDir();
		case NF_PATH_MUSIC:
			return nFileGetMusicDir();
		case NF_PATH_VIDEOS:
			return nFileGetVideosDir();
		case NF_PATH_DOWNLOADS:
			return nFileGetDownloadsDir();
	}

	return 0;
}

/*
	Функция	: nFileAppendFilenameToDir

	Описание: Возвращает имя файла filename в папке dir.
		Считает filename, начинающийся с "\", путём относительно dir. Т.е. nFileAppendFilenameToDir(L"\Subdir\file.txt", L"\Dir") вернёт L"\Dir\Subdir\file.txt".
		Если нужна проверка на абсолютное имя в filename, следует использовать nFileGetAbsoluteFilename.

	История	: 17.03.17	Создан
*/
N_API wchar_t * N_APIENTRY_EXPORT nFileAppendFilenameToDir(const wchar_t *filename, const wchar_t *dir)
{
	wchar_t *newfilename;

	if(!dir || !filename) return 0;

	newfilename = nAllocMemory((wcslen(dir) + wcslen(filename) + 2) * sizeof(wchar_t));
	if(!newfilename) return 0;
	wcscpy(newfilename, dir);

	// Если нет слэша в конце dir
	if(*newfilename) { // Если newfilename не нулевой длины
		if((newfilename[wcslen(newfilename) - 1] != '\\') && (newfilename[wcslen(newfilename) - 1] != '/')) {
			if((filename[0] != '\\') && (filename[0] != '/')) // Если нет слэша в начале filename
				wcscat(newfilename, L"/");
		}
	}
	wcscat(newfilename, filename);

	return newfilename;
}

/*
	Функция	: nFileGetAbsoluteFilename

	Описание: Возвращает имя файла filename в папке, обозначенной в переменной relpath

	История	: 04.03.17	Создан
*/
N_API wchar_t * N_APIENTRY_EXPORT nFileGetAbsoluteFilename(const wchar_t *filename, unsigned int relpath)
{
	if(!filename) return 0;

	if(wcslen(filename) >= 2) {
		// Если файл уже имеет абсолютный путь
		if((filename[1] == ':') || (filename[0] == '.' && (filename[1] == '\\' || filename[1] == '/')) || (filename[0] == '\\' || filename[0] == '/')) {
			wchar_t *newfilename;

			newfilename = nAllocMemory((wcslen(filename)+1)*sizeof(wchar_t));

			if(!newfilename) return 0;

			wcscpy(newfilename, filename);

			return newfilename;
		}
	}

	return nFileAppendFilenameToDir(filename, nFileGetDir(relpath));
}
