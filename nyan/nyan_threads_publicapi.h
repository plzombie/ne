/*
	Файл	: nyan_threads_publicapi.h

	Описание: Публичные функции для работы с потоками

	История	: 28.05.13	Создан

*/

#ifndef NYAN_THREADS_PUBLICAPI_H
#define NYAN_THREADS_PUBLICAPI_H

#include <stdbool.h>
#include <stdint.h>

#include "nyan_decls_publicapi.h"

NYAN_FUNC(unsigned int, nCreateTask, (void N_APIENTRY func(void *param), void *args, unsigned int waitingtime, int noffunccalls))
NYAN_FUNC(bool, nDestroyTask, (unsigned int taskid))
NYAN_FUNC(bool, nRunTaskOnAllThreads, (void N_APIENTRY func(void *param), void *args, bool wait_for_running))
NYAN_FUNC(unsigned int, nGetMaxThreadsForTasks, (void))
NYAN_FUNC(bool, nGetNumberOfTaskFunctionCalls, (unsigned int taskid, int *noffunccalls))
NYAN_FUNC(bool, nSetNumberOfTaskFunctionCalls, (unsigned int taskid, int noffunccalls))

NYAN_FUNC(uintptr_t, nCreateMutex, (void))
NYAN_FUNC(bool, nDestroyMutex, (uintptr_t mutexid))
NYAN_FUNC(bool, nLockMutex, (uintptr_t mutexid))
NYAN_FUNC(bool, nUnlockMutex, (uintptr_t mutexid))

NYAN_FUNC(void, nSleep, (unsigned int sleeptime))

#endif
