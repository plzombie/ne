/*
	Файл	: nyan_vis_draw.c

	Описание: Вывод 2d изображения и 3d мешей, переключение между выводом 2d и 3d изображений

	История	: 16.08.12	Создан

*/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "nyan_fps_publicapi.h"
#include "nyan_log_publicapi.h"
#include "nyan_vis_draw_publicapi.h"
#include "nyan_vis_init_publicapi.h"
#include "nyan_vis_texture_publicapi.h"
#include "nyan_vismodes_publicapi.h"
#include "nyan_text.h"

#include "nyan_vis_draw.h"
#include "nyan_vis_texture.h"
#include "nyan_vis_init.h"

#include "nyan_nglapi.h"

int nv_draw_state = NV_DRAW_STATE_NO;

// Глобальное освещение
float nv_global_ambient[4] = {1.0, 1.0, 1.0, 1.0};

/*
	Функция	: nvDrawInit

	Описание: Инициализирует рисование

	История	: 16.08.12	Создан

*/
void nvDrawInit(void)
{
	if(!nv_isinit) return;

	nlPrint(F_NVDRAWINIT); nlAddTab(1);

	nv_draw_state = NV_DRAW_STATE_NO;

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVDRAWINIT, N_OK);
}

/*
	Функция	: nvDrawClose

	Описание: Денициализирует рисование

	История	: 16.08.12	Создан

*/
void nvDrawClose(void)
{
	if(!nv_isinit) return;

	nlPrint(F_NVDRAWCLOSE); nlAddTab(1);

	if(nv_draw_state == NV_DRAW_STATE_2D) nvEnd2d();
	if(nv_draw_state == NV_DRAW_STATE_3D) nvEnd3d();

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVDRAWCLOSE, N_OK);
}

/*
	Функция	: nvBegin2d

	Описание: Начало рисования 2d графики

	История	: 16.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvBegin2d(void)
{
	if(nv_draw_state == NV_DRAW_STATE_2D || !nv_isinit) return;

	if(nv_draw_state == NV_DRAW_STATE_3D) funcptr_nglBatch3dEnd();

	nv_draw_state = NV_DRAW_STATE_2D;

	funcptr_nglBatch2dBegin();
}

/*
	Функция	: nvEnd2d

	Описание: Конец рисования 2d графики

	История	: 16.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvEnd2d(void)
{
	if(nv_draw_state != NV_DRAW_STATE_2D || !nv_isinit) return;

	nv_draw_state = NV_DRAW_STATE_NO;

	funcptr_nglBatch2dEnd();
}

/*
	Функция	: nvSetAmbientLight

	Описание: Устанавливает глобальное освещение

	История	: 24.04.13	Создан

*/
N_API void N_APIENTRY_EXPORT nvSetAmbientLight(float r, float g, float b, float a)
{
	if(!nv_isinit) return;

	nv_global_ambient[0] = r;
	nv_global_ambient[1] = g;
	nv_global_ambient[2] = b;
	nv_global_ambient[3] = a;

	funcptr_nglBatch3dSetAmbientLight(r, g, b, a);
}

/*
	Функция	: nvGetAmbientLight

	Описание: Возвращает глобальное освещение

	История	: 24.04.13	Создан

*/
N_API void N_APIENTRY_EXPORT nvGetAmbientLight(float *r, float *g, float *b, float *a)
{
	if(!nv_isinit) return;

	*r = nv_global_ambient[0];
	*g = nv_global_ambient[1];
	*b = nv_global_ambient[2];
	*a = nv_global_ambient[3];
}

/*
	Функция	: nvBegin3d

	Описание: Начало рисования 3d графики

	История	: 16.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvBegin3d(void)
{
	if(nv_draw_state == NV_DRAW_STATE_3D || !nv_isinit) return;

	if(nv_draw_state == NV_DRAW_STATE_2D) funcptr_nglBatch2dEnd();

	nv_draw_state = NV_DRAW_STATE_3D;

	funcptr_nglBatch3dBegin();
}

/*
	Функция	: nvEnd3d

	Описание: Конец рисования 3d графики

	История	: 16.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvEnd3d(void)
{
	if(nv_draw_state != NV_DRAW_STATE_3D || !nv_isinit) return;

	nv_draw_state = NV_DRAW_STATE_NO;

	funcptr_nglBatch3dEnd();
}

/*
	Функция	: nvDraw2dPoint

	Описание: Рисует линию

	История	: 19.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvDraw2dPoint(unsigned int batch_currenttex, nv_2dvertex_type *varray)
{
	unsigned int nglid;

	if(nv_draw_state != NV_DRAW_STATE_2D || !nv_isinit) return;

	nglid = nvGetNGLTextureId(batch_currenttex);

	funcptr_nglBatch2dAdd(NV_DRAWPOINT, nglid, 1, varray);
}

/*
	Функция	: nvDraw2dLine

	Описание: Рисует линию

	История	: 16.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvDraw2dLine(unsigned int batch_currenttex, nv_2dvertex_type *varray)
{
	unsigned int nglid;

	if(nv_draw_state != NV_DRAW_STATE_2D || !nv_isinit) return;

	nglid = nvGetNGLTextureId(batch_currenttex);

	funcptr_nglBatch2dAdd(NV_DRAWLINE, nglid, 2, varray);
}

/*
	Функция	: nvDraw2dTriangle

	Описание: Рисует треугольник

	История	: 16.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvDraw2dTriangle(unsigned int batch_currenttex, nv_2dvertex_type *varray)
{
	unsigned int nglid;

	if(nv_draw_state != NV_DRAW_STATE_2D || !nv_isinit) return;

	nglid = nvGetNGLTextureId(batch_currenttex);

	funcptr_nglBatch2dAdd(NV_DRAWTRIANGLE, nglid, 3, varray);
}

/*
	Функция	: nvDraw2dQuad

	Описание: Рисует квадрат

	История	: 16.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvDraw2dQuad(unsigned int batch_currenttex, nv_2dvertex_type *varray)
{
	unsigned int nglid;
	nv_2dvertex_type fvarray[6];

	if(nv_draw_state != NV_DRAW_STATE_2D || !nv_isinit) return;

	nglid = nvGetNGLTextureId(batch_currenttex);

	fvarray[0] = varray[0];
	fvarray[1] = varray[1];
	fvarray[2] = varray[2];
	fvarray[3] = varray[0];
	fvarray[4] = varray[2];
	fvarray[5] = varray[3];
	funcptr_nglBatch2dAdd(NV_DRAWTRIANGLE, nglid, 6, fvarray);
}

/*
	Функция	: nvDraw2d

	Описание: Рисует примитив
			ptype - Тип примитива, NV_DRAWLINE или NV_DRAWTRIANGLE
			primitives - Количество примитивов
			batch_currenttex - Id текстуры
			varray - Указатель на массив вершин
			Количество вершин - 2*primitives для NV_DRAWLINE и 3*primitives для NV_DRAWTRIANGLE

	История	: 16.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvDraw2d(unsigned int ptype, unsigned int primitives, unsigned int batch_currenttex, nv_2dvertex_type *varray)
{
	unsigned int nglid;

	if(nv_draw_state != NV_DRAW_STATE_2D || !nv_isinit) return;

	nglid = nvGetNGLTextureId(batch_currenttex);

	if(ptype >= NV_DRAW_MIN && ptype <= NV_DRAW_MAX)
		funcptr_nglBatch2dAdd(ptype, nglid, ptype*primitives, varray);
}

/*
	Функция	: nvDraw2dPicture

	Описание: Рисует изображение (по размеру указанной текстуры)
			x, y - Позиция изображения
			scalex, scaley - Растяжение изображения по х, у
			Возвращает true, если изображение под курсором мыши

	История	: 16.08.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nvDraw2dPicture(int x, int y, unsigned int batch_currenttex, float scalex, float scaley, unsigned int color)
{
	unsigned int nglid, usizex, usizey;
	nv_2dvertex_type vs[6];

	if(nv_draw_state != NV_DRAW_STATE_2D || !nv_isinit) return false;

	if(nvGetTextureNGLIdAndWH(batch_currenttex, &nglid, &usizex, &usizey)) { // Выводить изображение только, если текстура загружена
		int sizex, sizey, mx, my;
		mx = nvGetStatusi(NV_STATUS_WINMX);
		my = nvGetStatusi(NV_STATUS_WINMY);
		sizex = (int)(usizex*scalex);
		sizey = (int)(usizey*scaley);
		vs[0].x = (float)x;         vs[0].y = (float)y;         vs[0].z = 0; vs[0].colorRGBA = color; vs[0].tx = 0; vs[0].ty = 1;
		vs[1].x = (float)(x+sizex); vs[1].y = (float)y;         vs[1].z = 0; vs[1].colorRGBA = color; vs[1].tx = 1; vs[1].ty = 1;
		vs[2].x = (float)(x+sizex); vs[2].y = (float)(y+sizey); vs[2].z = 0; vs[2].colorRGBA = color; vs[2].tx = 1; vs[2].ty = 0;
		vs[3] = vs[0];
		vs[4] = vs[2];
		vs[5].x = (float)x;         vs[5].y = (float)(y+sizey); vs[5].z = 0; vs[5].colorRGBA = color; vs[5].tx = 0; vs[5].ty = 0;

		funcptr_nglBatch2dAdd(NV_DRAWTRIANGLE, nglid, 6, vs);
		if(mx>=x && my>=y && mx<x+sizex && my<y+sizey) {
			int cr_sx, cr_sy, cr_ex, cr_ey;

			cr_sx = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONSX);
			cr_sy = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONSY);
			cr_ex = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONEX);
			cr_ey = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONEY);
			
			if(mx >= cr_sx && my >= cr_sy && mx < cr_ex && my < cr_ey)
				return true;
			else
				return false;
		} else
			return false;
	} else
		return false;
}

/*
	Функция	: nvDraw2dButton

	Описание: Рисует кнопку (ширина - ширина_текстуры/количество анимаций; высота - высота_текстуры/количество_состояний)
			x, y - Позиция кнопки
			anims - Количество анимаций
			framelen - Длина кадра анимации в миллисекундах
			stats - Количество состояний
				3 - Кнопка отжата/не под курсором, кнопка под курсором мыши, кнопка нажата
				2 - Кнопка отжата/не под курсором, кнопка нажата
				1 или любое другое число - Одно состояние для всех случаев
			scalex, scaley - Растяжение кнопки по х, у
			color_focused - Цвет кнопки, когда она под курсором мыши
			color_pushed - Цвет кнопки, когда она нажата
			color - Цвет кнопки в остальных случаях
			Возвращает true, если кнопка под курсором мыши

	История	: 16.08.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nvDraw2dButton(int x, int y, unsigned int anims, unsigned int framelen, unsigned int stats, unsigned int batch_currenttex, float scalex, float scaley, unsigned int color, unsigned int color_focused, unsigned int color_pushed)
{
	unsigned int nglid, usizex, usizey;;
	nv_2dvertex_type vs[6];

	if(nv_draw_state != NV_DRAW_STATE_2D || !nv_isinit) return false;

	if(nvGetTextureNGLIdAndWH(batch_currenttex, &nglid, &usizex, &usizey)) { // Выводить изображение только, если текстура загружена
		int sizex, sizey, mx, my, mbl;
		float txstart, txend, tystart, tyend;
		unsigned int newcolor;
		int64_t framelen_clocks, framestart_clock;
		bool focused;
		mx = nvGetStatusi(NV_STATUS_WINMX);
		my = nvGetStatusi(NV_STATUS_WINMY);
		mbl = nvGetStatusi(NV_STATUS_WINMBL);
		sizex = usizex/anims;
		sizey = usizey;
		if(stats == 2 || stats == 3)
			sizey = sizey/stats;
		if(mx>=x && my>=y && mx<x+sizex && my<y+sizey) {
			int cr_sx, cr_sy, cr_ex, cr_ey;

			cr_sx = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONSX);
			cr_sy = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONSY);
			cr_ex = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONEX);
			cr_ey = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONEY);
			
			if(mx >= cr_sx && my >= cr_sy && mx < cr_ex && my < cr_ey)
				focused = true;
			else
				focused = false;
		} else
			focused = false;
		if(focused) {
			if(mbl == 1) {
				newcolor = color_pushed;
				if(stats == 2) {
					tystart = 0.0;
					tyend = 0.5;
				} else if(stats == 3) {
					tystart = 0.3333333f;
					tyend = 0.6666667f;
				} else {
					tystart = 0.0; // Всего одно состояние
					tyend = 1.0;
				}
			} else {
				newcolor = color_focused;
				if(stats == 3) {
					tystart = 0.0;
					tyend = 0.3333333f;
				} else {
					tystart = (float)(1.0-sizey/(float)usizey);
					tyend = 1.0;
				}

			}
		} else {
			tystart = (float)(1.0-sizey/(float)usizey);
			tyend = 1.0;
			newcolor = color;
		}
		framelen_clocks = framelen*N_CLOCKS_PER_SEC/1000;
		framestart_clock = nFrameStartClock();
		txstart = ( sizex*((framestart_clock/framelen_clocks)%anims) ) / (float)usizex;
		txend = ( sizex*((framestart_clock/framelen_clocks)%anims+1) ) / (float)usizex;
		sizex = (int)(sizex*scalex);
		sizey = (int)(sizey*scaley);
		vs[0].x = (float)x;         vs[0].y = (float)y;         vs[0].z = 0; vs[0].colorRGBA = newcolor; vs[0].tx = txstart; vs[0].ty = tyend;
		vs[1].x = (float)(x+sizex); vs[1].y = (float)y;         vs[1].z = 0; vs[1].colorRGBA = newcolor; vs[1].tx = txend;   vs[1].ty = tyend;
		vs[2].x = (float)(x+sizex); vs[2].y = (float)(y+sizey); vs[2].z = 0; vs[2].colorRGBA = newcolor; vs[2].tx = txend;   vs[2].ty = tystart;
		vs[3] = vs[0];
		vs[4] = vs[2];
		vs[5].x = (float)x;         vs[5].y = (float)(y+sizey); vs[5].z = 0; vs[5].colorRGBA = newcolor; vs[5].tx = txstart; vs[5].ty = tystart;

		funcptr_nglBatch2dAdd(NV_DRAWTRIANGLE, nglid, 6, vs);
		return focused;
	} else
		return false;
}

/*
	Функция	: nvDraw3dMesh

	Описание: Рисует 3d сетку

	История	: 04.02.14	Создан

*/
N_API void N_APIENTRY_EXPORT nvDraw3dMesh(unsigned int batch_currenttex, unsigned int vertices, nv_3dvertex_type *varray)
{
	unsigned int nglid;

	if(nv_draw_state != NV_DRAW_STATE_3D || !nv_isinit) return;

	nglid = nvGetNGLTextureId(batch_currenttex);

	funcptr_nglBatch3dDrawMesh(nglid, vertices, varray);
}

/*
	Функция	: nvDraw3dIndexedMesh

	Описание: Рисует 3d сетку с индексированными вершинами

	История	: 28.06.16	Создан

*/
N_API void N_APIENTRY_EXPORT nvDraw3dIndexedMesh(unsigned int batch_currenttex, unsigned int vertices, nv_3dvertex_type *varray, unsigned int triangles, unsigned int *iarray)
{
	unsigned int nglid;

	if(nv_draw_state != NV_DRAW_STATE_3D || !nv_isinit) return;

	nglid = nvGetNGLTextureId(batch_currenttex);

	funcptr_nglBatch3dDrawIndexedMesh(nglid, vertices, varray, triangles, iarray);
}
