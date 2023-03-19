/*
	Файл	: nyan_vis_texture.c

	Описание: Работа с текстурами

	История	: 15.08.12	Создан

*/

#include <stdlib.h>
#include <string.h>

#include "nyan_log_publicapi.h"
#include "nyan_mem_publicapi.h"
#include "nyan_threads_publicapi.h"
#include "nyan_vis_texture_publicapi.h"
#include "nyan_text.h"

#include "nyan_vis_texture.h"
#include "nyan_vis_init.h"
#include "nyan_vis_draw.h"

#include "nyan_nglapi.h"

#include "nyan_apifordlls.h"
#include "../commonsrc/core/nyan_array.h"

nv_texobj_type *nv_texobjs = 0;
unsigned int nv_maxtexobjs = 0, nv_alloctexobjs = 0;

uintptr_t nv_textures_sync_mutex = 0; // Мьютекс для синхронизации работы с текстурами

#define NVLOCKTEXTURESYNCMUTEX if(nv_isinit){nLockMutex(nv_textures_sync_mutex);}
#define NVUNLOCKTEXTURESYNCMUTEX if(nv_isinit){nUnlockMutex(nv_textures_sync_mutex);}

static nv_tex_plugin_type *nv_tex_plugins;
static unsigned int nv_tex_maxplugins = 0;
static unsigned int nv_tex_allocplugins = 0;

/*
	Функция	: nvCheckTextureArray

	Описание: Проверяет свободное место для текстуры в массиве

	История	: 19.03.23	Создан

*/
static bool nvCheckTextureArray(void *array_el, bool set_free)
{
	nv_texobj_type *el;
	
	el = (nv_texobj_type *)array_el;
	
	if(set_free) el->status = NV_TEX_STATUS_FREE;
	
	return (el->status == NV_TEX_STATUS_FREE)?true:false;
}

/*
	Функция	: nvCreateTexture

	Описание: Создаёт текстуру

	История	: 15.08.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvCreateTexture(int flags)
{
	unsigned int i;

	if(!nv_isinit) return 0;

	nlPrint(F_NVCREATETEXTURE); nlAddTab(1);

	NVLOCKTEXTURESYNCMUTEX
		// Выделение памяти под структуры
		if(!nArrayAdd(
			&n_ea, (void **)(&nv_texobjs),
			&nv_maxtexobjs,
			&nv_alloctexobjs,
			nvCheckTextureArray,
			&i,
			NYAN_ARRAY_DEFAULT_STEP,
			sizeof(nv_texobj_type))
		) {
			NVUNLOCKTEXTURESYNCMUTEX
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATETEXTURE, N_FALSE, N_ID, 0);
			return false;
		}

		nv_texobjs[i].status = NV_TEX_STATUS_EMPTY;
		nv_texobjs[i].flags = flags;
		nv_texobjs[i].ltype = NV_TEX_LTYPE_LOADVOID;
		nv_texobjs[i].nglid = 0;
	NVUNLOCKTEXTURESYNCMUTEX

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATETEXTURE, N_OK, N_ID, i+1);

	return (i+1);
}

/*
	Функция	: nvCreateTextureFromFile

	Описание: Создаёт текстуру, загружающуюся из файла

	История	: 15.06.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvCreateTextureFromFile(const wchar_t *fname, int flags)
{
	int id;

	if(!nv_isinit) return 0;

	nlPrint(LOG_FDEBUGFORMAT7, F_NVCREATETEXTUREFROMFILE, N_FNAME, fname); nlAddTab(1);

	NVLOCKTEXTURESYNCMUTEX
		id = nvCreateTexture(flags);

		if(id) {
			nv_texobjs[id-1].fname = nAllocMemory((wcslen(fname)+1)*sizeof(wchar_t));

			if(nv_texobjs[id-1].fname) {
				nv_texobjs[id-1].ltype = NV_TEX_LTYPE_LOADFNAME;
				wmemcpy(nv_texobjs[id-1].fname, fname, wcslen(fname)+1);
			} else {
				nvDestroyTexture(id);
				id = 0;
			}
		}
	NVUNLOCKTEXTURESYNCMUTEX

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATETEXTUREFROMFILE, N_OK, N_ID, id);

	return id;
}

/*
	Функция	: nvCreateTextureFromMemory

	Описание: Создаёт текстуру, загружающуюся из памяти

	История	: 11.08.14	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvCreateTextureFromMemory(nv_texture_type *tex, int flags)
{
	int id;

	if(!nv_isinit) return 0;

	nlPrint(F_NVCREATETEXTUREFROMMEMORY); nlAddTab(1);

	NVLOCKTEXTURESYNCMUTEX
		id = nvCreateTexture(flags);

		if(id) {
			nv_texobjs[id-1].ltype = NV_TEX_LTYPE_LOADMEMORY;

			nv_texobjs[id-1].tex = *tex;
		}
	NVUNLOCKTEXTURESYNCMUTEX

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATETEXTUREFROMMEMORY, N_OK, N_ID, id);

	return id;
}

/*
	Функция	: nvSetTextureLoadTypeVoid

	Описание: Устанавливает текстуру как пустую (без источника для загрузки)

	История	: 11.08.14	Создан

*/
N_API bool N_APIENTRY_EXPORT nvSetTextureLoadTypeVoid(unsigned int id)
{
	if(!nv_isinit) return false;

	NVLOCKTEXTURESYNCMUTEX
		if(id > nv_maxtexobjs || id == 0) {
			NVUNLOCKTEXTURESYNCMUTEX
			return false;
		}

		if(nv_texobjs[id-1].status != NV_TEX_STATUS_EMPTY) {
			NVUNLOCKTEXTURESYNCMUTEX
			return false;
		}

		if(nv_texobjs[id-1].ltype == NV_TEX_LTYPE_LOADFNAME)
			nFreeMemory(nv_texobjs[id-1].fname);

		nv_texobjs[id-1].ltype = NV_TEX_LTYPE_LOADVOID;
	NVUNLOCKTEXTURESYNCMUTEX

	return true;
}

/*
	Функция	: nvSetTextureLoadTypeFromFile

	Описание: Устанавливает текстуру как загружающуюся из файла

	История	: 11.08.14	Создан

*/
N_API bool N_APIENTRY_EXPORT nvSetTextureLoadTypeFromFile(unsigned int id, const wchar_t *fname)
{
	if(!nv_isinit) return false;

	NVLOCKTEXTURESYNCMUTEX
		if(id > nv_maxtexobjs || id == 0) {
			NVUNLOCKTEXTURESYNCMUTEX
			return false;
		}

		if(nv_texobjs[id-1].status != NV_TEX_STATUS_EMPTY) {
			NVUNLOCKTEXTURESYNCMUTEX
			return false;
		}

		if(nv_texobjs[id-1].ltype == NV_TEX_LTYPE_LOADFNAME)
			nFreeMemory(nv_texobjs[id-1].fname);
		else
			nv_texobjs[id-1].ltype = NV_TEX_LTYPE_LOADFNAME;

		nv_texobjs[id-1].fname = nAllocMemory((wcslen(fname)+1)*sizeof(wchar_t));
		wmemcpy(nv_texobjs[id-1].fname, fname, wcslen(fname)+1);
	NVUNLOCKTEXTURESYNCMUTEX

	return true;
}

/*
	Функция	: nvSetTextureLoadTypeFromMemory

	Описание: Устанавливает текстуру как загружающуюся из памяти

	История	: 11.08.14	Создан

*/
N_API bool N_APIENTRY_EXPORT nvSetTextureLoadTypeFromMemory(unsigned int id, nv_texture_type *tex)
{
	if(!nv_isinit) return false;

	NVLOCKTEXTURESYNCMUTEX
		if(id > nv_maxtexobjs || id == 0) {
			NVUNLOCKTEXTURESYNCMUTEX
			return false;
		}

		if(nv_texobjs[id-1].status != NV_TEX_STATUS_EMPTY) {
			NVUNLOCKTEXTURESYNCMUTEX
			return false;
		}

		if(nv_texobjs[id-1].ltype == NV_TEX_LTYPE_LOADFNAME)
			nFreeMemory(nv_texobjs[id-1].fname);

		nv_texobjs[id-1].ltype = NV_TEX_LTYPE_LOADMEMORY;

		nv_texobjs[id-1].tex = *tex;
	NVUNLOCKTEXTURESYNCMUTEX

	return true;
}

/*
	Функция	: nvLoadImageToMemory

	Описание: Загружает изображение из файла в память, в структуру, описывающую текстуру

	История	: 26.12.16	Создан

*/
N_API bool N_APIENTRY_EXPORT nvLoadImageToMemory(const wchar_t *fname, nv_texture_type *tex)
{
	unsigned int i;
	wchar_t *filext;

	filext = wcsrchr(fname, L'.');

	if(filext) filext++; else return false;

	NVLOCKTEXTURESYNCMUTEX
		for(i = 0; i<nv_tex_maxplugins; i++)
			if(nv_tex_plugins[i].SupportExt(fname, filext))
				if(nv_tex_plugins[i].Load(fname, tex)) {
					NVUNLOCKTEXTURESYNCMUTEX
					return true;
				}
	NVUNLOCKTEXTURESYNCMUTEX

	return false;
}

/*
	Функция	: nvTexLoadFromFile

	Описание: Загружает текстуру из файла

	История	: 15.06.12	Создан

*/
bool nvLoadTextureFromFile(nv_texobj_type *texobj)
{
	if(!nv_isinit) return false;

	if(texobj->status != NV_TEX_STATUS_LOADING) return false;

	return nvLoadImageToMemory(texobj->fname, &(texobj->tex));
}

/*
	Функция	: nvUpdateTextureFromMemory

	Описание: Обновляет текстуру, загружая данные из буфера buffer. Формат текстуры (данных в массиве buffer) должен быть тем же, что и при создании текстуры

	История	: 12.02.13	Создан

*/
N_API bool N_APIENTRY_EXPORT nvUpdateTextureFromMemory(unsigned int id, unsigned char *buffer)
{
	bool result;

	NVLOCKTEXTURESYNCMUTEX
		if(!nv_isinit || id > nv_maxtexobjs || id == 0) {
			NVUNLOCKTEXTURESYNCMUTEX
			return false;
		}

		if(nv_texobjs[id-1].status != NV_TEX_STATUS_LOADED) {
			NVUNLOCKTEXTURESYNCMUTEX
			return false;
		}

		result = funcptr_nglUpdateTexture(nv_texobjs[id-1].nglid, buffer);
	NVUNLOCKTEXTURESYNCMUTEX

	return result;
}

/*
	Функция	: nvLoadTexture

	Описание: Загружает текстуру

	История	: 15.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nvLoadTexture(unsigned int id)
{
	bool success = false;
	nv_texobj_type texobj;

	nlPrint(LOG_FDEBUGFORMAT4, F_NVLOADTEXTURE, N_ID, id); nlAddTab(1);

	NVLOCKTEXTURESYNCMUTEX
		if(!nv_isinit || id > nv_maxtexobjs || id == 0) {
			NVUNLOCKTEXTURESYNCMUTEX
			nlAddTab(-1);
			nlPrint(LOG_FDEBUGFORMAT5, F_NVLOADTEXTURE, N_FALSE, N_ID, id);
			return false;
		}

		if(nv_texobjs[id-1].status != NV_TEX_STATUS_EMPTY || nv_texobjs[id-1].ltype == NV_TEX_LTYPE_LOADVOID) {
			NVUNLOCKTEXTURESYNCMUTEX
			nlAddTab(-1);
			nlPrint(LOG_FDEBUGFORMAT5, F_NVLOADTEXTURE, N_FALSE, N_ID, id);
			return false;
		}

		nv_texobjs[id-1].status = NV_TEX_STATUS_LOADING;
		texobj = nv_texobjs[id-1];
	NVUNLOCKTEXTURESYNCMUTEX

	if(texobj.ltype == NV_TEX_LTYPE_LOADFNAME)
		success = nvLoadTextureFromFile(&texobj);
	else if(texobj.ltype == NV_TEX_LTYPE_LOADMEMORY)
		success = true;

	if(success) {
		texobj.nglid = funcptr_nglLoadTexture(texobj.flags, texobj.tex.sizex, texobj.tex.sizey, texobj.tex.nglcolorformat, texobj.tex.nglrowalignment, texobj.tex.buffer);

		success = texobj.nglid?true:false;

		if(texobj.ltype != NV_TEX_LTYPE_LOADMEMORY)
			nFreeMemory(texobj.tex.buffer);
	}

	if(success) texobj.status = NV_TEX_STATUS_LOADED; else
		texobj.status = NV_TEX_STATUS_EMPTY;

	NVLOCKTEXTURESYNCMUTEX
		nv_texobjs[id-1] = texobj;
	NVUNLOCKTEXTURESYNCMUTEX

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVLOADTEXTURE, N_OK, N_SUCCESS, (int)success);

	return success;
}

/*
	Функция	: nvUnloadTexture

	Описание: Выгружает текстуру

	История	: 15.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nvUnloadTexture(unsigned int id)
{
	nlPrint(LOG_FDEBUGFORMAT4,F_NVUNLOADTEXTURE,N_ID,id); nlAddTab(1);

	NVLOCKTEXTURESYNCMUTEX
		if(!nv_isinit || id > nv_maxtexobjs || id == 0) {
			NVUNLOCKTEXTURESYNCMUTEX
			nlAddTab(-1);
			nlPrint(LOG_FDEBUGFORMAT5, F_NVUNLOADTEXTURE, N_FALSE, N_ID, id);
			return false;
		}

		if(nv_texobjs[id-1].status != NV_TEX_STATUS_LOADED) {
			NVUNLOCKTEXTURESYNCMUTEX
			nlAddTab(-1);
			nlPrint(LOG_FDEBUGFORMAT5, F_NVUNLOADTEXTURE, N_FALSE, N_ID, id);
			return false;
		}

		nv_texobjs[id-1].status = NV_TEX_STATUS_UNLOADING;

		funcptr_nglFreeTexture(nv_texobjs[id-1].nglid);

		nv_texobjs[id-1].status = NV_TEX_STATUS_EMPTY;
		nv_texobjs[id-1].nglid = 0;
	NVUNLOCKTEXTURESYNCMUTEX

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_NVUNLOADTEXTURE,N_OK);

	return true;
}


/*
	Функция	: nvDestroyTexture

	Описание: Уничтожает текстуру

	История	: 15.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nvDestroyTexture(unsigned int id)
{
	nlPrint(LOG_FDEBUGFORMAT4, F_NVDESTROYTEXTURE, N_ID, id); nlAddTab(1);

	NVLOCKTEXTURESYNCMUTEX
		if(!nv_isinit || id > nv_maxtexobjs || id == 0) {
			NVUNLOCKTEXTURESYNCMUTEX
			nlAddTab(-1);
			nlPrint(LOG_FDEBUGFORMAT5, F_NVDESTROYTEXTURE, N_FALSE, N_ID, id);
			return false;
		}

		if(nv_texobjs[id-1].status == NV_TEX_STATUS_FREE) {
			NVUNLOCKTEXTURESYNCMUTEX
			nlAddTab(-1);
			nlPrint(LOG_FDEBUGFORMAT5, F_NVDESTROYTEXTURE, N_FALSE, N_ID, id);
			return false;
		}

		// Ждём, если удаляемая текстура в данный момент грузится
		while(nv_texobjs[id-1].status == NV_TEX_STATUS_LOADING) {
			NVUNLOCKTEXTURESYNCMUTEX
			nSleep(10);
			NVLOCKTEXTURESYNCMUTEX
		}

		switch(nv_texobjs[id-1].ltype) {
			case NV_TEX_LTYPE_LOADFNAME:
				nFreeMemory(nv_texobjs[id-1].fname);
				break;
			case NV_TEX_LTYPE_LOADFNAMEWITHALPHAFROMSUBFNAME:
				nFreeMemory(nv_texobjs[id-1].fname);
				nFreeMemory(nv_texobjs[id-1].subfname);
				break;
		}
		if(nv_texobjs[id-1].status == NV_TEX_STATUS_LOADED) nvUnloadTexture(id);
		nv_texobjs[id-1].status = NV_TEX_STATUS_FREE;
	NVUNLOCKTEXTURESYNCMUTEX

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVDESTROYTEXTURE, N_OK);

	return true;
}

/*
	Функция	: nvDestroyAllTextures

	Описание: Уничтожает все текстуры

	История	: 15.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvDestroyAllTextures(void)
{
	if(!nv_isinit) return;

	NVLOCKTEXTURESYNCMUTEX
		if(nv_alloctexobjs > 0) {
			unsigned int i;

			for(i=0;i<nv_maxtexobjs;i++)
				if(nv_texobjs[i].status != NV_TEX_STATUS_FREE)
					nvDestroyTexture(i+1);

			nFreeMemory(nv_texobjs);
			nv_texobjs = 0;
			nv_alloctexobjs = 0;
			nv_maxtexobjs = 0;
		}
	NVUNLOCKTEXTURESYNCMUTEX
}

/*
	Функция	: nvGetTextureStatus

	Описание: Возвращает статус текстуры.

	История	: 17.08.17	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvGetTextureStatus(unsigned int id)
{
	unsigned int status;

	NVLOCKTEXTURESYNCMUTEX
		if(!nv_isinit || id > nv_maxtexobjs || id == 0) {
			NVUNLOCKTEXTURESYNCMUTEX
			return NV_TEX_STATUS_FREE;
		}

		status = nv_texobjs[id-1].status;
	NVUNLOCKTEXTURESYNCMUTEX

	return status;
}

/*
	Функция	: nvGetTextureLType

	Описание: Возвращает способ загрузки текстуры.

	История	: 17.08.17	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvGetTextureLType(unsigned int id)
{
	unsigned int ltype;

	NVLOCKTEXTURESYNCMUTEX
		if(!nv_isinit || id > nv_maxtexobjs || id == 0) {
			NVUNLOCKTEXTURESYNCMUTEX
			return NV_TEX_LTYPE_LOADVOID;
		}

		if(nv_texobjs[id-1].status == NV_TEX_STATUS_FREE) {
			NVUNLOCKTEXTURESYNCMUTEX
			return NV_TEX_LTYPE_LOADVOID;
		}

		ltype = nv_texobjs[id-1].ltype;
	NVUNLOCKTEXTURESYNCMUTEX

	return ltype;
}

/*
	Функция	: nvGetTextureWH

	Описание: Помещает ширину и высоту текстуры в wid, hei

	История	: 19.08.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nvGetTextureWH(unsigned int id, unsigned int *wid, unsigned int *hei)
{
	NVLOCKTEXTURESYNCMUTEX
		if(!nv_isinit || id > nv_maxtexobjs || id == 0) {
			NVUNLOCKTEXTURESYNCMUTEX
			*wid = 0;
			*hei = 0;
			return false;
		}

		if(nv_texobjs[id-1].status != NV_TEX_STATUS_LOADED) {
			NVUNLOCKTEXTURESYNCMUTEX
			*wid = 0;
			*hei = 0;
			return false;
		}

		*wid = nv_texobjs[id-1].tex.sizex;
		*hei = nv_texobjs[id-1].tex.sizey;
	NVUNLOCKTEXTURESYNCMUTEX

	return true;
}

/*
	Функция	: nvGetTextureFormat

	Описание: Помещает формат пикселей и выравнивание строки текстуры в colorformat, rowalignment

	История	: 11.08.14	Создан

*/
N_API bool N_APIENTRY_EXPORT nvGetTextureFormat(unsigned int id, unsigned int *colorformat, unsigned int *rowalignment)
{
	NVLOCKTEXTURESYNCMUTEX
		if(!nv_isinit || id > nv_maxtexobjs || id == 0) {
			NVUNLOCKTEXTURESYNCMUTEX
			*colorformat = 0;
			*rowalignment = 0;
			return false;
		}

		if(nv_texobjs[id-1].status != NV_TEX_STATUS_LOADED) {
			NVUNLOCKTEXTURESYNCMUTEX
			*colorformat = 0;
			*rowalignment = 0;
			return false;
		}

		*colorformat = nv_texobjs[id-1].tex.nglcolorformat;
		*rowalignment = nv_texobjs[id-1].tex.nglrowalignment;
	NVUNLOCKTEXTURESYNCMUTEX

	return true;
}

//

/*
	Функция	: nvGetTextureNGLIdAndWH

	Описание: По id текстуры движка возвращает id текстуры рендера, ширину и высоту текстуры

	История	: 16.06.18	Создан

*/
bool nvGetTextureNGLIdAndWH(unsigned int id, unsigned int *nglid, unsigned int *wid, unsigned int *hei)
{
	NVLOCKTEXTURESYNCMUTEX
		if(!nv_isinit || id > nv_maxtexobjs || id == 0) {
			NVUNLOCKTEXTURESYNCMUTEX
			*nglid = 0;
			*wid = 0;
			*hei = 0;
			return false;
		}

		if(nv_texobjs[id-1].status != NV_TEX_STATUS_LOADED) {
			NVUNLOCKTEXTURESYNCMUTEX
			*nglid = 0;
			*wid = 0;
			*hei = 0;
			return false;
		}

		*nglid = nv_texobjs[id - 1].nglid;
		*wid = nv_texobjs[id - 1].tex.sizex;
		*hei = nv_texobjs[id - 1].tex.sizey;
	NVUNLOCKTEXTURESYNCMUTEX

	return true;
}

/*
	Функция	: nvGetNGLTextureId

	Описание: По id текстуры движка возвращает id текстуры рендера

	История	: 09.03.17	Создан

*/
unsigned int nvGetNGLTextureId(unsigned int id)
{
	unsigned int nglid;

	NVLOCKTEXTURESYNCMUTEX
		if(!nv_isinit || id > nv_maxtexobjs || id == 0) {
			NVUNLOCKTEXTURESYNCMUTEX
			return 0;
		}

		if(nv_texobjs[id-1].status != NV_TEX_STATUS_LOADED) {
			NVUNLOCKTEXTURESYNCMUTEX
			return 0;
		}

		nglid = nv_texobjs[id - 1].nglid;
	NVUNLOCKTEXTURESYNCMUTEX

	return nglid;
}

/*
	Функция	: nvAddTexturePlugin

	Описание: Добавляет плагин для загрузки изображений из файла

	История	: 13.07.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nvAddTexturePlugin(const wchar_t *name, nv_tex_plugin_type *nv_texplg)
{
	unsigned int new_plugin;
	
	if(nv_texplg->size != sizeof(nv_tex_plugin_type)) {
		nlPrint(LOG_FDEBUGFORMAT, F_NVADDTEXTUREPLUGIN, N_FALSE);
		return false;
	}

	NVLOCKTEXTURESYNCMUTEX
		if(!nArrayAdd(
			&n_ea, (void **)(&nv_tex_plugins),
			&nv_tex_maxplugins,
			&nv_tex_allocplugins,
			nCheckArrayAlwaysFalse,
			&new_plugin,
			NYAN_ARRAY_DEFAULT_STEP,
			sizeof(nv_tex_plugin_type))
		) {
			NVUNLOCKTEXTURESYNCMUTEX
			nlPrint(LOG_FDEBUGFORMAT, F_NVADDTEXTUREPLUGIN, N_FALSE);
			return false;
		}

		nv_tex_plugins[new_plugin] = *nv_texplg;
	NVUNLOCKTEXTURESYNCMUTEX

	nlPrint(LOG_FDEBUGFORMAT7, F_NVADDTEXTUREPLUGIN, NV_ADDTEXTUREPLUGIN, name);

	return true;
}

/*
	Функция	: nvDeleteAllTexturePlugins

	Описание: Удаляет все плагины для загрузки изображений из файла

	История	: 14.07.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvDeleteAllTexturePlugins(void)
{
	NVLOCKTEXTURESYNCMUTEX
		if(nv_tex_allocplugins) nFreeMemory(nv_tex_plugins);

		nv_tex_plugins = 0;
		nv_tex_maxplugins = 0;
		nv_tex_allocplugins = 0;
	NVUNLOCKTEXTURESYNCMUTEX

	nlPrint(LOG_FDEBUGFORMAT, F_NVDELETEALLTEXTUREPLUGINS, N_OK);
}
