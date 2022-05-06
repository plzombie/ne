/*
	Файл	: nyan_draw_publicapi.h

	Описание: Некоторые константы, отвечающие за рисование

	История	: 15.08.12	Создан

*/

#ifndef NYAN_DRAW_PUBLICAPI_H
#define NYAN_DRAW_PUBLICAPI_H

// Возвращает цвет вершины
#define NV_COLOR(R,G,B,A) ((unsigned int)(((int)(A) << 24) | ((int)(B) << 16) | ((int)(G) << 8) | (int)(R)))

// Тип, описывающий вершину(2D)
typedef struct {
	float tx;
	float ty;
	unsigned int colorRGBA;
	float x;
	float y;
	float z;
} nv_2dvertex_type;

// Тип, описывающий вершину(3D)
typedef struct {
	float tx;
	float ty;
	float cr;
	float cg;
	float cb;
	float ca;
	float nx;
	float ny;
	float nz;
	float x;
	float y;
	float z;
} nv_3dvertex_type;

// Тип, описывающий дельту между двумя вершинами(3D)
typedef struct {
	float nx;
	float ny;
	float nz;
	float x;
	float y;
	float z;
} nv_3ddeltavertex_type;

// Константы, определяющие что рисовать
#define NV_DRAW_MIN 1
#define NV_DRAWPOINT 1 
#define NV_DRAWLINE 2 
#define NV_DRAWTRIANGLE 3
#define NV_DRAW_MAX 3

#endif
