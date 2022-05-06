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

#include "../forks/tinygl/src/zgl.h"
#include "../forks/tinygl/examples/glu.h"

#include "../nyan/nyan_texformat_publicapi.h"

#include "ngl_init.h"
#include "ngl_batch.h"
#include "ngl_texture.h"

// Параметры текстуры
int ngl_tex_maxanis = 0; // Максимальный уровень анизотропии
int ngl_tex_anis = 666; // Уровень анизотропии
int ngl_tex_enabledflags = NGL_TEX_FLAGS_ALL; // Разрешённые флаги текстур

/*
	Функция	: nglIsTex

	Описание: Возвращает true, если id - текстура

	История	: 5.10.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglIsTex(unsigned int id)
{
	if(ngl_isinit == false || id == 0) return false;

	// ТОДО: Добавить список текстур для tinygl
	//if(!ngl_textures[id-1].isused) return false;

	return true;
}

/*
	Функция	: nglFreeTexture

	Описание: Освобождает текстуру

	История	: 5.10.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglFreeTexture(unsigned int id)
{
	if(!ngl_isinit) return false;
	
	if(ngl_batch_state != NGL_BATCH_STATE_NO) return false;

	glDeleteTextures(1, &id);
	
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
	if(!ngl_isinit) return false;
	
	if(ngl_batch_state != NGL_BATCH_STATE_NO) return false;

	// ТОДО: Добавить список текстур для tinygl
	/*if(ngl_maxtextures) {
		for(i = 0; i<ngl_maxtextures; i++)
			if(ngl_textures[i].isused)
				nglFreeTexture(i+1);

		ngl_maxtextures = 0;
		ngl_ea->nFreeMemory(ngl_textures);
		ngl_textures = 0;
	}*/
	
	return true;
}

/*
Функция	: nglTexGetBytesPerPixel

Описание: Количество байт на пиксель

История	: 17.06.12	Создан

*/
int nglTexGetBytesPerPixel(unsigned int nglcolorformat)
{
	switch(nglcolorformat) {
		case NGL_COLORFORMAT_R8G8B8A8:
		case NGL_COLORFORMAT_B8G8R8A8:
		case NGL_COLORFORMAT_A8B8G8R8:
			return 4;
		case NGL_COLORFORMAT_R8G8B8:
		case NGL_COLORFORMAT_B8G8R8:
			return 3;
		case NGL_COLORFORMAT_L8A8:
		case NGL_COLORFORMAT_X1R5G5B5:
		case NGL_COLORFORMAT_R5G6B5:
			return 2;
		case NGL_COLORFORMAT_L8:
			return 1;
		default:
			return 0;
	}
}

/*
Функция	: nglTexGetRowSize

Описание: Возвращает размер строки текстуры. НЕ проверяет поддержку формата движком (это должно делаться в nglTexConvertToSupported)

История	: 25.06.12	Создан

*/
int nglTexGetRowSize(unsigned int nglcolorformat, int sizex, int packrow)
{
	int bpp; // Байт на пиксель

	bpp = nglTexGetBytesPerPixel(nglcolorformat);

	if(((sizex*bpp) % packrow) == 0) return (sizex*bpp); else
		return ((sizex*bpp/packrow)*packrow+packrow);
}

/*
Функция	: nglTexGetSize

Описание: Возвращает размер текстуры. НЕ проверяет поддержку формата движком (это должно делаться в nglTexConvertToSupported)

История	: 17.06.12	Создан

*/
int nglTexGetSize(unsigned int nglcolorformat, int sizex, int sizey, int packrow)
{
	return nglTexGetRowSize(nglcolorformat, sizex, packrow)*sizey;
}

/*
	Функция	: nglLoadTexture

	Описание: Загружает текстуру

	История	: 14.08.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nglLoadTexture(unsigned int flags, unsigned int sizex, unsigned int sizey, unsigned int nglcolorformat, int nglrowalignment, unsigned char *buffer)
{
	bool success = false, bufmayfree = false; 
	unsigned int curtex = 0;
	
	if(!ngl_isinit) return 0;
	
	if(ngl_batch_state != NGL_BATCH_STATE_NO) return 0;
	
	ngl_ea->nlPrint(F_NGLLOADTEXTURE); ngl_ea->nlAddTab(1);
	
	switch(nglcolorformat) {
		case NGL_COLORFORMAT_R8G8B8:
			break;
		case NGL_COLORFORMAT_B8G8R8:
			{
				unsigned char *p1, *p2, *p11, *p12;
				unsigned int texsize, al, i, j;
				if(((sizex*3) % nglrowalignment) == 0) al = 0; else // Выравнивание строки
					al = nglrowalignment-sizex*3%nglrowalignment;
				p1 = buffer; p2 = buffer+2;
				texsize = nglTexGetSize(nglcolorformat, sizex, sizey, nglrowalignment);
				buffer = ngl_ea->nAllocMemory(texsize);
				if(buffer)
					bufmayfree = true;
				else
					return false;
				memcpy(buffer, p1, texsize);
				p11 = buffer; p12 = buffer+2;
				for(i = 0; i<sizex; i++) {
					for(j = 0; j<sizey; j++) {
						*p12 = *p1;
						*p11 = *p2;
						p1 += 3; p2 += 3;
						p11 += 3; p12 += 3;
					}
					p1 += al; p2 += al; p11 += al; p12 += al;
				}
				nglcolorformat = NGL_COLORFORMAT_R8G8B8;
			}
			break;
		case NGL_COLORFORMAT_A8B8G8R8:
		case NGL_COLORFORMAT_B8G8R8A8:
		case NGL_COLORFORMAT_L8A8:
		case NGL_COLORFORMAT_L8:
			{
				unsigned char *p1, *p2;
				unsigned int texsize, i;

				p1 = buffer;

				texsize = nglTexGetSize(NGL_COLORFORMAT_R8G8B8, sizex, sizey, nglrowalignment);
				buffer = ngl_ea->nAllocMemory(texsize);
				if(buffer)
					bufmayfree = true;
				else
					return false;

				p2 = buffer;

				for(i = 0; i < sizex*sizey; i++) {
					unsigned char b, g, r, a;

					if(nglcolorformat == NGL_COLORFORMAT_A8B8G8R8) {
						a = *(p1++);
						b = *(p1++);
						g = *(p1++);
						r = *(p1++);
						*(p2++) = r*(int)a/255;
						*(p2++) = g*(int)a/255;
						*(p2++) = b*(int)a/255;
					} else if(nglcolorformat == NGL_COLORFORMAT_B8G8R8A8) {
						b = *(p1++);
						g = *(p1++);
						r = *(p1++);
						a = *(p1++);
						*(p2++) = r*(int)a/255;
						*(p2++) = g*(int)a/255;
						*(p2++) = b*(int)a/255;
					} else if(nglcolorformat == NGL_COLORFORMAT_L8A8) {
						r = *(p1++);
						a = *(p1++);
						*(p2++) = r*(int)a/255;
						*(p2++) = r*(int)a/255;
						*(p2++) = r*(int)a/255;
					} else  {
						r = *(p1++);
						*(p2++) = r;
						*(p2++) = r;
						*(p2++) = r;
					}
				}
			}

			nglcolorformat = NGL_COLORFORMAT_R8G8B8;

			break;
		case NGL_COLORFORMAT_X1R5G5B5:
		case NGL_COLORFORMAT_R5G6B5:
			{
				unsigned int rowal, i, j;
				unsigned short *p;
				unsigned char *p2;
				p = (unsigned short *)buffer;
				buffer = ngl_ea->nAllocMemory(sizex*sizey*3);
				if(buffer)
					bufmayfree = true;
				else
					return false;
				p2 = buffer;
				rowal = (sizex*2)%4;
				if((sizex*3)%4 == 0) nglrowalignment = 4; else nglrowalignment = 1;
				for(i = 0; i<sizey; i++) {
					for(j = 0; j<sizex; j++) {
						if(nglcolorformat == NGL_COLORFORMAT_X1R5G5B5) {
							*p2 = (((*p)>>10)&0x001F)<<3; p2++;
							*p2 = (((*p)>>5)&0x001F)<<3; p2++;
							*p2 = ((*p)&0x001F)<<3; p2++;
						} else {
							*p2 = (((*p)>>11)&0x001F)<<3; p2++;
							*p2 = (((*p)>>5)&0x003F)<<2; p2++;
							*p2 = ((*p)&0x001F)<<3; p2++;
						}
						p++;
					}
					p += rowal;
				}
				nglcolorformat = NGL_COLORFORMAT_R8G8B8;
			}
			break;
		default:
			ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT3, F_NGLLOADTEXTURE, ERR_UNSUPPORTEDNGLCOLORFORMAT, nglcolorformat);
			return 0;
	}

	if(nglcolorformat == NGL_COLORFORMAT_R8G8B8 && (sizex*3)%4 == 0) {
		glGenTextures(1, &curtex);
		glBindTexture(GL_TEXTURE_2D, curtex);

		glTexImage2D(GL_TEXTURE_2D, 0, 3, sizex, sizey, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	}
	
	if(curtex) success = true;

	if(bufmayfree) ngl_ea->nFreeMemory(buffer);
	
	ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NGLLOADTEXTURE, success?N_OK:N_FALSE, N_ID, curtex);
	
	return curtex;
}

/*
	Функция	: nglUpdateTexture

	Описание: Загружает текстуру

	История	: 12.02.13	Создан

*/
N_API bool N_APIENTRY_EXPORT nglUpdateTexture(unsigned int texid, unsigned char *buffer)
{
	bool success = true;
	
	if(!ngl_isinit) return false;
	
	if(ngl_batch_state != NGL_BATCH_STATE_NO) return false;
	
	// 111

	return success;
}
