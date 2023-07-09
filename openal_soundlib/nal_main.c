/*
	Файл	: nal_main.c

	Описание: Аудиодвижок. Основные функции

	История	: 25.12.12	Создан

*/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>
//#include <process.h>

#include "../forks/al/al.h"
#include "../forks/al/alc.h"

#include "../nyan/nyan_audiosourcestatus_publicapi.h"

#include "nal.h"
#include "nal_text.h"

#include "nal_init.h"
#include "nal_main.h"

#if defined(_MSC_VER) && !defined(__POCC__)
	#define inline __inline
#endif

nal_buffer_type *nal_buffers = 0; // Буферы для хранения аудио
unsigned int nal_maxbuffers = 0;

nal_source_type *nal_sources = 0;
unsigned int nal_maxsources = 0;

nal_audiostream_type *nal_audiostreams = 0;
unsigned int nal_maxaudiostreams = 0;

static unsigned int nal_task;

static uintptr_t nal_mutex;

#define STACK_SIZE    8192
#define SERVICE_UPDATE_PERIOD 20

/*
	Функция	: nalCreateBuffer

	Описание: Создаёт буфер OpenAL

	История	: 26.06.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nalCreateBuffer(void)
{
	unsigned int i;

	if(!nal_isinit) return 0;

	nal_ea->nlPrint(F_NALCREATEBUFFER); nal_ea->nlAddTab(1);

	// Выделение памяти под аудио буферы
	for(i = 0;i < nal_maxbuffers;i++)
		if(nal_buffers[i].status == NAL_BUFFER_STATUS_FREE)
			break;

	if(i == nal_maxbuffers) {
		nal_buffer_type *_nal_buffers;
		_nal_buffers = nal_ea->nReallocMemory(nal_buffers, (nal_maxbuffers+1024)*sizeof(nal_buffer_type));

		if(_nal_buffers)
			nal_buffers = _nal_buffers;
		else {
			nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NALCREATEBUFFER, N_FALSE, N_ID, 0);
			return false;
		}

		for(i=nal_maxbuffers;i<nal_maxbuffers+1024;i++)
			nal_buffers[i].status = NAL_BUFFER_STATUS_FREE;

		i = nal_maxbuffers;

		nal_maxbuffers += 1024;
	}

	nal_buffers[i].status = NAL_BUFFER_STATUS_EMPTY;
	nal_buffers[i].attachedsources = 0;

	nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NALCREATEBUFFER, N_OK, N_ID, i+1);

	return (i+1);
}

/*
	Функция	: nalDestroyAllBuffers

	Описание: Уничтожает все буферы

	История	: 26.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nalDestroyAllBuffers(void)
{
	bool success = true;

	if(!nal_isinit) return false;

	if(nal_maxbuffers > 0) {
		unsigned int i;

		for(i=0;i<nal_maxbuffers;i++)
			if(nal_buffers[i].status != NAL_BUFFER_STATUS_FREE) {
				if(!nalDestroyBuffer(i+1))
					success = false;
			}

		if(success) {
			nal_ea->nFreeMemory(nal_buffers);
			nal_buffers = 0;
			nal_maxbuffers = 0;
		}
	}

	return success;
}

/*
	Функция	: nalDestroyBuffer

	Описание: Уничтожает буфер

	История	: 26.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nalDestroyBuffer(unsigned int id)
{
	if(!nal_isinit || id > nal_maxbuffers || id == 0) return false;

	if(nal_buffers[id-1].status == NAL_BUFFER_STATUS_FREE || nal_buffers[id-1].attachedsources > 0) return false;

	nal_ea->nlPrint(LOG_FDEBUGFORMAT4, F_NALDESTROYBUFFER, N_ID, id); nal_ea->nlAddTab(1);

	nalUnloadBuffer(id);
	nal_buffers[id-1].status = NAL_BUFFER_STATUS_FREE;

	nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT, F_NALDESTROYBUFFER, N_OK);

	return true;
}

/*
	Функция	: nalLoadBuffer

	Описание: Загружает буфер
		nos - Количество семплов
		noc - Кол-во каналов
		freq - Частота
		sf - Формат звука
		bps - байт в одном семпле
		buf - передаваемые данные

	История	: 26.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nalLoadBuffer(unsigned int id, unsigned int nos, unsigned int noc, unsigned int freq, unsigned int sf, unsigned int bps, void *buf)
{
	bool success = true;
	unsigned int wavFormat = 0;

	if(!nal_isinit || id > nal_maxbuffers || id == 0) return false;

	if(nal_buffers[id-1].status != NAL_BUFFER_STATUS_EMPTY) return false;

	nal_ea->nlPrint(LOG_FDEBUGFORMAT4, F_NALLOADBUFFER, N_ID, id); nal_ea->nlAddTab(1);

	switch(sf) {
		case NA_SOUND_8BIT_MONO:
			wavFormat = AL_FORMAT_MONO8;
			break;
		case NA_SOUND_8BIT_STEREO:
			wavFormat = AL_FORMAT_STEREO8;
			break;
		case NA_SOUND_16BIT_MONO:
			wavFormat = AL_FORMAT_MONO16;
			break;
		case NA_SOUND_16BIT_STEREO:
			wavFormat = AL_FORMAT_STEREO16;
			break;
		default:
			success = false;
	}
	if(success) {
		alGenBuffers(1, &nal_buffers[id-1].alid);
		alBufferData(nal_buffers[id-1].alid, wavFormat, buf, nos*noc*bps, freq);

		nal_buffers[id-1].status = NAL_BUFFER_STATUS_LOADED;
	}

	nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NALLOADBUFFER, N_OK, N_SUCCESS, success);

	return success;
}

/*
	Функция	: nalUnloadBuffer

	Описание: Выгружает буфер

	История	: 26.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nalUnloadBuffer(unsigned int id)
{
	if(!nal_isinit || id > nal_maxbuffers || id == 0) return false;

	if(nal_buffers[id-1].status != NAL_BUFFER_STATUS_LOADED || nal_buffers[id-1].attachedsources > 0) return false;

	nal_ea->nlPrint(LOG_FDEBUGFORMAT4, F_NALUNLOADBUFFER, N_ID, id); nal_ea->nlAddTab(1);

	alDeleteBuffers(1, &nal_buffers[id-1].alid);

	nal_buffers[id-1].status = NAL_BUFFER_STATUS_EMPTY;

	nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT, F_NALUNLOADBUFFER, N_OK);

	return true;
}

/*
	Функция	: nalCreateSource

	Описание: Создаёт источник звука

	История	: 26.06.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nalCreateSource(unsigned int bufid,int loop)
{
	unsigned int i;

	if(!nal_isinit || bufid > nal_maxbuffers || bufid == 0) return 0;

	if(nal_buffers[bufid-1].status != NAL_BUFFER_STATUS_LOADED) return 0;

	nal_ea->nlPrint(LOG_FDEBUGFORMAT4, F_NALCREATESOURCE, N_ID, bufid); nal_ea->nlAddTab(1);

	// Выделение памяти под источники звука
	for(i = 0;i < nal_maxsources;i++)
		if(!nal_sources[i].used)
			break;

	if(i == nal_maxsources) {
		nal_source_type *_nal_sources;

		_nal_sources = nal_ea->nReallocMemory(nal_sources, (nal_maxsources+1024)*sizeof(nal_source_type));

		if(_nal_sources)
			nal_sources = _nal_sources;
		else {
			nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NALCREATESOURCE, N_FALSE, N_ID, 0);
			return false;
		}

		for(i = nal_maxsources;i < nal_maxsources+1024;i++)
			nal_sources[i].used = false;
		i = nal_maxsources;
		nal_maxsources += 1024;
	}

	nal_sources[i].used = true;

	nal_sources[i].bufid = bufid;

	nal_sources[i].initialpos = 0;

	nal_buffers[bufid-1].attachedsources++;

	alGenSources(1, &nal_sources[i].alid);

	alSourcei( nal_sources[i].alid, AL_BUFFER	, nal_buffers[bufid-1].alid);
	alSourcef( nal_sources[i].alid, AL_PITCH		, 1.0);
	alSourcef( nal_sources[i].alid, AL_GAIN		, 1.0);
	alSourcei( nal_sources[i].alid, AL_LOOPING	, loop);
	alSourcei( nal_sources[i].alid, AL_SEC_OFFSET	, 0);

	nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NALCREATESOURCE, N_OK, N_ID, i+1);

	return (i+1);
}

/*
	Функция	: nalDestroyAllSources

	Описание: Уничтожает все источники звука

	История	: 26.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nalDestroyAllSources(void)
{
	if(!nal_isinit) return false;

	if(nal_maxsources > 0) {
		unsigned int i;

		for(i=1;i<=nal_maxsources;i++)
			 nalDestroySource(i);

		nal_ea->nFreeMemory(nal_sources);
		nal_sources = 0;
		nal_maxsources = 0;
	}

	return true;
}

/*
	Функция	: nalDestroySource

	Описание: Уничтожает источник звука

	История	: 26.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nalDestroySource(unsigned int id)
{
	if(!nal_isinit || id > nal_maxsources || id == 0) return false;

	if(!nal_sources[id-1].used) return false;

	nal_ea->nlPrint(LOG_FDEBUGFORMAT4, F_NALDESTROYSOURCE, N_ID, id); nal_ea->nlAddTab(1);

	nal_sources[id-1].used = false;

	alDeleteSources(1, &nal_sources[id-1].alid);

	nal_buffers[nal_sources[id-1].bufid-1].attachedsources--;

	nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT, F_NALDESTROYSOURCE, N_OK);

	return true;
}

/*
	Функция	: nalPlaySource

	Описание: Play

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalPlaySource(unsigned int id)
{
	ALint state;

	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	alGetSourcei(nal_sources[id-1].alid, AL_SOURCE_STATE, &state);

	alSourcePlay(nal_sources[id-1].alid);

	if(state == AL_INITIAL || state == AL_STOPPED)
		alSourcei(nal_sources[id-1].alid, AL_SEC_OFFSET, nal_sources[id-1].initialpos);

	nal_sources[id-1].initialpos = 0;
}

/*
	Функция	: nalPauseSource

	Описание: Pause

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalPauseSource(unsigned int id)
{
	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	alSourcePause(nal_sources[id-1].alid);
}

/*
	Функция	: nalStopSource

	Описание: Stop

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalStopSource(unsigned int id)
{
	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	alSourceStop(nal_sources[id-1].alid);
}

/*
	Функция	: nalReplaySource

	Описание: Replay

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalReplaySource(unsigned int id)
{
	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	alSourceStop(nal_sources[id-1].alid);
	alSourcePlay(nal_sources[id-1].alid);

	nal_sources[id-1].initialpos = 0;
}

/*
	Функция	: nalGetSourceLoop

	Описание: Возвращает параметр loop (зацикливание)

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalGetSourceLoop(unsigned int id, int *loop)
{
	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	alGetSourcei(nal_sources[id-1].alid, AL_LOOPING, loop);
}

/*
	Функция	: nalSetSourceLoop

	Описание: Устанавливает зацикливание

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalSetSourceLoop(unsigned int id, int loop)
{
	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	alSourcei(nal_sources[id-1].alid, AL_LOOPING, loop);
}

/*
	Функция	: nalGetSourceSecOffset

	Описание: Возвращает позицию воспроизведения в секундах

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalGetSourceSecOffset(unsigned int id, unsigned int *offset)
{
	ALint state;

	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	alGetSourcei(nal_sources[id-1].alid, AL_SOURCE_STATE, &state);

	if(state == AL_INITIAL || state == AL_STOPPED)
		*offset = nal_sources[id-1].initialpos;
	else
		alGetSourcei(nal_sources[id-1].alid, AL_SEC_OFFSET, (ALint *)offset);
}

/*
	Функция	: nalSetSourceSecOffset

	Описание: Устанавливает позицию воспроизведения в секундах

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalSetSourceSecOffset(unsigned int id, unsigned int offset)
{
	ALint state;

	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	alGetSourcei(nal_sources[id-1].alid, AL_SOURCE_STATE, &state);

	if(state == AL_INITIAL || state == AL_STOPPED)
		nal_sources[id-1].initialpos = (ALint)offset;
	else
		alSourcei(nal_sources[id-1].alid, AL_SEC_OFFSET, (ALint)offset);
}

/*
	Функция	: nalGetSourceLength

	Описание: Возвращает длину [используемого буфера] в секундах

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalGetSourceLength(unsigned int id, unsigned int *length)
{
	ALint buff, bufb, bufc, bufs, secbufs;

	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	alGetBufferi(nal_buffers[nal_sources[id-1].bufid-1].alid, AL_FREQUENCY, &buff);
	alGetBufferi(nal_buffers[nal_sources[id-1].bufid-1].alid, AL_BITS, &bufb);
	alGetBufferi(nal_buffers[nal_sources[id-1].bufid-1].alid, AL_CHANNELS, &bufc);
	alGetBufferi(nal_buffers[nal_sources[id-1].bufid-1].alid, AL_SIZE, &bufs);
	secbufs = buff*bufc*(bufb>>3); // размер секундного буфера
	*length = (bufs+secbufs-1)/secbufs;
}

/*
	Функция	: nalGetSourceGain

	Описание: Возвращает громкость

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalGetSourceGain(unsigned int id, float *gain)
{
	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	alGetSourcef(nal_sources[id-1].alid, AL_GAIN, gain);
}

/*
	Функция	: nalSetSourceGain

	Описание: Устанавливает громкость

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalSetSourceGain(unsigned int id, float gain)
{
	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	if(gain > 1.0)
		alSourcef(nal_sources[id-1].alid, AL_GAIN, 1.0);
	else if(gain < 0.0)
		alSourcef(nal_sources[id-1].alid, AL_GAIN, 0.0);
	else
		alSourcef(nal_sources[id-1].alid, AL_GAIN, gain);
}

/*
	Функция	: nalGetSourceStatus

	Описание: Возвращает статус источника звука (не создан, остановлен, проигрывается, на паузе)

	История	: 17.08.17	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nalGetSourceStatus(unsigned int id)
{
	unsigned int status = NA_SOURCE_STOP;
	ALint source_state;

	if(!nal_isinit || id > nal_maxsources || id == 0) return NA_SOURCE_FREE;

	if(!nal_sources[id-1].used) return NA_SOURCE_FREE;

	alGetSourcei(nal_sources[id-1].alid, AL_SOURCE_STATE, &source_state);

	switch(source_state) {
		case AL_INITIAL:
		case AL_STOPPED:
			status = NA_SOURCE_STOP;
			break;
		case AL_PLAYING:
			status = NA_SOURCE_PLAY;
			break;
		case AL_PAUSED:
			status = NA_SOURCE_PAUSE;
			break;
	}

	return status;
}

void nalASLock(void)
{
	nal_ea->nLockMutex(nal_mutex);
	//nal_ea->nlPrint(L"Lock ss");
}

void nalASUnlock(void)
{
	nal_ea->nUnlockMutex(nal_mutex);
	//nal_ea->nlPrint(L"Unlock ss");
}

/*
	Функция	: nalCreateAudioStream

	Описание: Создаёт аудиопоток

	История	: 14.09.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nalCreateAudioStream(na_audiofile_type *aud, unsigned int files, na_audiofile_plg_read_type readaf, int loop)
{
	unsigned int i, j, maxbufsize = 0;
	bool success = true;

	if(!nal_isinit) return success;

	nal_ea->nlPrint(LOG_FDEBUGFORMAT4, F_NALCREATEAUDIOSTREAM, N_FNAMES, files); nal_ea->nlAddTab(1);

	nalASLock();

	// Выделение памяти под источники звука
	for(i = 0;i < nal_maxaudiostreams;i++)
		if(!nal_audiostreams[i].used)
			break;

	if(i == nal_maxaudiostreams) {
		nal_audiostream_type *_nal_audiostreams;

		_nal_audiostreams = nal_ea->nReallocMemory(nal_audiostreams, (nal_maxaudiostreams+1024)*sizeof(nal_audiostream_type));

		if(_nal_audiostreams)
			nal_audiostreams = _nal_audiostreams;
		else {
			nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NALCREATEAUDIOSTREAM, N_FALSE, N_ID, 0);
			nalASUnlock();
			return false;
		}

		for(i = nal_maxaudiostreams;i < nal_maxaudiostreams+1024;i++)
			nal_audiostreams[i].used = false;
		i = nal_maxaudiostreams;
		nal_maxaudiostreams += 1024;
	}

	nal_audiostreams[i].aud = nal_ea->nAllocMemory(files*sizeof(na_audiofile_type));
	nal_audiostreams[i].wavFormat = nal_ea->nAllocMemory(files*sizeof(unsigned int));
	nal_audiostreams[i].secs = nal_ea->nAllocMemory(files*sizeof(int));
	if(!nal_audiostreams[i].aud || !nal_audiostreams[i].wavFormat || !nal_audiostreams[i].secs) {
		if(nal_audiostreams[i].aud) nal_ea->nFreeMemory(nal_audiostreams[i].aud);
		if(nal_audiostreams[i].wavFormat) nal_ea->nFreeMemory(nal_audiostreams[i].wavFormat);
		if(nal_audiostreams[i].secs) nal_ea->nFreeMemory(nal_audiostreams[i].secs);

		nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NALCREATEAUDIOSTREAM, N_FALSE, N_ID, 0);
		nalASUnlock();
		return false;
	}
	nal_audiostreams[i].status = NAL_SSOURCE_STOP;
	nal_audiostreams[i].noafs = files;
	nal_audiostreams[i].curaf = 0;
	nal_audiostreams[i].curraf = 0;
	nal_audiostreams[i].curpos = 0;
	nal_audiostreams[i].currpos = 0;
	nal_audiostreams[i].nobs = 4; // Можно оптимизировать, например, если есть один аудиофайл длиной в 3 секунды, то можно использовать 3 буфера
	nal_audiostreams[i].readaf = readaf;

	for(j = 0;j < files;j++) {
		nal_audiostreams[i].aud[j] = aud[j];

		switch(aud[j].sf) {
			case NA_SOUND_8BIT_MONO:
				nal_audiostreams[i].wavFormat[j] = AL_FORMAT_MONO8;
				break;
			case NA_SOUND_8BIT_STEREO:
				nal_audiostreams[i].wavFormat[j] = AL_FORMAT_STEREO8;
				break;
			case NA_SOUND_16BIT_MONO:
				nal_audiostreams[i].wavFormat[j] = AL_FORMAT_MONO16;
				break;
			case NA_SOUND_16BIT_STEREO:
				nal_audiostreams[i].wavFormat[j] = AL_FORMAT_STEREO16;
				break;
			default:
				success = false;
		}

		if(success) {
			nal_audiostreams[i].secs[j] = aud[j].nos/aud[j].freq;
			if(aud[j].nos % aud[j].freq)
				nal_audiostreams[i].secs[j]++;
			if(maxbufsize < aud[j].noc*aud[j].bps*aud[j].freq)
				maxbufsize = aud[j].noc*aud[j].bps*aud[j].freq;
		} else
			break;
	}

	if(success) {
		nal_audiostreams[i].bufdata = nal_ea->nAllocMemory(maxbufsize);
		if(!nal_audiostreams[i].bufdata) {
			nal_ea->nFreeMemory(nal_audiostreams[i].aud);
			nal_ea->nFreeMemory(nal_audiostreams[i].wavFormat);
			nal_ea->nFreeMemory(nal_audiostreams[i].secs);

			nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NALCREATEAUDIOSTREAM, N_FALSE, N_ID, 0);
			nalASUnlock();
			return false;
		}
		alGenSources(1, &nal_audiostreams[i].alid);
		alSourcef(nal_audiostreams[i].alid, AL_PITCH, 1.0);
		alSourcef(nal_audiostreams[i].alid, AL_GAIN, 1.0);
		alSourcei(nal_audiostreams[i].alid, AL_SEC_OFFSET, 0);
		alGenBuffers(nal_audiostreams[i].nobs, nal_audiostreams[i].bufid);
		nal_audiostreams[i].used = true;
		nal_audiostreams[i].mustreload = false;
		nal_audiostreams[i].loop = loop;
		i++;
	} else {
		i = 0;
		free(nal_audiostreams[i].aud);
		free(nal_audiostreams[i].wavFormat);
		free(nal_audiostreams[i].secs);
	}

	nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NALCREATEAUDIOSTREAM, N_OK, N_ID, i);

	nalASUnlock();

	return (i);
}

/*
	Функция	: nalPlayAudioStream

	Описание: Play

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalPlayAudioStream(unsigned int id)
{
	if(!nal_isinit || id > nal_maxaudiostreams || id == 0) return;

	if(!nal_audiostreams[id-1].used) return;
	if(nal_audiostreams[id-1].status == NAL_SSOURCE_PLAY) return;

	nalASLock();
		if(nal_audiostreams[id-1].status == NAL_SSOURCE_PAUSE)
			alSourcePlay(nal_audiostreams[id-1].alid);
		else if(nal_audiostreams[id-1].mustreload == true) // NAL_SSOURCE_STOP, NAL_SSOURCE_REPLAY, NAL_SSOURCE_GOTO
			nal_audiostreams[id-1].mustreload = false;
		else
			nal_audiostreams[id-1].mustreload = true;
		nal_audiostreams[id-1].status = NAL_SSOURCE_PLAY;
	nalASUnlock();
}

/*
	Функция	: nalPauseAudioStream

	Описание: Pause

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalPauseAudioStream(unsigned int id)
{
	if(!nal_isinit || id > nal_maxaudiostreams || id == 0) return;

	if(!nal_audiostreams[id-1].used) return;
	if(nal_audiostreams[id-1].status != NAL_SSOURCE_PLAY) return;

	nalASLock();
		if(nal_audiostreams[id-1].mustreload) {
			nal_audiostreams[id-1].mustreload = false;
			nal_audiostreams[id-1].status = NAL_SSOURCE_STOP;
		} else {
			nal_audiostreams[id-1].status = NAL_SSOURCE_PAUSE;
			alSourcePause(nal_audiostreams[id-1].alid);
		}
	nalASUnlock();
}

/*
	Функция	: nalStopAudioStream

	Описание: Stop

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalStopAudioStream(unsigned int id)
{
	if(!nal_isinit || id > nal_maxaudiostreams || id == 0) return;

	if(!nal_audiostreams[id-1].used) return;
	if(nal_audiostreams[id-1].status == NAL_SSOURCE_STOP) return;

	nalASLock();
		nal_audiostreams[id-1].status = NAL_SSOURCE_STOP;
		if(nal_audiostreams[id-1].mustreload && nal_audiostreams[id-1].status == NAL_SSOURCE_PLAY)
			nal_audiostreams[id-1].mustreload = false; // В данном случае запись уже остановлена, действия не требуются
		else
			nal_audiostreams[id-1].mustreload = true;
	nalASUnlock();
}

/*
	Функция	: nalReplayAudioStream

	Описание: Replay

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalReplayAudioStream(unsigned int id)
{
	if(!nal_isinit || id > nal_maxaudiostreams || id == 0) return;

	if(!nal_audiostreams[id-1].used) return;
	if(nal_audiostreams[id-1].status == NAL_SSOURCE_REPLAY) return;

	nalASLock();
		nal_audiostreams[id-1].status = NAL_SSOURCE_REPLAY;
		nal_audiostreams[id-1].mustreload = true;
	nalASUnlock();
}

/*
	Функция	: nalGetAudioStreamLoop

	Описание: Возвращает параметр loop (зацикливание)

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalGetAudioStreamLoop(unsigned int id, int *loop)
{
	if(!nal_isinit || id > nal_maxaudiostreams || id == 0) return;

	if(!nal_audiostreams[id-1].used) return;

	*loop = nal_audiostreams[id-1].loop;
}

/*
	Функция	: nalSetAudioStreamLoop

	Описание: Устанавливает зацикливание

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalSetAudioStreamLoop(unsigned int id, int loop)
{
	if(!nal_isinit || id > nal_maxaudiostreams || id == 0) return;

	if(!nal_audiostreams[id-1].used) return;

	nal_audiostreams[id-1].loop = loop;
}

/*
	Функция	: nalGetAudioStreamSecOffset

	Описание: Возвращает позицию воспроизведения в секундах

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalGetAudioStreamSecOffset(unsigned int id, unsigned int *offset, unsigned int *track)
{
	if(!nal_isinit || id > nal_maxaudiostreams || id == 0) return;

	if(!nal_audiostreams[id-1].used) return;

	*offset = nal_audiostreams[id-1].curpos;
	*track = nal_audiostreams[id-1].curaf;
}

/*
	Функция	: nalSetAudioStreamSecOffset

	Описание: Устанавливает позицию воспроизведения в секундах

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalSetAudioStreamSecOffset(unsigned int id, unsigned int offset, unsigned int track)
{
	if(!nal_isinit || id > nal_maxaudiostreams || id == 0) return;

	if(!nal_audiostreams[id-1].used) return;

	nalASLock();
		nal_audiostreams[id-1].curpos = offset;
		nal_audiostreams[id-1].currpos = offset;
		nal_audiostreams[id-1].curaf = track;
		nal_audiostreams[id-1].curraf = track;
		if( !((nal_audiostreams[id-1].mustreload && nal_audiostreams[id-1].status == NAL_SSOURCE_PLAY) || (!nal_audiostreams[id-1].mustreload && nal_audiostreams[id-1].status == NAL_SSOURCE_STOP)) ) {
			if(nal_audiostreams[id-1].status == NAL_SSOURCE_PLAY)
				nal_audiostreams[id-1].status = NAL_SSOURCE_GOTO_PLAY;
			else
				nal_audiostreams[id-1].status = NAL_SSOURCE_GOTO;
			nal_audiostreams[id-1].mustreload = true;
		}
	nalASUnlock();
}

/*
	Функция	: nalGetAudioStreamLength

	Описание: Возвращает длину [используемого буфера] в секундах

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalGetAudioStreamLength(unsigned int id, unsigned int *length, unsigned int *tracks)
{
	if(!nal_isinit || id > nal_maxaudiostreams || id == 0) return;

	if(!nal_audiostreams[id-1].used) return;

	*length = nal_audiostreams[id-1].secs[nal_audiostreams[id-1].curaf];
	*tracks = nal_audiostreams[id-1].noafs;
}

/*
	Функция	: nalGetAudioStreamGain

	Описание: Возвращает громкость

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalGetAudioStreamGain(unsigned int id, float *gain)
{
	if(!nal_isinit || id > nal_maxaudiostreams || id == 0) return;

	if(!nal_audiostreams[id-1].used) return;

	alGetSourcef(nal_audiostreams[id-1].alid, AL_GAIN, gain);
}

/*
	Функция	: nalSetAudioStreamGain

	Описание: Устанавливает громкость

	История	: 14.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalSetAudioStreamGain(unsigned int id, float gain)
{
	if(!nal_isinit || id > nal_maxaudiostreams || id == 0) return;

	if(!nal_audiostreams[id-1].used) return;

	if(gain > 1.0)
		alSourcef(nal_audiostreams[id-1].alid, AL_GAIN, 1.0);
	else if(gain < 0.0)
		alSourcef(nal_audiostreams[id-1].alid, AL_GAIN, 0.0);
	else
		alSourcef(nal_audiostreams[id-1].alid, AL_GAIN, gain);
}

/*
	Функция	: nalGetAudioStreamStatus

	Описание: Возвращает статус аудиопотока (не создан, остановлен, проигрывается, на паузе)

	История	: 17.08.17	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nalGetAudioStreamStatus(unsigned int id)
{
	unsigned int status = NA_SOURCE_STOP;

	if(!nal_isinit || id > nal_maxaudiostreams || id == 0) return NA_SOURCE_FREE;

	if(!nal_audiostreams[id-1].used) return NA_SOURCE_FREE;

	switch(nal_audiostreams[id-1].status) {
		case NAL_SSOURCE_STOP:
			status = NA_SOURCE_STOP;
			break;
		case NAL_SSOURCE_PLAY:
		case NAL_SSOURCE_REPLAY:
		case NAL_SSOURCE_GOTO_PLAY:
			status = NA_SOURCE_PLAY;
			break;
		case NAL_SSOURCE_PAUSE:
		case NAL_SSOURCE_GOTO:
			status = NA_SOURCE_PAUSE;
			break;
	}

	return status;
}

/*
	Функция	: nalDestroyAudioStream

	Описание: Уничтожает аудиопоток

	История	: 26.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nalDestroyAudioStream(unsigned int id)
{
	unsigned int tempbuf;
	int processed;

	if(!nal_isinit || id > nal_maxaudiostreams || id == 0) return false;

	nalASLock();
		if(!nal_audiostreams[id-1].used) { nalASUnlock(); return false; }

		nal_ea->nlPrint(LOG_FDEBUGFORMAT4, F_NALDESTROYAUDIOSTREAM, N_ID, id); nal_ea->nlAddTab(1);
		nal_audiostreams[id-1].used = false;

		nal_ea->nFreeMemory(nal_audiostreams[id-1].aud);
		nal_ea->nFreeMemory(nal_audiostreams[id-1].wavFormat);
		nal_ea->nFreeMemory(nal_audiostreams[id-1].secs);
		nal_ea->nFreeMemory(nal_audiostreams[id-1].bufdata);

		alSourceStop(nal_audiostreams[id-1].alid);

		nal_ea->nSleep(SERVICE_UPDATE_PERIOD);

		alGetSourcei(nal_audiostreams[id-1].alid, AL_BUFFERS_PROCESSED, &processed);
		while(processed) {
			processed--;
			alSourceUnqueueBuffers(nal_audiostreams[id-1].alid, 1, &tempbuf);
		}

		alDeleteSources(1, &nal_audiostreams[id-1].alid);

		alDeleteBuffers(nal_audiostreams[id-1].nobs, nal_audiostreams[id-1].bufid);

		nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT, F_NALDESTROYAUDIOSTREAM, N_OK);
	nalASUnlock();
	nal_ea->nSleep(100);
	return true;
}

/*
	Функция	: nalDestroyAllAudioStreams

	Описание: Уничтожает все аудиопотоки

	История	: 24.07.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nalDestroyAllAudioStreams(void)
{
	if(!nal_isinit) return false;

	if(nal_maxaudiostreams > 0) {
		unsigned int i;

		nalASLock();
			for(i=1;i<=nal_maxaudiostreams;i++)
				nalDestroyAudioStream(i);
			nal_ea->nFreeMemory(nal_audiostreams);
			nal_audiostreams = 0;
			nal_maxaudiostreams = 0;
		nalASUnlock();
	}

	return true;
}

/*
	Функция	: nalUpdateAudioStreamBuffer

	Описание: Обновляет указанный буфер аудиопотока
			i - id источника звука
			curraf = nal_audiostreams[i].curraf

	История	: 17.09.16	Создан

*/
static unsigned int nalUpdateAudioStreamBuffer(unsigned int i, unsigned int curraf, unsigned int *bufid)
{
	unsigned int reads;

	if((nal_audiostreams[i].aud[curraf].nos-nal_audiostreams[i].aud[curraf].freq*nal_audiostreams[i].currpos) < nal_audiostreams[i].aud[curraf].freq) // Осталось меньше секунды
		reads = nal_audiostreams[i].aud[curraf].nos-nal_audiostreams[i].aud[curraf].freq*nal_audiostreams[i].currpos;
	else
		reads = nal_audiostreams[i].aud[curraf].freq;
	nal_audiostreams[i].readaf(nal_audiostreams[i].currpos,
		reads,
		nal_audiostreams[i].bufdata,
		&nal_audiostreams[i].aud[curraf]);


	alBufferData(*bufid,
		nal_audiostreams[i].wavFormat[curraf],
		nal_audiostreams[i].bufdata,
		reads*nal_audiostreams[i].aud[curraf].noc*nal_audiostreams[i].aud[curraf].bps,
		nal_audiostreams[i].aud[curraf].freq);

	alSourceQueueBuffers(nal_audiostreams[i].alid, 1, bufid);

	nal_audiostreams[i].currpos++;
	if(nal_audiostreams[i].currpos == nal_audiostreams[i].secs[curraf]) {
		nal_audiostreams[i].currpos = 0;
		nal_audiostreams[i].curraf++;
		if(nal_audiostreams[i].curraf == nal_audiostreams[i].noafs)
			nal_audiostreams[i].curraf = 0;
		curraf = nal_audiostreams[i].curraf;
	}

	return curraf;
}


/*
	Функция	: nalASTaskFunc

	Описание: Задача для обработки аудиопотоков

	История	: 14.09.12	Создан

*/
void N_APIENTRY nalASTaskFunc(void *parm)
{
	int error;
	unsigned int i, j;
	unsigned int tempbuf, curaf, curraf;
	int processed;

	(void)parm; // Неиспользуемая переменная

	//nal_ea->nlPrint(L"nal_isinit %d", nal_isinit);
	nalASLock();
	for(i = 0;i < nal_maxaudiostreams;i++) {
		//nal_ea->nlPrint(L"processing %d", i);
		if(nal_audiostreams[i].used) {
			curaf = nal_audiostreams[i].curaf; // Файл, проигрываемый в данный момент
			curraf = nal_audiostreams[i].curraf; // Файл, с которого происходит чтение с диска
			if(nal_audiostreams[i].mustreload && nal_audiostreams[i].status == NAL_SSOURCE_PLAY) {
				for(j = 0;j < nal_audiostreams[i].nobs;j++) {
					//wprintf(L"process i %d c %d < %d cr %d < %d\n",i,nal_audiostreams[i].curpos,nal_audiostreams[i].secs[curraf],nal_audiostreams[i].currpos,nal_audiostreams[i].secs[curraf]);

					curraf = nalUpdateAudioStreamBuffer(i, curraf, nal_audiostreams[i].bufid+j);
					if(curaf != curraf && 
						(nal_audiostreams[i].aud[curaf].bps  != nal_audiostreams[i].aud[curraf].bps ||
						nal_audiostreams[i].aud[curaf].freq != nal_audiostreams[i].aud[curraf].freq ||
						nal_audiostreams[i].aud[curaf].noc  != nal_audiostreams[i].aud[curraf].noc))
						break;
				}
				nal_audiostreams[i].mustreload = false;
				alSourcePlay(nal_audiostreams[i].alid);
			} else if(nal_audiostreams[i].mustreload && (nal_audiostreams[i].status == NAL_SSOURCE_STOP || nal_audiostreams[i].status == NAL_SSOURCE_REPLAY || nal_audiostreams[i].status == NAL_SSOURCE_GOTO || nal_audiostreams[i].status == NAL_SSOURCE_GOTO_PLAY)) {
				ALenum error;

				if(!(nal_audiostreams[i].status == NAL_SSOURCE_GOTO || nal_audiostreams[i].status == NAL_SSOURCE_GOTO_PLAY)) {
					nal_audiostreams[i].curpos = 0;
					nal_audiostreams[i].currpos = 0;
					nal_audiostreams[i].curaf = 0;
					nal_audiostreams[i].curraf = 0;
				}

				alGetError();

				alSourceStop(nal_audiostreams[i].alid);
				error = alGetError();
				if(error)
					nal_ea->nlPrint(L"nalASTaskFunc(): openal error %d in line %d", error, __LINE__);

				nal_ea->nSleep(SERVICE_UPDATE_PERIOD);

				alGetSourcei(nal_audiostreams[i].alid, AL_BUFFERS_PROCESSED, &processed);
				while(processed){
					processed--;
					alSourceUnqueueBuffers(nal_audiostreams[i].alid, 1, &tempbuf);
				}
				error = alGetError();
				if(error)
					nal_ea->nlPrint(L"nalASTaskFunc(): openal error %d in line %d", error, __LINE__);

				if(nal_audiostreams[i].status == NAL_SSOURCE_REPLAY || nal_audiostreams[i].status == NAL_SSOURCE_GOTO_PLAY)
					nal_audiostreams[i].status = NAL_SSOURCE_PLAY;
				else
					nal_audiostreams[i].mustreload = false;
			} else if(nal_audiostreams[i].status == NAL_SSOURCE_PLAY) {
				ALint state;

				alGetSourcei(nal_audiostreams[i].alid, AL_BUFFERS_PROCESSED, &processed);
				while(processed){
					ALint queued;

					//wprintf(L"process i %d c %d < %d cr %d < %d\n",i,nal_audiostreams[i].curpos,nal_audiostreams[i].secs[curaf],nal_audiostreams[i].currpos,nal_audiostreams[i].secs[curraf]);

					if((nal_audiostreams[i].curpos+1) == nal_audiostreams[i].secs[curaf] && !nal_audiostreams[i].loop && (nal_audiostreams[i].curaf+1) == nal_audiostreams[i].noafs) {
						nal_audiostreams[i].status = NAL_SSOURCE_STOP;
						nal_audiostreams[i].mustreload = true;
						processed = 0;
					} else {
						processed--;

						tempbuf = 0;
						alSourceUnqueueBuffers(nal_audiostreams[i].alid, 1, &tempbuf);

						if(curaf != curraf && 
							(nal_audiostreams[i].aud[curaf].bps  != nal_audiostreams[i].aud[curraf].bps ||
							nal_audiostreams[i].aud[curaf].freq != nal_audiostreams[i].aud[curraf].freq ||
							nal_audiostreams[i].aud[curaf].noc  != nal_audiostreams[i].aud[curraf].noc)) { // Если начали читать следующий аудиофайл, и их параметры не совпадают

							alGetSourcei(nal_audiostreams[i].alid, AL_BUFFERS_QUEUED, &queued);

							/*wprintf(L"processed %d queued %d bps %u/%u freq %u %u noc %u %u\n", processed, queued,
								nal_audiostreams[i].aud[curaf].bps, nal_audiostreams[i].aud[curraf].bps,
								nal_audiostreams[i].aud[curaf].freq, nal_audiostreams[i].aud[curraf].freq,
								nal_audiostreams[i].aud[curaf].noc, nal_audiostreams[i].aud[curraf].noc);*/

							if(queued == 0) {
								//wprintf(L"1. alGetError() returns %d\n", alGetError());
								for(j = 0;j < nal_audiostreams[i].nobs;j++) {
										
									curraf = nalUpdateAudioStreamBuffer(i, curraf, nal_audiostreams[i].bufid + j);
								}
								//wprintf(L"2. alGetError() returns %d\n", alGetError());
							}
						} else
							curraf = nalUpdateAudioStreamBuffer(i, curraf, &tempbuf);

						nal_audiostreams[i].curpos++;
						if(nal_audiostreams[i].curpos == nal_audiostreams[i].secs[curaf]) {
							nal_audiostreams[i].curpos = 0;
							nal_audiostreams[i].curaf++;
							if(nal_audiostreams[i].curaf == nal_audiostreams[i].noafs)
								nal_audiostreams[i].curaf = 0;
							curaf = nal_audiostreams[i].curaf;
						}

					}

					//wprintf(L"00. alGetError() returns %d\n", alGetError());
					alGetSourcei(nal_audiostreams[i].alid, AL_BUFFERS_QUEUED, &queued);
					//wprintf(L"01. alGetError() returns %d\n", alGetError());	
				}

				alGetSourcei(nal_audiostreams[i].alid, AL_SOURCE_STATE, &state);
				if(state != AL_PLAYING) {
					//wprintf(L"02. alGetError() returns %d\n", alGetError());
					alSourcePlay(nal_audiostreams[i].alid);
					//wprintf(L"03. alGetError() returns %d\n", alGetError());
				}
			}
		}
	}
	nalASUnlock();
	error = alGetError(); if(error) nal_ea->nlPrint(ERR_OPENAL, error);
	//nal_ea->nlPrint(L"exit from nalASTaskFunc");
}


/*
	Функция	: nalCreateAST

	Описание: Создаёт задачу для аудиопотоков

	История	: 14.09.12	Создан

*/
bool nalCreateAST(void)
{
	nal_mutex = nal_ea->nCreateMutex();
	nal_task = nal_ea->nCreateTask(nalASTaskFunc, 0, SERVICE_UPDATE_PERIOD, -1);

	if(!nal_mutex || !nal_task) {
		if(nal_mutex) {
			nal_ea->nDestroyMutex(nal_mutex);
			nal_mutex = 0;
		}

		if(nal_task) {
			nal_ea->nDestroyTask(nal_task);
			nal_task = 0;
		}

		return false;
	}

	return true;
}

/*
	Функция	: nalDestroyAST

	Описание: Уничтожает задачу
		Вызывается до "nal_isinit = false;"

	История	: 14.09.12	Создан

*/
void nalDestroyAST(void)
{
	nal_ea->nDestroyTask(nal_task);
	nal_ea->nDestroyMutex(nal_mutex);
}
