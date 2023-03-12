/*
	Файл	: nyan_array.c

	Описание: Массивы, общие функции

	История	: 12.03.23	Создан

*/

#include "nyan_array.h"

#include <stdio.h>

typedef bool (* nyan_array_check_type)(void *array_el, bool set_free);

/*
	Функция	: naCheckArrayAlwaysFalse

	Описание: Проверяет свободное место в массиве, возвращает set_free

	История	: 12.03.23	Создан

*/
bool naCheckArrayAlwaysFalse(void *array_el, bool set_free)
{
	(void)array_el;
	
	return set_free;
}

/*
	Функция	: nArrayAdd

	Описание: Добавляет элемент в массив
	          ea - указатель на структуру движка
	          array - указатель на массив (my_array_t *my_array; (void **)&my_array)
	          array_max - максимальный используемый элемент в массиве (нужно чтобы не
	                      перебирать весь массив в поисках пустого элемента
	          array_alloc - количество элементов, под которые выделена память в массиве
	          nyan_array_check - функция, которая проверяет, что элемент пустой (аргумент
	                             set_free также делает элемент пустым)
	          array_addel - номер добавленного элемента (с нуля)
	          array_alloc_step - количество добавляемых в массив пустых элементов (при их отсутствии)
	          array_el_len - размер пустого элемента
	          Возвращает true если пустой элемент добавлен в массив, false в случае ошибки

	История	: 12.03.23	Создан

*/
bool nArrayAdd(engapi_type *ea, void **array, unsigned int *array_max, unsigned int *array_alloc, nyan_array_check_type nyan_array_check, unsigned int *array_addel, unsigned int array_alloc_step, size_t array_el_len)
{
	unsigned char *_array;
	unsigned int _array_max, _array_alloc, _array_addel;
	bool success = false;
	
	_array = *array;
	_array_max = *array_max;
	_array_alloc = *array_alloc;
	_array_addel = *array_addel;
	
	/*wprintf(L"before\n\t"
		L"s %u max %u alloc %u addel %u array %x\n",
		(unsigned int)success,
		_array_max, _array_alloc, _array_addel, _array);*/
	
	// Если память ещё не была выделена
	if(!_array_alloc) {
		unsigned int i;
		unsigned char *p;
		
		_array = ea->nAllocMemory(array_alloc_step*array_el_len);
		if(!_array) goto N_ARRAY_ADD_EXIT;
		
		_array_alloc = array_alloc_step;
		p = _array;
		for(i = 0; i < _array_alloc; i++) {
			nyan_array_check(p, true);
			p += array_el_len;
		}
	}
	
	// Если свободная память закончилась
	if(_array_alloc == _array_max) {
		unsigned int i;
		unsigned char *p;
		
		// Смотрим, есть ли свободный элемент
		p = _array;
		for(_array_addel = 0; _array_addel < _array_max; _array_addel++) {
			if(nyan_array_check(p, false)) break;

			p += array_el_len;
		}
		
		// Если нет, то выделяем
		if(_array_addel == _array_max) {
			unsigned char *__array;
			
			__array = ea->nReallocMemory(_array, (_array_alloc+array_alloc_step)*array_el_len);
			if(__array)
				_array = __array;
			else
				goto N_ARRAY_ADD_EXIT;
			_array_alloc += array_alloc_step;

			p = _array+_array_max*array_el_len;
			for(i = _array_max; i < _array_alloc; i++) {
				nyan_array_check(p, true);
				p += array_el_len;
			}
			
			_array_max++;
		}
	} else {// Если свободная память есть, то просто берём первый свободный элемент массива
		_array_addel = _array_max;
	
		_array_max++;
	}
	
	success = true;
	
N_ARRAY_ADD_EXIT:
	/*wprintf(L"after\n\t"
		L"s %u max %u alloc %u addel %u array %x\n",
		(unsigned int)success,
		_array_max, _array_alloc, _array_addel, _array);*/
	
	*array = _array;
	*array_max = _array_max;
	*array_alloc = _array_alloc;
	*array_addel = _array_addel;

	return success;
}
