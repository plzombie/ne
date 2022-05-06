/*
	Файл	: nyan_plgloader.c

	Описание: Загрузка плагинов

	История	: 18.08.12	Создан

*/

#include <stdlib.h>
#include <string.h>

#include "nyan_au_init_publicapi.h"
#include "nyan_filesys_publicapi.h"
#include "nyan_log_publicapi.h"
#include "nyan_mem_publicapi.h"
#include "nyan_plgtypes_publicapi.h"
#include "nyan_plgloader_publicapi.h"
#include "nyan_vis_texture_publicapi.h"
#include "nyan_text.h"

#include "nyan_apifordlls.h"
#include "nyan_getproc.h"

static void **dllhandles;
static unsigned int maxdllhandles = 0, allocdllhandles = 0;

typedef bool (N_APIENTRY *n_plg_setupdll_type)(engapi_type *engapi);
typedef plg_api_type *(N_APIENTRY *n_plg_getapi_type)(void);

/*
	Функция	: nAddPlugin

	Описание: Загружает плагин. Возвращает true при удаче, иначе false

	История	: 18.08.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nAddPlugin(const wchar_t *dllname)
{
	bool success = true;
	n_plg_setupdll_type plgSetupDll;
	n_plg_getapi_type plgGetApi;
	plg_api_type *plg_api;

	nlPrint(LOG_FDEBUGFORMAT7,F_NADDPLUGIN,N_FNAME,dllname); nlAddTab(1);

	if(!allocdllhandles) {
		dllhandles = nAllocMemory(1024*sizeof(void *));

		if(dllhandles)
			allocdllhandles = 1024;
		else
			success = false;
	} else if(maxdllhandles == allocdllhandles) {
		void **_dllhandles;

		_dllhandles = nReallocMemory(dllhandles, (allocdllhandles+1024)*sizeof(void *));

		if(_dllhandles) {
			dllhandles = _dllhandles;
			allocdllhandles += 1024;
		} else
			success = false;
	}

	if(success) {
		dllhandles[maxdllhandles] = nLoadModule(dllname);

		if(!dllhandles[maxdllhandles]) success = false;
	}

	// Получаем указатели на функции, описывающие плагин
	if(success) {
		plgSetupDll = (n_plg_setupdll_type)nGetProcAddress(dllhandles[maxdllhandles], "plgSetupDll", "_plgSetupDll");
		plgGetApi = (n_plg_getapi_type)nGetProcAddress(dllhandles[maxdllhandles], "plgGetApi", "_plgGetApi");
		if(!plgSetupDll || !plgGetApi) success = false;
	}

	// Передаём плагину API движка
	if(success) {
		success = plgSetupDll(&n_ea);
	}

	// Получаем указатели на API плагина
	if(success) {
		plg_api = plgGetApi();
		if(!plg_api) success = false;
	}

	// Регистрируем плагин
	if(success) {
		switch(plg_api->type) {
			case N_PLG_TYPE_TEXLOADER:
				if(((nv_tex_plugin_type *)(plg_api->api))->size != sizeof(nv_tex_plugin_type)) success = false;

				if(success) {
					if(plg_api->name)
						success = nvAddTexturePlugin(plg_api->name, plg_api->api);
					else
						success = nvAddTexturePlugin(dllname, plg_api->api);
				}

				break;
			case N_PLG_TYPE_AFLOADER:
				if(((na_audiofile_plugin_type *)(plg_api->api))->size != sizeof(na_audiofile_plugin_type)) success = false;

				if(success) {
					if(plg_api->name)
						success = naAddAudioFilePlugin(plg_api->name, plg_api->api);
					else
						success = naAddAudioFilePlugin(dllname, plg_api->api);
				}

				break;
			case N_PLG_TYPE_FILESYS:
				if(((fs_fileplg_type *)(plg_api->api))->size != sizeof(fs_fileplg_type)) success = false;

				if(success) {
					if(plg_api->name)
						success = nAddFilePlugin(plg_api->name, plg_api->api);
					else
						success = nAddFilePlugin(dllname, plg_api->api);
				}

				break;
			default:
				success = false;
		}

		// Если загружен успешно, то увеличиваем счётчик на 1
		if(success)
			maxdllhandles++;
		else // Если нет, то закрываем dll
			nFreeLib(dllhandles[maxdllhandles]);
	}

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NADDPLUGIN, N_OK, N_SUCCESS, success);

	return success;
}

/*
	Функция	: plgDeleteAll

	Описание: Удаляет все загруженные плагины

	История	: 18.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nDeleteAllPlugins(void)
{
	nlPrint(F_NDELETEALLPLUGINS); nlAddTab(1);

	nDeleteAllFilePlugins();
	naDeleteAllAudioFilePlugins();
	nvDeleteAllTexturePlugins();

	if(allocdllhandles) {
		unsigned int i;

		for(i=0;i<maxdllhandles;i++)
			nFreeLib(dllhandles[i]);

		nFreeMemory(dllhandles);
	}

	dllhandles = 0;
	maxdllhandles = 0;
	allocdllhandles = 0;

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NDELETEALLPLUGINS, N_OK);
}


