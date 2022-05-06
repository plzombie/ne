/*
	Файл	: nyan_vis_spritesheets.c

	Описание: Sprite Sheets

	История	: 07.11.16	Создан

*/

#include "nyan_text.h"

#include "nyan_vis_draw.h"
#include "nyan_vis_texture.h"
#include "nyan_vis_init.h"

#include "nyan_vismodes_publicapi.h"

#include "nyan_nglapi.h"

#include "nyan_log_publicapi.h"
#include "nyan_mem_publicapi.h"
#include "nyan_vis_init_publicapi.h"
#include "nyan_vis_spritesheets_publicapi.h"
#include "nyan_vis_texture_publicapi.h"

#include "../nyan_container/nyan_format_nek3spritesheets.h"

#include <string.h>

typedef struct {
	unsigned int nofsprites;
	unsigned int spritetype;
	void *sprites;
	wchar_t **spritenames;
	nv_2dvertex_type *vs; // Временный буфер для формирования вершин треугольников, из которых состоят спрайты
	bool isused;
} nv_spritesheet_type;

static nv_spritesheet_type *nv_spritesheets;
static unsigned int nv_maxspritesheets = 0;

/*
	Функция	: nvCreateEmptySpriteSheet

	Описание: Создаёт пустой sprite sheet

	История	: 07.11.16	Создан

*/
static unsigned int nvCreateEmptySpriteSheet(unsigned int spritetype)
{
	unsigned int i;

	// Выделение памяти под структуры
	for(i = 0;i < nv_maxspritesheets;i++)
		if(nv_spritesheets[i].isused == false)
			break;

	if(i == nv_maxspritesheets) {
		nv_spritesheet_type *_nv_spritesheets;
		_nv_spritesheets = nReallocMemory(nv_spritesheets, (nv_maxspritesheets+1024)*sizeof(nv_spritesheet_type));
		if(_nv_spritesheets)
			nv_spritesheets = _nv_spritesheets;
		else {
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATEEMPTYSPRITESHEET, N_FALSE, N_ID, 0);
			return false;
		}
		for(i=nv_maxspritesheets;i<nv_maxspritesheets+1024;i++)
			nv_spritesheets[i].isused = false;
		i = nv_maxspritesheets;
		nv_maxspritesheets += 1024;
	}


	nv_spritesheets[i].nofsprites = 0;
	nv_spritesheets[i].spritetype = spritetype;
	nv_spritesheets[i].sprites = 0;
	nv_spritesheets[i].spritenames = 0;

	if(spritetype == NEK3_SPRITETYPE_RECTANGLE) {
		nv_spritesheets[i].vs = nAllocMemory(6*sizeof(nv_2dvertex_type));

		if(!(nv_spritesheets[i].vs)) return 0;
	} else if(spritetype == NEK3_SPRITETYPE_POLYGONAL) {
		nv_spritesheets[i].vs = nAllocMemory(3*sizeof(nv_2dvertex_type));

		if(!(nv_spritesheets[i].vs)) return 0;
	} else
		return 0;

	nv_spritesheets[i].isused = true;

	return i+1;
}

/*
	Функция	: nvCreateSpriteSheetFromTileset

	Описание: Создаёт sprite sheet из набора тайлов фиксированного размера.
		width, height - размер текстуры, в которой хранится набор тайлов
		cols, rows - количество строк и столбцов тайлов

	История	: 07.11.16	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvCreateSpriteSheetFromTileset(unsigned int width, unsigned int height, unsigned int cols, unsigned int rows, const wchar_t *basicname)
{
	unsigned int id, i;

	if(width == 0 || height == 0 || cols == 0 || rows == 0) return 0;

	nlPrint(F_NVCREATESPRITESHEETFROMTILESET); nlAddTab(1);

	id = nvCreateEmptySpriteSheet(NEK3_SPRITETYPE_RECTANGLE);
	if(!id) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATESPRITESHEETFROMTILESET, N_FALSE);
		return 0;
	}

	nv_spritesheets[id-1].nofsprites = cols * rows;

	nv_spritesheets[id-1].sprites = nAllocMemory(nv_spritesheets[id-1].nofsprites*sizeof(nek3_rectangle_sprite_type));
	if(!nv_spritesheets[id-1].sprites) goto ERROR;

	nv_spritesheets[id-1].spritenames = nAllocMemory(nv_spritesheets[id-1].nofsprites*sizeof(wchar_t *));
	if(!nv_spritesheets[id-1].spritenames) goto ERROR;

	memset(nv_spritesheets[id-1].spritenames, 0, nv_spritesheets[id-1].nofsprites*sizeof(wchar_t *));

	if(basicname) {
		size_t str_size;

		str_size = wcslen(basicname)+64;

		for(i = 0; i < nv_spritesheets[id-1].nofsprites; i++) {
			nv_spritesheets[id-1].spritenames[i] = nAllocMemory((str_size+1)*sizeof(wchar_t));
			if(!nv_spritesheets[id-1].spritenames[i]) goto ERROR;

			swprintf(nv_spritesheets[id-1].spritenames[i], str_size, L"%ls%u", basicname, i);
			nv_spritesheets[id-1].spritenames[i][str_size] = 0;
		}
	} else {
		for(i = 0; i < nv_spritesheets[id-1].nofsprites; i++)
			nv_spritesheets[id-1].spritenames[i] = 0;
	}

	for(i = 0; i < nv_spritesheets[id-1].nofsprites; i++) {
		nek3_rectangle_sprite_type *sprite;

		sprite = (nek3_rectangle_sprite_type *)(nv_spritesheets[id-1].sprites)+i;

		sprite->twid = sprite->wid = width/cols;
		sprite->thei = sprite->hei = height/rows;
		sprite->offx = 0;
		sprite->offy = 0;
		sprite->tx[0] = (float)(i%cols)/(float)cols;
		sprite->ty[0] = (float)(rows-i/cols)/(float)rows;
		sprite->tx[1] = (float)(i%cols+1)/(float)cols;
		sprite->ty[1] = (float)(rows-i/cols)/(float)rows;
		sprite->tx[2] = (float)(i%cols+1)/(float)cols;
		sprite->ty[2] = (float)(rows-i/cols-1)/(float)rows;
		sprite->tx[3] = (float)(i%cols)/(float)cols;
		sprite->ty[3] = (float)(rows-i/cols-1)/(float)rows;
	}

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATESPRITESHEETFROMTILESET, N_OK, N_ID, id);

	return id;

ERROR:
	nv_spritesheets[id-1].nofsprites = 0;
	if(nv_spritesheets[id-1].sprites) {
		nFreeMemory(nv_spritesheets[id-1].sprites);
		nv_spritesheets[id-1].sprites = 0;
	}
	if(nv_spritesheets[id-1].spritenames) {
		unsigned int i;

		for(i = 0; i < nv_spritesheets[id-1].nofsprites; i++) {
			if(nv_spritesheets[id-1].spritenames[i]) {
				nFreeMemory(nv_spritesheets[id-1].spritenames[i]);
				nv_spritesheets[id-1].spritenames[i] = 0;
			}
		}

		nFreeMemory(nv_spritesheets[id-1].spritenames);
		nv_spritesheets[id-1].spritenames = 0;
	}

	if(nv_spritesheets[id-1].vs) {
		nFreeMemory(nv_spritesheets[id-1].vs);
	}

	id = 0;
	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATESPRITESHEETFROMTILESET, N_OK, N_ID, id);

	return id;
}

/*
	Функция	: nvCreateSpriteSheetFromFile

	Описание: Загружаеь sprite sheet из файла
		width, height - размер текстуры, в которой хранится набор тайлов
		cols, rows - количество строк и столбцов тайлов

	История	: 02.11.17	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvCreateSpriteSheetFromFile(const wchar_t *fname)
{
	unsigned int id;

	nlPrint(LOG_FDEBUGFORMAT7, F_NVCREATESPRITESHEETFROMTILESET, N_FNAME, fname); nlAddTab(1);

	id = 0;

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATESPRITESHEETFROMTILESET, N_OK, N_ID, id);

	return id;
}

/*
	Функция	: nvCreateSpriteSheetFromPreallocMemory

	Описание: Загружаеь sprite sheet из памяти.
		Память выделяется в вызывающей nvCreateSpriteSheetFromPreallocMemory функции, освобождается в nvDestroySpriteSheet.
		nofsprites - количество спрайтов
		spritetype - тип спрайтов
		sprites - указатель на массив спрайтов
		spritenames - указатель на массив имён спрайтов

	История	: 02.11.17	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvCreateSpriteSheetFromPreallocMemory(unsigned int nofsprites, unsigned int spritetype, void *sprites, wchar_t **spritenames)
{
	unsigned int id;

	nlPrint(F_NVCREATESPRITESHEETFROMTILESET); nlAddTab(1);

	id = nvCreateEmptySpriteSheet(spritetype);
	if(!id) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATESPRITESHEETFROMPREALLOCMEMORY, N_FALSE);
		return 0;
	}

	nv_spritesheets[id-1].nofsprites = nofsprites;
	nv_spritesheets[id-1].sprites = sprites;
	nv_spritesheets[id-1].spritenames = spritenames;

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATESPRITESHEETFROMPREALLOCMEMORY, N_OK, N_ID, id);

	return id;
}

/*
	Функция	: nvGetSpriteNameById

	Описание: Возвращает имя спрайта по его id
		Возвращаемый указатель на имя спрайта хранится в структуре sprite sheet

	История	: 07.11.16	Создан

*/
N_API const wchar_t * N_APIENTRY_EXPORT nvGetSpriteNameById(unsigned int ssheetid, unsigned int sid)
{
	if(ssheetid == 0 || ssheetid > nv_maxspritesheets) return 0;
	if(sid == 0 || sid > nv_spritesheets[ssheetid-1].nofsprites) return 0;

	return nv_spritesheets[ssheetid-1].spritenames[sid-1];
}

/*
	Функция	: nvGetSpriteIdByName

	Описание: Возвращает id спрайта по его имени

	История	: 07.11.16	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvGetSpriteIdByName(unsigned int ssheetid, const wchar_t *name)
{
	unsigned int i;

	if(ssheetid == 0 || ssheetid > nv_maxspritesheets) return 0;

	for(i = 0; i < nv_spritesheets[ssheetid-1].nofsprites; i++)
		if(!wcscmp(name, nv_spritesheets[ssheetid-1].spritenames[i]))
			return i+1;

	return 0;
}

/*
	Функция	: nvDrawRectangleSprite

	Описание: Рисует спрайт, заданный прямоугольником

	История	: 30.11.16	Создан

*/
static void nvDrawRectangleSprite(unsigned int nglid, int x, int y, float scalex, float scaley, unsigned int color, nek3_rectangle_sprite_type *sprite, nv_2dvertex_type *vs)
{
	int offx, offy, tsizex, tsizey;

	tsizex = (int)(sprite->twid*scalex);
	tsizey = (int)(sprite->thei*scaley);
	offx = (int)(sprite->offx*scalex);
	offy = (int)(sprite->offy*scaley);

	vs[0].x = (float)(x+offx);        vs[0].y = (float)(y+offy);        vs[0].z = 0; vs[0].colorRGBA = color;
	vs[1].x = (float)(x+offx+tsizex); vs[1].y = (float)(y+offy);        vs[1].z = 0; vs[1].colorRGBA = color;
	vs[2].x = (float)(x+offx+tsizex); vs[2].y = (float)(y+offy+tsizey); vs[2].z = 0; vs[2].colorRGBA = color;
	vs[0].tx = sprite->tx[0]; vs[0].ty = sprite->ty[0];
	vs[1].tx = sprite->tx[1]; vs[1].ty = sprite->ty[1];
	vs[2].tx = sprite->tx[2]; vs[2].ty = sprite->ty[2];
	vs[3] = vs[0];
	vs[4] = vs[2];
	vs[5].x = (float)(x+offx);        vs[5].y = (float)(y+offy+tsizey); vs[5].z = 0; vs[5].colorRGBA = color;
	vs[5].tx = sprite->tx[3]; vs[5].ty = sprite->ty[3];

	funcptr_nglBatch2dAdd(NV_DRAWTRIANGLE, nglid, 6, vs);
}

/*
	Функция	: nvDrawPolygonalSprite

	Описание: Рисует полигональный спрайт

	История	: 31.12.17	Создан

*/
static void nvDrawPolygonalSprite(unsigned int nglid, int x, int y, float scalex, float scaley, unsigned int color, nek3_polygonal_sprite_type *sprite, nv_2dvertex_type *vs)
{
	unsigned int i, nofvertices;
	nv_2dvertex_type *orig_vertices;

	nofvertices = sprite->head.nofvertices;
	orig_vertices = sprite->vertices;

	for(i = 0; i < nofvertices; i+= 3) {
		memcpy(vs, orig_vertices, 3*sizeof(nv_2dvertex_type));
		vs[0].x = vs[0].x*scalex+(float)x; vs[0].y = vs[0].y*scaley+(float)y; vs[0].colorRGBA = color;
		vs[1].x = vs[1].x*scalex+(float)x; vs[1].y = vs[1].y*scaley+(float)y; vs[1].colorRGBA = color;
		vs[2].x = vs[2].x*scalex+(float)x; vs[2].y = vs[2].y*scaley+(float)y; vs[2].colorRGBA = color;

		funcptr_nglBatch2dAdd(NV_DRAWTRIANGLE, nglid, 3, vs);

		orig_vertices += 3;
	}
}

/*
	Функция	: nvDrawSprite

	Описание: Рисует спрайт
			ssheetid - Sprite Sheet id
			sid - id спрайта
			x, y - Позиция спрайта
			scalex, scaley - Растяжение спрайта по х, у
			Возвращает true, если спрайт под курсором мыши

	История	: 07.11.16	Создан

*/
N_API bool N_APIENTRY_EXPORT nvDrawSprite(unsigned int ssheetid, unsigned int sid, int x, int y, unsigned int batch_currenttex, float scalex, float scaley, unsigned int color)
{
	unsigned int nglid;
	unsigned int orig_sizex, orig_sizey;
	int sizex, sizey, mx, my;

	if(nv_draw_state != NV_DRAW_STATE_2D || !nv_isinit) return false;
	if(ssheetid == 0 || ssheetid > nv_maxspritesheets) return false;
	if(sid == 0 || sid > nv_spritesheets[ssheetid-1].nofsprites) return false;

	nglid = nvGetNGLTextureId(batch_currenttex);

	switch(nv_spritesheets[ssheetid-1].spritetype) {
		case NEK3_SPRITETYPE_RECTANGLE:
			nvDrawRectangleSprite(nglid, x, y, scalex, scaley, color, (nek3_rectangle_sprite_type *)(nv_spritesheets[ssheetid-1].sprites)+(sid-1), nv_spritesheets[ssheetid-1].vs);
			break;
		case NEK3_SPRITETYPE_POLYGONAL:
			nvDrawPolygonalSprite(nglid, x, y, scalex, scaley, color, (nek3_polygonal_sprite_type *)(nv_spritesheets[ssheetid-1].sprites)+(sid-1), nv_spritesheets[ssheetid-1].vs);
			break;
		default:
			return false;
	}

	if(!nvGetSpriteWH(ssheetid, sid, &orig_sizex, &orig_sizey))
		return false;

	mx = nvGetStatusi(NV_STATUS_WINMX);
	my = nvGetStatusi(NV_STATUS_WINMY);
	sizex = (int)(orig_sizex*scalex);
	sizey = (int)(orig_sizey*scaley);

	if(mx>=x && my>=y && mx<(x+sizex) && my<(y+sizey)) {
		int cr_sx, cr_sy, cr_ex, cr_ey;

		cr_sx = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONSX);
		cr_sy = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONSY);
		cr_ex = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONEX);
		cr_ey = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONEY);

		if(mx >= cr_sx && my >= cr_sy && mx < cr_ex && my < cr_ey)
			return true;
		else
			return false;
	} else
		return false;
}

/*
	Функция	: nvGetSpriteSheetSize

	Описание: Возвращает количество спрайтов в Sprite Sheet

	История	: 07.11.16	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nvGetSpriteSheetSize(unsigned int ssheetid)
{
	if(ssheetid == 0 || ssheetid > nv_maxspritesheets) return 0;

	return nv_spritesheets[ssheetid-1].nofsprites;
}

/*
	Функция	: nvDestroySpriteSheet

	Описание: Уничтожает Sprite Sheet

	История	: 07.11.16	Создан

*/
N_API bool N_APIENTRY_EXPORT nvDestroySpriteSheet(unsigned int ssheetid)
{
	if(ssheetid == 0 || ssheetid > nv_maxspritesheets) return false;
	if(nv_spritesheets[ssheetid-1].isused == false) return false;

	nlPrint(LOG_FDEBUGFORMAT4, F_NVDESTROYSPRITESHEET, N_ID, ssheetid); nlAddTab(1);

	if(nv_spritesheets[ssheetid-1].nofsprites) {
		unsigned int i;

		for(i = 0; i < nv_spritesheets[ssheetid-1].nofsprites; i++) {
			if(nv_spritesheets[ssheetid-1].spritenames[i])
				nFreeMemory(nv_spritesheets[ssheetid-1].spritenames[i]);
		}

		nFreeMemory(nv_spritesheets[ssheetid-1].spritenames);
		nFreeMemory(nv_spritesheets[ssheetid-1].sprites);
	}

	if(nv_spritesheets[ssheetid-1].vs) {
		nFreeMemory(nv_spritesheets[ssheetid-1].vs);
	}

	nv_spritesheets[ssheetid-1].isused = false;

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVDESTROYSPRITESHEET, N_OK);

	return true;
}

/*
	Функция	: nvDestroyAllSpriteSheets

	Описание: Уничтожает все Sprite Sheets

	История	: 07.11.16	Создан

*/
N_API void N_APIENTRY_EXPORT nvDestroyAllSpriteSheets(void)
{
	if(nv_maxspritesheets) {
		unsigned int i;

		for(i = 0; i < nv_maxspritesheets; i++) {
			if(nv_spritesheets[i].isused)
				nvDestroySpriteSheet(i+1);
		}

		nFreeMemory(nv_spritesheets);
		nv_spritesheets = 0;
		nv_maxspritesheets = 0;
	}
}

/*
	Функция	: nvGetSpriteWH

	Описание: Помещает ширину и высоту спрайта в wid, hei

	История	: 07.11.16	Создан

*/
N_API bool N_APIENTRY_EXPORT nvGetSpriteWH(unsigned int ssheetid, unsigned int sid, unsigned int *wid, unsigned int *hei)
{
	if(ssheetid == 0 || ssheetid > nv_maxspritesheets) { *wid = 0; *hei = 0; return false; }
	if(nv_spritesheets[ssheetid-1].isused == false || sid == 0 || sid > nv_spritesheets[ssheetid-1].nofsprites) { *wid = 0; *hei = 0; return false; }

	switch(nv_spritesheets[ssheetid-1].spritetype) {
		case NEK3_SPRITETYPE_RECTANGLE:
			*wid = ((nek3_rectangle_sprite_type *)(nv_spritesheets[ssheetid-1].sprites))[sid-1].wid;
			*hei = ((nek3_rectangle_sprite_type *)(nv_spritesheets[ssheetid-1].sprites))[sid-1].hei;
			break;
		case NEK3_SPRITETYPE_POLYGONAL:
			*wid = ((nek3_polygonal_sprite_type *)(nv_spritesheets[ssheetid-1].sprites))[sid-1].head.wid;
			*hei = ((nek3_polygonal_sprite_type *)(nv_spritesheets[ssheetid-1].sprites))[sid-1].head.hei;
			break;
		default:
			return false;
	}

	return true;
}
