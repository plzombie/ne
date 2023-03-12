/*
	Файл	: nyan_au_init.c

	Описание: Инициализация аудиодвижка

	История	: 30.08.12	Создан

*/

#include <stdlib.h>
#include <string.h>

#include "../commonsrc/core/nyan_array.h"

#include "nyan_au_init_publicapi.h"
#include "nyan_au_main_publicapi.h"
#include "nyan_log_publicapi.h"
#include "nyan_mem_publicapi.h"

#include "nyan_text.h"

#include "nyan_apifordlls.h"
#include "nyan_getproc.h"

#include "nyan_nalapi.h"

#include "nyan_au_main.h"

#include "nyan_threads.h"

nalSetupDll_type funcptr_nalSetupDll = 0;
nalInit_type funcptr_nalInit = 0;
nalClose_type funcptr_nalClose = 0;
nalCreateBuffer_type funcptr_nalCreateBuffer = 0;
nalDestroyAllBuffers_type funcptr_nalDestroyAllBuffers = 0;
nalDestroyBuffer_type funcptr_nalDestroyBuffer = 0;
nalLoadBuffer_type funcptr_nalLoadBuffer = 0;
nalUnloadBuffer_type funcptr_nalUnloadBuffer = 0;
nalCreateSource_type funcptr_nalCreateSource = 0;
nalDestroyAllSources_type funcptr_nalDestroyAllSources = 0;
nalDestroySource_type funcptr_nalDestroySource = 0;
nalPlaySource_type funcptr_nalPlaySource = 0;
nalPlayAudioStream_type funcptr_nalPlayAudioStream = 0;
nalPauseSource_type funcptr_nalPauseSource = 0;
nalPauseAudioStream_type funcptr_nalPauseAudioStream = 0;
nalStopSource_type funcptr_nalStopSource = 0;
nalStopAudioStream_type funcptr_nalStopAudioStream = 0;
nalReplaySource_type funcptr_nalReplaySource = 0;
nalReplayAudioStream_type funcptr_nalReplayAudioStream = 0;
nalGetSourceLoop_type funcptr_nalGetSourceLoop = 0;
nalSetSourceLoop_type funcptr_nalSetSourceLoop = 0;
nalGetSourceSecOffset_type funcptr_nalGetSourceSecOffset = 0;
nalSetSourceSecOffset_type funcptr_nalSetSourceSecOffset = 0;
nalGetSourceLength_type funcptr_nalGetSourceLength = 0;
nalGetSourceGain_type funcptr_nalGetSourceGain = 0;
nalSetSourceGain_type funcptr_nalSetSourceGain = 0;
nalGetSourceStatus_type funcptr_nalGetSourceStatus = 0;
nalGetAudioStreamLoop_type funcptr_nalGetAudioStreamLoop = 0;
nalSetAudioStreamLoop_type funcptr_nalSetAudioStreamLoop = 0;
nalGetAudioStreamGain_type funcptr_nalGetAudioStreamGain = 0;
nalSetAudioStreamGain_type funcptr_nalSetAudioStreamGain = 0;
nalCreateAudioStream_type funcptr_nalCreateAudioStream = 0;
nalDestroyAllAudioStreams_type funcptr_nalDestroyAllAudioStreams = 0;
nalDestroyAudioStream_type funcptr_nalDestroyAudioStream = 0;
nalGetAudioStreamSecOffset_type funcptr_nalGetAudioStreamSecOffset = 0;
nalSetAudioStreamSecOffset_type funcptr_nalSetAudioStreamSecOffset = 0;
nalGetAudioStreamLength_type funcptr_nalGetAudioStreamLength = 0;
nalGetAudioStreamStatus_type funcptr_nalGetAudioStreamStatus = 0;

#ifndef N_NODYLIB
	#define NALAPI_IMPORTFUNC(name) funcptr_##name = (name##_type)nGetProcAddress(nal_dllhandle, #name, "_"#name); \
		if(!funcptr_##name) { \
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT8, F_NAATTACHLIB, ERR_CANTIMPORTFUNC, L###name); \
			return false; \
		}
#else
	#define NALAPI_IMPORTFUNC(name) funcptr_##name = name;
#endif

#ifndef N_NODYLIB
static void *nal_dllhandle;
#endif

unsigned int na_isinit = false;
static int na_islibattached = false;

typedef struct {
	na_audiofile_plugin_type plugin;
	unsigned int usecounter;
} na_audiofile_protected_plugin_type;

static na_audiofile_protected_plugin_type *na_audiofile_plugins;
static unsigned int na_audiofile_maxplugins = 0;
static unsigned int na_audiofile_allocplugins = 0;

static n_sysmutex_type na_afmutex; // Мьютекс, используемый для синхронизации чтения аудиофайлов
static bool na_isafmutexinit = false;

#define NLOCKAFMUTEX if(na_isafmutexinit){nLockSystemMutex(&na_afmutex);}
#define NUNLOCKAFMUTEX if(na_isafmutexinit){nUnlockSystemMutex(&na_afmutex);}

/*
	Функция	: naAttachLib

	Описание: Прикрепляет аудиобиблиотеку к движку

	История	: 25.12.12	Создан

*/
N_API bool N_APIENTRY_EXPORT naAttachLib(const wchar_t *dllname)
{
	if(na_isinit) { nlPrint(LOG_FDEBUGFORMAT, F_NAATTACHLIB, N_FALSE); return false; }

#ifdef N_NODYLIB
	nlPrint(F_NAATTACHLIB); nlAddTab(1);
#else
	if(!dllname) { nlPrint(LOG_FDEBUGFORMAT, F_NAATTACHLIB, N_FALSE); return false; }

	nlPrint(LOG_FDEBUGFORMAT7, F_NAATTACHLIB, N_FNAME, dllname); nlAddTab(1);

	if(na_islibattached)
		nFreeLib(nal_dllhandle);

	na_islibattached = false;

	nal_dllhandle = nLoadModule(dllname);
	if(!nal_dllhandle) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT8, F_NAATTACHLIB, ERR_CANTLOADDLL, dllname);
		return false;
	}
#endif

	// Импортирую nalSetupDll
	NALAPI_IMPORTFUNC(nalSetupDll)

	// Импортирую nalInit
	NALAPI_IMPORTFUNC(nalInit)

	NALAPI_IMPORTFUNC(nalClose)

	NALAPI_IMPORTFUNC(nalCreateBuffer)

	NALAPI_IMPORTFUNC(nalDestroyAllBuffers)

	NALAPI_IMPORTFUNC(nalDestroyBuffer)

	NALAPI_IMPORTFUNC(nalLoadBuffer)

	NALAPI_IMPORTFUNC(nalUnloadBuffer)

	NALAPI_IMPORTFUNC(nalCreateSource)

	NALAPI_IMPORTFUNC(nalDestroyAllSources)

	NALAPI_IMPORTFUNC(nalDestroySource)

	NALAPI_IMPORTFUNC(nalPlaySource)

	NALAPI_IMPORTFUNC(nalPlayAudioStream)

	NALAPI_IMPORTFUNC(nalPauseSource)

	NALAPI_IMPORTFUNC(nalPauseAudioStream)

	NALAPI_IMPORTFUNC(nalStopSource)

	NALAPI_IMPORTFUNC(nalStopAudioStream)

	NALAPI_IMPORTFUNC(nalReplaySource)

	NALAPI_IMPORTFUNC(nalReplayAudioStream)

	NALAPI_IMPORTFUNC(nalGetSourceLoop)

	NALAPI_IMPORTFUNC(nalSetSourceLoop)

	NALAPI_IMPORTFUNC(nalGetSourceSecOffset)

	NALAPI_IMPORTFUNC(nalSetSourceSecOffset)

	NALAPI_IMPORTFUNC(nalGetSourceLength)

	NALAPI_IMPORTFUNC(nalGetSourceGain)

	NALAPI_IMPORTFUNC(nalSetSourceGain)

	NALAPI_IMPORTFUNC(nalGetSourceStatus)

	NALAPI_IMPORTFUNC(nalGetAudioStreamLoop)

	NALAPI_IMPORTFUNC(nalSetAudioStreamLoop)

	NALAPI_IMPORTFUNC(nalGetAudioStreamGain)

	NALAPI_IMPORTFUNC(nalSetAudioStreamGain)

	NALAPI_IMPORTFUNC(nalCreateAudioStream)

	NALAPI_IMPORTFUNC(nalDestroyAllAudioStreams)

	NALAPI_IMPORTFUNC(nalDestroyAudioStream)

	NALAPI_IMPORTFUNC(nalGetAudioStreamSecOffset)

	NALAPI_IMPORTFUNC(nalSetAudioStreamSecOffset)

	NALAPI_IMPORTFUNC(nalGetAudioStreamLength)

	NALAPI_IMPORTFUNC(nalGetAudioStreamStatus)

	// Настраиваю dll, передаю функции движка
	if(!funcptr_nalSetupDll(&n_ea)) {
#ifndef N_NODYLIB
		nFreeLib(nal_dllhandle);
#endif
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT8, F_NAATTACHLIB, ERR_CANTLOADDLL, dllname);
		return false;
	}

	na_islibattached = true;

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NAATTACHLIB, N_OK);

	return true;
}

/*
	Функция	: naInit

	Описание: Инициализирует аудиодвижок

	История	: 31.05.12	Создан

*/
bool naInit(void)
{
	if(na_isinit) return false;

	if(!na_islibattached) naAttachLib(L"nal.dll");

	if(!na_islibattached) { nlPrint(LOG_FDEBUGFORMAT2, F_NAINIT, N_FALSE, ERR_UMUSTATTACHALIBBSTE); return false; }

	nlPrint(F_NAINIT); nlAddTab(1);

	if(!nCreateSystemMutex(&na_afmutex)) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NAINIT, N_FALSE);
		return false;
	}

	na_isafmutexinit = true;

	if(!funcptr_nalInit()) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NAINIT, N_FALSE);
#ifndef N_NODYLIB
		nFreeLib(nal_dllhandle);
#endif
		na_islibattached = false;
		na_isafmutexinit = false;
		nDestroySystemMutex(&na_afmutex);
		return false;
	}

	na_isinit = true;

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NAINIT, N_OK);

	return true;
}

/*
	Функция	: naClose

	Описание: Завершает работу аудиодвижка

	История	: 31.05.12	Создан

*/
bool naClose(void)
{
	if(!na_isinit) return false;

	nlPrint(F_NACLOSE); nlAddTab(1);

	naDestroyAllAudioStreams();
	naDestroyAllSources();
	naDestroyAllBuffers();

	na_isinit = false;

	funcptr_nalClose();

	na_isafmutexinit = false;

	nDestroySystemMutex(&na_afmutex);

#ifndef N_NODYLIB
	if(na_islibattached) {
		na_islibattached = false;
		nFreeLib(nal_dllhandle);
	}
#endif

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NACLOSE, N_OK);

	return true;
}

/*
	Функция	: naLoadAudioFile

	Описание: Инициализирует структуру (na_audiofile_type *aud)

	История	: 01.09.12	Создан

*/
N_API bool N_APIENTRY_EXPORT naLoadAudioFile(const wchar_t *fname, na_audiofile_type *aud)
{
	bool success = false;
	unsigned int i;
	wchar_t *filext;

	filext = wcsrchr(fname,L'.');

	if(filext) filext++; else return false;

	NLOCKAFMUTEX

	for(i=0;i<na_audiofile_maxplugins;i++)
		if(na_audiofile_plugins[i].plugin.SupportExt(fname, filext)) {
			success = na_audiofile_plugins[i].plugin.Load(fname, aud);
			if(success) {
				aud->selplg = i;
				aud->selplgusecounter = na_audiofile_plugins[i].usecounter;
				break;
			}
		}

	NUNLOCKAFMUTEX

	return success;
}

/*
	Функция	: naReadAudioFile

	Описание: Читает аудиосемплы
			offset - указывает количество секунд, которые надо пропустить от начала записи
			nofs - Количество семплов, которые надо прочитать
			aud - Структура, содержащая данные об аудиофайле

	История	: 01.09.12	Создан

*/
N_API bool N_APIENTRY_EXPORT naReadAudioFile(unsigned int offset, unsigned int nofs, void *buf, na_audiofile_type *aud)
{
	bool success;

	NLOCKAFMUTEX

	if(aud->selplg >= na_audiofile_maxplugins) {
		NUNLOCKAFMUTEX
		
		return false;
	}

	if(aud->selplgusecounter != na_audiofile_plugins[aud->selplg].usecounter) {
		NUNLOCKAFMUTEX
		
		return false;
	}

	success = na_audiofile_plugins[aud->selplg].plugin.Read(offset, nofs, buf, aud);

	NUNLOCKAFMUTEX

	return success;
}

/*
	Функция	: naUnloadAudioFile

	Описание: Закрывает

	История	: 01.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT naUnloadAudioFile(na_audiofile_type *aud)
{
	NLOCKAFMUTEX

	if(aud->selplg >= na_audiofile_maxplugins) {
		NUNLOCKAFMUTEX

		return;
	}

	if(aud->selplgusecounter != na_audiofile_plugins[aud->selplg].usecounter) {
		NUNLOCKAFMUTEX

		return;
	}

	na_audiofile_plugins[aud->selplg].plugin.Unload(aud);

	NUNLOCKAFMUTEX
}


/*
	Функция	: naCheckAudioPluginArray

	Описание: Проверяет свободное место для плагина в массиве

	История	: 12.03.23	Создан

*/
static bool naCheckAudioPluginArray(void *array_el, bool set_free)
{
	na_audiofile_protected_plugin_type *el;
	
	el = (na_audiofile_protected_plugin_type *)array_el;
	
	if(set_free) el->usecounter = 0;
	
	return (el->usecounter == 0)?true:false;
}

/*
	Функция	: naAddAudioFilePlugin

	Описание: Добавляет плагин для загрузки изображений из файла

	История	: 13.07.12	Создан

*/
N_API bool N_APIENTRY_EXPORT naAddAudioFilePlugin(const wchar_t *name, na_audiofile_plugin_type *au_fileplg)
{
	unsigned int new_plugin = 0;
	
	if(au_fileplg->size != sizeof(na_audiofile_plugin_type)) {
		nlPrint(LOG_FDEBUGFORMAT, F_NAADDAUDIOFILEPLUGIN, N_FALSE);
		return false;
	}

	NLOCKAFMUTEX

	if(!nArrayAdd(
		&n_ea, (void **)(&na_audiofile_plugins),
		&na_audiofile_maxplugins,
		&na_audiofile_allocplugins,
		naCheckAudioPluginArray,
		&new_plugin,
		NYAN_ARRAY_DEFAULT_STEP,
		sizeof(na_audiofile_protected_plugin_type))
	) {
		NUNLOCKAFMUTEX
		nlPrint(LOG_FDEBUGFORMAT, F_NAADDAUDIOFILEPLUGIN, N_FALSE);
		return false;
	}
	na_audiofile_plugins[new_plugin].plugin = *au_fileplg;
	na_audiofile_plugins[new_plugin].usecounter++;

	NUNLOCKAFMUTEX

	nlPrint(LOG_FDEBUGFORMAT7, F_NAADDAUDIOFILEPLUGIN, NA_ADDAUDIOFILEPLUGIN, name);

	return true;
}

/*
	Функция	: naDeleteAllAudioFilePlugins

	Описание: Удаляет все плагины для загрузки аудиофайлов

	История	: 01.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT naDeleteAllAudioFilePlugins(void)
{
	NLOCKAFMUTEX

	// Т.к. streaming sources содержат указатели на na_audiofile_type, мы не можем оставлять их в живых
	naDestroyAllAudioStreams();

	if(na_audiofile_maxplugins) nFreeMemory(na_audiofile_plugins);

	na_audiofile_plugins = 0;
	na_audiofile_maxplugins = 0;
	na_audiofile_allocplugins = 0;

	NUNLOCKAFMUTEX

	nlPrint(LOG_FDEBUGFORMAT, F_NADELETEALLAUDIOFILEPLUGINS, N_OK);
}
