/*
	Файл	: nyan_vis_justdraw.c

	Описание: Простой вывод 2d и 3d графики

	История	: 07.01.17	Создан

*/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "nyan_vis_justdraw_publicapi.h"

#include "nyan_vis_draw.h"
#include "nyan_vis_justdraw.h"
#include "nyan_vis_texture.h"
#include "nyan_vis_init.h"

#include "nyan_nglapi.h"

N_API void N_APIENTRY_EXPORT nvJustDraw2dTriangle(float x1, float y1, float x2, float y2, float x3, float y3, unsigned int color1, unsigned int color2, unsigned int color3)
{
	nv_2dvertex_type varray[3];
	
	if(nv_draw_state != NV_DRAW_STATE_2D || !nv_isinit) return;

	varray[0].tx = varray[0].ty = varray[0].z = varray[1].tx = varray[1].ty = varray[1].z = varray[2].tx = varray[2].ty = varray[2].z = 0;
	
	varray[0].x = x1;
	varray[0].y = y1;
	varray[0].colorRGBA = color1;
	
	varray[1].x = x2;
	varray[1].y = y2;
	varray[1].colorRGBA = color2;

	varray[2].x = x3;
	varray[2].y = y3;
	varray[2].colorRGBA = color3;

	funcptr_nglBatch2dAdd(NV_DRAWTRIANGLE, 0, 3, varray);
}

N_API void N_APIENTRY_EXPORT nvJustDraw2dLine(float x1, float y1, float x2, float y2, unsigned int color1, unsigned int color2)
{
	nv_2dvertex_type varray[2];
	
	if(nv_draw_state != NV_DRAW_STATE_2D || !nv_isinit) return;

	varray[0].tx = varray[0].ty = varray[0].z = varray[1].tx = varray[1].ty = varray[1].z = 0;
	
	varray[0].x = x1;
	varray[0].y = y1;
	varray[0].colorRGBA = color1;
	
	varray[1].x = x2;
	varray[1].y = y2;
	varray[1].colorRGBA = color2;

	funcptr_nglBatch2dAdd(NV_DRAWLINE, 0, 2, varray);
}

N_API void N_APIENTRY_EXPORT nvJustDraw2dPoint(float x, float y, unsigned int color)
{
	nv_2dvertex_type varray[1];
	
	if(nv_draw_state != NV_DRAW_STATE_2D || !nv_isinit) return;

	varray[0].tx = varray[0].ty = varray[0].z = 0;
	
	varray[0].x = x;
	varray[0].y = y;
	varray[0].colorRGBA = color;

	funcptr_nglBatch2dAdd(NV_DRAWPOINT, 0, 1, varray);
}
