/*
	Файл	: nyan_array.h

	Описание: Заголовок для nyan_array.c

	История	: 12.03.23	Создан

*/

#include "../../nyan/nyan_engapi.h"

#include <stdbool.h>

#define NYAN_ARRAY_DEFAULT_STEP 64

typedef bool (* nyan_array_check_type)(void *array_el, bool set_free);

extern bool naCheckArrayAlwaysFalse(void *array_el, bool set_free);
extern bool nArrayAdd(engapi_type *ea, void **array, unsigned int *array_max, unsigned int *array_alloc, nyan_array_check_type nyan_array_check, unsigned int *array_addel, unsigned int array_alloc_step, size_t array_el_len);
