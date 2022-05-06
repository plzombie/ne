/*
	Файл	: nyan_container_strings.h

	Описание: Заголовочный файл для вспомогательных функций для работы со строками

	История	: 04.06.18	Создан

*/

#ifndef NYAN_CONTAINER_STRINGS_H
#define NYAN_CONTAINER_STRINGS_H

#include <stddef.h>
#include <wchar.h>

#include "nyan_container_defs.h"

extern size_t ncfCalculateUTF32ToUTF16StringSize(unsigned int *str, size_t size);
extern size_t ncfCalculateWideCharToUTF16StringSize(wchar_t *str, size_t size);
extern size_t ncfCalculateUTF16ToUTF32StringSize(unsigned short *str, size_t size);
extern size_t ncfCalculateUTF16ToWideCharStringSize(unsigned short *str, size_t size);
extern unsigned int ncfUTF16ToUTF32(unsigned int *out_str, size_t out_size, unsigned short *inp_str, size_t inp_size);
extern unsigned int ncfUTF16ToWideChar(wchar_t *out_str, size_t out_size, unsigned short *inp_str, size_t inp_size);
extern unsigned int ncfUTF32ToUTF16(unsigned short *out_str, size_t out_size, unsigned int *inp_str, size_t inp_size);
extern unsigned int ncfWideCharToUTF16(unsigned short *out_str, size_t out_size, wchar_t *inp_str, size_t inp_size);

#endif
