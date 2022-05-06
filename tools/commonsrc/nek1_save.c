/*
	Файл	: nek1_save.c

	Описание: Код сохранения шрифтов в формат nek1

	История	: 15.07.17	Создан

*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <string.h> 
#include <io.h>
#include <math.h>

#include "nek1_save.h"

bool nek1Save(char *filename, nek1_font_type *font)
{
	int f;
	unsigned int noofallocatedblocks; // Количество блоков, под которые выделена память (т.е. суммарное кол-во эл-тов font.blockalloc, содержащих true)
	unsigned int i;
	nyan_filetypechunk_type nek1; // Чанк, содержащий тип файла
	nyan_chunkhead_type chunkhead; // Заголовок чанка

	// Запись шрифта в файл	
	f = _open(filename, _O_CREAT | _O_TRUNC | _O_WRONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
	if(!f) {
		printf("%s", "error: can't open output file\n");
		return false;
	}

	noofallocatedblocks = 0;
	for(i = 0; i < font->noofblocks; i++) {
		if(font->blockalloc[i] == true)
			noofallocatedblocks++;
	}

	// Запись чанка, содержащего тип файла
	ncfSetFileType(&nek1, "NEK1", NEK1_READERVERSION, NEK1_WRITERVERSION);
	_write(f, &nek1, sizeof(nek1));

	// Запись чанка, содержащего заголовок файла
	memcpy(chunkhead.cname, "FILEHEAD", 8);
	chunkhead.csize = 4;
	_write(f, &chunkhead, sizeof(chunkhead));
	_write(f, &(font->noofblocks), 4);
	// Запись чанка, содержащего данные файла
	if(font->noofblocks) {
		memcpy(chunkhead.cname, "FILEDATA", 8);
		chunkhead.csize = font->noofblocks + noofallocatedblocks * sizeof(nek1_glyph_type) * 1024;
		_write(f, &chunkhead, sizeof(chunkhead));
		_write(f, font->blockalloc, font->noofblocks);
		for(i = 0; i <= font->noofblocks; i++)
			if(font->blockalloc[i])
				_write(f, font->glyphs[i], sizeof(nek1_glyph_type) * 1024);
	}
	// Запись чанков, содержащих информацию о кернинге
	if(font->kpairshead.noofkpairs) {
		memcpy(chunkhead.cname, "KPRSHEAD", 8);
		chunkhead.csize = 4;
		_write(f, &chunkhead, sizeof(chunkhead));
		_write(f, &(font->kpairshead), 4);

		memcpy(chunkhead.cname, "KPRSDATA", 8);
		chunkhead.csize = sizeof(nek1_kpair_type)*font->kpairshead.noofkpairs;
		_write(f, &chunkhead, sizeof(chunkhead));
		_write(f, font->kpairs, sizeof(nek1_kpair_type)*font->kpairshead.noofkpairs);
	}
	_close(f);

	return true;
}

