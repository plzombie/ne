/*
	Файл	: nyan_container_strings.c

	Описание: Вспомогательные функции для работы со строками

	История	: 04.06.18	Создан

*/

#include "nyan_container_strings.h"

#include <stdbool.h>
#include <limits.h>
#include <stdint.h>

/*
	Функция	: ncfCalculateUTF32ToUTF16StringSize

	Описание: Возвращает количество символов, получающихся при перекодировании строки str размера size из utf32 le в utf16 le. В случае ошибки возвращает 0.

	История	: 06.06.18	Создан

*/
size_t ncfCalculateUTF32ToUTF16StringSize(unsigned int *str, size_t size)
{
	size_t i, ret = 0;

	for(i = 0; i < size; i++) {
		if(str[i] > 0x10FFFF)
			return 0;
		else if(str[i] >= 0x10000) // surrogate pairs
			ret += 2;
		else // single code unit
			ret++;
	}

	return ret;
}

/*
	Функция	: ncfCalculateWideCharToUTF16StringSize

	Описание: Возвращает количество символов, получающихся при перекодировании строки str размера size из widechar в utf16 le. В случае ошибки возвращает 0.

	История	: 04.06.18	Создан
		06.06.18 Переименована из ncfCalculateUTF16StringSize в ncfCalculateWideCharToUTF16StringSize

*/
size_t ncfCalculateWideCharToUTF16StringSize(wchar_t *str, size_t size)
{
#ifndef N_WCHAR32
	(void)str;

	return size;
#else
	return ncfCalculateUTF32ToUTF16StringSize((unsigned int *)str, size);
#endif
}

/*
	Функция	: ncfCalculateUTF16ToUTF32StringSize

	Описание: Возвращает количество символов, получающихся при перекодировании строки str размера size из utf16 le в utf32 le. В случае ошибки возвращает 0.

	История	: 04.06.18	Создан

*/
size_t ncfCalculateUTF16ToUTF32StringSize(unsigned short *str, size_t size)
{
	size_t i, ret = 0;
	bool surrogate = false;

	for(i = 0; i < size; i++) {
		if(str[i] >= 0xD800 && str[i] <= 0xDBFF) { // High (leading) surrogate
			if(surrogate)
				return 0;

			surrogate = true;
		} else if(str[i] >= 0xDC00 && str[i] <= 0xDFFF) { // Low (trailing) surrogate
			if(!surrogate)
				return 0;

			surrogate = false;
			ret++;
		} else { // single code unit
			if(surrogate)
				return 0;

			ret++;
		}
	}

	return ret;
}

/*
	Функция	: ncfCalculateUTF16ToWideCharStringSize

	Описание: Возвращает количество символов, получающихся при перекодировании строки str размера size из utf16 le в widechar. В случае ошибки возвращает 0.

	История	: 04.06.18	Создан
		Переименована из ncfCalculateWideCharStringSize в ncfCalculateUTF16ToWideCharStringSize

*/
size_t ncfCalculateUTF16ToWideCharStringSize(unsigned short *str, size_t size)
{
#ifndef N_WCHAR32
	(void) str;

	return size;
#else
	return ncfCalculateUTF16ToUTF32StringSize(str, size);
#endif
}

/*
	Функция	: ncfUTF16ToUTF32

	Описание: Конвертирует строку inp_str длиной inp_size символов UTF16 LE в строку out_str длиной out_size символов UTF32 LE.
		Если получившаяся строка out_str короче out_size символов, то дополняет её нулями
		В случае успеха возвращает NCF_SUCCESS, иначе NCF_FAIL

	История	: 06.06.18	Создан

*/
unsigned int ncfUTF16ToUTF32(unsigned int *out_str, size_t out_size, unsigned short *inp_str, size_t inp_size)
{
	size_t i;
	size_t j = 0;
	bool surrogate = false;
	
	if(out_size < inp_size)
		return NCF_FAIL;
		
	for(i = 0; i < inp_size; i++) {
		if(inp_str[i] >= 0xD800 && inp_str[i] <= 0xDBFF) { // High (leading) surrogate
			if(surrogate)
				return NCF_FAIL;

			surrogate = true;
			out_str[j] = ((wchar_t)((inp_str[i] - 0xD800)&0x3FF) << 10) + 0x10000;
		} else if(inp_str[i] >= 0xDC00 && inp_str[i] <= 0xDFFF) { // Low (trailing) surrogate
			if(!surrogate)
				return NCF_FAIL;

			surrogate = false;
			out_str[j] += (inp_str[i] - 0xDC00) & 0x3FF;
			j++;
		} else { // BMP (codepoints from UCS-2)
			if(surrogate)
				return NCF_FAIL;

			out_str[j] = inp_str[i];
			j++;
		}
	}

	for(i = j; i < out_size; i++)
		out_str[i] = 0;
	
	return NCF_SUCCESS;
}

/*
	Функция	: ncfUTF16ToWideChar

	Описание: Конвертирует строку inp_str длиной inp_size символов UTF16 LE в строку out_str длиной out_size символов wchar_t.
		Если получившаяся строка out_str короче out_size символов, то дополняет её нулями
		В случае успеха возвращает NCF_SUCCESS, иначе NCF_FAIL

	История	: 04.06.18	Создан

*/
unsigned int ncfUTF16ToWideChar(wchar_t *out_str, size_t out_size, unsigned short *inp_str, size_t inp_size)
{
#ifndef N_WCHAR32
	size_t i;

	if(out_size < inp_size)
		return NCF_FAIL;

	for(i = 0; i < inp_size; i++)
		out_str[i] = inp_str[i];

	for(i = inp_size; i < out_size; i++)
		out_str[i] = 0;
		
	return NCF_SUCCESS;
#else
	return ncfUTF16ToUTF32((unsigned int *)out_str, out_size, inp_str, inp_size);
#endif
}

/*
	Функция	: ncfUTF32ToUTF16

	Описание: Конвертирует строку inp_str длиной inp_size символов UTF32 LE в строку out_str длиной out_size символов UTF16 LE.
		Если размер сконвертированной строки длиннее out_size символов 
		(например inp_size > out_size или inp_size == out_size и часть символов из inp_str записывается суррогатной парой),
		то возвращает NCF_FAIL.
		Если inp_size короче out_size, то дополняет строку нулями.
		В случае успеха возвращает NCF_SUCCESS, иначе NCF_FAIL

	История	: 06.06.18	Создан

*/
unsigned int ncfUTF32ToUTF16(unsigned short *out_str, size_t out_size, unsigned int *inp_str, size_t inp_size)
{
	size_t i;
	size_t j = 0;
	
	if(out_size < inp_size)
		return NCF_FAIL;
		
	for(i = 0; i < inp_size; i++) {
		if(inp_str[i] >= 0x10000) {
			if((j+2) > out_size)
				return NCF_FAIL;

			out_str[j] = (((inp_str[i]-0x10000)>>10)&0x3FF)+0xD800;
			j++;
			out_str[j] = ((inp_str[i]-0x10000)&0x3FF)+0xDC00;
			j++;
		} else {
			if((j+1) > out_size)
				return NCF_FAIL;

			out_str[j] = inp_str[i];
			j++;
		}
	}

	for(i = j; i < out_size; i++)
		out_str[i] = 0;
	
	return NCF_SUCCESS;
}

/*
	Функция	: ncfWideCharToUTF16

	Описание: Конвертирует строку inp_str длиной inp_size символов widechar в строку out_str длиной out_size символов UTF16 LE.
		Если размер сконвертированной строки длиннее out_size символов 
		(например inp_size > out_size или inp_size == out_size и часть символов из inp_str записывается суррогатной парой),
		то возвращает NCF_FAIL.
		Если inp_size короче out_size, то дополняет строку нулями.
		В случае успеха возвращает NCF_SUCCESS, иначе NCF_FAIL

	История	: 04.06.18	Создан

*/
unsigned int ncfWideCharToUTF16(unsigned short *out_str, size_t out_size, wchar_t *inp_str, size_t inp_size)
{
#ifndef N_WCHAR32
	size_t i;

	if(out_size < inp_size)
		return NCF_FAIL;

	for(i = 0; i < inp_size; i++)
		out_str[i] = inp_str[i];

	for(i = inp_size; i < out_size; i++)
		out_str[i] = 0;
		
	return NCF_SUCCESS; 
#else
	return ncfUTF32ToUTF16(out_str, out_size, (unsigned int *)inp_str, inp_size);
#endif
}
