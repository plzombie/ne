/*
	Файл	: nyan_au_main.c

	Описание: Аудиодвижок. Основные функции

	История	: 31.05.12	Создан

*/

#include <stdlib.h>
#include <string.h>

#include "../commonsrc/core/nyan_array.h"

#include "nyan_au_init_publicapi.h"
#include "nyan_au_main_publicapi.h"
#include "nyan_log_publicapi.h"
#include "nyan_mem_publicapi.h"
#include "nyan_text.h"

#include "nyan_apifordlls.h"

#include "nyan_nalapi.h"

#include "nyan_au_init.h"
#include "nyan_au_main.h"

na_buffer_type *na_buffers = 0;
unsigned int na_allocbuffers = 0;
unsigned int na_maxbuffers = 0;

na_audiostream_type *na_audiostreams = 0;
unsigned int na_allocaudiostreams = 0;
unsigned int na_maxaudiostreams = 0;

/*
	Функция	: naCheckBufferArray

	Описание: Проверяет свободное место для аудиобуффера в массиве

	История	: 12.03.23	Создан

*/
static bool naCheckBufferArray(void *array_el, bool set_free)
{
	na_buffer_type *el;
	
	el = (na_buffer_type *)array_el;
	
	if(set_free) el->status = NA_BUFFER_STATUS_FREE;
	
	return (el->status == NA_BUFFER_STATUS_FREE)?true:false;
}

/*
	Функция	: naCreateBuffer

	Описание: Создаёт буфер

	История	: 26.06.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT naCreateBuffer(const wchar_t *fname)
{
	unsigned int i = 0;

	if(!na_isinit) return 0;

	nlPrint(LOG_FDEBUGFORMAT6, F_NACREATEBUFFER, N_FNAME, fname); nlAddTab(1);

	if(
		!nArrayAdd(&n_ea, (void **)(&na_buffers),
		&na_maxbuffers,
		&na_allocbuffers,
		naCheckBufferArray,
		&i,
		NYAN_ARRAY_DEFAULT_STEP,
		sizeof(na_buffer_type))
	) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NACREATEBUFFER, N_FALSE, N_ID, 0);
		return false;
	}

	na_buffers[i].fname = nAllocMemory((wcslen(fname)+1)*sizeof(wchar_t));
	if(!na_buffers[i].fname) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NACREATEBUFFER, N_FALSE, N_ID, 0);
		return false;
	}
	wmemcpy(na_buffers[i].fname, fname, wcslen(fname)+1);

	na_buffers[i].nalid = funcptr_nalCreateBuffer();
	if(!na_buffers[i].nalid) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NACREATEBUFFER, N_FALSE, N_ID, 0);
		nFreeMemory(na_buffers[i].fname);
		return false;
	}

	na_buffers[i].status = NA_BUFFER_STATUS_EMPTY;

	i++;

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NACREATEBUFFER, N_OK, N_ID, i);

	return (i);
}

/*
	Функция	: naDestroyAllBuffers

	Описание: Уничтожает все буферы

	История	: 26.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT naDestroyAllBuffers(void)
{
	bool success = true;

	if(!na_isinit) return false;

	if(na_allocbuffers > 0) {
		unsigned int i;

		for(i=0;i<na_maxbuffers;i++)
			if(na_buffers[i].status != NA_BUFFER_STATUS_FREE) {
				if(!naDestroyBuffer(i+1))
					success = false;
			}

		if(success) {
			nFreeMemory(na_buffers);
			na_buffers = 0;
			na_allocbuffers = 0;
			na_maxbuffers = 0;
		}
	}
	return success;
}

/*
	Функция	: naDestroyBuffer

	Описание: Уничтожает буфер

	История	: 26.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT naDestroyBuffer(unsigned int id)
{
	bool success;

	if(!na_isinit || id > na_maxbuffers || id == 0) { nlPrint(LOG_FDEBUGFORMAT5, F_NADESTROYBUFFER, N_FALSE, N_ID, id); return false; }

	if(na_buffers[id-1].status == NA_BUFFER_STATUS_FREE) { nlPrint(LOG_FDEBUGFORMAT5, F_NADESTROYBUFFER, N_FALSE, N_ID, id); return false; }

	nlPrint(LOG_FDEBUGFORMAT4, F_NADESTROYBUFFER, N_ID, id); nlAddTab(1);

	success = funcptr_nalDestroyBuffer(na_buffers[id-1].nalid);

	if(success) {
		nFreeMemory(na_buffers[id-1].fname);
		na_buffers[id-1].status = NA_BUFFER_STATUS_FREE;
	}

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NADESTROYBUFFER, success?N_OK:N_FALSE);

	return success;
}
#include "nyan_threads.h"
/*
	Функция	: naLoadBuffer

	Описание: Загружает буфер

	История	: 26.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT naLoadBuffer(unsigned int id)
{
	bool success = false;
	na_audiofile_type aud;

	if(!na_isinit || id > na_maxbuffers || id == 0) { nlPrint(LOG_FDEBUGFORMAT5, F_NALOADBUFFER, N_FALSE, N_ID, id); return false; }

	if(na_buffers[id-1].status != NA_BUFFER_STATUS_EMPTY) { nlPrint(LOG_FDEBUGFORMAT5, F_NALOADBUFFER, N_FALSE, N_ID, id); return false; }

	nlPrint(LOG_FDEBUGFORMAT4, F_NALOADBUFFER, N_ID, id); nlAddTab(1);

	success = naLoadAudioFile(na_buffers[id-1].fname, &aud);

	if(success) {
		void *buf;

		buf = nAllocMemory(aud.nos*aud.noc*aud.bps);
		if(!buf) {
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NALOADBUFFER, N_FALSE, N_SUCCESS, 0);
			naUnloadAudioFile(&aud);
			return false;
		}

		success = naReadAudioFile(0, aud.nos, buf, &aud);
		naUnloadAudioFile(&aud);

		if(success) {
			success = funcptr_nalLoadBuffer(na_buffers[id-1].nalid, aud.nos, aud.noc, aud.freq, aud.sf, aud.bps, buf);

			if(success)
				na_buffers[id-1].status = NA_BUFFER_STATUS_LOADED;
		}
		nFreeMemory(buf);
	}

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NALOADBUFFER, N_OK, N_SUCCESS, success);

	return success;
}

/*
	Функция	: naUnloadBuffer

	Описание: Выгружает буфер

	История	: 26.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT naUnloadBuffer(unsigned int id)
{
	bool success;

	if(!na_isinit || id > na_maxbuffers || id == 0) { nlPrint(LOG_FDEBUGFORMAT5, F_NAUNLOADBUFFER, N_FALSE, N_ID, id); return false; }

	if(na_buffers[id-1].status != NA_BUFFER_STATUS_LOADED) { nlPrint(LOG_FDEBUGFORMAT5, F_NAUNLOADBUFFER, N_FALSE, N_ID, id); return false; }

	nlPrint(LOG_FDEBUGFORMAT4, F_NAUNLOADBUFFER, N_ID, id); nlAddTab(1);

	success = funcptr_nalUnloadBuffer(na_buffers[id-1].nalid);

	if(success)
		na_buffers[id-1].status = NA_BUFFER_STATUS_EMPTY;

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NAUNLOADBUFFER, success?N_OK:N_FALSE);

	return success;
}

/*
	Функция	: naCreateSource

	Описание: Создаёт источник звука

	История	: 26.06.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT naCreateSource(unsigned int bufid,int loop)
{
	unsigned int id;

	if(!na_isinit || bufid > na_maxbuffers || bufid == 0) { nlPrint(LOG_FDEBUGFORMAT5, F_NACREATESOURCE, N_FALSE, N_ID, 0); return false; }

	if(na_buffers[bufid-1].status != NA_BUFFER_STATUS_LOADED) { nlPrint(LOG_FDEBUGFORMAT5, F_NACREATESOURCE, N_FALSE, N_ID, 0); return false; }

	nlPrint(LOG_FDEBUGFORMAT4, F_NACREATESOURCE, N_ID, bufid); nlAddTab(1);

	id = funcptr_nalCreateSource(na_buffers[bufid-1].nalid, loop);

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NACREATESOURCE, N_OK, N_ID, id);

	return (id);
}

/*
	Функция	: naDestroyAllSources

	Описание: Уничтожает все источники звука

	История	: 26.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT naDestroyAllSources(void)
{
	if(!na_isinit) return false;

	return funcptr_nalDestroyAllSources();
}

/*
	Функция	: naDestroySource

	Описание: Уничтожает источник звука

	История	: 26.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT naDestroySource(unsigned int id)
{
	bool success;

	if(!na_isinit) return false;

	nlPrint(LOG_FDEBUGFORMAT4, F_NADESTROYSOURCE, N_ID, id); nlAddTab(1);

	success = funcptr_nalDestroySource(id);

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NADESTROYSOURCE, success?N_OK:N_FALSE);

	return success;
}

/*
	Функция	: naPlaySource

	Описание: Play

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT naPlaySource(unsigned int id)
{
	if(!na_isinit) return;

	funcptr_nalPlaySource(id);
}

/*
	Функция	: naPauseSource

	Описание: Pause

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT naPauseSource(unsigned int id)
{
	if(!na_isinit) return;

	funcptr_nalPauseSource(id);
}

/*
	Функция	: naStopSource

	Описание: Stop

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT naStopSource(unsigned int id)
{
	if(!na_isinit) return;

	funcptr_nalStopSource(id);
}

/*
	Функция	: naReplaySource

	Описание: Replay

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT naReplaySource(unsigned int id)
{
	if(!na_isinit) return;

	funcptr_nalReplaySource(id);
}

/*
	Функция	: naGetSourceLoop

	Описание: Возвращает параметр loop (зацикливание)

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT naGetSourceLoop(unsigned int id, int *loop)
{
	if(!na_isinit) return;

	funcptr_nalGetSourceLoop(id, loop);
}

/*
	Функция	: naSetSourceLoop

	Описание: Устанавливает зацикливание

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT naSetSourceLoop(unsigned int id, int loop)
{
	if(!na_isinit) return;

	funcptr_nalSetSourceLoop(id, loop);
}

/*
	Функция	: naGetSourceSecOffset

	Описание: Возвращает позицию воспроизведения в секундах

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT naGetSourceSecOffset(unsigned int id, unsigned int *offset)
{
	if(!na_isinit) return;

	funcptr_nalGetSourceSecOffset(id, offset);
}

/*
	Функция	: naSetSourceSecOffset

	Описание: Устанавливает позицию воспроизведения в секундах

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT naSetSourceSecOffset(unsigned int id, unsigned int offset)
{
	if(!na_isinit) return;

	funcptr_nalSetSourceSecOffset(id, offset);
}

/*
	Функция	: naGetSourceLength

	Описание: Возвращает длину [используемого буфера] в секундах

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT naGetSourceLength(unsigned int id, unsigned int *length)
{
	if(!na_isinit) return;

	funcptr_nalGetSourceLength(id, length);
}

/*
	Функция	: naGetSourceGain

	Описание: Возвращает громкость

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT naGetSourceGain(unsigned int id, float *gain)
{
	if(!na_isinit) return;

	funcptr_nalGetSourceGain(id, gain);
}

/*
	Функция	: naSetSourceGain

	Описание: Устанавливает громкость

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT naSetSourceGain(unsigned int id, float gain)
{
	if(!na_isinit) return;

	funcptr_nalSetSourceGain(id, gain);
}

/*
	Функция	: nalGetSourceStatus

	Описание: Возвращает статус источника звука (не создан, остановлен, проигрывается, на паузе)

	История	: 17.08.17	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT naGetSourceStatus(unsigned int id)
{
	if(!na_isinit) return NA_SOURCE_FREE;

	return funcptr_nalGetSourceStatus(id);
}

/*
	Функция	: naCreateAudioStream

	Описание: Создаёт аудиопоток

	История	: 15.09.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT naCreateAudioStream(const wchar_t *fname, int loop)
{
	unsigned int id;

	if(!na_isinit) return 0;

	nlPrint(LOG_FDEBUGFORMAT6, F_NACREATEAUDIOSTREAM, N_FNAME, fname); nlAddTab(1);

	id = naCreateAudioStreamEx(&fname, 1, loop);

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NACREATEAUDIOSTREAM, N_OK, N_ID, id);

	return id;
}

/*
	Функция	: naCheckAudioStreamArray

	Описание: Проверяет свободное место для аудиопотока в массиве

	История	: 12.03.23	Создан

*/
static bool naCheckAudioStreamArray(void *array_el, bool set_free)
{
	na_audiostream_type *el;
	
	el = (na_audiostream_type *)array_el;
	
	if(set_free) el->used = false;
	
	return (el->used == false)?true:false;
}

/*
	Функция	: naCreateAudioStreamEx

	Описание: Создаёт аудиопоток

	История	: 14.09.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT naCreateAudioStreamEx(const wchar_t **fname, unsigned int files, int loop)
{
	unsigned int i = 0, j;

	if(!na_isinit) return 0;

	nlPrint(LOG_FDEBUGFORMAT4, F_NACREATEAUDIOSTREAMEX, N_FNAMES, files); nlAddTab(1);

	if(
		!nArrayAdd(&n_ea, (void **)(&na_audiostreams),
		&na_maxaudiostreams,
		&na_allocaudiostreams,
		naCheckAudioStreamArray,
		&i,
		NYAN_ARRAY_DEFAULT_STEP,
		sizeof(na_audiostream_type))
	) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NACREATEAUDIOSTREAMEX, N_FALSE, N_ID, 0);
		return false;
	}

	na_audiostreams[i].aud = nAllocMemory(files*sizeof(na_audiofile_type));
	if(!na_audiostreams[i].aud) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NACREATEAUDIOSTREAM, N_FALSE, N_ID, 0);
		return false;
	}
	na_audiostreams[i].noafs = files;

	for(j = 0;j < files;j++) {
		if(!naLoadAudioFile(fname[j], na_audiostreams[i].aud+j)) {
			unsigned int k;

			for(k = 0; k < j; k++)
				naUnloadAudioFile(na_audiostreams[i].aud+k);

			nFreeMemory(na_audiostreams[i].aud);

			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NACREATEAUDIOSTREAM, N_FALSE, N_ID, 0);
			return false;
		}
	}

	na_audiostreams[i].nalid = funcptr_nalCreateAudioStream(na_audiostreams[i].aud, files, naReadAudioFile, loop);

	if(na_audiostreams[i].nalid) {
		na_audiostreams[i].used = true;
		i++;
	} else {
		for(j = 0; j < files; j++)
			naUnloadAudioFile(na_audiostreams[i].aud+j);

		nFreeMemory(na_audiostreams[i].aud);

		i = 0;
	}

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NACREATEAUDIOSTREAMEX, N_OK, N_ID, i);

	return i;
}

/*
	Функция	: naPlayAudioStream

	Описание: Play

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT naPlayAudioStream(unsigned int id)
{
	if(!na_isinit || id > na_maxaudiostreams || id == 0) return;

	if(!na_audiostreams[id-1].used) return;

	funcptr_nalPlayAudioStream(na_audiostreams[id-1].nalid);
}

/*
	Функция	: naPauseAudioStream

	Описание: Pause

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT naPauseAudioStream(unsigned int id)
{
	if(!na_isinit || id > na_maxaudiostreams || id == 0) return;

	if(!na_audiostreams[id-1].used) return;

	funcptr_nalPauseAudioStream(na_audiostreams[id-1].nalid);
}

/*
	Функция	: naStopAudioStream

	Описание: Stop

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT naStopAudioStream(unsigned int id)
{
	if(!na_isinit || id > na_maxaudiostreams || id == 0) return;

	if(!na_audiostreams[id-1].used) return;

	funcptr_nalStopAudioStream(na_audiostreams[id-1].nalid);
}

/*
	Функция	: naReplayAudioStream

	Описание: Replay

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT naReplayAudioStream(unsigned int id)
{
	if(!na_isinit || id > na_maxaudiostreams || id == 0) return;

	if(!na_audiostreams[id-1].used) return;

	funcptr_nalReplayAudioStream(na_audiostreams[id-1].nalid);
}

/*
	Функция	: naGetAudioStreamLoop

	Описание: Возвращает параметр loop (зацикливание)

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT naGetAudioStreamLoop(unsigned int id, int *loop)
{
	if(!na_isinit || id > na_maxaudiostreams || id == 0) return;

	if(!na_audiostreams[id-1].used) return;

	funcptr_nalGetAudioStreamLoop(na_audiostreams[id-1].nalid, loop);
}

/*
	Функция	: naSetAudioStreamLoop

	Описание: Устанавливает зацикливание

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT naSetAudioStreamLoop(unsigned int id, int loop)
{
	if(!na_isinit || id > na_maxaudiostreams || id == 0) return;

	if(!na_audiostreams[id-1].used) return;

	funcptr_nalSetAudioStreamLoop(na_audiostreams[id-1].nalid, loop);
}

/*
	Функция	: naGetAudioStreamSecOffset

	Описание: Возвращает позицию воспроизведения в секундах

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT naGetAudioStreamSecOffset(unsigned int id, unsigned int *offset, unsigned int *track)
{
	if(!na_isinit || id > na_maxaudiostreams || id == 0) return;

	if(!na_audiostreams[id-1].used) return;

	funcptr_nalGetAudioStreamSecOffset(na_audiostreams[id-1].nalid, offset, track);
}

/*
	Функция	: naSetAudioStreamSecOffset

	Описание: Устанавливает позицию воспроизведения в секундах

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT naSetAudioStreamSecOffset(unsigned int id, unsigned int offset, unsigned int track)
{
	if(!na_isinit || id > na_maxaudiostreams || id == 0) return;

	if(!na_audiostreams[id-1].used) return;

	funcptr_nalSetAudioStreamSecOffset(na_audiostreams[id-1].nalid, offset, track);
}

/*
	Функция	: naGetAudioStreamLength

	Описание: Возвращает длину [используемого буфера] в секундах

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT naGetAudioStreamLength(unsigned int id, unsigned int *length, unsigned int *tracks)
{
	if(!na_isinit || id > na_maxaudiostreams || id == 0) return;

	if(!na_audiostreams[id-1].used) return;

	funcptr_nalGetAudioStreamLength(na_audiostreams[id-1].nalid, length, tracks);
}

/*
	Функция	: naGetAudioStreamGain

	Описание: Возвращает громкость

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT naGetAudioStreamGain(unsigned int id, float *gain)
{
	if(!na_isinit || id > na_maxaudiostreams || id == 0) return;

	if(!na_audiostreams[id-1].used) return;

	funcptr_nalGetAudioStreamGain(na_audiostreams[id-1].nalid, gain);
}

/*
	Функция	: naSetAudioStreamGain

	Описание: Устанавливает громкость

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT naSetAudioStreamGain(unsigned int id, float gain)
{
	if(!na_isinit || id > na_maxaudiostreams || id == 0) return;

	if(!na_audiostreams[id-1].used) return;

	funcptr_nalSetAudioStreamGain(na_audiostreams[id-1].nalid, gain);
}

/*
	Функция	: nalGetAudioStreamStatus

	Описание: Возвращает статус аудиопотока (не создан, остановлен, проигрывается, на паузе)

	История	: 17.08.17	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT naGetAudioStreamStatus(unsigned int id)
{
	if(!na_isinit || id > na_maxaudiostreams || id == 0) return NA_SOURCE_FREE;

	if(!na_audiostreams[id-1].used) return NA_SOURCE_FREE;

	return funcptr_nalGetAudioStreamStatus(na_audiostreams[id-1].nalid);
}

/*
	Функция	: naDestroyAudioStream

	Описание: Уничтожает аудиопоток

	История	: 26.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT naDestroyAudioStream(unsigned int id)
{
	bool success;

	if(!na_isinit || id > na_maxaudiostreams || id == 0) { nlPrint(LOG_FDEBUGFORMAT5, F_NADESTROYAUDIOSTREAM, N_FALSE, N_ID, id); return false; }

	if(!na_audiostreams[id-1].used) { nlPrint(LOG_FDEBUGFORMAT5, F_NADESTROYAUDIOSTREAM, N_FALSE, N_ID, id); return false; }

	nlPrint(LOG_FDEBUGFORMAT4, F_NADESTROYAUDIOSTREAM, N_ID, id); nlAddTab(1);

	success = funcptr_nalDestroyAudioStream(na_audiostreams[id-1].nalid);

	if(success) {
		unsigned int i;

		for(i = 0;i < na_audiostreams[id-1].noafs;i++)
			naUnloadAudioFile(&na_audiostreams[id-1].aud[i]);

		nFreeMemory(na_audiostreams[id-1].aud);
		na_audiostreams[id-1].used = false;
	}

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NADESTROYAUDIOSTREAM, success?N_OK:N_FALSE);

	return success;
}

/*
	Функция	: naDestroyAllAudioStreams

	Описание: Уничтожает все аудиопотоки

	История	: 24.07.12	Создан

*/
N_API bool N_APIENTRY_EXPORT naDestroyAllAudioStreams(void)
{
	bool success = true;

	if(!na_isinit) return false;

	if(na_allocaudiostreams > 0) {
		unsigned int i;

		for(i=0;i<na_maxaudiostreams;i++)
			if(na_audiostreams[i].used) {
				if(!naDestroyAudioStream(i+1))
					success = false;
			}

		if(success) {
			nFreeMemory(na_audiostreams);
			na_audiostreams = 0;
			na_allocaudiostreams = 0;
			na_maxaudiostreams = 0;
		}
	}

	return success;
}
