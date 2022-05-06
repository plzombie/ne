/*
	Файл	: nyan_log.c

	Описание: Журнал сообщений

	История	: 03.08.12	Создан

*/

#define __STDC_WANT_LIB_EXT1__ 1

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <wchar.h>

#if defined(N_POSIX)
	#include "../unixsupport/wfsopen.h"
#elif defined(N_ANDROID)
	#include "../extclib/wcstombsl.h"
	#include <android/log.h>

	#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "NyanEngine", __VA_ARGS__))
#elif defined(N_WINDOWS) || defined(N_DOS) || defined(N_CAUSEWAY)
	#include <share.h>
#endif

#include "nyan_decls_publicapi.h"

#include "nyan_text.h"

#include "nyan_log.h"

#include "nyan_threads.h"

#include "nyan_filesys_publicapi.h"
#include "nyan_filesys_dirpaths_publicapi.h"
#include "nyan_fps_publicapi.h"
#include "nyan_log_publicapi.h"
#include "nyan_mem_publicapi.h"

static int log_tabs = 0; // Количество символов табуляции перед сообщением
static bool log_opened = false; // Показывает, открыт ли журнал сообщений
static bool log_enabled = true; // Обозначает, надо ли добавлять записи в журнал сообещений
static int log_linenum = 0; // Номер строки в журнале сообщения
static n_sysmutex_type log_mutex;
static FILE *log_f; // id файла журнала сообщений

/*
	Функция	: nlEnable

	Описание: Устанавливает, вести ли журнал сообщений

	История	: 17.08.17	Создан

*/
N_API void N_APIENTRY_EXPORT nlEnable(bool enable)
{
	log_enabled = enable;
}

/*
	Функция	: nlAddTab

	Описание: Добавляет\удаляет символы табуляции

	История	: 04.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nlAddTab(int tabs)
{
	if(log_opened) {
		nLockSystemMutex(&log_mutex);
			log_tabs += tabs;
		nUnlockSystemMutex(&log_mutex);
	} else
		log_tabs += tabs;
}

/*
	Функция	: nlOpen

	Описание: Открывает журнал сообщений

	История	: 04.08.12	Создан

*/
void nlOpen(void)
{
#ifndef N_ANDROID
	wchar_t timestr[80];
	wchar_t *logfilename;
	time_t timer;

	if (log_opened) return;

	// Создаю папку с локальными данными движка
	if(!nFileCreateDir(L"Nyan", NF_PATH_DATA_LOCAL)) return;
	
	// Создаю файл сообщений
	if(!nFileCreate(L"Nyan/gleng.log", true, NF_PATH_DATA_LOCAL)) return;

	// Получаю имя для файла журнала сообщений
	logfilename = nFileGetAbsoluteFilename(L"Nyan/gleng.log", NF_PATH_DATA_LOCAL);
	if(!logfilename) return;

	if(!nCreateSystemMutex(&log_mutex)) {
		nFreeMemory(logfilename);
		return;
	}

	log_opened = true;
	log_linenum = 1;


	log_f = _wfsopen(logfilename, L"w", _SH_DENYWR);
	nFreeMemory(logfilename);
	if(!log_f) { log_opened = false; return; }
	timer = time(0);
	if(!wcsftime(timestr, 80, LOG_TIME1, localtime(&timer)))
		timestr[0] = 0;
	fwprintf(log_f,LOG_OPENED,timestr);
#else
	if (log_opened) return;

	if(!nCreateSystemMutex(&log_mutex))
		return;

	log_opened = true;
	log_linenum = 1;
#endif
}
/*
	Функция	: nlPrint

	Описание: Печатает сообщение

	История	: 04.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nlPrint(const wchar_t *format, ...)
{
#ifndef N_ANDROID
	int i;
	va_list arglist;

	if(!log_enabled) return;

	if(log_opened)  {
		static wchar_t timestr[80];
		time_t timer;
		struct tm timer_tm;

		timer = time(0);
		if(
#ifdef N_POSIX
			localtime_r(&timer, &timer_tm) == NULL
#elif __STDC_LIB_EXT1__
			localtime_s(&timer, &timer_tm) == NULL
#else
			localtime_s(&timer_tm, &timer)
#endif
			) {
			memset(&timer_tm, 0, sizeof(struct tm));
			timer_tm.tm_mday = 1;
		}
		if(!wcsftime(timestr, 80, LOG_TIME2, &timer_tm))
			timestr[0] = 0;

		nLockSystemMutex(&log_mutex);
			fwprintf(log_f, LOG_LINEFORMAT, log_linenum, timestr, (double)nClock()/N_CLOCKS_PER_SEC);
			for(i = 0;i < log_tabs;i++)
				fputwc(L'\t', log_f);

			va_start(arglist, format);
			vfwprintf(log_f,format, arglist);
			va_end(arglist);

			fputwc(L'\n', log_f);

			log_linenum ++;
		nUnlockSystemMutex(&log_mutex);
	} else {
		for(i = 0;i < log_tabs;i++)
			fputwc(L'\t', stdout);

		va_start(arglist, format);
		vwprintf(format, arglist);
		va_end(arglist);

		fputwc(L'\n', stdout);
	}

	fflush(log_f);
#else
	//va_list arglist;
	static char tempstr[1024];
	//static wchar_t tempwstr[1024];

	if(!log_enabled) return;

	//va_start(arglist, format);
	//vswprintf(tempwstr, 1023, format, arglist);
	//va_end(arglist);
	//tempwstr[1023] = 0;
	//wcstombsl(tempstr, tempwstr, 256);

	wcstombsl(tempstr, format, 1024);

	if(log_opened) {
		nLockSystemMutex(&log_mutex);

		LOGI("%s", tempstr);

		log_linenum++;
		nUnlockSystemMutex(&log_mutex);
	} else {
		LOGI("%s", tempstr);
	}
#endif
}

/*
	Функция	: nlClose

	Описание: Закрывает журнал сообщений

	История	: 04.08.12	Создан

*/
void nlClose(void)
{
#ifndef N_ANDROID
	wchar_t timestr[80];
	time_t timer;

	if(log_opened == false) return;

	nDestroySystemMutex(&log_mutex);

	log_opened = false;
	timer = time(0);
	if(!wcsftime(timestr, 80, LOG_TIME1, localtime(&timer)))
		timestr[0] = 0;
	fwprintf(log_f, LOG_CLOSED, timestr);
	fclose(log_f);
#else
	if (log_opened == false) return;

	log_opened = false;
#endif
}

