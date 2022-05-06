/*
	Файл	: nyan_vis_draw_publicapi.h

	Описание: Публичные функции для вывода 2d изображения и 3d мешей, переключения между выводом 2d и 3d изображений

	История	: 04.07.17	Создан

*/

#ifndef NYAN_VIS_DRAW_PUBLICAPI_H
#define NYAN_VIS_DRAW_PUBLICAPI_H

#include <stdbool.h>

#include "nyan_decls_publicapi.h"

#include "nyan_draw_publicapi.h"

NYAN_FUNC(void, nvBegin2d, (void))
NYAN_FUNC(void, nvEnd2d, (void))
NYAN_FUNC(void, nvSetAmbientLight, (float r, float g, float b, float a))
NYAN_FUNC(void, nvGetAmbientLight, (float *r, float *g, float *b, float *a))
NYAN_FUNC(void, nvBegin3d, (void))
NYAN_FUNC(void, nvEnd3d, (void))
NYAN_FUNC(void, nvDraw2dPoint, (unsigned int batch_currenttex, nv_2dvertex_type *varray))
NYAN_FUNC(void, nvDraw2dLine, (unsigned int batch_currenttex, nv_2dvertex_type *varray))
NYAN_FUNC(void, nvDraw2dTriangle, (unsigned int batch_currenttex, nv_2dvertex_type *varray))
NYAN_FUNC(void, nvDraw2dQuad, (unsigned int batch_currenttex, nv_2dvertex_type *varray))
NYAN_FUNC(void, nvDraw2d, (unsigned int ptype, unsigned int primitives, unsigned int batch_currenttex, nv_2dvertex_type *varray))
NYAN_FUNC(bool, nvDraw2dPicture, (int x, int y, unsigned int batch_currenttex, float scalex, float scaley, unsigned int color))
NYAN_FUNC(bool, nvDraw2dButton, (int x, int y, unsigned int anims, unsigned int framelen, unsigned int stats, unsigned int batch_currenttex, float scalex, float scaley, unsigned int color, unsigned int color_focused, unsigned int color_pushed))
NYAN_FUNC(void, nvDraw3dMesh, (unsigned int batch_currenttex, unsigned int vertices, nv_3dvertex_type *varray))
NYAN_FUNC(void, nvDraw3dIndexedMesh, (unsigned int batch_currenttex, unsigned int vertices, nv_3dvertex_type *varray, unsigned int triangles, unsigned int *iarray))

#endif
