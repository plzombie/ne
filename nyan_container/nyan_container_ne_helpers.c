/*
	Файл	: nyan_container_ne_helpers.c

	Описание: Вспомогательные функции для работы с мультимедиа контейнером движка

	История	: 23.03.16	Создан

*/

#include <string.h>
#include <stdbool.h>

#include "nyan_container_ne_helpers.h"

#include "nyan_container_strings.h"

#include "../nyan/nyan_filesys_publicapi.h"
#include "../nyan/nyan_mem_publicapi.h"

/*
	Функция	: ncfReadAndCheckFiletypeChunk

	Описание: Поиск и чтение чанка FILETYPE, начиная с текущей позиции.
		Если действие завершилось успешно, то возвращает NCF_SUCCESS, позиция в файле устанавливается после чанка FILETYPE

	История	: 23.03.16	Создан

*/
unsigned int ncfReadAndCheckFiletypeChunk(unsigned int f, nyan_filetype_type *filetype, char *typestr, unsigned short reader_version)
{
	nyan_chunkhead_type chunkhead; // Заголовок чанка

	while (1) {
		if(nFileTell(f) >= nFileLength(f))
			return NCF_ERROR_CANTFINDFILETYPE;

		if(nFileRead(f, &chunkhead, sizeof(nyan_chunkhead_type)) != sizeof(nyan_chunkhead_type))
			return NCF_ERROR_WRONGFILETYPE;

		if(!memcmp(chunkhead.cname, "FILETYPE", 8)) {
			if(chunkhead.csize != 8)
				return NCF_ERROR_WRONGFILETYPE;

			nFileRead(f, filetype, sizeof(nyan_filetype_type));

			if(memcmp(filetype->filetype, typestr, 4) != 0 || filetype->reader_version > reader_version)
				return NCF_ERROR_WRONGFILETYPE;

			break;
		}
		else if(nFileSeek(f, chunkhead.csize, FILE_SEEK_CUR) == -1L)
			return NCF_ERROR_CANTFINDFILETYPE;
	}

	return NCF_SUCCESS;
}

//#include "../nyan/nyan_log_publicapi.h"

/*
	Функция	: ncfSeekForChunk

	Описание: Ищет заданный чанк, начиная с текущей позиции.
		В случае успеха возвращает NCF_SUCCESS, позиция в файле устанавливается на область данных чанка.

	История	: 23.03.16	Создан

*/
unsigned int ncfSeekForChunk(unsigned int f, nyan_chunkhead_type *chunkhead, char *chunkname)
{
	//nlPrint(L"ncfSeekForChunk before: nFileSeek(f, 0, FILE_SEEK_CUR) = %lld nFileTell = %lld nFileLength = %lld", nFileSeek(f, 0, FILE_SEEK_CUR), nFileTell(f), nFileLength(f));

	while(1) {
		if(nFileTell(f) >= nFileLength(f))
			return NCF_ERROR_CANTFINDCHUNK;

		if (nFileRead(f, chunkhead, sizeof(nyan_chunkhead_type)) != sizeof(nyan_chunkhead_type)) {
			//nlPrint(L"ncfSeekForChunk after: nFileSeek(f, 0, FILE_SEEK_CUR) = %lld nFileTell = %lld nFileLength = %lld", nFileSeek(f, 0, FILE_SEEK_CUR), nFileTell(f), nFileLength(f));
			return NCF_ERROR_DAMAGEDFILE;
		}

		if(!memcmp(chunkhead->cname, chunkname, 8)) {
			break;
		}
		else if(nFileSeek(f, chunkhead->csize, FILE_SEEK_CUR) == -1L) {
			//nlPrint(L"ncfSeekForChunk after: nFileSeek(f, 0, FILE_SEEK_CUR) = %lld nFileTell = %lld nFileLength = %lld", nFileSeek(f, 0, FILE_SEEK_CUR), nFileTell(f), nFileLength(f));
			return NCF_ERROR_CANTFINDCHUNK;
		}
	}
	//nlPrint(L"ncfSeekForChunk after: nFileSeek(f, 0, FILE_SEEK_CUR) = %lld nFileTell = %lld nFileLength = %lld", nFileSeek(f, 0, FILE_SEEK_CUR), nFileTell(f), nFileLength(f));
	return NCF_SUCCESS;
}

/*
	Функция	: ncfSeekForChunkAndCheckMinSize

	Описание: Ищет заданный чанк, начиная с текущей позиции.
		В случае успеха возвращает NCF_SUCCESS, позиция в файле устанавливается на область данных чанка.
		Если чанк найден, но его размер меньше minchunksize, возвращает NCF_ERROR_DAMAGEDFILE

	История	: 23.03.16	Создан

*/
unsigned int ncfSeekForChunkAndCheckMinSize(unsigned int f, nyan_chunkhead_type *chunkhead, char *chunkname, unsigned long long minchunksize)
{
	unsigned int ret;

	ret = ncfSeekForChunk(f, chunkhead, chunkname);

	if(ret == NCF_SUCCESS) {
		if(chunkhead->csize < minchunksize)
			ret = NCF_ERROR_DAMAGEDFILE;
	}

	//nlPrint(L"ncfSeekForChunkAndCheckMinSize = %u: chunkhead->csize %llu minchunksize %llu", ret, chunkhead->csize, minchunksize);

	return ret;
}

/*
	Функция	: ncfReadUTF16String

	Описание: Читает строку в формате UTF16 (пока ограничено UCS2) длиной inp_size символов и сохраняет её в строку out_str длиной out_size символов wchar_t.
		Если получившаяся строка out_str короче out_size символов, то дополняет её нулями
		В случае успеха возвращает NCF_SUCCESS, иначе NCF_FAIL

	История	: 23.03.16	Создан
		04.06.18 Изменил API чтобы проще потом было читать нормальные UTF16 строки

*/
unsigned int ncfReadUTF16String(unsigned int f, wchar_t *out_str, size_t out_size, size_t inp_size)
{
	if(out_size < inp_size)
		return NCF_FAIL;

	if(sizeof(wchar_t) == 2) {
		size_t i;

		if(nFileRead(f, out_str, inp_size*2) != (long long)(inp_size*2))
			return NCF_FAIL;

		for(i = inp_size; i < out_size; i++) {
			out_str[i] = 0;
		}
	} else {
		unsigned short *temp_str;
		long long readed_bytes;

		temp_str = nAllocMemory(inp_size*2);
		if(!temp_str)
			return NCF_FAIL;

		readed_bytes = nFileRead(f, temp_str, inp_size*2);

		if(readed_bytes == (long long)(inp_size*2)) {
			ncfUTF16ToWideChar(out_str, out_size, temp_str, inp_size);

			nFreeMemory(temp_str);
		} else {
			nFreeMemory(temp_str);

			return NCF_FAIL;
		}
	}

	return NCF_SUCCESS;
}

/*
	Функция	: ncfWriteUTF16String

	Описание: Пишет строку в формате UTF16 LE длиной out_size символов из строки inp_str длиной inp_size символов wchar_t.
		Если размер строки, которая будет записана, длиннее out_size символов 
		(например inp_size > out_size или inp_size == out_size и часть символов из inp_str записывается суррогатной парой),
		то возвращает NCF_FAIL.
		Если inp_size короче out_size, то дополняет строку нулями.
		В случае успеха возвращает NCF_SUCCESS, иначе NCF_FAIL

	История	: 23.03.16	Создан
		04.06.18 Изменил API чтобы проще потом было читать нормальные UTF16 строки

*/
unsigned int ncfWriteUTF16String(unsigned int f, wchar_t *inp_str, size_t inp_size, size_t out_size)
{
	if(inp_size > out_size)
		return NCF_FAIL;

	if(sizeof(wchar_t) == 2) {
		if(nFileWrite(f, inp_str, inp_size*2) != (long long)(inp_size*2))
			return NCF_FAIL;

		if(inp_size < out_size) {
			short zero_char = 0;
			size_t i;

			for(i = inp_size; i < out_size; i++) {
				if(nFileWrite(f, &zero_char, 2) != 2)
					return NCF_FAIL;
			}
		}
	} else {
		unsigned short *temp_str;
		long long written_bytes;

		temp_str = nAllocMemory(out_size*2);
		if(!temp_str)
			return NCF_FAIL;

		if(ncfWideCharToUTF16(temp_str, out_size, inp_str, inp_size) != NCF_SUCCESS) {
			nFreeMemory(temp_str);

			return NCF_FAIL;
		}

		written_bytes = nFileWrite(f, temp_str, out_size*2);

		nFreeMemory(temp_str);

		if(written_bytes != (long long)(out_size*2))
			return NCF_FAIL;
	}

	return NCF_SUCCESS;
}
