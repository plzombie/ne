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

#include "softgl/softgl.h"

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

	// ТОДО: Запилить такую функцию в softgl
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

	sglDestroyTexture(id);
	
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

	sglDestroyAllTextures();
	
	return true;
}

/*
	Функция	: nglLoadTexture

	Описание: Загружает текстуру

	История	: 14.08.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nglLoadTexture(unsigned int flags, unsigned int sizex, unsigned int sizey, unsigned int nglcolorformat, int nglrowalignment, unsigned char *buffer)
{
	bool success = false;
	unsigned int curtex = 0;
	
	if(!ngl_isinit) return 0;
	
	if(ngl_batch_state != NGL_BATCH_STATE_NO) return 0;
	
	ngl_ea->nlPrint(F_NGLLOADTEXTURE); ngl_ea->nlAddTab(1);
	
	curtex = sglCreateTexture(sizex, sizey, nglcolorformat, nglrowalignment, buffer);
	
	if(curtex) success = true;
	
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
	
	// ТОДО: допилить функцию по обновлению текстуры в softgl
	return success;
}
