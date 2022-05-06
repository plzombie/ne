/*
	Файл	: nyan_engapi.h

	Описание: Функции движка, которые передаются в используемые движком dll

	История	: 05.08.12	Создан

*/

#ifndef NYAN_ENGAPI_H
#define NYAN_ENGAPI_H

#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

#ifndef N_APIENTRY
#include "nyan_decls_publicapi.h"
#endif

typedef struct {
	unsigned int mysize;
	void (N_APIENTRY *nlAddTab)(int tabs);
	void (N_APIENTRY *nlPrint)(const wchar_t *format, ...);
	long long int (N_APIENTRY *nFileRead)(unsigned int id, void *dst, size_t num);
	long long int (N_APIENTRY *nFileWrite)(unsigned int id, void *src, size_t num);
	long long int (N_APIENTRY *nFileLength)(unsigned int id);
	long long int (N_APIENTRY *nFileTell)(unsigned int id);
	long long int (N_APIENTRY *nFileSeek)(unsigned int id,long long int offset,int origin);
	unsigned int (N_APIENTRY *nFileOpen)(const wchar_t *fname);
	bool (N_APIENTRY *nFileClose)(unsigned int id);
	void * (N_APIENTRY *nAllocMemory)(size_t size);
	void (N_APIENTRY *nFreeMemory)(void *ptr);
	void * (N_APIENTRY *nReallocMemory)(void *ptr, size_t size);
	unsigned int (N_APIENTRY *nCreateTask)(void N_APIENTRY func(void *param), void *args, unsigned int waitingtime, int noffunccalls);
	bool (N_APIENTRY *nDestroyTask)(unsigned int taskid);
	bool (N_APIENTRY *nRunTaskOnAllThreads)(void N_APIENTRY func(void *param), void *args, bool wait_for_running);
	unsigned int (N_APIENTRY *nGetMaxThreadsForTasks)(void);
	bool (N_APIENTRY *nGetNumberOfTaskFunctionCalls)(unsigned int taskid, int *noffunccalls);
	bool (N_APIENTRY *nSetNumberOfTaskFunctionCalls)(unsigned int taskid, int noffunccalls);
	uintptr_t (N_APIENTRY *nCreateMutex)(void);
	bool (N_APIENTRY *nDestroyMutex)(uintptr_t mutexid);
	bool (N_APIENTRY *nLockMutex)(uintptr_t mutexid);
	bool (N_APIENTRY *nUnlockMutex)(uintptr_t mutexid);
	void (N_APIENTRY *nSleep)(unsigned int sleeptime);
	int64_t (N_APIENTRY *nClock)(void);
} engapi_type;

#endif
