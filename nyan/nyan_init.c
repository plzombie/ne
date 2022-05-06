/*
	Файл	: nyan_init.c

	Описание: Инициализация движка

	История	: 03.08.12	Создан

*/

#include <stdio.h>

#include "nyan_au_init_publicapi.h"
#include "nyan_log_publicapi.h"
#include "nyan_filesys_publicapi.h"
#include "nyan_filesys_dirpaths_publicapi.h"
#include "nyan_plgloader_publicapi.h"
#include "nyan_vis_texture_publicapi.h"
#include "nyan_text.h"

#include "nyan_log.h"
#include "nyan_vis_init.h"
#include "nyan_au_init.h"
#include "nyan_apifordlls.h"
#include "nyan_filesys.h"
#include "nyan_threads.h"

#include "../plugins/plg_nek0.h"
#include "../plugins/plg_tga.h"
#include "../plugins/plg_bmp.h"
#include "../plugins/plg_pcx.h"
#include "../plugins/plg_wav.h"
#include "../plugins/plg_files.h"

#if defined(N_POSIX) || defined(N_ANDROID)
	#include <string.h>

	#include "nyan_mem_publicapi.h"

	#include "../extclib/mbstowcsl.h"
#endif

#include "nyan_init.h"

static bool n_isinit = false;

/*
	Функция	: nIsInit

	Описание: Возвращает true, если движёк запущен

	История	: 04.08.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nIsInit(void)
{
	return n_isinit;
}

#if defined(N_POSIX) || defined(N_ANDROID)
/*
	Функция	: nMountDataDirs

	Описание: Добавляет дополнительные папки, в которых нужно искать данные

	История	: 12.06.18	Создан

*/
static void nMountDataDirs(void)
{
	char *datadirs[2];
	size_t i;

	datadirs[0] = getenv("XDG_DATA_DIRS");
	if(!(datadirs[0]))
		datadirs[0] = "/usr/local/share/:/usr/share/";

	datadirs[1] = getenv("XDG_CONFIG_DIRS");
	if(!(datadirs[1]))
		datadirs[1] = "/etc/xdg";

	for(i = 0; i < 2; i++) {
		wchar_t *tempstr, *mountdir;
		size_t len, j;

		len = strlen(datadirs[i])+1;

		tempstr = nAllocMemory(len*sizeof(wchar_t));

		mbstowcsl(tempstr, datadirs[i], len);

		mountdir = tempstr;

		for(j = 0; j < len; j++) {
			if(tempstr[j] == ':' || tempstr[j] == 0) {
				tempstr[j] = 0;
				nMountDir(mountdir);
				mountdir = tempstr+j+1;
			}
		}

		free(tempstr);
	}
}
#endif

/*
	Функция	: nInit

	Описание: Инициализирует движёк

	История	: 04.08.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nInit(void)
{
	if(n_isinit) return false;
	
	nlOpen();
	
	nlPrint(F_NINIT); nlAddTab(1);
	
	if(!nFileInitMutex()) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NINIT, N_FALSE);
		nlClose();

		return false;
	}

	if(!nInitThreadsLib()) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NINIT, N_FALSE);
		nlClose();
		
		return false;
	}

	if(!naInit()) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NINIT, N_FALSE);
		nlClose();
		
		return false;
	}
	
	if(!nvInit()) {
		naClose();
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NINIT, N_FALSE);
		nlClose();
		
		return false;
	}
	
	nvAddTexturePlugin(L"Experimental img files (*.nek0)", &plgNEK0);
	nvAddTexturePlugin(L"PCX files", &plgPCX);
	nvAddTexturePlugin(L"TGA files", &plgTGA);
	nvAddTexturePlugin(L"Windows BMP files", &plgBMP);
	naAddAudioFilePlugin(L"Wav files", &plgWAV);
#ifndef N_ANDROID
	nAddFilePlugin(L"Standart file IO", &plgFILE);
#endif

	nMountDir(nFileGetDir(NF_PATH_DATA_ROAMING));
	nMountDir(nFileGetDir(NF_PATH_DATA_LOCAL));
	nMountDir(nFileGetDir(NF_PATH_GAMESAVES));
#if defined(N_POSIX) || defined(N_ANDROID)
	nMountDataDirs();
#endif

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NINIT, N_OK);
	
	n_isinit = true;
	
	return true;
}

/*
	Функция	: nClose

	Описание: Деинициализирует движёк

	История	: 04.08.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nClose(void)
{
	if(!n_isinit) return false;
	
	nlPrint(F_NCLOSE); nlAddTab(1);
	
	nStopAllTasks();
		
	if(!nvClose())
		nlPrint(LOG_FDEBUGFORMAT6, F_NCLOSE, F_NVCLOSE, ERR_RETURNSFALSE);
	
	if(!naClose())
		nlPrint(LOG_FDEBUGFORMAT6, F_NCLOSE, F_NACLOSE, ERR_RETURNSFALSE);

	if (!nCloseAllFiles())
		nlPrint(LOG_FDEBUGFORMAT6, F_NCLOSE, F_NCLOSEALLFILES, ERR_RETURNSFALSE);

	nDeleteAllPlugins();

	if(!nDestroyThreadsLib())
		nlPrint(LOG_FDEBUGFORMAT6, F_NCLOSE, F_NDESTROYTHREADSLIB, ERR_RETURNSFALSE);

	nFileDestroyMutex();

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NCLOSE, N_OK);
	
	nlClose();
	
	n_isinit = false;
	
	return true;
}

/*
	Функция	: nUpdate

	Описание: Деинициализирует движёк

	История	: 21.10.12	Создан

*/
N_API void N_APIENTRY_EXPORT nUpdate(void)
{
	if(!n_isinit) return;
	
	nvFlip();
	nUpdateThreadsLib();
}
