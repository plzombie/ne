/*
	Файл	: dll_plg_minimp3.c

	Описание: Плагин для загрузки mp3 файлов

	История	: 22.11.24	Создан

*/

#include <stdbool.h>
#include <wchar.h>
#include <limits.h>

#if defined(N_POSIX)
	#define N_API __attribute__ ((visibility("default")))
	#define N_APIENTRY
	#define _wcsicmp wcscasecmp
#else
	#define N_API __declspec(dllexport)
	#define N_APIENTRY __cdecl
#endif

#include "../nyan/nyan_text.h"
#include "../nyan/nyan_engapi.h"
#include "../nyan/nyan_file_publicapi.h"
#include "../nyan/nyan_plgtypes_publicapi.h"
#include "../nyan/nyan_audiofile_publicapi.h"

#define MINIMP3_IMPLEMENTATION
#include "../forks/minimp3/minimp3_ex.h"

engapi_type *ea = 0;

bool N_APIENTRY minimp3_plgSupportExt(const wchar_t *fname, const wchar_t *fext);
bool N_APIENTRY minimp3_plgLoad(const wchar_t *fname, na_audiofile_type *aud);
bool N_APIENTRY minimp3_plgRead(unsigned int offset, unsigned int nofs, void *buf, na_audiofile_type *aud);
void N_APIENTRY minimp3_plgUnload(na_audiofile_type *aud);

na_audiofile_plugin_type plg = {sizeof(na_audiofile_plugin_type), &minimp3_plgSupportExt, &minimp3_plgLoad, &minimp3_plgRead, &minimp3_plgUnload};

plg_api_type plg_api = {sizeof(plg_api_type), N_PLG_TYPE_AFLOADER, L"MPEG 1 Part 3 *.mp3 files via minimp3", &plg};

typedef struct {
	mp3dec_ex_t dec;
	mp3dec_io_t io;
} minimp3_plgdata_type;

/*
	Функция	: callbackRead

	Описание: callback функция для чтения

	История	: 22.11.24	Создан

*/
size_t callbackRead(void *buf, size_t size, void *data)
{
	return (size_t)ea->nFileRead((unsigned int)data, buf, size);
}

/*
	Функция	: callbackSeek

	Описание: callback функция для поиска

	История	: 22.11.24	Создан

*/
int callbackSeek(uint64_t pos, void *data)
{
	return ea->nFileSeek((unsigned int)data, pos, FILE_SEEK_SET) != pos;
}

/*
	Функция	: plgGetApi

	Описание: Возвращает API плагина

	История	: 22.11.24	Создан

*/
N_API plg_api_type * N_APIENTRY plgGetApi(void)
{
	if(!ea) return 0;

	return &plg_api;
}

/*
	Функция	: plgSetupDll

	Описание: Настройка dll плагина

	История	: 22.11.24	Создан

*/
N_API bool N_APIENTRY plgSetupDll(engapi_type *engapi)
{
	if(engapi) {
		if(engapi->mysize == sizeof(engapi_type)) {
			ea = engapi;

			ea->nlPrint(L"%ls", L"lieff/minimp3\n"
				L"https://github.com/lieff/minimp3\n"
				L"To the extent possible under law, the author(s) have dedicated\n"
				L"all copyright and related and neighboring rights to this software\n"
				L"to the public domain worldwide.\n"
				L"This software is distributed without any warranty.\n"
				L"See <http://creativecommons.org/publicdomain/zero/1.0/>.");

			return true;
		}
	}

	return false;
}

/*
	Функция	: minimp3_plgSupportExt

	Описание: Проверяет поддержку типа файла

	История	: 22.11.24	Создан

*/
bool N_APIENTRY minimp3_plgSupportExt(const wchar_t *fname, const wchar_t *fext)
{
	(void)fname; // Неиспользуемая переменная

	if(_wcsicmp(fext,L"mp3") == 0) return true;
	return false;
}

/*
	Функция	: minimp3_plgLoad

	Описание: Загружает ogg файл

	История	: 22.11.24	Создан

*/
bool N_APIENTRY minimp3_plgLoad(const wchar_t *fname, na_audiofile_type *aud)
{
	minimp3_plgdata_type *plgdata;

	plgdata = (minimp3_plgdata_type *)malloc(sizeof(minimp3_plgdata_type));

	memset(plgdata, 0, sizeof(minimp3_plgdata_type));

	plgdata->io.read = callbackRead;
	plgdata->io.seek = callbackSeek;
	plgdata->io.read_data = plgdata->io.seek_data = (void *)ea->nFileOpen(fname);
	if(!plgdata->io.read_data) {
		free(plgdata);

		return false;
	}

	if(mp3dec_ex_open_cb(&plgdata->dec, &plgdata->io, MP3D_SEEK_TO_SAMPLE)) {
		ea->nFileClose((unsigned int)(plgdata->io.read_data));
		free(plgdata);

		return false;
	}

	if(plgdata->dec.samples > UINT_MAX || (plgdata->dec.info.channels != 1 && plgdata->dec.info.channels != 2)) {
		mp3dec_ex_close(&plgdata->dec);
		ea->nFileClose((unsigned int)(plgdata->io.read_data));
		free(plgdata);

		return false;
	}

	aud->bps = sizeof(mp3d_sample_t);
	aud->noc = plgdata->dec.info.channels;
	aud->nos = (unsigned int)(plgdata->dec.samples)/aud->noc;
	aud->freq = plgdata->dec.info.hz;
	if(aud->noc == 2)
		aud->sf = NA_SOUND_16BIT_STEREO;
	else if(aud->noc == 1)
		aud->sf = NA_SOUND_16BIT_MONO;

	aud->plgdata = plgdata;

	return true;
}

/*
	Функция	: minimp3_plgRead

	Описание: Читает mp3 файл
			offset - указывает количество секунд, которые надо пропустить от начала записи
			nofs - Количество семплов, которые надо прочитать
			aud - Структура, содержащая данные об аудиофайле
			Все три значения __всегда__ правильные, т.е. проверка не требуется
	История	: 22.11.24	Создан

*/
bool N_APIENTRY minimp3_plgRead(unsigned int offset, unsigned int nofs, void *buf, na_audiofile_type *aud)
{
	minimp3_plgdata_type *plgdata;
	uint64_t position;
	int dec_hz, dec_channels;
	unsigned int aud_noc;

	plgdata = (minimp3_plgdata_type *)aud->plgdata;

	aud_noc = aud->noc;

	dec_hz = plgdata->dec.info.hz;
	dec_channels = plgdata->dec.info.channels;

	if(dec_channels != aud_noc) return false;
	if(dec_hz != aud->freq) return false;

	if(UINT64_MAX/dec_hz < dec_channels) return false;
	if(UINT64_MAX/((uint64_t)dec_hz*(uint64_t)dec_channels) < offset) return false;

	position = (uint64_t)dec_hz*(uint64_t)dec_channels*offset;

	if(plgdata->dec.cur_sample != position)
		if(mp3dec_ex_seek(&plgdata->dec, position)) return false;

	if(mp3dec_ex_read(&plgdata->dec, (mp3d_sample_t *)buf, nofs*aud_noc) != nofs*aud_noc) {
		if(plgdata->dec.last_error)
			return false;
	}
	
	return true;
}

/*
	Функция	: minimp3_plgUnload

	Описание: Выгружает ogg файл

	История	: 22.11.24	Создан

*/
void N_APIENTRY minimp3_plgUnload(na_audiofile_type *aud)
{
	minimp3_plgdata_type *plgdata;

	plgdata = (minimp3_plgdata_type *)aud->plgdata;

	mp3dec_ex_close(&plgdata->dec);

	ea->nFileClose((unsigned int)(plgdata->io.read_data));

	free(plgdata);
}
