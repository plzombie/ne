/*
	Файл	: nyan_container_ne_helpers.h

	Описание: Заголовок для вспомогательных функций для работы с мультимедиа контейнером движка

	История	: 23.03.16	Создан

*/

#ifndef NYAN_CONTAINER_NE_HELPERS_H
#define NYAN_CONTAINER_NE_HELPERS_H

#include <stddef.h>
#include <wchar.h>

#include "nyan_container.h"

#include "nyan_container_defs.h"

extern unsigned int ncfReadAndCheckFiletypeChunk(unsigned int f, nyan_filetype_type *filetype, char *typestr, unsigned short reader_version);
extern unsigned int ncfSeekForChunk(unsigned int f, nyan_chunkhead_type *chunkhead, char *chunkname);
extern unsigned int ncfSeekForChunkAndCheckMinSize(unsigned int f, nyan_chunkhead_type *chunkhead, char *chunkname, unsigned long long minchunksize);
extern unsigned int ncfReadUTF16String(unsigned int f, wchar_t *out_str, size_t out_size, size_t inp_size);
extern size_t ncfCalculateUTF16StringSize(wchar_t *str, size_t size);
extern unsigned int ncfWriteUTF16String(unsigned int f, wchar_t *inp_str, size_t inp_size, size_t out_size);

#endif
