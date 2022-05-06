/*
	Файл	: ngl_texture.c

	Описание: Работа с текстурами

	История	: 5.10.12	Создан

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "ngl.h"
#include "ngl_text.h"

#include "../nyan/nyan_texformat_publicapi.h"

#include "ngl_init.h"
#include "ngl_batch.h"
#include "ngl_texture.h"

// Параметры текстуры
int ngl_tex_maxanis = 0; // Максимальный уровень анизотропии
int ngl_tex_anis = 666; // Уровень анизотропии
int ngl_tex_enabledflags = NGL_TEX_FLAGS_ALL; // Разрешённые флаги текстур

// Массивы текстур
ngl_texture_type *ngl_textures = 0;
unsigned int ngl_maxtextures = 0;

uintptr_t ngl_textures_sync_mutex = 0; // Мьютекс для синхронизации работы с текстурами

/*
	Функция	: nglIsTex

	Описание: Возвращает true, если id - текстура

	История	: 5.10.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglIsTex(unsigned int id)
{
	bool result = true;

	if(ngl_isinit == false || id == 0) return false;

	ngl_ea->nLockMutex(ngl_textures_sync_mutex);
		if(id > ngl_maxtextures)
			result = false;
		else if(!ngl_textures[id-1].isused)
			result = false;
	ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);

	return result;
}

/*
	Функция	: nglTexGetScale

	Описание: Возвращает числа, на которые надо домножить координаты текстуры

	История	: 5.10.12	Создан

*/
void nglTexGetScale(unsigned int id,unsigned int *flags, float *scalex, float *scaley)
{
	if(ngl_isinit == false || id > ngl_maxtextures || id == 0) { *flags = 0; *scalex = 1.0; *scaley = 1.0; return; }

	if(!ngl_textures[id-1].isused) { *flags = 0; *scalex = 1.0; *scaley = 1.0; return; }

	*flags = ngl_textures[id-1].flags;
	*scalex = ngl_textures[id-1].cscalex;
	*scaley = ngl_textures[id-1].cscaley;
}

/*
	Функция	: nglTexGetBytesPerPixel

	Описание: Количество байт на пиксель

	История	: 5.10.12	Создан

*/
int nglTexGetBytesPerPixel(unsigned int nglcolorformat)
{
	switch(nglcolorformat) {
		case NGL_COLORFORMAT_R8G8B8:
			return 3;
		case NGL_COLORFORMAT_R8G8B8A8:
			return 4;
		case NGL_COLORFORMAT_B8G8R8:
			return 3;
		case NGL_COLORFORMAT_B8G8R8A8:
			return 4;
		case NGL_COLORFORMAT_A8B8G8R8:
			return 4;
		case NGL_COLORFORMAT_L8:
			return 1;
		case NGL_COLORFORMAT_L8A8:
			return 2;
		case NGL_COLORFORMAT_X1R5G5B5:
			return 2;
		case NGL_COLORFORMAT_R5G6B5:
			return 2;
		default:
			return 0;
	}
}

/*
	Функция	: nglTexGetRowSize

	Описание: Возвращает размер строки текстуры. НЕ проверяет поддержку формата движком (это должно делаться в nglTexConvertToSupported)

	История	: 5.10.12	Создан

*/
int nglTexGetRowSize(unsigned int nglcolorformat,int sizex,int packrow)
{
	int bpp; // Байт на пиксель

	bpp = nglTexGetBytesPerPixel(nglcolorformat);

	if(((sizex*bpp) % packrow) == 0) return (sizex*bpp); else
		return ((sizex*bpp/packrow)*packrow+packrow);
}

/*
	Функция	: nglTexGetSize

	Описание: Возвращает размер текстуры. НЕ проверяет поддержку формата движком (это должно делаться в nglTexConvertToSupported)

	История	: 5.10.12	Создан

*/
int nglTexGetSize(unsigned int nglcolorformat,int sizex,int sizey,int packrow)
{
	return nglTexGetRowSize(nglcolorformat, sizex, packrow)*sizey;
}

/*
	Функция	: nglFreeTexture

	Описание: Освобождает текстуру

	История	: 5.10.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglFreeTexture(unsigned int id)
{
	ngl_texture_type texture;

	if(!ngl_isinit) return false;

	ngl_ea->nLockMutex(ngl_textures_sync_mutex);
		if(id < 1 || id > ngl_maxtextures) {
			ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NGLFREETEXTURE, N_FALSE, N_ID, id);
			return false;
		}
		if(!ngl_textures[id-1].isused) {
			ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLFREETEXTURE, N_FALSE, N_ID, id);
			return false;
		}
		texture = ngl_textures[id-1];
		ngl_textures[id-1].isused = false;
	ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);

	if(texture.bufmayfree) ngl_ea->nFreeMemory(texture.buffer);
	
	ngl_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NGLFREETEXTURE, N_OK, N_ID, id);
	
	return true;
}

/*
	Функция	: nglFreeAllTextures

	Описание: Освобождает текстуру

	История	: 5.10.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglFreeAllTextures(void)
{
	unsigned int i;
	
	if(!ngl_isinit) return false;
	
	ngl_ea->nLockMutex(ngl_textures_sync_mutex);
		if(ngl_maxtextures) {
			for(i = 0;i<ngl_maxtextures;i++)
				if(ngl_textures[i].isused)
					nglFreeTexture(i+1);
				
			ngl_ea->nFreeMemory(ngl_textures);
		
			ngl_textures = 0;
			ngl_maxtextures = 0;
		}
	ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
	
	return true;
}

/*
	Функция	: nglTexConvertToSupported

	Описание: Конвертирует текстуру в поддерживаемый формат

	История	: 5.10.12	Создан

*/
bool nglTexConvertToSupported(ngl_texture_type *tex)
{
	switch(tex->nglcolorformat) {
		case NGL_COLORFORMAT_R8G8B8:
		case NGL_COLORFORMAT_R8G8B8A8:
		case NGL_COLORFORMAT_B8G8R8:
		case NGL_COLORFORMAT_B8G8R8A8:
		case NGL_COLORFORMAT_A8B8G8R8:
		case NGL_COLORFORMAT_L8:
		case NGL_COLORFORMAT_L8A8:
		case NGL_COLORFORMAT_X1R5G5B5:
		case NGL_COLORFORMAT_R5G6B5:
			break;
		default:
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT3, F_NGLTEXCONVERTTOSUPPORTED, ERR_UNSUPPORTEDNGLCOLORFORMAT, tex->nglcolorformat);
			return false;
	}
	
	return true;
}

/*
	Функция	: nglLoadTexture

	Описание: Загружает текстуру

	История	: 14.08.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nglLoadTexture(unsigned int flags, unsigned int sizex, unsigned int sizey, unsigned int nglcolorformat, int nglrowalignment, unsigned char *buffer)
{
	unsigned int curtex, texsize; // Текущая текстура (id которой вернут пользователю)
	bool success = true;
	
	if(!ngl_isinit) return 0;
	
	ngl_ea->nlPrint(F_NGLLOADTEXTURE); ngl_ea->nlAddTab(1);
	
	ngl_ea->nLockMutex(ngl_textures_sync_mutex);

	// Выделение памяти под текстуры
	for(curtex = 0;curtex < ngl_maxtextures; curtex++)
		if(!ngl_textures[curtex].isused)
			break;
		
	if(curtex == ngl_maxtextures) {
		ngl_texture_type *_ngl_textures;
			
		_ngl_textures = ngl_ea->nReallocMemory(ngl_textures,(ngl_maxtextures+1024)*sizeof(ngl_texture_type));
			
		if(_ngl_textures)
			ngl_textures = _ngl_textures;
		else {
			ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
			ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NGLLOADTEXTURE, N_FALSE, N_ID, 0);
			return false;
		}
			
		for(curtex=ngl_maxtextures;curtex<ngl_maxtextures+1024;curtex++)
			ngl_textures[curtex].isused = false;
			
		curtex = ngl_maxtextures;
			
		ngl_maxtextures += 1024;
	}

	texsize = nglTexGetSize(nglcolorformat, sizex, sizey, nglrowalignment);

	ngl_textures[curtex].bufmayfree = true;
	ngl_textures[curtex].flags = flags;
	ngl_textures[curtex].oasizex = sizex;
	ngl_textures[curtex].oasizey = sizey;
	ngl_textures[curtex].osizex = sizex;
	ngl_textures[curtex].osizey = sizey;
	ngl_textures[curtex].sizex = sizex;
	ngl_textures[curtex].sizey = sizey;
	ngl_textures[curtex].nglcolorformat = nglcolorformat;
	ngl_textures[curtex].nglrowalignment = nglrowalignment;
	ngl_textures[curtex].buffer = ngl_ea->nAllocMemory(texsize);
	if(ngl_textures[curtex].buffer) {
		memcpy(ngl_textures[curtex].buffer, buffer, texsize);

		success = nglTexConvertToSupported(&ngl_textures[curtex]);
	} else
		success = false;
	
	ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NGLLOADTEXTURE, success?N_OK:N_FALSE, N_ID, success?curtex+1:0);
	
	if(success) {
		ngl_textures[curtex].isused = true;
		ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
		return (curtex+1);
	} else {
		if(ngl_textures[curtex].buffer)
			ngl_ea->nFreeMemory(ngl_textures[curtex].buffer);
		ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
		return 0;
	}
}

/*
	Функция	: nglUpdateTexture

	Описание: Загружает текстуру

	История	: 12.02.13	Создан

*/
N_API bool N_APIENTRY_EXPORT nglUpdateTexture(unsigned int texid, unsigned char *buffer)
{
	unsigned int texsize;
	bool success = true;
	
	if(!ngl_isinit) return false;

	ngl_ea->nLockMutex(ngl_textures_sync_mutex);
	
	if((texid == 0) || (texid > ngl_maxtextures)) {
		ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
		return false;
	}
	
	if(ngl_textures[texid-1].isused == false) {
		ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
		return false;
	}
	
	if(ngl_textures[texid-1].bufmayfree)
		ngl_ea->nFreeMemory(ngl_textures[texid-1].buffer);
	ngl_textures[texid-1].bufmayfree = true;

	texsize = nglTexGetSize(ngl_textures[texid-1].nglcolorformat, ngl_textures[texid-1].sizex, ngl_textures[texid-1].sizey, ngl_textures[texid-1].nglrowalignment);

	ngl_textures[texid-1].buffer = ngl_ea->nAllocMemory(texsize);
	if(ngl_textures[texid-1].buffer) {
		memcpy(ngl_textures[texid-1].buffer, buffer, texsize);

		success = nglTexConvertToSupported(&ngl_textures[texid-1]);
	} else
		success = false;
	
	if(success) {
		ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
		return true;
	} else {
		if(ngl_textures[texid-1].buffer)
			ngl_ea->nFreeMemory(ngl_textures[texid-1].buffer);
		ngl_textures[texid-1].isused = false;
		ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
		return false;
	}
}
