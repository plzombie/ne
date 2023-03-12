/*
	Файл	: nyan_au_main.h

	Описание: Заголовок для nyan_au_main.c

	История	: 31.05.12	Создан

*/


// na_buffers.status
#define NA_BUFFER_STATUS_FREE 0
#define NA_BUFFER_STATUS_EMPTY 1
#define NA_BUFFER_STATUS_LOADED 3

typedef struct {
	int status; // Состояние буфера
	//unsigned int attachedsources;
	unsigned int nalid;
	wchar_t *fname;
} na_buffer_type;

typedef struct {
	bool used;
	unsigned int nalid;
	unsigned int noafs; // Количество аудиофайлов, прикреплённых к источнику звука
	na_audiofile_type *aud;
} na_audiostream_type;

extern na_buffer_type *na_buffers;
extern unsigned int na_allocbuffers;
extern unsigned int na_maxbuffers;

extern na_audiostream_type *na_audiostreams;
extern unsigned int na_allocaudiostreams;
extern unsigned int na_maxaudiostreams;
