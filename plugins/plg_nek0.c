/*
	Файл	: plg_nek0.c

	Описание: Плагин для загрузки nek0

	История	: 15.09.12	Создан

*/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../extclib/_wcsicmp.h"

#include "../nyan/nyan_text.h"
#include "../nyan/nyan_log_publicapi.h"
#include "../nyan/nyan_filesys_publicapi.h"
#include "../nyan/nyan_mem_publicapi.h"
#include "../nyan/nyan_file_publicapi.h"
#include "../nyan/nyan_texformat_publicapi.h"
#include "../nyan/nyan_plgtypes_publicapi.h"

#include "../nyan_container/nyan_container_ne_helpers.h"

#include "plg_nek0.h"

#define NEK0_READERVERSION 0

typedef struct {
	unsigned int sizex;
	unsigned int sizey;
	unsigned int datatype;
} nek0head_type;

typedef struct {
	nek0head_type base;
	int nglrowalignment; // Выравнивание строки
} nek0headext1_type;

bool N_APIENTRY plgNEK0SupportExt(const wchar_t *fname, const wchar_t *fext);
bool N_APIENTRY plgNEK0Load(const wchar_t *fname, nv_texture_type *tex);

nv_tex_plugin_type plgNEK0 = {sizeof(nv_tex_plugin_type), &plgNEK0SupportExt, &plgNEK0Load};

/*
	Функция	: plgNEK0SupportExt

	Описание: Проверяет поддержку типа файла

	История	: 15.09.12	Создан

*/
bool N_APIENTRY plgNEK0SupportExt(const wchar_t *fname, const wchar_t *fext)
{
	(void)fname; // Неиспользуемая переменная

	if(_wcsicmp(fext,L"nek0") == 0) return true;
	return false;
}

/*
	Функция	: plgNEK0Load

	Описание: Загружает nek0 файл

	История	: 15.09.12	Создан

*/
bool N_APIENTRY plgNEK0Load(const wchar_t *fname, nv_texture_type *tex)
{
	unsigned int f, ret;
	nek0head_type *head;
	unsigned int bpp;
	uint64_t texsize;
	nyan_filetype_type filetype; // Тип файла
	nyan_chunkhead_type chunkhead; // Заголовок чанка

	nlPrint(LOG_FDEBUGFORMAT7, F_PLGNEK0LOAD, N_FNAME, fname); nlAddTab(1);

	f = nFileOpen(fname);

	if(f == 0) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGNEK0LOAD, ERR_FILENOTFOUNDED);
		return false;
	}

	// Поиск и чтение чанка FILETYPE
	ret = ncfReadAndCheckFiletypeChunk(f, &filetype, "NEK0", NEK0_READERVERSION);
	switch(ret) {
		case NCF_SUCCESS:
			break;
		case NCF_ERROR_WRONGFILETYPE:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGNEK0LOAD, ERR_WRONGFILETYPE);
			nFileClose(f);
			return false;
		case NCF_ERROR_CANTFINDFILETYPE:
		default:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGNEK0LOAD, ERR_CANTFINDFILETYPE);
			nFileClose(f);
			return false;
	}

	// Поиск и чтение чанка FILEHEAD
	ret = ncfSeekForChunkAndCheckMinSize(f, &chunkhead, "FILEHEAD", sizeof(nek0head_type));
	switch(ret) {
		case NCF_SUCCESS:
			if(chunkhead.csize > SIZE_MAX) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGNEK0LOAD, ERR_FILEISTOOLARGE);
				nFileClose(f);
				return false;
			}
			head = malloc((size_t)(chunkhead.csize));
			if(!head) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGNEK0LOAD, ERR_WRONGFILEHEAD);
				nFileClose(f);
				return false;
			}

			nFileRead(f, head, (size_t)(chunkhead.csize));

			switch(head->datatype) {
				case NGL_COLORFORMAT_R8G8B8:
				case NGL_COLORFORMAT_B8G8R8:
				case NGL_COLORFORMAT_R8G8B8A8:
				case NGL_COLORFORMAT_B8G8R8A8:
				case NGL_COLORFORMAT_A8B8G8R8:
				case NGL_COLORFORMAT_L8A8:
				case NGL_COLORFORMAT_X1R5G5B5:
				case NGL_COLORFORMAT_R5G6B5:
				case NGL_COLORFORMAT_L8:
					if(chunkhead.csize < sizeof(nek0headext1_type)) {
						nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT9, F_PLGNEK0LOAD, ERR_CANTALLOCMEM, (long long)chunkhead.csize);
						free(head);
						nFileClose(f);
						return false;
					}
			}

			break;
		case NCF_ERROR_DAMAGEDFILE:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGNEK0LOAD, ERR_WRONGFILEHEAD);
			nFileClose(f);
			return false;
		case NCF_ERROR_CANTFINDCHUNK:
		default:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGNEK0LOAD, ERR_CANTFINDFILEHEAD);
			nFileClose(f);
			return false;
	}

	tex->sizex = head->sizex;
	tex->sizey = head->sizey;
	tex->nglrowalignment = ((nek0headext1_type *) head)->nglrowalignment;

	switch(head->datatype) {
		case NGL_COLORFORMAT_R8G8B8:
		case NGL_COLORFORMAT_B8G8R8:
			tex->nglcolorformat = head->datatype;
			bpp = 3;
			break;
		case NGL_COLORFORMAT_R8G8B8A8:
		case NGL_COLORFORMAT_B8G8R8A8:
		case NGL_COLORFORMAT_A8B8G8R8:
			tex->nglcolorformat = head->datatype;
			bpp = 4;
			break;
		case NGL_COLORFORMAT_L8A8:
		case NGL_COLORFORMAT_X1R5G5B5:
		case NGL_COLORFORMAT_R5G6B5:
			tex->nglcolorformat = head->datatype;
			bpp = 2;
			break;
		case NGL_COLORFORMAT_L8:
			tex->nglcolorformat = head->datatype;
			bpp = 1;
			break;
		default:
			free(head);
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGNEK0LOAD, ERR_UNSUPPORTEDDATATYPE);
			nFileClose(f);
			return false;
	}

	free(head);

	if(tex->nglrowalignment <= 0) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGNEK0LOAD, ERR_FILEISDAMAGED);
		nFileClose(f);
		return false;
	} else {
		uint64_t rowsize;
		rowsize = bpp*(uint64_t)tex->sizex;
		if(rowsize%tex->nglrowalignment > 0)
			rowsize += tex->nglrowalignment-(rowsize%tex->nglrowalignment);
		texsize = tex->sizey*rowsize;
	}

	if(texsize > SIZE_MAX) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGNEK0LOAD, ERR_FILEISTOOLARGE);
		nFileClose(f);
		return false;
	}

	// Поиск чанка FILEDATA
	ret = ncfSeekForChunkAndCheckMinSize(f, &chunkhead, "FILEDATA", texsize);
	switch(ret) {
		case NCF_SUCCESS:
			break;
		case NCF_ERROR_DAMAGEDFILE:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGNEK0LOAD, ERR_WRONGFILEDATA);
			nFileClose(f);
			return false;
		case NCF_ERROR_CANTFINDCHUNK:
		default:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGNEK0LOAD, ERR_CANTFINDFILEDATA);
			nFileClose(f);
			return false;
	}

	// Чтение чанка FILEDATA
	tex->buffer = nAllocMemory((size_t)texsize);
	if(!tex->buffer) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT9, F_PLGNEK0LOAD, ERR_CANTALLOCMEM, (long long)texsize);
		nFileClose(f);
		return false;
	}
	nFileRead(f, tex->buffer, (size_t)texsize);
	nFileSeek(f, chunkhead.csize-texsize, FILE_SEEK_CUR);

	nFileClose(f);

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGNEK0LOAD, N_OK);

	return true;
}
