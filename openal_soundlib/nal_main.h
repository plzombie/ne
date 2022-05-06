/*
	Файл	: nal_main.h

	Описание: Заголовок для nal_main.c

	История	: 25.12.12	Создан

*/

#include "../nyan/nyan_audiofile_publicapi.h"

// nal_buffers.status
#define NAL_BUFFER_STATUS_FREE 0
#define NAL_BUFFER_STATUS_EMPTY 1
#define NAL_BUFFER_STATUS_LOADED 2

// nal_audiostream.status
#define NAL_SSOURCE_STOP 0
#define NAL_SSOURCE_PLAY 1
#define NAL_SSOURCE_PAUSE 2
#define NAL_SSOURCE_REPLAY 3
#define NAL_SSOURCE_GOTO 4
#define NAL_SSOURCE_GOTO_PLAY 5

typedef struct {
	int status; // Состояние буфера
	unsigned int attachedsources;
	unsigned int alid;
} nal_buffer_type;

typedef struct {
	unsigned int bufid;
	ALuint alid; // source id
	ALint initialpos; // Текущая позиция
	bool used;
} nal_source_type;

typedef struct {
	unsigned int bufid[4];
	unsigned int noafs; // Количество аудиофайлов, прикреплённых к источнику звука
	unsigned int curaf; // Текущий аудиофайл
	unsigned int curraf; // Текущий читаемый аудиофайл
	na_audiofile_plg_read_type readaf; // Функция для чтения аудиофайла
	na_audiofile_type *aud;
	unsigned int curpos; // Текущая позиция в секундах (от 0)
	unsigned int currpos; // Количество прочитанных из файла секунд (от 0)
	unsigned int *secs; // Количество секунд
	unsigned int status;
	ALuint alid; // source id
	unsigned int *wavFormat;
	unsigned char *bufdata;
	bool used;
	bool loop;
	bool mustreload;
	unsigned char nobs; // Количество буферов, от 1 до 4
} nal_audiostream_type;

extern nal_buffer_type *nal_buffers;
extern unsigned int nal_maxbuffers;

extern nal_source_type *nal_sources;
extern unsigned int nal_maxsources;

extern nal_audiostream_type *nal_audiostreams;
extern unsigned int nal_maxaudiostreams;

extern bool nalCreateAST(void);
extern void nalDestroyAST(void);
extern N_API bool N_APIENTRY_EXPORT nalDestroyAllBuffers(void);
extern N_API bool N_APIENTRY_EXPORT nalDestroyAllSources(void);
extern N_API bool N_APIENTRY_EXPORT nalDestroyAllAudioStreams(void);
extern N_API bool N_APIENTRY_EXPORT nalDestroyBuffer(unsigned int id);
extern N_API bool N_APIENTRY_EXPORT nalUnloadBuffer(unsigned int id);
extern N_API bool N_APIENTRY_EXPORT nalDestroySource(unsigned int id);
extern N_API unsigned int N_APIENTRY_EXPORT nalCreateAudioStreamEx(wchar_t **fname, unsigned int files, int loop);
