/*
	Файл	: nek1_save.h

	Описание: Заголовок для кода сохранения шрифтов в формат nek1

	История	: 15.07.17	Создан

*/

#ifndef NEK1_SAVE_H
#define NEK1_SAVE_H

#include "../../nyan_container/nyan_container.h"
#include "../../nyan_container/nyan_format_nek1fonts.h"

#define NEK1_READERVERSION 0
#define NEK1_WRITERVERSION 0

typedef struct {
	unsigned int noofblocks; // Количество блоков
	char *blockalloc; // Если blockalloc[i] == true, то память под текущий блок выделена, если нет - блок нужно пропустить
	nek1_glyph_type **glyphs; // Память под глифы выделяется блоками по 1024.
							  // Чтобы получить доступ к символу n: s = glyphs[n/1024][n%1024];
	unsigned int allockpairs; // Количество кернинговых пар, под которые выделено памяти в kpairs
	nek1_kpairshead_type kpairshead; // Заголовок, описывающий кернинговые пары
	nek1_kpair_type *kpairs; // Кернинговые пары
} nek1_font_type;

bool nek1Save(char *filename, nek1_font_type *font);

#endif
