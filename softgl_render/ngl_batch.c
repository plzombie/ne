/*
	Файл	: ngl_batch.с

	Описание: Вывод графики, 2d/3d проекция

	История	: 15.08.12	Создан

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "ngl.h"
#include "ngl_text.h"

#include "softgl/softgl.h"

#include "../nyan/nyan_texformat_publicapi.h"
#include "../nyan/nyan_draw_publicapi.h"

#include "ngl_init.h"
#include "ngl_batch.h"
#include "ngl_texture.h"

static float ngl_global_ambient[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

int ngl_batch_state = 0;

N_API void N_APIENTRY_EXPORT nglBatch2dEnd(void);
N_API void N_APIENTRY_EXPORT nglBatch3dEnd(void);

/*
	Функция	: nglBatch2dDraw

	Описание: Рисует 2d

	История	: 11.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch2dDraw(void)
{
	if(ngl_batch_state != NGL_BATCH_STATE_2D || !ngl_isinit) return false;
	
	return true;
}

/*
	Функция	: nglBatch2dAdd

	Описание: Добавляет примитив

	История	: 11.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch2dAdd(int batch2d_type, unsigned int batch_currenttex, unsigned int vertices, nv_2dvertex_type *varray)
{
	if(ngl_batch_state != NGL_BATCH_STATE_2D || !ngl_isinit) return false;

	if(ngl_windpix == 100 && ngl_windpiy == 100)
		sglDraw2d(batch2d_type, batch_currenttex, vertices, (sgl_2dvertex_type *)varray);
	else {
		sgl_2dvertex_type new_varray[4];
		nv_2dvertex_type *varray_ptr;
		unsigned int figures, vertices_per_figure, i;
		
		if(batch2d_type == SGL_DRAWPOINT)
			vertices_per_figure = 1;
		else if(batch2d_type == SGL_DRAWLINE)
			vertices_per_figure = 2;
		else
			vertices_per_figure = 3;

		figures = vertices/vertices_per_figure;
		varray_ptr = varray;

		for(i = 0; i < figures; i++) {
			unsigned int j;

			for(j = 0; j < vertices_per_figure; j++) {
				new_varray[j].x = (float)ngl_windpix*varray_ptr->x/100.0f;
				new_varray[j].y = (float)ngl_windpiy*varray_ptr->y/100.0f;
				new_varray[j].z = varray_ptr->z;
				new_varray[j].tx = varray_ptr->tx;
				new_varray[j].ty = varray_ptr->ty;
				new_varray[j].colorRGBA = varray_ptr->colorRGBA;
				
				varray_ptr++;
			}

			sglDraw2d(batch2d_type,batch_currenttex, vertices_per_figure, new_varray);
		}
	}
	
	return true;
}

/*
	Функция	: nglBatch3dDrawMesh

	Описание: Рисует 3d модель

	История	: 01.05.13	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch3dDrawMesh(unsigned int batch_currenttex, unsigned int vertices, nv_3dvertex_type *varray)
{
	if(ngl_batch_state != NGL_BATCH_STATE_3D || !ngl_isinit || vertices == 0) return false;
	
	// Вывод 3d модели -^_^-
	
	return true;
}

/*
	Функция	: nglBatch3dDrawIndexedMesh

	Описание: Рисует 3d модель с индексированными вершинами

	История	: 28.06.16	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch3dDrawIndexedMesh(unsigned int batch_currenttex, unsigned int vertices, nv_3dvertex_type *varray, unsigned int triangles, unsigned int *iarray)
{
	nv_3dvertex_type new_varray[3];
	unsigned int i;
	
	if(ngl_batch_state != NGL_BATCH_STATE_3D || !ngl_isinit || vertices == 0 || triangles == 0) return false;
	
	for(i = 0; i < triangles; i++) {
		new_varray[0] = varray[iarray[i*3]];
		new_varray[1] = varray[iarray[i*3+1]];
		new_varray[2] = varray[iarray[i*3+2]];
		
		nglBatch3dDrawMesh(batch_currenttex, 3, new_varray);
	}
	
	return true;
}

/*
	Функция	: nglBatch2dBegin

	Описание: Начало вывода 2d графики

	История	: 05.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch2dBegin(void)
{
	if(ngl_batch_state == NGL_BATCH_STATE_2D || !ngl_isinit) return;

	if(ngl_batch_state == NGL_BATCH_STATE_3D) nglBatch3dEnd();

	ngl_batch_state = NGL_BATCH_STATE_2D;
}

/*
	Функция	: nglBatch2dEnd

	Описание: Конец вывода 2d графики

	История	: 05.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch2dEnd(void)
{
	if(ngl_batch_state != NGL_BATCH_STATE_2D || !ngl_isinit) return;

	nglBatch2dDraw();

	ngl_batch_state = NGL_BATCH_STATE_NO;
}

/*
	Функция	: nglBatch3dSetModelviewMatrix

	Описание: Устанавливает матрицу преобразования

	История	: 16.01.15	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch3dSetModelviewMatrix(double *matrix)
{
	if(!ngl_isinit) return false;
	if(ngl_batch_state == NGL_BATCH_STATE_NO) return false;

	// Установка матрицы преобразования

	return true;
}

/*
	Функция	: nglBatch3dSetAmbientLight

	Описание: Устанавливает глобальное освещение
	
	История	: 24.04.13	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch3dSetAmbientLight(float r, float g, float b, float a)
{
	if(!ngl_isinit) return;
	
	ngl_global_ambient[0] = r;
	ngl_global_ambient[1] = g;
	ngl_global_ambient[2] = b;
	ngl_global_ambient[3] = a;
	
	if(ngl_batch_state == NGL_BATCH_STATE_3D) {
		// Установка ambient light
	}
}

/*
	Функция	: nglBatch3dBegin

	Описание: Начало вывода 3d графики

	История	: 05.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch3dBegin(void)
{
	if(ngl_batch_state == NGL_BATCH_STATE_3D || !ngl_isinit) return;

	if(ngl_batch_state == NGL_BATCH_STATE_2D) nglBatch2dEnd();

	ngl_batch_state = NGL_BATCH_STATE_3D;

	// Установка матрици проекции
		
	// Установка ambient light
}

/*
	Функция	: nglBatch3dEnd

	Описание: Конец вывода 3d графики

	История	: 05.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch3dEnd(void)
{
	if(ngl_batch_state != NGL_BATCH_STATE_3D || !ngl_isinit) return;

	ngl_batch_state = NGL_BATCH_STATE_NO;
}

/*
	Функция	: nglReadScreen

	Описание: Читает изображение с экрана

	История	: 24.08.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglReadScreen(unsigned char *buffer)
{
	unsigned int sx, sy, scolortype; unsigned char *surface;
	
	if (!ngl_isinit) return false;

	if(ngl_batch_state == NGL_BATCH_STATE_2D) nglBatch2dEnd();
	if(ngl_batch_state == NGL_BATCH_STATE_3D) nglBatch3dEnd();
	
	// Вывод в buffer содержимого экрана. Формат ngl_winx x ngl_winy x RGBA
	if(sglGetSurface(&sx, &sy, &scolortype, &surface)) {
		if(ngl_windpix == 100 && ngl_windpiy == 100) {
			unsigned int i;

			for(i = 0; i < sy; i++)
				memcpy(buffer+(sy-i-1)*4*sx, surface+i*4*sx, sx*4);
		} else {
			unsigned int i, j;

			for(i = 0; i < ngl_winy; i++) {
				for(j = 0; j < ngl_winx; j++) {
					memcpy(buffer+((ngl_winy-i-1)*ngl_winx+j)*4, surface+((ngl_windpiy*i/100)*sx+(ngl_windpix*j/100))*4, 4);
				}
			}
		}
	}

	return true;
}
