/*
	Файл	: nyan_vis_3dmodel.с

	Описание: Вывод 3d моделей

	История	: 29.01.13	Создан

*/

//#include <Windows.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "nyan_filesys_publicapi.h"
#include "nyan_fps_publicapi.h"
#include "nyan_log_publicapi.h"
#include "nyan_mem_publicapi.h"
#include "nyan_vis_3dmodel_publicapi.h"
#include "nyan_vis_draw_publicapi.h"
#include "nyan_vis_texture_publicapi.h"
#include "nyan_text.h"

#include "nyan_vis_init.h"
#include "nyan_vis_draw.h"
#include "nyan_vis_texture.h"
#include "nyan_vis_3dmodel.h"

#include "../nyan_container/nyan_container_ne_helpers.h"

#include "nyan_nglapi.h"

#define NEK2_READERVERSION 0

typedef struct {
	char animname[20];
	unsigned int startframe;
	unsigned int endframe;
	unsigned int fps;
} nv_3dmodel_anim_type;

// Значения nv_3dmodel_header_type.trisformat
#define NV_3DMODEL_TRISFORMAT_PLAIN 0 // Каждый треугольник имеет три вершины: vertices[i*3], vertices[i*3+1] и vertices[i*3+2], где i - номер треугольника. Всего noftriangles*3 вершин.
#define NV_3DMODEL_TRISFORMAT_INDEXED 1 // Каждый треугольник имеет три индекса, указывающие номера используемых вершин. Индексы хранятся в массиве vertexindexes[i*3], vertexindexes[i*3+1] и vertexindexes[i*3+2], где i - номер треугольника.

// Значения nv_3dmodel_header_type.animformat
#define NV_3DMODEL_ANIMFORMAT_VERTEX 0 // Вершинная анимация

typedef struct {
	unsigned int nofframes; // Количество кадров
	unsigned int noftriangles; // Количество треугольников в кадре
	unsigned int nofvertices; // Количество вершин
	unsigned int nofskins; // Количество скинов
	unsigned int nofanims; // Количество анимаций
	unsigned int trisformat; // Формат в котором хранятся треугольники
	unsigned int animformat; // Формат анимации
} nv_3dmodel_header_type;

typedef struct {
	nv_3dmodel_header_type header;
	unsigned int status;
	unsigned int refcount; // Количество ссылок на модель
	unsigned int *vertexindexes; // Индексы вершин (для trisformat == NV_3DMODEL_TRISFORMAT_INDEXED)
	nv_3dvertex_type *vertices; // Данные вершин для статической модели (без анимации)
	nv_3ddeltavertex_type *deltavertices; // Разница между статической моделью и кадром анимации. Кадры идут последовательно (0, 1, ..., frames-1)
	nv_3dvertex_type *vertices_cash; // (для анимации, предварительно вычисленные вершины)
	nv_3dmodel_anim_type *anims; // Данные об анимациях
	unsigned int *skintexids; // ngl id текстур
	wchar_t **skintexnames; // Имена текстур
	char *modname; // Имя модели
} nv_3dmodel_type;

#define NV_3DMODEL_STATUS_FREE 0
#define NV_3DMODEL_STATUS_USED 1
#define NV_3DMODEL_STATUS_LOADED 2

static nv_3dmodel_type *nv_3dmodels;
static unsigned int nv_max3dmodels = 0;

/*
	Функция	: nvCreate3dModel

	Описание: Создаёт 3d модель. Возвращает 0 - при неудаче, иначе id модели.

	История	: 29.01.13	Создан
*/
N_API unsigned int N_APIENTRY_EXPORT nvCreate3dModel(const wchar_t *filename, const char *modname)
{
	unsigned int i, j, ret;
	unsigned int f = 0;
	nyan_filetype_type filetype; // Тип файла
	nyan_chunkhead_type chunkhead; // Заголовок чанка
	size_t vNtrdata_size, skindata_size, animdata_size, vafrdata_size;

	if(!nv_isinit) return 0;

	if(nv_draw_state != NV_DRAW_STATE_NO) return 0;

	nlPrint(LOG_FDEBUGFORMAT7, F_NVCREATE3DMODEL, N_FNAME, filename); nlAddTab(1);

	// Поиск свободного id для 3д модели или выделение памяти под новый id
	for(i = 0;i < nv_max3dmodels;i++)
		if(nv_3dmodels[i].status == NV_3DMODEL_STATUS_FREE)
			break;

	if(i == nv_max3dmodels) {
		nv_3dmodel_type *_nv_3dmodels;
		_nv_3dmodels = nReallocMemory(nv_3dmodels, (nv_max3dmodels+1024)*sizeof(nv_3dmodel_type));
		if(_nv_3dmodels)
			nv_3dmodels = _nv_3dmodels;
		else {
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATE3DMODEL, N_FALSE, N_ID, 0);
			return 0;
		}
		for(i=nv_max3dmodels;i<nv_max3dmodels+1024;i++)
			nv_3dmodels[i].status = NV_3DMODEL_STATUS_FREE;
		i = nv_max3dmodels;
		nv_max3dmodels += 1024;
	}

	// Копирование имени 3d модели
	nv_3dmodels[i].modname = nAllocMemory(strlen(modname) + 1);
	if (!nv_3dmodels[i].modname) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATE3DMODEL, N_FALSE, N_ID, 0);
		return 0;
	}
	memcpy(nv_3dmodels[i].modname, modname, strlen(modname) + 1);

	// Чтение файла
	f = nFileOpen(filename);
	if(f == 0) {
		nFreeMemory(nv_3dmodels[i].modname);
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATE3DMODEL, ERR_FILENOTFOUNDED);
		return 0;
	}

	nv_3dmodels[i].vertexindexes = 0;
	nv_3dmodels[i].vertices = 0;
	nv_3dmodels[i].deltavertices = 0;
	nv_3dmodels[i].vertices_cash = 0;
	nv_3dmodels[i].anims = 0;
	nv_3dmodels[i].skintexids = 0;
	nv_3dmodels[i].skintexnames = 0;

	ret = ncfReadAndCheckFiletypeChunk(f, &filetype, "NEK2", NEK2_READERVERSION);
	switch(ret) {
		case NCF_ERROR_WRONGFILETYPE:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATE3DMODEL, ERR_WRONGFILETYPE);
			goto FREE_ON_FAIL;
		case NCF_ERROR_CANTFINDFILETYPE:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATE3DMODEL, ERR_CANTFINDFILETYPE);
			goto FREE_ON_FAIL;
		case NCF_SUCCESS:
			break;
	}

	// Поиск и чтение чанка FILEHEAD
	ret = ncfSeekForChunkAndCheckMinSize(f, &chunkhead, "FILEHEAD", 4);
	switch(ret) {
		case NCF_SUCCESS:
			if(nFileRead(f, &nv_3dmodels[i].header, sizeof(nv_3dmodel_header_type)) != sizeof(nv_3dmodel_header_type)) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATE3DMODEL, ERR_WRONGFILEHEAD);
				goto FREE_ON_FAIL;
			}
			nFileSeek(f, chunkhead.csize-sizeof(nv_3dmodel_header_type), FILE_SEEK_CUR);

			break;
		case NCF_ERROR_DAMAGEDFILE:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATE3DMODEL, ERR_WRONGFILEHEAD);
			goto FREE_ON_FAIL;
		case NCF_ERROR_CANTFINDCHUNK:
		default:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATE3DMODEL, ERR_CANTFINDFILEHEAD);
			goto FREE_ON_FAIL;
	}

	// Поиск и чтение чанка V&TRDATA
	vNtrdata_size = nv_3dmodels[i].header.nofvertices*sizeof(nv_3dvertex_type);
	//nlPrint(L"vNtrdata_size %u nv_3dmodels[i].header.nofvertices %u", vNtrdata_size, nv_3dmodels[i].header.nofvertices);
	if(nv_3dmodels[i].header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED)
		vNtrdata_size += nv_3dmodels[i].header.noftriangles*3*sizeof(unsigned int);
	ret = ncfSeekForChunkAndCheckMinSize(f, &chunkhead, "V&TRDATA", vNtrdata_size);
	switch(ret) {
		case NCF_SUCCESS:
			// Чтение вершин
			nv_3dmodels[i].vertices = nAllocMemory(nv_3dmodels[i].header.nofvertices*sizeof(nv_3dvertex_type));
			nv_3dmodels[i].vertices_cash = nAllocMemory(nv_3dmodels[i].header.nofvertices*sizeof(nv_3dvertex_type));
			if(!nv_3dmodels[i].vertices || !nv_3dmodels[i].vertices_cash) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATE3DMODEL, N_FALSE, N_ID, 0);
				goto FREE_ON_FAIL;
			}
			if(nFileRead(f, nv_3dmodels[i].vertices, nv_3dmodels[i].header.nofvertices*sizeof(nv_3dvertex_type)) != (long long)(nv_3dmodels[i].header.nofvertices*sizeof(nv_3dvertex_type))) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT2, F_NVCREATE3DMODEL, ERR_WRONGCHUNK, L"V&TRDATA");
				goto FREE_ON_FAIL;
			}

			// Чтение треугольников
			if(nv_3dmodels[i].header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED) {
				nv_3dmodels[i].vertexindexes = nAllocMemory(nv_3dmodels[i].header.noftriangles*3*sizeof(unsigned int));
				if(!nv_3dmodels[i].vertexindexes) {
					nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATE3DMODEL, N_FALSE, N_ID, 0);
					goto FREE_ON_FAIL;
				}
				if(nFileRead(f, nv_3dmodels[i].vertexindexes, nv_3dmodels[i].header.noftriangles*3*sizeof(unsigned int)) != (long long)(nv_3dmodels[i].header.noftriangles*3*sizeof(unsigned int))) {
					nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT2, F_NVCREATE3DMODEL, ERR_WRONGCHUNK, L"V&TRDATA");
					goto FREE_ON_FAIL;
				}
			}

			nFileSeek(f, chunkhead.csize-vNtrdata_size, FILE_SEEK_CUR);

			break;
		case NCF_ERROR_DAMAGEDFILE:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATE3DMODEL, ERR_FILEISDAMAGED);
			goto FREE_ON_FAIL;
		case NCF_ERROR_CANTFINDCHUNK:
		default:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT2, F_NVCREATE3DMODEL, ERR_CANTFINDCHUNK, L"V&TRDATA");
			goto FREE_ON_FAIL;
	}

	if(nv_3dmodels[i].header.nofskins) {
		// Поиск и чтение чанка SKINDATA
		skindata_size = nv_3dmodels[i].header.nofskins*sizeof(unsigned int);
		ret = ncfSeekForChunkAndCheckMinSize(f, &chunkhead, "SKINDATA", skindata_size);
		switch(ret) {
			case NCF_SUCCESS:
				// Чтение вершин
				nv_3dmodels[i].skintexids = nAllocMemory(nv_3dmodels[i].header.nofskins*sizeof(unsigned int));
				nv_3dmodels[i].skintexnames = nAllocMemory(nv_3dmodels[i].header.nofskins*sizeof(wchar_t *));
				if(!nv_3dmodels[i].skintexids || !nv_3dmodels[i].skintexnames) {
					nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATE3DMODEL, N_FALSE, N_ID, 0);
					goto FREE_ON_FAIL;
				}
				memset(nv_3dmodels[i].skintexids, 0, nv_3dmodels[i].header.nofskins*sizeof(unsigned int));
				for(j = 0; j < nv_3dmodels[i].header.nofskins; j++) {
					unsigned int name_len;

					if(nFileRead(f, &name_len, sizeof(unsigned int)) != sizeof(unsigned int)) {
						nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT2, F_NVCREATE3DMODEL, ERR_WRONGCHUNK, L"SKINDATA");
						goto FREE_ON_FAIL;
					}
					skindata_size += name_len*2;
					nv_3dmodels[i].skintexnames[j] = nAllocMemory((name_len+1)*sizeof(wchar_t));
					if(!nv_3dmodels[i].skintexnames[j]) {
						nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATE3DMODEL, N_FALSE, N_ID, 0);
						goto FREE_ON_FAIL;
					}
					if(ncfReadUTF16String(f, nv_3dmodels[i].skintexnames[j], name_len, name_len) != NCF_SUCCESS) {
						nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT2, F_NVCREATE3DMODEL, ERR_WRONGCHUNK, L"SKINDATA");
						goto FREE_ON_FAIL;
					}
					nv_3dmodels[i].skintexnames[j][name_len] = 0;
					//nlPrint(L"nv_3dmodels[%u].skintexnames[%u] %ls", i, j, nv_3dmodels[i].skintexnames[j]);
				}

				nFileSeek(f, chunkhead.csize-skindata_size, FILE_SEEK_CUR);

				break;
			case NCF_ERROR_DAMAGEDFILE:
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATE3DMODEL, ERR_FILEISDAMAGED);
				goto FREE_ON_FAIL;
			case NCF_ERROR_CANTFINDCHUNK:
			default:
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT2, F_NVCREATE3DMODEL, ERR_CANTFINDCHUNK, L"SKINDATA");
				goto FREE_ON_FAIL;
		}
	}

	if(nv_3dmodels[i].header.nofanims) {
		// Поиск и чтение чанка ANIMDATA
		animdata_size = nv_3dmodels[i].header.nofanims*sizeof(nv_3dmodel_anim_type);
		ret = ncfSeekForChunkAndCheckMinSize(f, &chunkhead, "ANIMDATA", animdata_size);
		switch(ret) {
			case NCF_SUCCESS:
				// Чтение вершин
				nv_3dmodels[i].anims = nAllocMemory(animdata_size);
				if(!nv_3dmodels[i].anims) {
					nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATE3DMODEL, N_FALSE, N_ID, 0);
					goto FREE_ON_FAIL;
				}
				if(nFileRead(f, nv_3dmodels[i].anims, animdata_size) != (long long)(animdata_size)) {
					nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT2, F_NVCREATE3DMODEL, ERR_WRONGCHUNK, L"ANIMDATA");
					goto FREE_ON_FAIL;
				}

				nFileSeek(f, chunkhead.csize-animdata_size, FILE_SEEK_CUR);

				break;
			case NCF_ERROR_DAMAGEDFILE:
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATE3DMODEL, ERR_FILEISDAMAGED);
				goto FREE_ON_FAIL;
			case NCF_ERROR_CANTFINDCHUNK:
			default:
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT2, F_NVCREATE3DMODEL, ERR_CANTFINDCHUNK, L"ANIMDATA");
				goto FREE_ON_FAIL;
		}
	}

	if(nv_3dmodels[i].header.nofframes) {
		// Поиск и чтение чанка VAFRDATA
		vafrdata_size = nv_3dmodels[i].header.nofframes*nv_3dmodels[i].header.nofvertices*sizeof(nv_3ddeltavertex_type);
		ret = ncfSeekForChunkAndCheckMinSize(f, &chunkhead, "VAFRDATA", vafrdata_size);
		switch(ret) {
			case NCF_SUCCESS:
				// Чтение вершин
				nv_3dmodels[i].deltavertices = nAllocMemory(vafrdata_size);
				if(!nv_3dmodels[i].deltavertices) {
					nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATE3DMODEL, N_FALSE, N_ID, 0);
					goto FREE_ON_FAIL;
				}
				if(nFileRead(f, nv_3dmodels[i].deltavertices, vafrdata_size) != (long long)(vafrdata_size)) {
					nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT2, F_NVCREATE3DMODEL, ERR_WRONGCHUNK, L"VAFRDATA");
					goto FREE_ON_FAIL;
				}

				nFileSeek(f, chunkhead.csize-vafrdata_size, FILE_SEEK_CUR);

				break;
			case NCF_ERROR_DAMAGEDFILE:
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATE3DMODEL, ERR_FILEISDAMAGED);
				goto FREE_ON_FAIL;
			case NCF_ERROR_CANTFINDCHUNK:
			default:
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT2, F_NVCREATE3DMODEL, ERR_CANTFINDCHUNK, L"VAFRDATA");
				goto FREE_ON_FAIL;
		}
	}

	nFileClose(f);

	nv_3dmodels[i].status = NV_3DMODEL_STATUS_USED;
	nv_3dmodels[i].refcount = 0;

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATE3DMODEL, N_OK, N_ID, i+1);

	return (i+1);

FREE_ON_FAIL:

	nFileClose(f);

	if(nv_3dmodels[i].vertexindexes)
		nFreeMemory(nv_3dmodels[i].vertexindexes);
	if(nv_3dmodels[i].vertices)
		nFreeMemory(nv_3dmodels[i].vertices);
	if(nv_3dmodels[i].deltavertices)
		nFreeMemory(nv_3dmodels[i].deltavertices);
	if(nv_3dmodels[i].vertices_cash)
		nFreeMemory(nv_3dmodels[i].vertices_cash);
	if(nv_3dmodels[i].anims)
		nFreeMemory(nv_3dmodels[i].anims);
	if(nv_3dmodels[i].skintexids)
		nFreeMemory(nv_3dmodels[i].skintexids);
	if(nv_3dmodels[i].skintexnames) {
		for(j = 0; j < nv_3dmodels[i].header.nofskins; j++)
			if(nv_3dmodels[i].skintexnames[j])
				nFreeMemory(nv_3dmodels[i].skintexnames[j]);

		nFreeMemory(nv_3dmodels[i].skintexnames);
	}

	nFreeMemory(nv_3dmodels[i].modname);

	return 0;
}

/*
	Функция	: nvLoad3dModel

	Описание: Загружает 3d модель.

	История	: 29.01.13	Создан

*/
N_API bool N_APIENTRY_EXPORT nvLoad3dModel(unsigned int id)
{
	unsigned int i;

	if(!nv_isinit) return false;

	if(nv_draw_state != NV_DRAW_STATE_NO) return false;

	if(id == 0 || id > nv_max3dmodels) return false;

	if(nv_3dmodels[id-1].status != NV_3DMODEL_STATUS_USED) return false;

	nlPrint(LOG_FDEBUGFORMAT4, F_NVLOAD3DMODEL, N_ID, id); nlAddTab(1);

	nv_3dmodels[id-1].status = NV_3DMODEL_STATUS_LOADED;

	for(i = 0; i < nv_3dmodels[id-1].header.nofskins; i++) {
		nv_3dmodels[id-1].skintexids[i] = nvCreateTextureFromFile(nv_3dmodels[id-1].skintexnames[i], NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_MIPMAP|NGL_TEX_FLAGS_LINEARMIPMAP|NGL_TEX_FLAGS_ANISOTROPY);
		if(nv_3dmodels[id-1].skintexids[i]) nvLoadTexture(nv_3dmodels[id-1].skintexids[i]);
	}

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVLOAD3DMODEL, N_OK, N_SUCCESS, 1);

	return true;
}

/*
	Функция	: nvUnload3dModel

	Описание: Выгружает 3d модель.

	История	: 29.01.13	Создан

*/
N_API bool N_APIENTRY_EXPORT nvUnload3dModel(unsigned int id)
{
	unsigned int i;

	if(!nv_isinit) return false;

	if(nv_draw_state != NV_DRAW_STATE_NO) return false;

	if(id == 0 || id > nv_max3dmodels) return false;

	if(nv_3dmodels[id-1].status != NV_3DMODEL_STATUS_LOADED) return false;

	if(nv_3dmodels[id-1].refcount) return false;

	nlPrint(LOG_FDEBUGFORMAT4, F_NVUNLOAD3DMODEL, N_ID, id); nlAddTab(1);

	nv_3dmodels[id-1].status = NV_3DMODEL_STATUS_USED;

	for(i = 0; i < nv_3dmodels[id-1].header.nofskins; i++)
		nvDestroyTexture(nv_3dmodels[id-1].skintexids[i]);

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVUNLOAD3DMODEL, N_OK, N_SUCCESS, 1);

	return true;
}

/*
	Функция	: nvDestroy3dModel

	Описание: Выгружает 3d модель.

	История	: 29.01.13	Создан

*/
N_API bool N_APIENTRY_EXPORT nvDestroy3dModel(unsigned int id)
{
	unsigned int i;

	if(!nv_isinit) return false;

	if(nv_draw_state != NV_DRAW_STATE_NO) return false;

	if(id == 0 || id > nv_max3dmodels) return false;

	if(nv_3dmodels[id-1].status == NV_3DMODEL_STATUS_FREE) return false;

	if(nv_3dmodels[id-1].refcount) return false;

	nlPrint(LOG_FDEBUGFORMAT4, F_NVDESTROY3DMODEL, N_ID, id); nlAddTab(1);

	nvUnload3dModel(id);

	nv_3dmodels[id-1].status = NV_3DMODEL_STATUS_FREE;

	nFreeMemory(nv_3dmodels[id-1].modname);

	nFreeMemory(nv_3dmodels[id-1].vertices);
	nFreeMemory(nv_3dmodels[id-1].vertices_cash);

	if(nv_3dmodels[id-1].header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED)
		nFreeMemory(nv_3dmodels[id-1].vertexindexes);

	if(nv_3dmodels[id-1].header.nofskins) {
		for(i = 0; i < nv_3dmodels[id-1].header.nofskins; i++)
			nFreeMemory(nv_3dmodels[id-1].skintexnames[i]);

		nFreeMemory(nv_3dmodels[id-1].skintexnames);
		nFreeMemory(nv_3dmodels[id-1].skintexids);
	}

	if(nv_3dmodels[id-1].header.nofframes)
		nFreeMemory(nv_3dmodels[id-1].deltavertices);

	if(nv_3dmodels[id-1].header.nofanims)
		nFreeMemory(nv_3dmodels[id-1].anims);

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVDESTROY3DMODEL, N_OK, N_SUCCESS, 1);

	return true;
}

/*
	Функция	: nvDestroyAll3dModels

	Описание: Выгружает 3d модель.

	История	: 29.01.13	Создан

*/
N_API void N_APIENTRY_EXPORT nvDestroyAll3dModels(void)
{
	unsigned int id;
	if(!nv_isinit) return;

	if(nv_draw_state != NV_DRAW_STATE_NO) return;

	if(nv_max3dmodels) {
		for(id = 1; id <= nv_max3dmodels; id++)
			if(nv_3dmodels[id-1].status != NV_3DMODEL_STATUS_FREE)
				nvDestroy3dModel(id);

		nv_max3dmodels = 0;
		nFreeMemory(nv_3dmodels);
		nv_3dmodels = 0;
	}
}

/*
	Функция	: nvDraw3dStaticModel

	Описание: Рисует 3d модель.

	История	: 29.08.13	Создан

*/
N_API void N_APIENTRY_EXPORT nvDrawStatic3dModel(unsigned int id, unsigned int skinid)
{
	unsigned int nglid;

	if(!nv_isinit) return;

	if(nv_draw_state != NV_DRAW_STATE_3D) return;

	if(id == 0 || id > nv_max3dmodels) return;

	if(nv_3dmodels[id-1].status != NV_3DMODEL_STATUS_LOADED) return;

	if(skinid >= nv_3dmodels[id-1].header.nofskins) skinid = 0;

	if(nv_3dmodels[id-1].header.nofskins == 0)
		nglid = 0;
	else
		nglid = nvGetNGLTextureId(nv_3dmodels[id-1].skintexids[skinid]);

	switch(nv_3dmodels[id-1].header.trisformat) {
		case NV_3DMODEL_TRISFORMAT_PLAIN:
			funcptr_nglBatch3dDrawMesh(nglid, nv_3dmodels[id-1].header.nofvertices, nv_3dmodels[id-1].vertices);
			break;
		case NV_3DMODEL_TRISFORMAT_INDEXED:
			funcptr_nglBatch3dDrawIndexedMesh(nglid, nv_3dmodels[id-1].header.nofvertices, nv_3dmodels[id-1].vertices, nv_3dmodels[id-1].header.noftriangles, nv_3dmodels[id-1].vertexindexes);
			break;
	}
}

/*
	Функция	: nvDraw3dModel

	Описание: Рисует 3d модель.
		skinid должен быть от 0 до nvGet3dModelNumberOfSkins(id)-1
		frame_start и frame_end должны быть от 0 до nvGet3dModelNumberOfFrames(id)-1, иначе вызывается nvDraw3dStaticModel
		frame_mid должен быть от 0 до 1 (хотя это и не проверяется, но...)

	История	: 29.01.13	Создан

*/
N_API void N_APIENTRY_EXPORT nvDraw3dModel(unsigned int id, unsigned int skinid, unsigned int frame_start, unsigned int frame_end, float frame_mid)
{
	unsigned int nglid;
	//unsigned int counter; // test

	if(!nv_isinit) return;

	if(nv_draw_state != NV_DRAW_STATE_3D) return;

	if(id == 0 || id > nv_max3dmodels) return;

	if(nv_3dmodels[id-1].status != NV_3DMODEL_STATUS_LOADED) return;

	if(frame_start >= nv_3dmodels[id-1].header.nofframes || frame_end >= nv_3dmodels[id-1].header.nofframes) { nvDrawStatic3dModel(id, skinid); return; }

	if(skinid >= nv_3dmodels[id-1].header.nofskins) skinid = 0;

	if(nv_3dmodels[id-1].header.nofskins == 0)
		nglid = 0;
	else if(nv_3dmodels[id-1].skintexids[skinid] > nv_maxtexobjs || nv_3dmodels[id-1].skintexids[skinid] == 0)
		nglid = 0;
	else if(nv_texobjs[nv_3dmodels[id-1].skintexids[skinid]-1].status != NV_TEX_STATUS_LOADED)
		nglid = 0;
	else
		nglid = nv_texobjs[nv_3dmodels[id-1].skintexids[skinid]-1].nglid;

	if(!frame_mid || frame_start == frame_end) {
		unsigned int i;
		nv_3dvertex_type *p;
		nv_3ddeltavertex_type *p2;

		memcpy(nv_3dmodels[id-1].vertices_cash, nv_3dmodels[id-1].vertices, nv_3dmodels[id-1].header.nofvertices*sizeof(nv_3dvertex_type));

		p = nv_3dmodels[id-1].vertices_cash;
		p2 = nv_3dmodels[id-1].deltavertices+frame_start*nv_3dmodels[id-1].header.nofvertices;

		for(i = 0; i < nv_3dmodels[id-1].header.nofvertices; i++) {
			(*p).x += (*p2).x;
			(*p).y += (*p2).y;
			(*p).z += (*p2).z;
			(*p).nx += (*p2).nx;
			(*p).ny += (*p2).ny;
			(*p).nz += (*p2).nz;
			p++; p2++;
		}
	} else {
		unsigned int i;
		nv_3dvertex_type *p;
		nv_3ddeltavertex_type *p2, *p3;

	//for(counter = 0; counter < 5000; counter++) { // test

		memcpy(nv_3dmodels[id-1].vertices_cash, nv_3dmodels[id-1].vertices, nv_3dmodels[id-1].header.nofvertices*sizeof(nv_3dvertex_type));

		p = nv_3dmodels[id-1].vertices_cash;
		p2 = nv_3dmodels[id-1].deltavertices+frame_start*nv_3dmodels[id-1].header.nofvertices;
		p3 = nv_3dmodels[id-1].deltavertices+frame_end*nv_3dmodels[id-1].header.nofvertices;
		for(i = 0; i < nv_3dmodels[id-1].header.nofvertices; i++) {
			//startframe+(endframe-startframe)*frame_mid эквивалентно startframe*(1-frame_mid)+endframe*frame_mid
			(*p).x += (*p2).x*(1-frame_mid)+(*p3).x*frame_mid;
			(*p).y += (*p2).y*(1-frame_mid)+(*p3).y*frame_mid;
			(*p).z += (*p2).z*(1-frame_mid)+(*p3).z*frame_mid;
			(*p).nx += (*p2).nx*(1-frame_mid)+(*p3).nx*frame_mid;
			(*p).ny += (*p2).ny*(1-frame_mid)+(*p3).ny*frame_mid;
			(*p).nz += (*p2).nz*(1-frame_mid)+(*p3).nz*frame_mid;
			// Раскомментировать, если будет слишком сильно тормозить ^_-
			/*(*p).x += (*p2).x;
			(*p).y += (*p2).y;
			(*p).z += (*p2).z;
			(*p).nx += (*p2).nx;
			(*p).ny += (*p2).ny;
			(*p).nz += (*p2).nz;*/
			p++; p2++; p3++;
		}

	//} // test

	}

	switch(nv_3dmodels[id-1].header.trisformat) {
		case NV_3DMODEL_TRISFORMAT_PLAIN:
			funcptr_nglBatch3dDrawMesh(nglid, nv_3dmodels[id-1].header.nofvertices, nv_3dmodels[id-1].vertices_cash);
			break;
		case NV_3DMODEL_TRISFORMAT_INDEXED:
			funcptr_nglBatch3dDrawIndexedMesh(nglid, nv_3dmodels[id-1].header.nofvertices, nv_3dmodels[id-1].vertices_cash, nv_3dmodels[id-1].header.noftriangles, nv_3dmodels[id-1].vertexindexes);
			break;
	}
}

/*
	Функция	: nvDrawAnimated3dModel

	Описание: Рисует 3d модель.
		skinid должен быть от 0 до nvGet3dModelNumberOfSkins(id)-1
		animid должен быть от 1 до nvGet3dModelNumberOfAnims(id)
		если animid == 0 или animid > nvGet3dModelNumberOfAnims(id), то вызывается nvDraw3dStaticModel
		в frame должен находиться текущий кадр (от - до + бесконечности, преобразуется в диапазон [0, количество_кадров_в_анимации) ), в начале анимации frame должен быть проинициализирован нулём
		если nextframe не равен нулю, то туда записывается следующий кадр, который должен быть передан в качестве frame при следующем запуске функции

	История	: 29.01.13	Создан

*/
N_API void N_APIENTRY_EXPORT nvDrawAnimated3dModel(unsigned int id, unsigned int skinid, unsigned int animid, float frame, float *nextframe)
{
	unsigned int frame_start, frame_end;
	float frame_mid;

	if(!nv_isinit) return;

	if(nv_draw_state != NV_DRAW_STATE_3D) return;

	if(id == 0 || id > nv_max3dmodels) return;

	if(nv_3dmodels[id-1].status != NV_3DMODEL_STATUS_LOADED) return;

	if(animid == 0 || animid > nv_3dmodels[id-1].header.nofanims) { nvDrawStatic3dModel(id, skinid); return; }

	animid--;

	if(nv_3dmodels[id-1].anims[animid].startframe == nv_3dmodels[id-1].anims[animid].endframe) {
		frame_start = nv_3dmodels[id-1].anims[animid].startframe;
		frame_end = nv_3dmodels[id-1].anims[animid].startframe;
		frame_mid = 0;
	} else {
		unsigned int uint_frame;

		if(frame < 0)
			frame = (float)(nv_3dmodels[id-1].anims[animid].endframe-nv_3dmodels[id-1].anims[animid].startframe+1)+(float)fmod(frame,nv_3dmodels[id-1].anims[animid].endframe-nv_3dmodels[id-1].anims[animid].startframe+1);
		
		uint_frame = (unsigned int)frame;

		if( (nv_3dmodels[id-1].anims[animid].startframe+uint_frame) > nv_3dmodels[id-1].anims[animid].endframe ) {
			frame = (float)fmod(frame,nv_3dmodels[id-1].anims[animid].endframe-nv_3dmodels[id-1].anims[animid].startframe+1);
			uint_frame = (unsigned int)frame;
		}

		//printf("frame %f %u <= %u <= %u\n", (double)frame, nv_3dmodels[id-1].anims[animid].startframe, nv_3dmodels[id-1].anims[animid].startframe+uint_frame, nv_3dmodels[id-1].anims[animid].endframe);

		frame_start = nv_3dmodels[id-1].anims[animid].startframe+uint_frame;
		if(frame_start == nv_3dmodels[id-1].anims[animid].endframe)
			frame_end = nv_3dmodels[id-1].anims[animid].startframe;
		else
			frame_end = frame_start+1;
		frame_mid = frame-(float)(uint_frame);

		frame = frame+(float)(nGetspf()*nv_3dmodels[id-1].anims[animid].fps);

		if(frame > (float)(nv_3dmodels[id-1].anims[animid].endframe-nv_3dmodels[id-1].anims[animid].startframe+1))
			frame = frame-(float)(nv_3dmodels[id-1].anims[animid].endframe-nv_3dmodels[id-1].anims[animid].startframe+1);

		if(nextframe)
			*nextframe = frame;
	}

	//nlPrint(L"a %d s %d e %d m %f f %f fs %d fe %d fps %d",animid,frame_start,frame_end,frame_mid,*frame,nv_3dmodels[id-1].anims[animid].startframe,nv_3dmodels[id-1].anims[animid].endframe,nv_3dmodels[id-1].anims[animid].fps);

	nvDraw3dModel(id, skinid, frame_start, frame_end, frame_mid);

}

/*
	Функция	: nvDrawAnimated3dModel2

	Описание: Рисует 3d модель.
		skinid должен быть от 0 до nvGet3dModelNumberOfSkins(id)-1
		animid должен быть от 1 до nvGet3dModelNumberOfAnims(id)
		если animid == 0 или animid > nvGet3dModelNumberOfAnims(id), то вызывается nvDraw3dStaticModel
		anim_start содержит время начала анимации

	История	: 25.12.17	Создан

*/
N_API void N_APIENTRY_EXPORT nvDrawAnimated3dModel2(unsigned int id, unsigned int skinid, unsigned int animid, int64_t anim_start)
{
	unsigned int frame_start, frame_end;
	float frame_mid;

	if(!nv_isinit) return;

	if(nv_draw_state != NV_DRAW_STATE_3D) return;

	if(id == 0 || id > nv_max3dmodels) return;

	if(nv_3dmodels[id-1].status != NV_3DMODEL_STATUS_LOADED) return;

	if(animid == 0 || animid > nv_3dmodels[id-1].header.nofanims) { nvDrawStatic3dModel(id, skinid); return; }

	animid--;

	if(nv_3dmodels[id-1].anims[animid].startframe == nv_3dmodels[id-1].anims[animid].endframe) {
		frame_start = nv_3dmodels[id-1].anims[animid].startframe;
		frame_end = nv_3dmodels[id-1].anims[animid].startframe;
		frame_mid = 0;
	} else {
		float frame; // Текущий кадр
		unsigned int uint_frame;

		frame = (float)((double)((nFrameStartClock()-anim_start)*nv_3dmodels[id-1].anims[animid].fps)/(double)(N_CLOCKS_PER_SEC));

		if(frame < 0)
			frame = (float)(nv_3dmodels[id-1].anims[animid].endframe-nv_3dmodels[id-1].anims[animid].startframe+1)+(float)fmod(frame,nv_3dmodels[id-1].anims[animid].endframe-nv_3dmodels[id-1].anims[animid].startframe+1);

		uint_frame = (unsigned int)frame;

		if( (nv_3dmodels[id-1].anims[animid].startframe+uint_frame) > nv_3dmodels[id-1].anims[animid].endframe ) {
			frame = (float)fmod(frame,nv_3dmodels[id-1].anims[animid].endframe-nv_3dmodels[id-1].anims[animid].startframe+1);
			uint_frame = (unsigned int)frame;
		}

		//printf("frame %f %u <= %u <= %u\n", (double)frame, nv_3dmodels[id-1].anims[animid].startframe, nv_3dmodels[id-1].anims[animid].startframe+uint_frame, nv_3dmodels[id-1].anims[animid].endframe);

		frame_start = nv_3dmodels[id-1].anims[animid].startframe+uint_frame;
		if(frame_start == nv_3dmodels[id-1].anims[animid].endframe)
			frame_end = nv_3dmodels[id-1].anims[animid].startframe;
		else
			frame_end = frame_start+1;
		frame_mid = frame-(float)(uint_frame);
	}

	nvDraw3dModel(id, skinid, frame_start, frame_end, frame_mid);
}

/*
	Функция	: nvGet3dModelNumberOfFrames

	Описание: Возвращает количество кадров в модели

	История	: 29.01.13	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvGet3dModelNumberOfFrames(unsigned int id)
{
	if(!nv_isinit) return 0;

	if(id == 0 || id > nv_max3dmodels) return 0;

	if(nv_3dmodels[id-1].status == NV_3DMODEL_STATUS_FREE) return 0;

	return nv_3dmodels[id-1].header.nofframes;
}

/*
	Функция	: nvGet3dModelNumberOfSkins

	Описание: Возвращает количество скинов в модели

	История	: 01.02.13	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvGet3dModelNumberOfSkins(unsigned int id)
{
	if(!nv_isinit) return 0;

	if(id == 0 || id > nv_max3dmodels) return 0;

	if(nv_3dmodels[id-1].status == NV_3DMODEL_STATUS_FREE) return 0;

	return nv_3dmodels[id-1].header.nofskins;
}

/*
	Функция	: nvGet3dModelNumberOfAnims

	Описание: Возвращает количество скинов в модели

	История	: 01.02.13	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvGet3dModelNumberOfAnims(unsigned int id)
{
	if(!nv_isinit) return 0;

	if(id == 0 || id > nv_max3dmodels) return 0;

	if(nv_3dmodels[id-1].status == NV_3DMODEL_STATUS_FREE) return 0;

	return nv_3dmodels[id-1].header.nofanims;
}

/*
	Функция	: nvGet3dModelRefCount

	Описание: Возвращает счётчик ссылок на модель

	История	: 25.04.13	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvGet3dModelRefCount(unsigned int id)
{
	if(!nv_isinit) return 0;

	if(id == 0 || id > nv_max3dmodels) return 0;

	if(nv_3dmodels[id-1].status == NV_3DMODEL_STATUS_FREE) return 0;

	return nv_3dmodels[id-1].refcount;
}

/*
	Функция	: nvGet3dModelAnimIdByName

	Описание: Возвращает id анимации по её имени

	История	: 01.02.13	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvGet3dModelAnimIdByName(unsigned int id, const char *animname)
{
	unsigned int i, animid = 0;

	if(!nv_isinit) return 0;

	if(id == 0 || id > nv_max3dmodels) return 0;

	if(nv_3dmodels[id-1].status == NV_3DMODEL_STATUS_FREE) return 0;

	for(i = 0; i < nv_3dmodels[id-1].header.nofanims; i++)
		if(!strncmp(nv_3dmodels[id-1].anims[i].animname, animname, 20)) {
			animid = i+1;
			break;
		}

	return animid;
}

/*
	Функция	: nvIncrease3dModelRefCount

	Описание: Увеличивает счётчик ссылок на модель

	История	: 25.04.13	Создан

*/
N_API void N_APIENTRY_EXPORT nvIncrease3dModelRefCount(unsigned int id)
{
	if(!nv_isinit) return;

	if(id == 0 || id > nv_max3dmodels) return;

	if(nv_3dmodels[id-1].status == NV_3DMODEL_STATUS_FREE) return;

	nv_3dmodels[id-1].refcount++;
}

/*
	Функция	: nvDecrease3dModelRefCount

	Описание: Уменьшает счётчик ссылок на модель

	История	: 25.04.13	Создан

*/
N_API void N_APIENTRY_EXPORT nvDecrease3dModelRefCount(unsigned int id)
{
	if(!nv_isinit) return;

	if(id == 0 || id > nv_max3dmodels) return;

	if(nv_3dmodels[id-1].status == NV_3DMODEL_STATUS_FREE) return;

	nv_3dmodels[id-1].refcount--;
}

/*
	Функция	: nvGet3dModelIdByName

	Описание: Возвращает id модели по её имени

	История	: 01.02.13	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvGet3dModelIdByName(const char *name)
{
	unsigned int i, modelid = 0;

	if(!nv_isinit) return 0;

	for(i = 0; i < nv_max3dmodels; i++)
		if(!strcmp(nv_3dmodels[i].modname, name)) {
			modelid = i+1;
			break;
		}

	return modelid;
}

/*
	Функция	: nvGet3dModelName

	Описание: Возвращает имя модели

	История	: 20.06.17	Создан

*/
N_API const char * N_APIENTRY_EXPORT nvGet3dModelName(unsigned int id)
{
	if(!nv_isinit) return 0;

	if(id == 0 || id > nv_max3dmodels) return 0;

	if(nv_3dmodels[id-1].status == NV_3DMODEL_STATUS_FREE) return 0;

	return nv_3dmodels[id-1].modname;
}
