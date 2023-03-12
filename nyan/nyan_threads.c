/*
	Файл	: nyan_tasks.c

	Описание: Работа с потоками

	История	: 28.05.13	Создан

*/

#ifndef _DEBUG
	#ifndef NDEBUG
		#define NDEBUG
	#endif
#endif
#include <assert.h>

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#define STACK_SIZE    8192

#define N_THREADLIB_WASTINGTIME 20

#include "nyan_decls_publicapi.h"

#include "../commonsrc/core/nyan_array.h"
#include "nyan_apifordlls.h"

#include "nyan_text.h"
#include "nyan_threads.h"
#include "nyan_threads_publicapi.h"
#include "nyan_log_publicapi.h"
#include "nyan_fps_publicapi.h"
#include "nyan_mem_publicapi.h"

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
	static void Sleep(int ms)
	{
		struct timespec duration, remaining;

		duration.tv_nsec = (ms%1000)*1000000;
		duration.tv_sec = ms/1000;

		while(thrd_sleep(&duration, &remaining) == -1) {
			duration = remaining;
		}
	}
#elif defined(N_WINDOWS)
	#define WIN32_LEAN_AND_MEAN
	#ifndef UNICODE
		#define UNICODE
	#endif
	#include <windows.h>
	#include <process.h>
#elif defined(N_POSIX) || defined(N_ANDROID)
	#include <unistd.h>
	#include <pthread.h>
	static void Sleep(int ms)
	{
		usleep(ms*1000);
	}
#else
	#include <time.h>
	static void Sleep(int ms)
	{
		int64_t start;
		start = nClock();

		while((nClock()-start)<ms*N_CLOCKS_PER_SEC/1000);
	}
#endif

typedef struct {
	void (N_APIENTRY *func)(void *param);
	void *args;
	unsigned int waitingtime;
	int noffunccalls;
	int64_t lastcall;
	bool used;
} n_task_type;

typedef struct {
	size_t size;
	n_sysmutex_type mutex;
	//unsigned int locked_times;
} n_mutex_type;

static n_task_type *n_tasks = 0;
static unsigned int n_maxtasks = 0;
static unsigned int n_alloctasks = 0;

static unsigned int n_nofrealthreads = 0; // Количество реальных потоков, которые были созданы в системе

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
	static int nThreadsLib_threadproc(void *args);
	static thrd_t n_thlib_localthread;
#elif defined(N_WINDOWS)
	static HANDLE n_thlib_localthread;
	/*pure winapi version (do not use)*///static DWORD WINAPI nThreadsLib_threadproc(/*_In_  */LPVOID args);
	/*crt version*/static unsigned __stdcall nThreadsLib_threadproc(void *args);
#elif defined(N_POSIX) || defined(N_ANDROID)
	static void *nThreadsLib_threadproc(void *args);
	static pthread_t n_thlib_localthread;
#endif
static n_sysmutex_type n_thlib_localmutex; // Мьютекс, используемый для синхронизации потоков

bool n_taskslib_isinit = false;

/*
	Функция	: nInitThreadsLib

	Описание: Инициализирует библиотеку работы с потоками

	История	: 28.05.13	Создан

*/
bool nInitThreadsLib(void)
{
	if(n_taskslib_isinit) return false;

	nlPrint(F_NINITTHREADSLIB); nlAddTab(1);

	if(!nCreateSystemMutex(&n_thlib_localmutex)) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NINITTHREADSLIB, N_FALSE);
		return false;
	}

	n_taskslib_isinit = true;

	n_nofrealthreads = 0;
	n_maxtasks = 0;
	n_alloctasks = 0;
#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
	if(thrd_create(&n_thlib_localthread, nThreadsLib_threadproc, NULL) == thrd_success)
		n_nofrealthreads = 1;
#elif defined(N_WINDOWS)
	/*pure winapi version (do not use)*///n_thlib_localthread = CreateThread(0, STACK_SIZE, nThreadsLib_threadproc, 0, 0, NULL);
	/*crt version*/n_thlib_localthread = (HANDLE)_beginthreadex(NULL, STACK_SIZE, nThreadsLib_threadproc, NULL, 0, NULL);
	if(n_thlib_localthread)
		n_nofrealthreads = 1;
#elif defined(N_POSIX) || defined(N_ANDROID)
	if(!pthread_create(&n_thlib_localthread, NULL, nThreadsLib_threadproc, NULL))
		n_nofrealthreads = 1;
#endif

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NINITTHREADSLIB, N_OK);

	return n_taskslib_isinit;
}

/*
	Функция	: nDestroyThreadsLib

	Описание: Деинициализирует библиотеку работы с потоками

	История	: 28.05.13	Создан

*/
bool nDestroyThreadsLib(void)
{
	if(!n_taskslib_isinit) return false;

	nlPrint(F_NDESTROYTHREADSLIB); nlAddTab(1);

	nLockSystemMutex(&n_thlib_localmutex);

	if(n_alloctasks) {
		unsigned int i;

		for(i = 0; i < n_maxtasks; i++)
			if(n_tasks[i].used)
				nDestroyTask(i+1);

		nFreeMemory(n_tasks);
		n_tasks = 0;
		n_alloctasks = 0;
		n_maxtasks = 0;
	}

	n_taskslib_isinit = false;

	nUnlockSystemMutex(&n_thlib_localmutex);

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
	if(n_nofrealthreads)
		thrd_join(n_thlib_localthread, NULL);
#elif defined(N_WINDOWS)
	if(n_nofrealthreads) {
		WaitForSingleObject(n_thlib_localthread, INFINITE);
		CloseHandle(n_thlib_localthread);
	}
#elif defined(N_POSIX) || defined(N_ANDROID)
	if(n_nofrealthreads)
		pthread_join(n_thlib_localthread, 0);
#endif
	n_nofrealthreads = 0;
	nDestroySystemMutex(&n_thlib_localmutex);

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NDESTROYTHREADSLIB, N_OK);

	return !n_taskslib_isinit;
}

/*
	Функция	: nStopAllTasks

	Описание: Останавливает все задачи (нужно перед деинициализацией библиотеки)

	История	: 25.02.18	Создан

*/
bool nStopAllTasks(void)
{
	if (!n_taskslib_isinit) return false;

	nLockSystemMutex(&n_thlib_localmutex);

	if(n_maxtasks) {
		unsigned int i;

		for(i = 0; i < n_maxtasks; i++)
			if(n_tasks[i].used)
				nSetNumberOfTaskFunctionCalls(i + 1, 0);
	}

	nUnlockSystemMutex(&n_thlib_localmutex);

	return true;
}

/*
	Функция	: nUpdateThreadsWithStep

	Описание: Вспомогательная функция. Выполняет потоки в следующей последовательности:
			st, st+step, st+step+step, ...
			и так далее, пока не перевысит n_maxtasks

	История	: 29.03.14	Создан
*/
unsigned int nUpdateThreadsWithStep(unsigned int st, unsigned int step)
{
	unsigned int i, j = 0;

	for(i = st; i < n_maxtasks; i+=step) {
		if(n_tasks[i].used)
			if(n_tasks[i].noffunccalls)
				if( (nClock()-n_tasks[i].lastcall) >= (int64_t)n_tasks[i].waitingtime*N_CLOCKS_PER_SEC/1000 ) {
					n_tasks[i].func(n_tasks[i].args);
					n_tasks[i].lastcall = nClock();
					if(n_tasks[i].noffunccalls > 0)
						n_tasks[i].noffunccalls--;
					j++;
				}
	}

	return j;
}

/*
	Функция	: nUpdateThreadsLib

	Описание: Выполняет потоки, если мы в системе, где невозможно организовать многопоточность

	История	: 29.03.14	Создан

*/
void nUpdateThreadsLib(void)
{
	if(!n_taskslib_isinit) return;

	if(!n_nofrealthreads) {
		nUpdateThreadsWithStep(0, 1);
	}
}

#if defined(N_WINDOWS) || defined(N_POSIX) || defined(N_ANDROID) || (__STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__))

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
int nThreadsLib_threadproc(void *args)
#elif defined(N_WINDOWS)
/*pure winapi version (do not use)*///DWORD WINAPI nThreadsLib_threadproc(/*_In_  */LPVOID args)
/*crt version*/unsigned __stdcall nThreadsLib_threadproc(void *args)
#elif defined(N_POSIX) || defined(N_ANDROID)
void *nThreadsLib_threadproc(void *args)
#endif
{
	(void)args; // Неиспользуемая переменная

	while(1) {
		unsigned int processed_threads = 0;

		nLockSystemMutex(&n_thlib_localmutex);

		if(!n_taskslib_isinit) {
			nUnlockSystemMutex(&n_thlib_localmutex);

			break;
		}

		processed_threads = nUpdateThreadsWithStep(0, 1);

		//Зачем я вообще написал эту строчку?nUnlockSystemMutex(&n_thlib_localmutex2);
		nUnlockSystemMutex(&n_thlib_localmutex);

		//nlPrint(L"processed_threads %u", processed_threads);
		if(!processed_threads)
			Sleep(N_THREADLIB_WASTINGTIME);
	}

	return 0;
}
#endif

/*
	Функция	: naCheckTaskArray

	Описание: Проверяет свободное место для задачи в массиве

	История	: 12.03.23	Создан

*/
static bool naCheckTaskArray(void *array_el, bool set_free)
{
	n_task_type *el;
	
	el = (n_task_type *)array_el;
	
	if(set_free) el->used = false;
	
	return (el->used == false)?true:false;
}

/*
	Функция	: nCreateTask

	Описание: Создаёт задачу

	История	: 28.05.13	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nCreateTask(void N_APIENTRY func(void *param), void *args, unsigned int waitingtime, int noffunccalls)
{
	unsigned int i = 0;

	if(!n_taskslib_isinit) return 0;

	nlPrint(F_NCREATETASK); nlAddTab(1);

	nLockSystemMutex(&n_thlib_localmutex);

	// Выделение памяти под потоки
	if(!nArrayAdd(
		&n_ea, (void **)(&n_tasks),
		&n_maxtasks,
		&n_alloctasks,
		naCheckTaskArray,
		&i,
		(n_nofrealthreads < 32)?32:n_nofrealthreads,
		sizeof(n_task_type))
	) {
		nUnlockSystemMutex(&n_thlib_localmutex);
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NCREATETASK, N_FALSE, N_ID, 0);
		return 0;
	}

	n_tasks[i].func = func;
	n_tasks[i].args = args;
	n_tasks[i].waitingtime = waitingtime;
	n_tasks[i].noffunccalls = noffunccalls;
	n_tasks[i].lastcall = nClock();
	n_tasks[i].used = true;

	nUnlockSystemMutex(&n_thlib_localmutex);

	i++;

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NCREATETASK, N_OK, N_ID, i);

	return i;
}

/*
	Функция	: nDestroyTask

	Описание: Уничтожает задачу

	История	: 28.05.13	Создан

*/
N_API bool N_APIENTRY_EXPORT nDestroyTask(unsigned int taskid)
{

	if(!n_taskslib_isinit) return false;

	nlPrint(LOG_FDEBUGFORMAT4, F_NDESTROYTASK, N_ID, taskid); nlAddTab(1);

	nLockSystemMutex(&n_thlib_localmutex);

	if(taskid == 0 || taskid > n_maxtasks) {
		nUnlockSystemMutex(&n_thlib_localmutex);
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NDESTROYTASK, N_FALSE);

		return false;
	}

	n_tasks[taskid-1].used = false;

	nUnlockSystemMutex(&n_thlib_localmutex);

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NDESTROYTASK, N_OK);

	return true;
}

/*
	Функция	: nRunTaskOnAllThreads

	Описание: Запускает задачу один раз на всех потоках. Ждёт выполнения, если wait_for_running равен true. В случае успеха возвращает true, иначе - false.

	История	: 07.12.17	Создан

*/
N_API bool N_APIENTRY_EXPORT nRunTaskOnAllThreads(void N_APIENTRY func(void *param), void *args, bool wait_for_running)
{
	unsigned int i, j;

	if(!n_taskslib_isinit) return false;

	nlPrint(F_NRUNTASKONALLTHREADS); nlAddTab(1);

	nLockSystemMutex(&n_thlib_localmutex);

	// Поиск неиспользуемых задач
	// В текущей реализации, поток номер m обрабатывает задачи n_nofrealthreads*i+m, где i = 1,2,3....
	// Так что надо найти n_nofrealthreads свободных задач подряд
	if(n_alloctasks) {
		for(i = 0; i <= n_alloctasks-n_nofrealthreads; i++) {
			for(j = 0; j < n_nofrealthreads; j++)
				if(n_tasks[i+j].used)
					break;

			if(j == n_nofrealthreads)
				break;
		}
	}

	// Поиск закончился неудачно. Выделяем память под новые задачи.
	if(i == n_alloctasks-n_nofrealthreads+1) {
		n_task_type *_n_tasks = 0;
		
		if(UINT_MAX-n_nofrealthreads >= n_alloctasks)
			_n_tasks = nReallocMemory(n_tasks, (n_alloctasks+n_nofrealthreads)*sizeof(n_task_type));
		
		if(_n_tasks) {
			i = n_alloctasks;
			n_alloctasks += n_nofrealthreads;
			n_maxtasks = n_alloctasks;
			n_tasks = _n_tasks;
		} else {
			nUnlockSystemMutex(&n_thlib_localmutex);
			return false;
		}
	} else if(i+n_nofrealthreads > n_maxtasks) // Поиск закончился удачно. Проверяем, не нужно ли нам сдвинуть максимальное количество задач
		n_maxtasks = i+n_nofrealthreads;

	// Установка задач.
	for(j = 0; j < n_nofrealthreads; j++) {
		n_tasks[i+j].func = func;
		n_tasks[i+j].args = args;
		n_tasks[i+j].waitingtime = 0;
		n_tasks[i+j].noffunccalls = 1;
		n_tasks[i+j].lastcall = nClock();
		n_tasks[i+j].used = true;
		nlPrint(LOG_FDEBUGFORMAT3, F_NRUNTASKONALLTHREADS, N_ID, i+j+1);
	}

	nUnlockSystemMutex(&n_thlib_localmutex);

	if(wait_for_running) {
		while(1) {
			nLockSystemMutex(&n_thlib_localmutex);
			for(j = 0; j < n_nofrealthreads; j++) {
				if((n_tasks[i + j].func == func) && (n_tasks[i+j].args == args) && (n_tasks[i+j].used == true) && (n_tasks[i+j].noffunccalls > 0))
					break;
			}
			nUnlockSystemMutex(&n_thlib_localmutex);
			if(j == n_nofrealthreads)
				break;
			Sleep(N_THREADLIB_WASTINGTIME);
		}
	}

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NRUNTASKONALLTHREADS, N_OK);

	return true;
}

/*
	Функция	: nGetMaxThreadsForTasks

	Описание: Возвращает максимальное количество потоков (кроме главного потока программы), которые могут использоваться для выполнения задач

	История	: 14.11.17	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nGetMaxThreadsForTasks(void)
{
	if(!n_taskslib_isinit)
		return 0;

	return n_nofrealthreads;
}

/*
	Функция	: nGetNumberOfTaskFunctionCalls

	Описание: Возвращает количество вызовов функции задачи

	История	: 07.08.14	Создан

*/
N_API bool N_APIENTRY_EXPORT nGetNumberOfTaskFunctionCalls(unsigned int taskid, int *noffunccalls)
{

	if(!n_taskslib_isinit) return false;

	nLockSystemMutex(&n_thlib_localmutex);

	if(taskid == 0 || taskid > n_maxtasks) {
		nUnlockSystemMutex(&n_thlib_localmutex);

		return false;
	}

	*noffunccalls = n_tasks[taskid-1].noffunccalls;

	nUnlockSystemMutex(&n_thlib_localmutex);

	return true;
}

/*
	Функция	: nSetNumberOfTaskFunctionCalls

	Описание: Задаёт количество вызовов функции задачи

	История	: 07.08.14	Создан

*/
N_API bool N_APIENTRY_EXPORT nSetNumberOfTaskFunctionCalls(unsigned int taskid, int noffunccalls)
{

	if(!n_taskslib_isinit) return false;

	nLockSystemMutex(&n_thlib_localmutex);

	if(taskid == 0 || taskid > n_maxtasks) {
		nUnlockSystemMutex(&n_thlib_localmutex);

		return false;
	}

	n_tasks[taskid-1].noffunccalls = noffunccalls;

	nUnlockSystemMutex(&n_thlib_localmutex);

	return true;
}

/*
	Функция	: nCreateMutex

	Описание: Создаёт мьютекс

	История	: 12.07.14	Создан

*/
N_API uintptr_t N_APIENTRY_EXPORT nCreateMutex(void)
{
	uintptr_t result = 0;
	n_mutex_type *data;

	if(!n_taskslib_isinit) return 0;

	nlPrint(F_NCREATEMUTEX); nlAddTab(1);

	data = nAllocMemory(sizeof(n_mutex_type));
	if(data) {
		if(nCreateSystemMutex(&(data->mutex))) {
			data->size = sizeof(n_mutex_type);
			//data->locked_times = 0;
			result = (uintptr_t)data;
		} else
			nFreeMemory(data);
	}

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT10, F_NCREATEMUTEX, N_OK, N_ID, (long long)result);

	return result;
}

/*
	Функция	: nDestroyMutex

	Описание: Уничтожает мьютекс

	История	: 12.07.14	Создан

*/
N_API bool N_APIENTRY_EXPORT nDestroyMutex(uintptr_t mutexid)
{
	n_mutex_type *data;
	bool success = false;

	if(!n_taskslib_isinit) return false;

	if(!mutexid) { nlPrint(LOG_FDEBUGFORMAT, F_NDESTROYMUTEX, ERR_TRYINGTOUSENULLOBJECT); return false; }

	data = (n_mutex_type *)mutexid;

	if(data->size != sizeof(n_mutex_type))
		return false;

	nlPrint(LOG_FDEBUGFORMAT9, F_NDESTROYMUTEX, N_ID, (long long)mutexid); nlAddTab(1);

	if(nDestroySystemMutex(&(data->mutex))) {
		data->size = 0;
		nFreeMemory(data);
		success = true;
	}

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NDESTROYMUTEX, (success)?(N_OK):(N_FALSE));

	return true;
}

/*
	Функция	: nLockMutex

	Описание: Захватывает мьютекс

	История	: 12.07.14	Создан

*/
N_API bool N_APIENTRY_EXPORT nLockMutex(uintptr_t mutexid)
{
	n_mutex_type *data;

	if(!n_taskslib_isinit) return false;

	if(!mutexid) return false;

	data = (n_mutex_type *)mutexid;

	if(data->size != sizeof(n_mutex_type))
		return false;

	//wprintf(L"thread %d trying to lock mutex %lld\n", GetCurrentThreadId(), (long long)mutexid);

	nLockSystemMutex(&(data->mutex));

	//wprintf(L"thread %d locked mutex %lld\n", GetCurrentThreadId(), (long long)mutexid);

	return true;
}

/*
Функция	: nUnlockMutex

Описание: Освобождает мьютекс

История	: 12.07.14	Создан

*/
N_API bool N_APIENTRY_EXPORT nUnlockMutex(uintptr_t mutexid)
{
	n_mutex_type *data;

	if(!n_taskslib_isinit) return false;

	if(!mutexid) return false;

	data = (n_mutex_type *)mutexid;

	if(data->size != sizeof(n_mutex_type))
		return false;

	//wprintf(L"thread %d trying to unlock mutex %lld\n", GetCurrentThreadId(), (long long)mutexid);

	nUnlockSystemMutex(&(data->mutex));

	//wprintf(L"thread %d unlocked mutex %lld\n", GetCurrentThreadId(), (long long)mutexid);

	return true;
}

/*
Функция	: nCreateSystemMutex

Описание: Создаёт системный мьютекс

История	: 12.07.14	Создан

*/
bool nCreateSystemMutex(n_sysmutex_type *mutex)
{
#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
	if(mtx_init(mutex, mtx_plain|mtx_recursive) == thrd_success)
		return true;
	else
		return false;
#elif defined(N_WINDOWS)
	*mutex = CreateMutex(NULL, FALSE, NULL);

	if(*mutex)
		return true;
	else
		return false;
#elif defined(N_POSIX) || defined(N_ANDROID)
	pthread_mutexattr_t n_thlib_mutexattr;

	if(pthread_mutexattr_init(&n_thlib_mutexattr))
		return false;
	pthread_mutexattr_settype(&n_thlib_mutexattr, PTHREAD_MUTEX_RECURSIVE);
	if(pthread_mutex_init(mutex, &n_thlib_mutexattr)) {
		pthread_mutexattr_destroy(&n_thlib_mutexattr);

		return false;
	}

	pthread_mutexattr_destroy(&n_thlib_mutexattr);

	return true;
#else
	return true;
#endif
}

/*
Функция	: nDestroySystemMutex

Описание: Уничтожает системный мьютекс

История	: 12.07.14	Создан

*/
bool nDestroySystemMutex(n_sysmutex_type *mutex)
{
#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
	mtx_destroy(mutex);

	return true;
#elif defined(N_WINDOWS)
	if(*mutex) {
		if(CloseHandle(*mutex)) {
			*mutex = 0;
			return true;
		} else
			return false;
	} else {
		nlPrint(LOG_FDEBUGFORMAT, F_NDESTROYSYSTEMMUTEX, ERR_TRYINGTOUSENULLOBJECT);

		return false;
	}
#elif defined(N_POSIX) || defined(N_ANDROID)
	if(pthread_mutex_destroy(mutex))
		return false;

	return true;
#else
	return true;
#endif
}

/*
Функция	: nLockSystemMutex

Описание: Захватывает системный мьютекс

История	: 12.07.14	Создан

*/
void nLockSystemMutex(n_sysmutex_type *mutex)
{
	//wprintf(L"n  LockSystemMutex ptr:%lld thread_id:%d\n", (long long)mutex, GetCurrentThreadId());
#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
	mtx_lock(mutex); // Нужна проверка на возврат thrd_success
#elif defined(N_WINDOWS)
	assert(*mutex != 0);
	WaitForSingleObject(*mutex, INFINITE);
#elif defined(N_POSIX) || defined(N_ANDROID)
	pthread_mutex_lock(mutex);
#endif
}

/*
Функция	: nTryToLockSystemMutex

Описание: Пытается захватить системный мьютекс

История	: 12.07.14	Создан

*/
bool nTryLockSystemMutex(n_sysmutex_type *mutex)
{
	//wprintf(L"n  LockSystemMutex ptr:%lld thread_id:%d\n", (long long)mutex, GetCurrentThreadId());
#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
	if(mtx_trylock(mutex) == thrd_success)
		return true;
	else
		return false;
#elif defined(N_WINDOWS)
	assert(*mutex != 0);
	if(WaitForSingleObject(*mutex, 0) == WAIT_OBJECT_0)
		return true;
	else
		return false;
#elif defined(N_POSIX) || defined(N_ANDROID)
	if(!pthread_mutex_trylock(mutex)) // == NULL
		return true;
	else // == EBUSY
		return false;
#else
	return true;
#endif
}

/*
Функция	: nUnlockSystemMutex

Описание: Освобождает системный мьютекс

История	: 12.07.14	Создан

*/
void nUnlockSystemMutex(n_sysmutex_type *mutex)
{
	//wprintf(L"nUnlockSystemMutex ptr:%lld thread_id:%d\n", (long long)mutex, GetCurrentThreadId());
#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
	mtx_unlock(mutex); // Нужна проверка на возврат thrd_success
#elif defined(N_WINDOWS)
	assert(*mutex != 0);
	ReleaseMutex(*mutex);
#elif defined(N_POSIX) || defined(N_ANDROID)
	pthread_mutex_unlock(mutex);
#endif
}

/*
Функция	: nSleep

Описание: Ждёт не менее sleeptime миллисекунд

История	: 12.07.14	Создан

*/
N_API void N_APIENTRY_EXPORT nSleep(unsigned int sleeptime)
{
	Sleep(sleeptime);
}
