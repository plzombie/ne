/*
	Файл	: nyan_mem.c

	Описание: Управление памятью

	История	: 18.08.12	Создан

*/

#include <stdlib.h>

#include "nyan_decls_publicapi.h"

#include "nyan_text.h"

#include "nyan_mem.h"
#include "nyan_mem_publicapi.h"
#include "nyan_log_publicapi.h"

/*
	Функция	: nAllocMemory

	Описание: Аналог malloc

	История	: 18.08.12	Создан

*/
N_API void * N_APIENTRY_EXPORT nAllocMemory(size_t size)
{
	void *_ptr;
	
	if(size)
		_ptr = malloc(size);
	else
		_ptr = 0;
	
	if(!_ptr)
		nlPrint(LOG_FDEBUGFORMAT9, F_NALLOCMEMORY, ERR_CANTALLOCMEM, (long long)size);

	// For debug only
	//nlPrint(LOG_FDEBUGFORMAT9, F_NALLOCMEMORY, L"Allocate memory, size", (long long)size);
	//nlPrint(LOG_FDEBUGFORMAT9, F_NALLOCMEMORY, L"Allocated memory ptr", (long long)_ptr);

	return _ptr;
}

/*
	Функция	: nFreeMemory

	Описание: Аналог free

	История	: 18.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nFreeMemory(void *ptr)
{
	if(!ptr)
		nlPrint(LOG_FDEBUGFORMAT, F_NFREEMEMORY, ERR_TRYINGTOFREENULLPTR);

	// For debug only
	//nlPrint(LOG_FDEBUGFORMAT9, F_NALLOCMEMORY, L"Free memory, ptr", (long long)ptr);

	free(ptr);
}

/*
	Функция	: nReallocMemory

	Описание: Аналог realloc

	История	: 18.08.12	Создан

*/
N_API void * N_APIENTRY_EXPORT nReallocMemory(void *ptr, size_t size)
{
	void *_ptr;
	
	if(size)
		_ptr = realloc(ptr, size);
	else
		_ptr = 0;
	
	if(!_ptr)
		nlPrint(LOG_FDEBUGFORMAT9, F_NREALLOCMEMORY, ERR_CANTALLOCMEM, (long long) size);

	// For debug only
	//nlPrint(LOG_FDEBUGFORMAT9, F_NREALLOCMEMORY, L"Rellocate memory, size", (long long)size);
		
	return _ptr;
}
