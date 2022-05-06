/*
	Файл	: nal_main.c

	Описание: Аудиодвижок. Основные функции

	История	: 25.12.12	Создан

*/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>

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

unsigned int nal_task = 0;

uintptr_t nal_mutex = 0;

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

	(void)nos; // Неиспользуемая переменная
	(void)noc;
	(void)freq;
	(void)bps;
	(void)buf;

	if(!nal_isinit || id > nal_maxbuffers || id == 0) return false;

	if(nal_buffers[id-1].status != NAL_BUFFER_STATUS_EMPTY) return false;

	nal_ea->nlPrint(LOG_FDEBUGFORMAT4, F_NALLOADBUFFER, N_ID, id); nal_ea->nlAddTab(1);

	switch(sf) {
		case NA_SOUND_8BIT_MONO:
		case NA_SOUND_8BIT_STEREO:
		case NA_SOUND_16BIT_MONO:
		case NA_SOUND_16BIT_STEREO:
			success = true;
			break;
		default:
			success = false;
	}

	if(success) {
		// Загрузка буфера через audio api

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

	// Выгрузка буфера через audio api

	nal_buffers[id-1].status = NAL_BUFFER_STATUS_EMPTY;

	nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT, F_NALUNLOADBUFFER, N_OK);

	return true;
}

/*
	Функция	: nalCreateSource

	Описание: Создаёт источник звука

	История	: 26.06.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nalCreateSource(unsigned int bufid, int loop)
{
	unsigned int i;

	(void)loop; // Неиспользуемая переменная

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

	nal_buffers[bufid-1].attachedsources++;

	// Создание источника звука через audio api и подключения к нему буфера nal_buffers[bufid-1]

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

	// Уничтожение источника звука

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
	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	// Проигрывание источника звука
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

	// Пауза источника звука
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

	// Остановка источника звука
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

	// Остановка источника звука
	// Проигрывание источника звука
}

/*
	Функция	: nalGetSourceLoop

	Описание: Возвращает параметр loop (зацикливание)

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalGetSourceLoop(unsigned int id, int *loop)
{
	(void)loop; // Неиспользуемая переменная

	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	// Возврат информации о том, зациклен ли источник звука
}

/*
	Функция	: nalSetSourceLoop

	Описание: Устанавливает зацикливание

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalSetSourceLoop(unsigned int id, int loop)
{
	(void)loop; // Неиспользуемая переменная

	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	// Установка информации о том, зациклен ли источник звука
}

/*
	Функция	: nalGetSourceSecOffset

	Описание: Возвращает позицию воспроизведения в секундах

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalGetSourceSecOffset(unsigned int id, unsigned int *offset)
{
	(void)offset; // Неиспользуемая переменная

	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	// Возврат смещения (позиции в аудиобуфере/аудиотрэке) источника звука
}

/*
	Функция	: nalSetSourceSecOffset

	Описание: Устанавливает позицию воспроизведения в секундах

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalSetSourceSecOffset(unsigned int id, unsigned int offset)
{
	(void)offset; // Неиспользуемая переменная

	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	// Установка смещения (позиции в аудиобуфере/аудиотрэке) источника звука
}

/*
	Функция	: nalGetSourceLength

	Описание: Возвращает длину [используемого буфера] в секундах

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalGetSourceLength(unsigned int id, unsigned int *length)
{
	int buff, bufb, bufc, bufs;

	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	buff = 44000; // Получение частоты аудиобуфера nal_buffers[nal_sources[id-1].bufid-1]
	bufb = 16; // Получение количества бит на семпл аудиобуфера nal_buffers[nal_sources[id-1].bufid-1]
	bufc = 2; // Получение каналов аудиобуфера nal_buffers[nal_sources[id-1].bufid-1]
	bufs = 440000; // Получение размера аудиобуфера nal_buffers[nal_sources[id-1].bufid-1] в байтах
	*length = bufs/(buff*bufc*(bufb>>3));
}

/*
	Функция	: nalGetSourceGain

	Описание: Возвращает громкость

	История	: 26.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nalGetSourceGain(unsigned int id, float *gain)
{
	(void)gain; // Неиспользуемая переменная
	if(!nal_isinit || id > nal_maxsources || id == 0) return;

	if(!nal_sources[id-1].used) return;

	// Возврат громкости источника звука
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
		gain = 1.0;
	else if(gain < 0.0)
		gain = 0.0;

	// Установка громкости источника звука
}

/*
	Функция	: nalGetSourceStatus

	Описание: Возвращает статус источника звука (не создан, остановлен, проигрывается, на паузе)

	История	: 17.08.17	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nalGetSourceStatus(unsigned int id)
{
	unsigned int status = NA_SOURCE_STOP;

	if(!nal_isinit || id > nal_maxsources || id == 0) return NA_SOURCE_FREE;

	if(!nal_sources[id-1].used) return NA_SOURCE_FREE;

	// Получение состояния источника nal_sources[id-1]

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
			case NA_SOUND_8BIT_STEREO:
			case NA_SOUND_16BIT_MONO:
			case NA_SOUND_16BIT_STEREO:
				success = true;
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
		// Создание аудиопотока через audio api
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
		if(nal_audiostreams[id-1].status == NAL_SSOURCE_PAUSE) {
			// Запуск проигрывания аудиопотока через audio api
		} else if(nal_audiostreams[id-1].mustreload == true) // NAL_SSOURCE_STOP, NAL_SSOURCE_REPLAY, NAL_SSOURCE_GOTO
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
			// Пауза аудиопотока через audio api
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
		if((nal_audiostreams[id-1].mustreload && nal_audiostreams[id-1].status == NAL_SSOURCE_PLAY) || (!nal_audiostreams[id-1].mustreload && nal_audiostreams[id-1].status == NAL_SSOURCE_STOP)) {
			nal_audiostreams[id-1].curpos = 0;
			nal_audiostreams[id-1].currpos = 0;
			nal_audiostreams[id-1].curaf = 0;
			nal_audiostreams[id-1].curraf = 0;
		} else {
			nal_audiostreams[id-1].status = NAL_SSOURCE_REPLAY;
			nal_audiostreams[id-1].mustreload = true;
		}
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
	(void)gain; // Неиспользуемая переменная

	if(!nal_isinit || id > nal_maxaudiostreams || id == 0) return;

	if(!nal_audiostreams[id-1].used) return;

	// Возврат громкости аудиопотока через audio api
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
		gain = 1.0;
	else if(gain < 0.0)
		gain = 0.0;

	// Установка громкости аудиопотока через audio api
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

		// Остановка проигрывания аудиопотока через audio api

		nal_ea->nSleep(SERVICE_UPDATE_PERIOD);

		processed = 1; // Получение количества отработавших буферов аудиопотока через audio api
		while(processed){
			processed--;
			// Удаление из очереди отработавшего буфера аудиопотока через audio api
		}

		if(!( (nal_audiostreams[id-1].mustreload && nal_audiostreams[id-1].status == NAL_SSOURCE_PLAY) ||
			(!nal_audiostreams[id-1].mustreload && (nal_audiostreams[id-1].status == NAL_SSOURCE_STOP || nal_audiostreams[id-1].status == NAL_SSOURCE_GOTO)) )) {
			// Уничтожение обрабатываемых буферов (nal_audiostreams[i].bufid) аудиопотока через audio api
		}

		// Удаление аудиопотока через audio api

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

	(void)bufid; // Неиспользуемая переменная

	if((nal_audiostreams[i].aud[curraf].nos-nal_audiostreams[i].aud[curraf].freq*nal_audiostreams[i].currpos) < nal_audiostreams[i].aud[curraf].freq) // Осталось меньше секунды
		reads = nal_audiostreams[i].aud[curraf].nos-nal_audiostreams[i].aud[curraf].freq*nal_audiostreams[i].currpos;
	else
		reads = nal_audiostreams[i].aud[curraf].freq;
	nal_audiostreams[i].readaf(nal_audiostreams[i].currpos,
		reads,
		nal_audiostreams[i].bufdata,
		&nal_audiostreams[i].aud[curraf]);

	// Загрузка данных в новый буфер аудиопотока через audio api
	// Указатель на данные - nal_audiostreams[i].bufdata
	// Размер - reads*nal_audiostreams[i].aud[curraf].noc*nal_audiostreams[i].aud[curraf].bps
	// Частота - nal_audiostreams[i].aud[curraf].freq

	// Добавление в очередь нового буфера аудиопотока через audio api

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

	Описание: Задача для аудиопотока

	История	: 14.09.12	Создан

*/
void N_APIENTRY nalASTaskFunc(void *parm)
{
	(void)parm; // Неиспользуемая переменная

	if(nal_isinit)
	{
		unsigned int i, j;
		unsigned int tempbuf, curaf, curraf;
		int processed;

		//nal_ea->nlPrint(L"nal_isinit %d", nal_isinit);
		nalASLock();
		for(i = 0;i < nal_maxaudiostreams;i++) {
			//nal_ea->nlPrint(L"processing %d", i);
			if(nal_audiostreams[i].used) {
				curaf = nal_audiostreams[i].curaf;
				curraf = nal_audiostreams[i].curraf;
				if(nal_audiostreams[i].mustreload && nal_audiostreams[i].status == NAL_SSOURCE_PLAY) {
					// Создание новых обрабатываемых буферов (nal_audiostreams[i].bufid) аудиопотока через audio api
					for(j = 0;j < nal_audiostreams[i].nobs;j++) {
						//wprintf(L"process i %d c %d < %d cr %d < %d\n",i,nal_audiostreams[i].curpos,nal_audiostreams[i].secs[curraf],nal_audiostreams[i].currpos,nal_audiostreams[i].secs[curraf]);

						curraf = nalUpdateAudioStreamBuffer(i, curraf, nal_audiostreams[i].bufid+j);
					}
					nal_audiostreams[i].mustreload = false;
					// Запуск проигрывания аудиопотока через audio api
				} else if(nal_audiostreams[i].mustreload && (nal_audiostreams[i].status == NAL_SSOURCE_STOP || nal_audiostreams[i].status == NAL_SSOURCE_REPLAY || nal_audiostreams[i].status == NAL_SSOURCE_GOTO || nal_audiostreams[i].status == NAL_SSOURCE_GOTO_PLAY)) {
					if(!(nal_audiostreams[i].status == NAL_SSOURCE_GOTO || nal_audiostreams[i].status == NAL_SSOURCE_GOTO_PLAY)) {
						nal_audiostreams[i].curpos = 0;
						nal_audiostreams[i].currpos = 0;
						nal_audiostreams[i].curaf = 0;
						nal_audiostreams[i].curraf = 0;
					}

					// Остановка аудиопотока через audio api

					nal_ea->nSleep(SERVICE_UPDATE_PERIOD);

					processed = 1; // Получение количества отработавших буферов аудиопотока через audio api
					while(processed){
						processed--;
						// Удаление из очереди отработавшего буфера аудиопотока через audio api
					}

					// Уничтожение обрабатываемых буферов (nal_audiostreams[i].bufid) аудиопотока через audio api

					if(nal_audiostreams[i].status == NAL_SSOURCE_REPLAY || nal_audiostreams[i].status == NAL_SSOURCE_GOTO_PLAY)
						nal_audiostreams[i].status = NAL_SSOURCE_PLAY;
					else
						nal_audiostreams[i].mustreload = false;
				} else if(nal_audiostreams[i].status == NAL_SSOURCE_PLAY) {
					processed = 1; // Получение количества отработавших буферов аудиопотока через audio api
					while(processed){
						//wprintf(L"process i %d c %d < %d cr %d < %d\n",i,nal_audiostreams[i].curpos,nal_audiostreams[i].secs[curraf],nal_audiostreams[i].currpos,nal_audiostreams[i].secs[curraf]);

						if((nal_audiostreams[i].curpos+1) == nal_audiostreams[i].secs[curaf] && !nal_audiostreams[i].loop && (nal_audiostreams[i].curaf+1) == nal_audiostreams[i].noafs) {
							nal_audiostreams[i].status = NAL_SSOURCE_STOP;
							nal_audiostreams[i].mustreload = true;
							processed = 0;
						} else {
							processed--;

							// Удаление отработавших буферов аудиопотока из очереди через audio api
							tempbuf = 0; // Получение id отработавшего буфера

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
					}
				}
			}
		}
		nalASUnlock();
	}
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
		Вызывается после "nal_isinit = false;"

	История	: 14.09.12	Создан

*/
void nalDestroyAST(void)
{
	nal_ea->nDestroyTask(nal_task);
	nal_ea->nDestroyMutex(nal_mutex);
}
