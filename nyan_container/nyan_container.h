/*
	Файл	: nyan_container.h

	Описание: Заголовок для формата мультимедиа контейнеров движка

	История	: 30.08.13	Создан

*/

#ifndef NYAN_CONTAINER_H
#define NYAN_CONTAINER_H

// Заголовок чанка
// cname - имя чанка
// csize - размер чанка (не считая заголовка)
typedef struct {
	char cname[8];
	unsigned long long csize;
} nyan_chunkhead_type;

typedef struct {
	char filetype[4]; // Тип файла
	unsigned short reader_version; // Минимальная версия совместимой программы-ридера
	unsigned short writer_version; // Версия совместимой программы, создавшей файл
} nyan_filetype_type;

typedef struct {
	nyan_chunkhead_type chunkhead; // chunkhead.cname = {'F', 'I', 'L', 'E', 'T', 'Y', 'P', 'E'}; chunkhead.csize = 8;
	nyan_filetype_type filetype;
} nyan_filetypechunk_type;

extern void ncfSetFileType(nyan_filetypechunk_type *ftchunk, char filetype[4], unsigned short reader_version, unsigned short writer_version);

#endif
