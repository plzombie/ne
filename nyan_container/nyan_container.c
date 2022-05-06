/*
	Файл	: nyan_container.c

	Описание: Функции для работы с мультимедиа контейнерами движка

	История	: 30.08.13	Создан

*/

#include "nyan_container.h"
#include <string.h>

/*
	Функция	: ncfSetFileType

	Описание: Функция для формирования чанка FILETYPE

	История	: 30.08.13	Создан

*/
void ncfSetFileType(nyan_filetypechunk_type *ftchunk, char filetype[4], unsigned short reader_version, unsigned short writer_version)
{
	memcpy((*ftchunk).chunkhead.cname, "FILETYPE", 8);
	(*ftchunk).chunkhead.csize = 8;
	
	memcpy((*ftchunk).filetype.filetype, filetype, 4);
	(*ftchunk).filetype.reader_version = reader_version;
	(*ftchunk).filetype.writer_version = writer_version;
}
