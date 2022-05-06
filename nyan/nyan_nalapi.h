/*
	Файл	: nyan_nalapi.h

	Описание: API аудио библиотеки

	История	: 25.12.12	Создан

*/

#ifndef NYAN_NALAPI_H
#define NYAN_NALAPI_H

#include <stdbool.h>

#include "nyan_decls_publicapi.h"

#include "nyan_audiofile_publicapi.h"

#include "nyan_engapi.h"

#ifndef N_NODYLIB
	#define NALAPI_FUNC(ret, name, params) typedef ret (N_APIENTRY *name##_type) params; extern name##_type funcptr_##name;
#else // Версия, не поддерживающая загрузку из dll
	#define NALAPI_FUNC(ret, name, params) typedef ret (N_APIENTRY *name##_type) params; extern name##_type funcptr_##name; extern ret N_APIENTRY name params;
#endif

NALAPI_FUNC(bool, nalSetupDll, (engapi_type *engapi))
NALAPI_FUNC(bool, nalInit, (void))
NALAPI_FUNC(bool, nalClose, (void))
NALAPI_FUNC(unsigned int, nalCreateBuffer, (void))
NALAPI_FUNC(bool, nalDestroyAllBuffers, (void))
NALAPI_FUNC(bool, nalDestroyBuffer, (unsigned int id))
NALAPI_FUNC(bool, nalLoadBuffer, (unsigned int id, unsigned int nos, unsigned int noc, unsigned int freq, unsigned int sf, unsigned int bps, void *buf))
NALAPI_FUNC(bool, nalUnloadBuffer, (unsigned int id))
NALAPI_FUNC(unsigned int, nalCreateSource, (unsigned int bufid,int loop))
NALAPI_FUNC(bool, nalDestroyAllSources, (void))
NALAPI_FUNC(bool, nalDestroySource, (unsigned int id))
NALAPI_FUNC(void, nalPlaySource, (unsigned int id))
NALAPI_FUNC(void, nalPlayAudioStream, (unsigned int id))
NALAPI_FUNC(void, nalPauseSource, (unsigned int id))
NALAPI_FUNC(void, nalPauseAudioStream, (unsigned int id))
NALAPI_FUNC(void, nalStopSource, (unsigned int id))
NALAPI_FUNC(void, nalStopAudioStream, (unsigned int id))
NALAPI_FUNC(void, nalReplaySource, (unsigned int id))
NALAPI_FUNC(void, nalReplayAudioStream, (unsigned int id))
NALAPI_FUNC(void, nalGetSourceLoop, (unsigned int id, int *loop))
NALAPI_FUNC(void, nalSetSourceLoop, (unsigned int id, int loop))
NALAPI_FUNC(void, nalGetSourceSecOffset, (unsigned int id, unsigned int *offset))
NALAPI_FUNC(void, nalSetSourceSecOffset, (unsigned int id, unsigned int offset))
NALAPI_FUNC(void, nalGetSourceLength, (unsigned int id, unsigned int *length))
NALAPI_FUNC(void, nalGetSourceGain, (unsigned int id, float *gain))
NALAPI_FUNC(void, nalSetSourceGain, (unsigned int id, float gain))
NALAPI_FUNC(unsigned int, nalGetSourceStatus, (unsigned int id))
NALAPI_FUNC(void, nalGetAudioStreamLoop, (unsigned int id, int *loop))
NALAPI_FUNC(void, nalSetAudioStreamLoop, (unsigned int id, int loop))
NALAPI_FUNC(void, nalGetAudioStreamGain, (unsigned int id, float *gain))
NALAPI_FUNC(void, nalSetAudioStreamGain, (unsigned int id, float gain))
NALAPI_FUNC(unsigned int, nalCreateAudioStream, (na_audiofile_type *aud, unsigned int files, na_audiofile_plg_read_type readaf, int loop))
NALAPI_FUNC(bool, nalDestroyAllAudioStreams, (void))
NALAPI_FUNC(bool, nalDestroyAudioStream, (unsigned int id))
NALAPI_FUNC(void, nalGetAudioStreamSecOffset, (unsigned int id, unsigned int *offset, unsigned int *track))
NALAPI_FUNC(void, nalSetAudioStreamSecOffset, (unsigned int id, unsigned int offset, unsigned int track))
NALAPI_FUNC(void, nalGetAudioStreamLength, (unsigned int id, unsigned int *length, unsigned int *tracks))
NALAPI_FUNC(unsigned int, nalGetAudioStreamStatus, (unsigned int id))

#endif
