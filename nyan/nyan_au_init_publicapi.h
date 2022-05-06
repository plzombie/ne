/*
	Файл	: nyan_au_init_publicapi.h

	Описание: Публичные функции для инициализации аудиодвижка

	История	: 04.07.17	Создан

*/

#ifndef NYAN_AU_INIT_PUBLICAPI_H
#define NYAN_AU_INIT_PUBLICAPI_H

#include <stdbool.h>
#include <wchar.h>

#include "nyan_decls_publicapi.h"

#include "nyan_audiofile_publicapi.h"

NYAN_FUNC(bool, naAttachLib, (const wchar_t *dllname))

NYAN_FUNC(bool, naLoadAudioFile, (const wchar_t *fname, na_audiofile_type *aud))
NYAN_FUNC(bool, naReadAudioFile, (unsigned int offset, unsigned int nofs, void *buf, na_audiofile_type *aud))
NYAN_FUNC(void, naUnloadAudioFile, (na_audiofile_type *aud))

NYAN_FUNC(bool, naAddAudioFilePlugin, (const wchar_t *name, na_audiofile_plugin_type *au_fileplg))
NYAN_FUNC(void, naDeleteAllAudioFilePlugins, (void))

#endif
