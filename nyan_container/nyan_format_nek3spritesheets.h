/*
	Файл	: nyan_format_nek3spritesheets.h

	Описание: Заголовок структур для формата nek3 (текстурные атласы/spritesheets)

	История	: 31.12.17	Создан

*/

#ifndef NYAN_FORMAT_NEK3SPRITESHEETS_H
#define NYAN_FORMAT_NEK3SPRITESHEETS_H

#define NEK3_SPRITETYPE_RECTANGLE  0
#define NEK3_SPRITETYPE_POLYGONAL  1

#include "../nyan/nyan_draw_publicapi.h"

// Структура спрайта-прямоугольника в файле и в памяти.
typedef struct {
	unsigned short wid; // Оригинальная ширина спрайта.
	unsigned short hei; // Оригинальная высота спрайта.
	unsigned short twid; // Ширина спрайта на текстуре.
	unsigned short thei; // Высота спрайта не текстуре.
	short offx; // Смещение спрайта по x (Для восстановления оригинального размера).
	short offy; // Смещение спрайта по y.
	float tx[4]; // Координаты спрайта на текстуре. Левый верхний, правый верхний, правый нижний и левый нижний соответственно.
	float ty[4];
} nek3_rectangle_sprite_type; // NV_SPRITETYPE_RECTANGLE

// Структура заголовка полигонального спрайта.
typedef struct {
	unsigned short wid; // Оригинальная ширина спрайта.
	unsigned short hei; // Оригинальная высота спрайта.
	unsigned int nofvertices; // Количество вершин, кратно трём.
} nek3_polygonal_sprite_head_type;

// Структура полигонального спрайта в памяти.
// В случае файла, вершины идут последовательно, после поля head.
typedef struct {
	nek3_polygonal_sprite_head_type head; // Заголовок.
	nv_2dvertex_type *vertices; // Вершины. Три вершины подряд образуют треугольник
} nek3_polygonal_sprite_type; // NV_SPRITETYPE_POLYGONAL

#endif
