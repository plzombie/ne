/*
	Файл	: dll_plg_stb_vorbis.c

	Описание: Плагин для загрузки трекерных модулей через libopenmpt

	История	: 18.08.22	Создан

*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

#include "../forks/libopenmpt/inc/libopenmpt/libopenmpt.h"

engapi_type *ea = 0;

bool N_APIENTRY libopenmpt_plgSupportExt(const wchar_t *fname, const wchar_t *fext);
bool N_APIENTRY libopenmpt_plgLoad(const wchar_t *fname, na_audiofile_type *aud);
bool N_APIENTRY libopenmpt_plgRead(unsigned int offset, unsigned int nofs, void *buf, na_audiofile_type *aud);
void N_APIENTRY libopenmpt_plgUnload(na_audiofile_type *aud);

na_audiofile_plugin_type plg = {sizeof(na_audiofile_plugin_type), &libopenmpt_plgSupportExt, &libopenmpt_plgLoad, &libopenmpt_plgRead, &libopenmpt_plgUnload};

plg_api_type plg_api = {sizeof(plg_api_type), N_PLG_TYPE_AFLOADER, L"Tracker modules via libopenmpt", &plg};

typedef struct {
	openmpt_module *module;
	unsigned char *buf;
	int16_t *lbuf, *rbuf;
	size_t samples_offset;
} libopenmpt_plgdata_type;

/*
	Функция	: plgGetApi

	Описание: Возвращает API плагина

	История	: 18.08.22	Создан

*/
N_API plg_api_type * N_APIENTRY plgGetApi(void)
{
	if(!ea) return 0;

	return &plg_api;
}

/*
	Функция	: plgSetupDll

	Описание: Настройка dll плагина

	История	: 18.08.22	Создан

*/
N_API bool N_APIENTRY plgSetupDll(engapi_type *engapi)
{
	if(engapi) {
		if(engapi->mysize == sizeof(engapi_type)) {
			ea = engapi;

			ea->nlPrint(L"%hs\n", openmpt_get_string("license"));
			
			return true;
		}
	}

	return false;
}

/*
	Функция	: stb_vorbis_plgSupportExt

	Описание: Проверяет поддержку типа файла

	История	: 18.08.22	Создан

*/
bool N_APIENTRY libopenmpt_plgSupportExt(const wchar_t *fname, const wchar_t *fext)
{
	(void)fname; // Неиспользуемая переменная

	if(_wcsicmp(fext,L"mod") == 0) return true;
	if(_wcsicmp(fext,L"xm") == 0) return true;
	if(_wcsicmp(fext,L"s3m") == 0) return true;
	if(_wcsicmp(fext,L"it") == 0) return true;
	return false;
}

static void libopenmpt_plgLogFunc(const char *message, void *data)
{
	(void)data;
	
	if(message)
		ea->nlPrint(L"libopenmpt: %hs", message);
}

/*
	Функция	: libopenmpt_plgLoad

	Описание: Загружает трекерный модуль

	История	: 18.08.22	Создан

*/
bool N_APIENTRY libopenmpt_plgLoad(const wchar_t *fname, na_audiofile_type *aud)
{
	openmpt_module *module;
	libopenmpt_plgdata_type *plgdata;
	void *buf = 0;
	size_t length = 0;
	unsigned int f;
	
	ea->nlPrint(LOG_FDEBUGFORMAT7, L"libopenmpt_plgLoad()", N_FNAME, fname); ea->nlAddTab(1);
	
	f = ea->nFileOpen(fname);
	if(!f) {
		ea->nlAddTab(-1); ea->nlPrint(LOG_FDEBUGFORMAT, L"libopenmpt_plgLoad()", ERR_FILENOTFOUNDED);
		return false;
	}
	
	length = ea->nFileLength(f);
	if(!length) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"libopenmpt_plgLoad()", L"false");
		ea->nFileClose(f);
		return false;
	}
	
	buf = ea->nAllocMemory(length);
	if(!buf) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"libopenmpt_plgLoad()", L"false");
		ea->nFileClose(f);
		return false;
	}
	
	ea->nFileRead(f, buf, length);
	ea->nFileClose(f);
	
	module = openmpt_module_create_from_memory2(buf, length, libopenmpt_plgLogFunc, 0, 0, 0, 0, 0, 0);
	if(!module) {
		ea->nFreeMemory(buf);
		ea->nlAddTab(-1); ea->nlPrint(LOG_FDEBUGFORMAT, L"libopenmpt_plgLoad()", ERR_FILEISDAMAGED);
		return false;
	}
	
	plgdata = ea->nAllocMemory(sizeof(libopenmpt_plgdata_type));
	if(!plgdata) {
		openmpt_module_destroy(module);
		ea->nFreeMemory(buf);
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"libopenmpt_plgLoad()", L"false");
		return false;
	}
	
	plgdata->lbuf = ea->nAllocMemory(48000*sizeof(int16_t));
	plgdata->rbuf = ea->nAllocMemory(48000*sizeof(int16_t));
	if(!plgdata->lbuf || !plgdata->rbuf) {
		openmpt_module_destroy(module);
		if(plgdata->lbuf) ea->nFreeMemory(plgdata->lbuf);
		if(plgdata->rbuf) ea->nFreeMemory(plgdata->rbuf);
		ea->nFreeMemory(plgdata);
		ea->nFreeMemory(buf);
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"libopenmpt_plgLoad()", L"false");
		return false;
	}
	
	plgdata->module = module;
	plgdata->buf = buf;
	plgdata->samples_offset = 0;
	aud->plgdata = plgdata;
	
	aud->sf = NA_SOUND_16BIT_STEREO;
	aud->bps = 2; // байт в одном семпле
	aud->nos = 48000*openmpt_module_get_duration_seconds(module); // Количество семплов
	aud->noc = 2; // Кол-во каналов
	aud->freq = 48000; // Частота
	
	ea->nlAddTab(-1); ea->nlPrint(LOG_FDEBUGFORMAT, L"stb_vorbis_plgLoad()", N_OK);

	return true;
}

/*
	Функция	: libopenmpt_plgRead

	Описание: Читает трекерный модуль
			offset - указывает количество секунд, которые надо пропустить от начала записи
			nofs - Количество семплов, которые надо прочитать
			aud - Структура, содержащая данные об аудиофайле
			Все три значения __всегда__ правильные, т.е. проверка не требуется
	История	: 18.08.22	Создан

*/
bool N_APIENTRY libopenmpt_plgRead(unsigned int offset, unsigned int nofs, void *buf, na_audiofile_type *aud)
{
	size_t frames_read, i;
	libopenmpt_plgdata_type *plgdata;
	int16_t *p, *lp, *rp;
	
	if(!aud->plgdata)
		return false;
	
	plgdata = aud->plgdata;
	
	if(plgdata->samples_offset != offset*aud->freq) {
		plgdata->samples_offset = offset*aud->freq;
		openmpt_module_set_position_seconds(plgdata->module, (double)offset);
	}
	
	frames_read = openmpt_module_read_stereo(plgdata->module,
		aud->freq, (size_t)nofs,
		plgdata->lbuf,
		plgdata->rbuf);
	
	plgdata->samples_offset += frames_read;
	
	p = buf;
	lp = plgdata->lbuf;
	rp = plgdata->rbuf;
	for(i = 0; i < frames_read; i++) {
		*(p++) = *(lp++);
		*(p++) = *(rp++);
	}
	
	if(frames_read != (size_t)nofs) {
		wprintf(L"gotcha");
		memset((short *)buf+frames_read*aud->noc, 0, (nofs-frames_read)*aud->noc*sizeof(short));
	}

	return true;
}

/*
	Функция	: libopenmpt_plgUnload

	Описание: Выгружает трекерный модуль

	История	: 18.08.22	Создан

*/
void N_APIENTRY libopenmpt_plgUnload(na_audiofile_type *aud)
{
	openmpt_module_destroy(((libopenmpt_plgdata_type *)aud->plgdata)->module);
	ea->nFreeMemory(((libopenmpt_plgdata_type *)aud->plgdata)->buf);
	ea->nFreeMemory(aud->plgdata);
}
