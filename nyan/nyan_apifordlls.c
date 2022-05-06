/*
	Файл	: nyan_apifordlls.c

	Описание: API для dll, используемых движком

	История	: 05.08.12	Создан

*/

#include <stdio.h>

#include "nyan_filesys_publicapi.h"
#include "nyan_fps_publicapi.h"
#include "nyan_log_publicapi.h"
#include "nyan_mem_publicapi.h"
#include "nyan_threads_publicapi.h"

#include "nyan_apifordlls.h"

engapi_type n_ea = { sizeof(engapi_type), nlAddTab, nlPrint, nFileRead, nFileWrite, nFileLength, nFileTell, nFileSeek, nFileOpen, nFileClose, nAllocMemory, nFreeMemory, nReallocMemory,
	nCreateTask, nDestroyTask, nRunTaskOnAllThreads, nGetMaxThreadsForTasks, nGetNumberOfTaskFunctionCalls, nSetNumberOfTaskFunctionCalls, nCreateMutex, nDestroyMutex, nLockMutex, nUnlockMutex, nSleep, nClock};
