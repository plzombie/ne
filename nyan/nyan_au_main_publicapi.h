/*
	Файл	: nyan_au_main_publicapi.h

	Описание: Публичные функции аудиодвижка

	История	: 04.07.17	Создан

*/

#ifndef NYAN_AU_MAIN_PUBLICAPI_H
#define NYAN_AU_MAIN_PUBLICAPI_H

#include <stdbool.h>
#include <wchar.h>

#include "nyan_decls_publicapi.h"

#include "nyan_audiosourcestatus_publicapi.h"

NYAN_FUNC(unsigned int, naCreateBuffer, (const wchar_t *fname))
NYAN_FUNC(bool, naDestroyAllBuffers, (void))
NYAN_FUNC(bool, naDestroyBuffer, (unsigned int id))
NYAN_FUNC(bool, naLoadBuffer, (unsigned int id))
NYAN_FUNC(bool, naUnloadBuffer, (unsigned int id))
NYAN_FUNC(unsigned int, naCreateSource, (unsigned int bufid, int loop))
NYAN_FUNC(bool, naDestroyAllSources, (void))
NYAN_FUNC(bool, naDestroySource, (unsigned int id))
NYAN_FUNC(void, naPlaySource, (unsigned int id))
NYAN_FUNC(void, naPauseSource, (unsigned int id))
NYAN_FUNC(void, naStopSource, (unsigned int id))
NYAN_FUNC(void, naReplaySource, (unsigned int id))
NYAN_FUNC(void, naGetSourceLoop, (unsigned int id, int *loop))
NYAN_FUNC(void, naSetSourceLoop, (unsigned int id, int loop))
NYAN_FUNC(void, naGetSourceSecOffset, (unsigned int id, unsigned int *offset))
NYAN_FUNC(void, naSetSourceSecOffset, (unsigned int id, unsigned int offset))
NYAN_FUNC(void, naGetSourceLength, (unsigned int id, unsigned int *length))
NYAN_FUNC(void, naGetSourceGain, (unsigned int id, float *gain))
NYAN_FUNC(void, naSetSourceGain, (unsigned int id, float gain))
NYAN_FUNC(unsigned int, naGetSourceStatus, (unsigned int id))
NYAN_FUNC(unsigned int, naCreateAudioStream, (const wchar_t *fname, int loop))
NYAN_FUNC(unsigned int, naCreateAudioStreamEx, (const wchar_t **fname, unsigned int files, int loop))
NYAN_FUNC(void, naPlayAudioStream, (unsigned int id))
NYAN_FUNC(void, naPauseAudioStream, (unsigned int id))
NYAN_FUNC(void, naStopAudioStream, (unsigned int id))
NYAN_FUNC(void, naReplayAudioStream, (unsigned int id))
NYAN_FUNC(void, naGetAudioStreamLoop, (unsigned int id, int *loop))
NYAN_FUNC(void, naSetAudioStreamLoop, (unsigned int id, int loop))
NYAN_FUNC(void, naGetAudioStreamSecOffset, (unsigned int id, unsigned int *offset, unsigned int *track))
NYAN_FUNC(void, naSetAudioStreamSecOffset, (unsigned int id, unsigned int offset, unsigned int track))
NYAN_FUNC(void, naGetAudioStreamLength, (unsigned int id, unsigned int *length, unsigned int *tracks))
NYAN_FUNC(void, naGetAudioStreamGain, (unsigned int id, float *gain))
NYAN_FUNC(void, naSetAudioStreamGain, (unsigned int id, float gain))
NYAN_FUNC(unsigned int, naGetAudioStreamStatus, (unsigned int id))
NYAN_FUNC(bool, naDestroyAllAudioStreams, (void))
NYAN_FUNC(bool, naDestroyAudioStream, (unsigned int id))

#endif
