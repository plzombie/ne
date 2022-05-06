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

#include "../nyan/nyan_texformat_publicapi.h"
#include "../nyan/nyan_draw_publicapi.h"

#include "ngl_init.h"
#include "ngl_batch.h"
#include "ngl_texture.h"

unsigned int ngl_batch_currenttex = 0; // ngl_textures[ngl_batch_currenttex-1], ngl_batch_currenttex>0
int ngl_batch_state = NGL_BATCH_STATE_NO;

float ngl_global_ambient[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

#define NGL_BATCH2D_MAXVERTICES 81920

nv_2dvertex_type *ngl_batch2d_varray = 0;
nv_2dvertex_type *ngl_batch2d_vcurrent = 0;
int ngl_batch2d_vertices = 0;
int ngl_batch2d_type = 0;

N_API void N_APIENTRY_EXPORT nglBatch2dEnd(void);
N_API void N_APIENTRY_EXPORT nglBatch3dEnd(void);

/*
	Функция	: nglBatchInit

	Описание: Инициализирует batching

	История	: 11.06.12	Создан

*/
bool nglBatchInit(void)
{
	if(!ngl_isinit) return false;

	ngl_ea->nlPrint(F_NGLBATCHINIT); ngl_ea->nlAddTab(1);

	ngl_batch_currenttex = 0;
	ngl_batch_state = NGL_BATCH_STATE_NO;
	ngl_batch2d_vertices = 0;
	ngl_batch2d_varray = ngl_ea->nAllocMemory(NGL_BATCH2D_MAXVERTICES*sizeof(nv_2dvertex_type));
	if(!ngl_batch2d_varray) {
		ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLBATCHINIT, N_FALSE);
		return false;
	}
	ngl_batch2d_vcurrent = ngl_batch2d_varray;

	ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLBATCHINIT, N_OK);

	return true;
}

/*
	Функция	: nglBatchClose

	Описание: Деинициализирует batching

	История	: 11.06.12	Создан

*/
void nglBatchClose(void)
{
	if(!ngl_isinit) return;

	ngl_ea->nlPrint(F_NGLBATCHCLOSE); ngl_ea->nlAddTab(1);

	if(ngl_batch_state == NGL_BATCH_STATE_2D) nglBatch2dEnd();
	if(ngl_batch_state == NGL_BATCH_STATE_3D) nglBatch3dEnd();

	ngl_batch_state = NGL_BATCH_STATE_NO;

	ngl_ea->nFreeMemory(ngl_batch2d_varray);
	ngl_batch2d_varray = 0;

	ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLBATCHCLOSE, N_OK);
}

/*
	Функция	: nglBatchSetCurrentTexture

	Описание: Устанавливает текущую текстуру

	История	: 23.05.18	Создан

*/
void nglBatchSetCurrentTexture(unsigned int batch_currenttex)
{
	if(nglIsTex(batch_currenttex)) { // Если устанавливаем текстуру
		if(!ngl_batch_currenttex) { // Текущая текстура не установлена (отключена)
			// Включить текстуры
			;
		}

		if(ngl_batch_currenttex != batch_currenttex) { // Текущая текстура не равна batch_currenttex
			// Сделать текущей текстуру batch_currenttex
			;
		} else {
			// Если текущая текстура GAPI не соответствует текстуре GAPI для batch_currenttex
			// Сделать текущей текстуру batch_currenttex
			;
		}

		ngl_batch_currenttex = batch_currenttex;
	} else { // Если выключаем текстуры
		if(ngl_batch_currenttex) {
			// Отключение текстур
			;
		}

		ngl_batch_currenttex = 0;
	}
}

/*
	Функция	: nglBatchGetCurrentTexture

	Описание: Возвращает текущую текстуру

	История	: 23.05.18	Создан

*/
unsigned int nglBatchGetCurrentTexture(void)
{
	return ngl_batch_currenttex;
}

/*
	Функция	: nglBatch2dDraw

	Описание: Рисует 2d

	История	: 11.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch2dDraw(void)
{
	unsigned int flags = 0; float scalex, scaley;
	unsigned int batch_currenttex;
	nv_2dvertex_type *p;

	if(ngl_batch_state != NGL_BATCH_STATE_2D || !ngl_isinit || ngl_batch2d_vertices == 0) return false;

	batch_currenttex = nglBatchGetCurrentTexture();

	if(batch_currenttex) {
		nglTexGetScale(batch_currenttex, &flags, &scalex, &scaley);
		if(flags & NGL_TEX_FLAGS_SCALECOORD) {
			int i;
			
			p = ngl_batch2d_varray;
			for(i=0;i<ngl_batch2d_vertices;i++) {
				p->tx *= scalex;
				p->ty *= scaley;
				p++;
			}
		}
	}

	switch(ngl_batch2d_type) {
		case NV_DRAWPOINT:
			// Вывести ngl_batch2d_vertices точек из массива ngl_batch2d_varray
			break;
		case NV_DRAWLINE:
			// Вывести ngl_batch2d_vertices/2 линий из массива ngl_batch2d_varray
			break;
		case NV_DRAWTRIANGLE:
			// Вывести ngl_batch2d_vertices/3 треугольников из массива ngl_batch2d_varray
			break;
	}

	ngl_batch2d_vcurrent = ngl_batch2d_varray;
	ngl_batch2d_vertices = 0;
	
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

	if(vertices > NGL_BATCH2D_MAXVERTICES) return false;

	if(ngl_batch2d_type != batch2d_type || nglBatchGetCurrentTexture() != batch_currenttex || (vertices+ngl_batch2d_vertices) > NGL_BATCH2D_MAXVERTICES) nglBatch2dDraw();

	nglBatchSetCurrentTexture(batch_currenttex);

	ngl_batch2d_type = batch2d_type;
	memcpy(ngl_batch2d_vcurrent,varray,vertices*sizeof(nv_2dvertex_type));
	ngl_batch2d_vcurrent += vertices;
	ngl_batch2d_vertices += vertices;
	
	return true;
}

/*
	Функция	: nglBatch3dDrawMesh

	Описание: Рисует 3d модель

	История	: 01.05.13	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch3dDrawMesh(unsigned int batch_currenttex, unsigned int vertices, nv_3dvertex_type *varray)
{
	(void)varray; // Неиспользуемая переменная

	if(ngl_batch_state != NGL_BATCH_STATE_3D || !ngl_isinit || vertices == 0) return false;

	nglBatchSetCurrentTexture(batch_currenttex);
	
	// Вывод 3d модели
	
	return true;
}

/*
	Функция	: nglBatch3dDrawIndexedMesh

	Описание: Рисует 3d модель с индексированными вершинами

	История	: 14.02.17	Создан

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

	// Установка матрици проекции
	
	nglBatchSetCurrentTexture(nglBatchGetCurrentTexture());
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
	(void)matrix; // Неиспользуемая переменная

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

	nglBatchSetCurrentTexture(nglBatchGetCurrentTexture());
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
	(void)buffer; // Неиспользуемая переменная

	if (!ngl_isinit) return false;

	if(ngl_batch_state == NGL_BATCH_STATE_2D) nglBatch2dEnd();
	if(ngl_batch_state == NGL_BATCH_STATE_3D) nglBatch3dEnd();
	
	// Вывод в buffer содержимого экрана. Формат ngl_winx x ngl_winy x RGBA
	return true;
}
