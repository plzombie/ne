/*
	Файл	: dll_plg_stb_vorbis.c

	Описание: Плагин для загрузки ogg vorbis файлов

	История	: 23.12.16	Создан

*/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>

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

#define STB_VORBIS_HEADER_ONLY

#include "../forks/stb/stb_vorbis.c"

#include "dll_plg_stb_vorbis.h"

engapi_type *ea = 0;

bool N_APIENTRY stb_vorbis_plgSupportExt(const wchar_t *fname, const wchar_t *fext);
bool N_APIENTRY stb_vorbis_plgLoad(const wchar_t *fname, na_audiofile_type *aud);
bool N_APIENTRY stb_vorbis_plgRead(unsigned int offset, unsigned int nofs, void *buf, na_audiofile_type *aud);
void N_APIENTRY stb_vorbis_plgUnload(na_audiofile_type *aud);

na_audiofile_plugin_type plg = {sizeof(na_audiofile_plugin_type), &stb_vorbis_plgSupportExt, &stb_vorbis_plgLoad, &stb_vorbis_plgRead, &stb_vorbis_plgUnload};

plg_api_type plg_api = {sizeof(plg_api_type), N_PLG_TYPE_AFLOADER, L"Ogg vorbis *.ogg files via stb_vorbis", &plg};

typedef struct {
	stb_vorbis *vf;
	unsigned char *buf;
} stb_vorbis_plgdata_type;

/*
	Функция	: plgGetApi

	Описание: Возвращает API плагина

	История	: 23.12.16	Создан

*/
N_API plg_api_type * N_APIENTRY plgGetApi(void)
{
	if(!ea) return 0;

	return &plg_api;
}

/*
	Функция	: plgSetupDll

	Описание: Настройка dll плагина

	История	: 23.12.16	Создан

*/
N_API bool N_APIENTRY plgSetupDll(engapi_type *engapi)
{
	if(engapi) {
		if(engapi->mysize == sizeof(engapi_type)) {
			ea = engapi;

			ea->nlPrint(L"%ls", L"Ogg Vorbis audio decoder - v1.09 - public domain\n"
				L"http://nothings.org/stb_vorbis/ \n\n"
				L"Original version written by Sean Barrett in 2007.\n\n"
				L"Originally sponsored by RAD Game Tools. Seeking sponsored\n"
				L"by Phillip Bennefall, Marc Andersen, Aaron Baker, Elias Software,\n"
				L"Aras Pranckevicius, and Sean Barrett.");

			return true;
		}
	}

	return false;
}

/*
	Функция	: stb_vorbis_plgSupportExt

	Описание: Проверяет поддержку типа файла

	История	: 23.12.16	Создан

*/
bool N_APIENTRY stb_vorbis_plgSupportExt(const wchar_t *fname, const wchar_t *fext)
{
	(void)fname; // Неиспользуемая переменная

	if(_wcsicmp(fext,L"ogg") == 0) return true;
	return false;
}

/*
	Функция	: stb_vorbis_plgLoad

	Описание: Загружает ogg файл

	История	: 23.12.16	Создан

*/
bool N_APIENTRY stb_vorbis_plgLoad(const wchar_t *fname, na_audiofile_type *aud)
{
	unsigned int f;
	stb_vorbis *vf;
	stb_vorbis_info vi;
	stb_vorbis_plgdata_type *plgdata;
	unsigned char *buf;
	int length;
	int error;

	ea->nlPrint(LOG_FDEBUGFORMAT7, L"stb_vorbis_plgLoad()", N_FNAME, fname); ea->nlAddTab(1);

	f = ea->nFileOpen(fname);

	if(!f) {
		ea->nlAddTab(-1); ea->nlPrint(LOG_FDEBUGFORMAT, L"stb_vorbis_plgLoad()", ERR_FILENOTFOUNDED);
		return false;
	}

	// Чтение файла в память
	length = (int)ea->nFileLength(f);

	buf = ea->nAllocMemory(length);

	if(!buf) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"stb_vorbis_plgLoad()", L"false");
		ea->nFileClose(f);
		return false;
	}

	ea->nFileRead(f, buf, length);

	ea->nFileClose(f);

	vf = stb_vorbis_open_memory(buf, length, &error, 0);

	if(!vf) {
		ea->nFreeMemory(buf);
		ea->nlAddTab(-1); ea->nlPrint(LOG_FDEBUGFORMAT, L"stb_vorbis_plgLoad()", ERR_FILEISDAMAGED);
		return false;
	}

	vi = stb_vorbis_get_info(vf);

	if(vi.channels == 1) aud->sf = NA_SOUND_16BIT_MONO;
	else if(vi.channels == 2) aud->sf = NA_SOUND_16BIT_STEREO;
	else {
		stb_vorbis_close(vf);
		ea->nFreeMemory(buf);
		ea->nlAddTab(-1); ea->nlPrint(LOG_FDEBUGFORMAT, L"stb_vorbis_plgLoad()", ERR_UNSUPPORTEDDATATYPE);
		return false;
	}

	plgdata = ea->nAllocMemory(sizeof(stb_vorbis_plgdata_type));
	if(!plgdata) {
		stb_vorbis_close(vf);
		ea->nFreeMemory(buf);
		ea->nlAddTab(-1); ea->nlPrint(LOG_FDEBUGFORMAT, L"stb_vorbis_plgLoad()", N_FALSE);
		return false;
	}

	plgdata->buf = buf;
	plgdata->vf = vf;

	aud->plgdata = plgdata;

	aud->bps = 2; // байт в одном семпле
	aud->nos = stb_vorbis_stream_length_in_samples(vf); // Количество семплов
	aud->noc = vi.channels; // Кол-во каналов
	aud->freq = vi.sample_rate; // Частота

	ea->nlAddTab(-1); ea->nlPrint(LOG_FDEBUGFORMAT, L"stb_vorbis_plgLoad()", N_OK);

	return true;
}

/*
	Функция	: stb_vorbis_plgRead

	Описание: Читает ogg файл
			offset - указывает количество секунд, которые надо пропустить от начала записи
			nofs - Количество семплов, которые надо прочитать
			aud - Структура, содержащая данные об аудиофайле
			Все три значения __всегда__ правильные, т.е. проверка не требуется
	История	: 23.12.16	Создан

*/
bool N_APIENTRY stb_vorbis_plgRead(unsigned int offset, unsigned int nofs, void *buf, na_audiofile_type *aud)
{
	int samples_read;

	if(!aud->plgdata)
		return false;

	stb_vorbis_seek(((stb_vorbis_plgdata_type *)aud->plgdata)->vf, offset*aud->freq);

	samples_read = stb_vorbis_get_samples_short_interleaved(((stb_vorbis_plgdata_type *)aud->plgdata)->vf, aud->noc, buf, nofs*aud->noc);

	if((unsigned int)samples_read != nofs) {
		memset((short *)buf+samples_read*aud->noc, 0, (nofs-samples_read)*aud->noc*sizeof(short));
	}

	return true;
}

/*
	Функция	: stb_vorbis_plgUnload

	Описание: Выгружает ogg файл

	История	: 23.12.16	Создан

*/
void N_APIENTRY stb_vorbis_plgUnload(na_audiofile_type *aud)
{
	stb_vorbis_close(((stb_vorbis_plgdata_type *)aud->plgdata)->vf);
	ea->nFreeMemory(((stb_vorbis_plgdata_type *)aud->plgdata)->buf);
}
