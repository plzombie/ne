/*
	Файл	: nyan_fps.c

	Описание: Подсчет кадров в секунду

	История	: 01.07.12	Создан

*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "nyan_decls_publicapi.h"

#include "nyan_fps.h"
#include "nyan_fps_publicapi.h"

static bool n_clock_initialized = false;

#if N_CLOCKS_PER_SEC != 1000000
	#error Need to tweek values on windows and linux
#endif

#if defined(N_WINDOWS)
	#include <Windows.h>

	typedef BOOL (WINAPI *QueryPerformanceCounter_type)(LARGE_INTEGER *lpPerformanceCount);
	typedef BOOL (WINAPI *QueryPerformanceFrequency_type)(LARGE_INTEGER *lpFrequency);
	typedef ULONGLONG (WINAPI *GetTickCount64_type)(void);

	static QueryPerformanceCounter_type QueryPerformanceCounter_funcptr = 0;
	static QueryPerformanceFrequency_type QueryPerformanceFrequency_funcptr = 0;
	static GetTickCount64_type GetTickCount64_funcptr = 0;
	
	static LARGE_INTEGER n_clock_firsttimeval_qpc;
	static LARGE_INTEGER n_clock_qpcfreq;
	static bool n_clock_useqpc = false;
	static ULONGLONG n_clock_firsttimeval_gtc64;
	static bool n_clock_usegtc64 = false;
	static DWORD n_clock_firsttimeval;
#elif defined(N_POSIX) || defined(N_ANDROID)
	static struct timespec n_clock_firsttimeval;
#else
	static clock_t n_clock_firsttimeval;
#endif

int64_t fps_deltaclocks = 1;
int64_t fps_lastclocks = 0;

double afps_deltaclocks = 1;
int64_t afps_lastnulldeltaclockslen = 1;
unsigned int afps_lastnulldeltas = 0;
unsigned int afps_nulldeltas = 0;

/*
	Функция	: nClock

	Описание: Возвращает количество микросекунд, прошедших с момента запуска программы (или первого удачного вызова nClock())
		В случае ошибки возвращает -1

	История	: 15.11.14	Созда

*/
N_API int64_t N_APIENTRY_EXPORT nClock(void)
{
#if defined(N_POSIX) || defined(N_ANDROID)
	struct timespec tv;
	long nsec_delta, microsec_delta;

	if(clock_gettime(CLOCK_MONOTONIC, &tv))
		return -1;

	if(!n_clock_initialized) {
		n_clock_firsttimeval = tv;
		n_clock_initialized = true;
	}

	nsec_delta = tv.tv_nsec-n_clock_firsttimeval.tv_nsec;
	microsec_delta = nsec_delta/1000;

	return (int64_t)(tv.tv_sec-n_clock_firsttimeval.tv_sec)*1000000+(int64_t)microsec_delta;
#elif defined(N_WINDOWS)
	HMODULE hmodule_kernel32;
	
	if(!n_clock_initialized) {
		hmodule_kernel32 = GetModuleHandleW(L"Kernel32.dll");
		if(hmodule_kernel32) {
			QueryPerformanceCounter_funcptr = (QueryPerformanceCounter_type)GetProcAddress(hmodule_kernel32, "QueryPerformanceCounter");
			QueryPerformanceFrequency_funcptr = (QueryPerformanceFrequency_type)GetProcAddress(hmodule_kernel32, "QueryPerformanceFrequency");
			GetTickCount64_funcptr = (GetTickCount64_type)GetProcAddress(hmodule_kernel32, "GetTickCount64");
		} else {
			QueryPerformanceCounter_funcptr = 0;
			QueryPerformanceFrequency_funcptr = 0;
			GetTickCount64_funcptr = 0;
		}

		if(QueryPerformanceCounter_funcptr && QueryPerformanceFrequency_funcptr) {
			if(!QueryPerformanceFrequency_funcptr(&n_clock_qpcfreq) || !QueryPerformanceCounter_funcptr(&n_clock_firsttimeval_qpc))
				n_clock_useqpc = false;
			else
				n_clock_useqpc = true;
		} else
			n_clock_useqpc = false;
		
		if(!n_clock_useqpc) {
			if(GetTickCount64_funcptr) {
				n_clock_firsttimeval_gtc64 = GetTickCount64_funcptr();
				n_clock_usegtc64 = true;
			} else {
				n_clock_useqpc = false;
				n_clock_firsttimeval = clock();
			}
		}

		n_clock_initialized = true;
	}

	if(n_clock_useqpc) {
		LARGE_INTEGER counter;
		LONGLONG QuadPart;
		int64_t seconds, parts;

		if(!QueryPerformanceCounter_funcptr(&counter))
			return -1;

		// a*b/c = a/c*b+(a%c)*b/c
		QuadPart = counter.QuadPart-n_clock_firsttimeval_qpc.QuadPart;
		seconds = QuadPart/n_clock_qpcfreq.QuadPart;
		parts = QuadPart%n_clock_qpcfreq.QuadPart;

		return seconds*N_CLOCKS_PER_SEC+parts*N_CLOCKS_PER_SEC/n_clock_qpcfreq.QuadPart;
	} else if(n_clock_usegtc64) {
		return (int64_t)(GetTickCount64_funcptr()- n_clock_firsttimeval_gtc64)*1000;
	} else {
		return (int64_t)(GetTickCount()-n_clock_firsttimeval)*1000;
	}
#else // ДОСы
	clock_t tv;

	tv = clock();

	if(tv == (clock_t)-1)
		return -1;

	if(!n_clock_initialized) {
		n_clock_firsttimeval = tv;
		n_clock_initialized = true;
	}

#if CLOCKS_PER_SEC == N_CLOCKS_PER_SEC
	return (int64_t)(tv-n_clock_firsttimeval);
#else
	return (int64_t)(tv-n_clock_firsttimeval)*N_CLOCKS_PER_SEC/CLOCKS_PER_SEC;
#endif

#endif
}

/*
	Функция	: nFrameStartClock

	Описание: Возвращает значение nClock() в момент начала текущего кадра

	История	: 15.11.14	Созда

*/
N_API int64_t N_APIENTRY_EXPORT nFrameStartClock(void)
{
	return fps_lastclocks;
}

/*
	Функция	: nGetfps

	Описание: Возвращает количество кадров в секунду

	История	: 01.07.12	Создан

*/
N_API double N_APIENTRY_EXPORT nGetfps(void)
{
	if(fps_deltaclocks == 0) return (double)N_CLOCKS_PER_SEC;

	return (double)N_CLOCKS_PER_SEC/fps_deltaclocks;
}

/*
	Функция	: nGetafps

	Описание: Возвращает среднее количество кадров в секунду

	История	: 31.08.13	Создан

*/
N_API double N_APIENTRY_EXPORT nGetafps(void)
{
	if(afps_deltaclocks == 0) return (double)N_CLOCKS_PER_SEC;

	return (double)N_CLOCKS_PER_SEC/afps_deltaclocks;
}

/*
	Функция	: nGetspf

	Описание: Возвращает количество секунд в кадре

	История	: 01.07.12	Создан

*/
N_API double N_APIENTRY_EXPORT nGetspf(void)
{
	return (double)fps_deltaclocks/N_CLOCKS_PER_SEC;
}

/*
	Функция	: nGetaspf

	Описание: Возвращает среднее количество секунд в кадре

	История	: 31.08.13	Создан

*/
N_API double N_APIENTRY_EXPORT nGetaspf(void)
{
	return afps_deltaclocks/N_CLOCKS_PER_SEC;
}
