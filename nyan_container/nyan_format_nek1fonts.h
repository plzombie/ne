/*
	Файл	: nyan_format_nek1fonts.h

	Описание: Заголовок структур для формата nek1 (шрифты)

	История	: 12.07.17	Создан

*/

#ifndef NYAN_FORMAT_NEK1FONTS_H
#define NYAN_FORMAT_NEK1FONTS_H

typedef struct {
	// Глиф рисуется из координат (offx, offy) в (offx+twid, offy+thei)
	// wid, hei - ширина и высота символа
	// wid - к-во пикселей, которые надо прибавить к координате x следующего символа
	// hei - к-во пикселей, которые надо прибавить к координате y идущей под символом строки
	unsigned short wid; // Оригинальная ширина символа
	unsigned short hei; // Оригинальная высота символа
	unsigned short twid; // Ширина глифа на текстуре
	unsigned short thei; // Высота глифа на текстуре
	short offx; // Смещение символа по x (Для восстановления оригинального размера)
	short offy; // Смещение символа по y
	// Позиция глифа на текстуре
	// (txstart, tystart) - левый нижний угол
	// (txend, tyend) - правый верхний угол
	float txstart; 
	float txend;
	float tystart;
	float tyend;
} nek1_glyph_type;

typedef struct {
	unsigned int symbol1;
	unsigned int symbol2;
	float kerning;
	float reserved;
} nek1_kpair_type;

typedef struct {
	unsigned int noofkpairs; // Количество кернинговых пар
} nek1_kpairshead_type;

#endif
