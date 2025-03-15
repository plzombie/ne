/*
	Файл	: nyan_threads.h

	Описание: Заголовок для nyan_threads.c

	История	: 28.05.13	Создан

*/

#ifndef NYAN_THREADS_H
#define NYAN_THREADS_H

#include <stdbool.h>

// N_SYSMUTEX_ZERO_INIT нужен чтобы избежать tentative definition.
// Сама инициализация производится через nCreateSystemMutex

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__) && (!defined(_WIN32) || !defined(__clang__))
	#include <threads.h>
	typedef mtx_t n_sysmutex_type;
	#define N_SYSMUTEX_ZERO_INIT 0
#elif defined(N_WINDOWS)
	#include <windows.h>
	typedef HANDLE n_sysmutex_type;
	#define N_SYSMUTEX_ZERO_INIT 0
#elif defined(N_POSIX) || defined(N_ANDROID)
	#include <pthread.h>
	typedef pthread_mutex_t n_sysmutex_type;
	#define N_SYSMUTEX_ZERO_INIT {{0}}
#else
	typedef int n_sysmutex_type;
	#define N_SYSMUTEX_ZERO_INIT 0
#endif

extern bool nInitThreadsLib(void);
extern bool nDestroyThreadsLib(void);
extern void nUpdateThreadsLib(void);
extern bool nStopAllTasks(void);

extern bool nCreateSystemMutex(n_sysmutex_type *mutex);
extern bool nDestroySystemMutex(n_sysmutex_type *mutex);
extern void nLockSystemMutex(n_sysmutex_type *mutex);
extern bool nTryLockSystemMutex(n_sysmutex_type *mutex);
extern void nUnlockSystemMutex(n_sysmutex_type *mutex);

extern bool n_taskslib_isinit;

#endif
