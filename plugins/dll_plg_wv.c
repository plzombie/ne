/*
	Файл	: dll_plg_wv.c

	Описание: Плагин для загрузки wv файлов

	История	: 20.10.12	Создан

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

#include "../forks/wavpack/wavpack.h"

#include "dll_plg_wv.h"

engapi_type *ea = 0;

//static int *plg_wvtempbuf = 0;
//static unsigned int plg_wvtempbufsize = 0;
//static int plg_wvloaded = 0;

bool N_APIENTRY wv_plgSupportExt(const wchar_t *fname, const wchar_t *fext);
bool N_APIENTRY wv_plgLoad(const wchar_t *fname, na_audiofile_type *aud);
bool N_APIENTRY wv_plgRead(unsigned int offset, unsigned int nofs, void *buf, na_audiofile_type *aud);
void N_APIENTRY wv_plgUnload(na_audiofile_type *aud);

na_audiofile_plugin_type plg = {sizeof(na_audiofile_plugin_type), &wv_plgSupportExt, &wv_plgLoad, &wv_plgRead, &wv_plgUnload};

plg_api_type plg_api = {sizeof(plg_api_type), N_PLG_TYPE_AFLOADER, L"WavPack *.wv files", &plg};

static int32_t read_bytes(void *id, void *data, int32_t bcount);
static int32_t write_bytes(void *id, void *data, int32_t bcount);
static int64_t get_pos(void *id);
static int set_pos_abs(void *id, int64_t pos);
static int set_pos_rel(void *id, int64_t delta, int mode);
static int push_back_byte(void *id, int c);
static int64_t get_length(void *id);
static int can_seek(void *id);
static int truncate_here(void *id);
static int close(void *id);

static WavpackStreamReader64 freader = {read_bytes, write_bytes, get_pos, set_pos_abs, set_pos_rel, push_back_byte, get_length, can_seek, truncate_here, close};

/*
	Функция	: plgGetApi

	Описание: Возвращает API плагина

	История	: 14.05.13	Создан

*/
N_API plg_api_type * N_APIENTRY plgGetApi(void)
{
	if(!ea) return 0;

	return &plg_api;
}

/*
	Функция	: plgSetupDll

	Описание: Настройка dll плагина

	История	: 20.10.12	Создан

*/
N_API bool N_APIENTRY plgSetupDll(engapi_type *engapi)
{
	if(engapi) {
		if(engapi->mysize == sizeof(engapi_type)) {
			uint32_t wvlibver;

			wvlibver = WavpackGetLibraryVersion();

			if(((wvlibver&0xff0000)>>16) < 5) {
				engapi->nlPrint(L"Too old Wavpack version, need 5 but current is %u.%u.%u", (wvlibver&0xff0000)>>16, (wvlibver&0xff00)>>8, wvlibver&0xff);

				return false;
			} else
				engapi->nlPrint(L"Wavpack version is %u.%u.%u", (wvlibver&0xff0000)>>16, (wvlibver&0xff00)>>8, wvlibver&0xff);

			ea = engapi;

			ea->nlPrint(L"%ls", L"this software using wavpack:\n\n"
				L"               Copyright (c) 1998 - 2013 Conifer Software\n"
				L"                          All rights reserved.\n"
				L"\n"
				L"Redistribution and use in source and binary forms, with or without\n"
				L"modification, are permitted provided that the following conditions are met:\n"
				L"\n"
				L"    * Redistributions of source code must retain the above copyright notice,\n"
				L"      this list of conditions and the following disclaimer.\n"
				L"    * Redistributions in binary form must reproduce the above copyright notice,\n"
				L"      this list of conditions and the following disclaimer in the\n"
				L"      documentation and/or other materials provided with the distribution.\n"
				L"    * Neither the name of Conifer Software nor the names of its contributors\n"
				L"      may be used to endorse or promote products derived from this software\n"
				L"      without specific prior written permission.\n"
				L"\n"
				L"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"\n"
				L"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
				L"IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n"
				L"ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR\n"
				L"ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n"
				L"DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR\n"
				L"SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER\n"
				L"CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,\n"
				L"OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n"
				L"OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.");

			return true;
		}
	}

	return false;
}

/*
	Функция	: wv_plgSupportExt

	Описание: Проверяет поддержку типа файла

	История	: 20.10.12	Создан

*/
bool N_APIENTRY wv_plgSupportExt(const wchar_t *fname, const wchar_t *fext)
{
	(void)fname; // Неиспользуемая переменная

	if(_wcsicmp(fext,L"wv") == 0) return true;
	return false;
}

/*
	Функция	: wv_plgLoad

	Описание: Загружает wv файл

	История	: 20.10.12	Создан

*/
bool N_APIENTRY wv_plgLoad(const wchar_t *fname, na_audiofile_type *aud)
{
	char errstr[80] = "";
	unsigned int f;

	ea->nlPrint(LOG_FDEBUGFORMAT7, L"wv_plgLoad()", N_FNAME, fname); ea->nlAddTab(1);

	f = ea->nFileOpen(fname);

	if(!f) {
		ea->nlAddTab(-1); ea->nlPrint(LOG_FDEBUGFORMAT, L"wv_plgLoad()", ERR_FILENOTFOUNDED);
		return false;
	}

	aud->plgdata = WavpackOpenFileInputEx64(&freader, (void *)(intptr_t)f, 0, errstr, OPEN_2CH_MAX | OPEN_NORMALIZE, 0);

	if(strlen(errstr) || !aud->plgdata) {
		ea->nFileClose(f);
		ea->nlAddTab(-1); ea->nlPrint(LOG_FDEBUGFORMAT, L"wv_plgLoad()", ERR_FILEISDAMAGED);
		return false;
	}

	if(WavpackGetReducedChannels(aud->plgdata) == 1) aud->sf = NA_SOUND_16BIT_MONO;
	else if(WavpackGetReducedChannels(aud->plgdata) == 2) aud->sf = NA_SOUND_16BIT_STEREO;
	else {
		ea->nFileClose(f);
		WavpackCloseFile(aud->plgdata);
		ea->nlAddTab(-1); ea->nlPrint(LOG_FDEBUGFORMAT, L"wv_plgLoad()", ERR_UNSUPPORTEDDATATYPE);
		return false;
	}

	aud->bps = 2; // байт в одном семпле
	aud->nos = WavpackGetNumSamples(aud->plgdata); // Количество семплов
	aud->noc = WavpackGetReducedChannels(aud->plgdata); // Кол-во каналов
	aud->freq = WavpackGetSampleRate(aud->plgdata); // Частота

	ea->nlAddTab(-1); ea->nlPrint(LOG_FDEBUGFORMAT, L"wv_plgLoad()", N_OK);

	return true;
}

/*
	Функция	: wv_plgRead

	Описание: Читает wv файл
			offset - указывает количество секунд, которые надо пропустить от начала записи
			nofs - Количество семплов, которые надо прочитать
			aud - Структура, содержащая данные об аудиофайле
			Все три значения __всегда__ правильные, т.е. проверка не требуется
	История	: 20.10.12	Создан

*/
bool N_APIENTRY wv_plgRead(unsigned int offset, unsigned int nofs, void *buf, na_audiofile_type *aud)
{
	double dconst;
	unsigned int i;
	short *pout;
	int32_t wvtempbuf[1000];
	unsigned int step;

	WavpackSeekSample(aud->plgdata, offset*aud->freq);

	pout = buf;

	step = 1000 / aud->noc;

	dconst = (1.0 / ((long long)1 << (WavpackGetBytesPerSample(aud->plgdata) * 8 - 1)));

	for(i = 0; i < nofs; i+=step) {
		unsigned int j;
		int *pin;
		uint32_t samples_to_read, samples_to_process;
		
		if(nofs - i < step)
			samples_to_read = nofs - i;
		else
			samples_to_read = step;

		WavpackUnpackSamples(aud->plgdata, wvtempbuf, samples_to_read);

		samples_to_process = samples_to_read*aud->noc;

		pin = wvtempbuf;

		for(j = 0; j < samples_to_process; j++) {
			double d;

			d = (*pin) * dconst;

			if(d >= 1.0)
				*pout = 32767;
			else if (d <= -1.0)
				*pout = -32768;
			else
				*pout = (short)(d * 32768.0);
			pin++; pout++;
		}
	}

	return true;
}

/*
	Функция	: wv_plgUnload

	Описание: Выгружает wv файл

	История	: 20.10.12	Создан

*/
void N_APIENTRY wv_plgUnload(na_audiofile_type *aud)
{
	WavpackCloseFile(aud->plgdata);
}

/*
	Функция	: ...

	Описание: Врапперы для функций работы с файлами

	История	: 08.02.14	Создан

*/
static int32_t read_bytes(void *id, void *data, int32_t bcount)
{
	int32_t ret = (int32_t)(ea->nFileRead((unsigned int)(intptr_t)id, data, bcount));

	return ret;
}

// Я бы добавил ещё can_write к функциям, но не мне решать

static int32_t write_bytes(void *id, void *data, int32_t bcount)
{
	return (int32_t)(ea->nFileWrite((unsigned int)(intptr_t)id, data, bcount));
}

static int64_t get_pos(void *id)
{
	return (int64_t)(ea->nFileTell((unsigned int)(intptr_t)id));
}

static int set_pos_abs(void *id, int64_t pos)
{
	long long ret;

	ret = ea->nFileSeek((unsigned int)(intptr_t)id, pos, FILE_SEEK_SET);

	if(ret == -1)
		return -1;
	else
		return 0;
}

static int set_pos_rel(void *id, int64_t delta, int mode)
{
	long long ret;

	if(mode == SEEK_SET)
		ret = ea->nFileSeek((unsigned int)(intptr_t)id, delta, FILE_SEEK_SET);
	else if(mode == SEEK_CUR)
		ret = ea->nFileSeek((unsigned int)(intptr_t)id, delta, FILE_SEEK_CUR);
	else
		ret = ea->nFileSeek((unsigned int)(intptr_t)id, delta, FILE_SEEK_END);

	if(ret == -1)
		return -1;
	else
		return 0;
}

static int push_back_byte(void *id, int c)
{
	if(ea->nFileSeek((unsigned int)(intptr_t)id, get_pos(id)-1, FILE_SEEK_SET) == -1) // Не уверен, что это правильная замена
		return EOF;
	else
		return c;
}

static int64_t get_length(void *id)
{
	return (int64_t)(ea->nFileLength((unsigned int)(intptr_t)id));
}

static int can_seek(void *id)
{
	(void)id; // Неиспользуемая переменная

	return true;
}

static int truncate_here(void *id)
{
	(void)id; // Неиспользуемая переменная

	return -1;
}

static int close(void *id)
{
	if(ea->nFileClose((unsigned int)(intptr_t)(id)))
		return 0;
	else
		return EOF;
}
