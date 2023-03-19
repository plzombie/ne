/*
	Файл	: nyan_vis_fonts.с

	Описание: Вывод текста, работа со шрифтами

	История	: 22.09.12	Создан

*/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "nyan_log_publicapi.h"
#include "nyan_filesys_publicapi.h"
#include "nyan_mem_publicapi.h"
#include "nyan_fps_publicapi.h"
#include "nyan_vis_init_publicapi.h"
#include "nyan_vis_fonts_publicapi.h"
#include "nyan_vis_texture_publicapi.h"
#include "nyan_texformat_publicapi.h"
#include "nyan_draw_publicapi.h"
#include "nyan_vismodes_publicapi.h"
#include "nyan_text.h"

#include "nyan_vis_draw.h"
#include "nyan_vis_texture.h"
#include "nyan_vis_init.h"
#include "nyan_vis_fonts.h"

#include "nyan_nglapi.h"

#include "nyan_apifordlls.h"
#include "../commonsrc/core/nyan_array.h"

#include "../nyan_container/nyan_container_ne_helpers.h"
#include "../nyan_container/nyan_format_nek1fonts.h"

#define NEK1_READERVERSION 0

typedef struct {
	unsigned int symbol2;
	float kerning;
} nv_kpair_ingame_type;

typedef struct {
	unsigned int noofblocks; // Количество блоков
	char *blockalloc; // Если blockalloc[i] == true, то память под текущий блок выделена, если нет - блок нужно пропустить
	nek1_glyph_type **glyphs; // Память под глифы выделяется блоками по 1024.
				// Чтобы получить доступ к символу n: s = glyphs[n/1024][n%1024];
	nek1_kpairshead_type kpairshead; // Заголовок, описывающий кернинговые пары
	nv_kpair_ingame_type **sortedkpairs; // Кернинговые пары, разбитые по 1-му символу; sortedkpairs[symbol1] - все пары, у которых 1-й элемент равен symbol1
	unsigned int *nofsortedkpairs; // Количества кернинговых пар для каждого символа
	bool used; // True - если шрифт используется
} nv_font_type;

static nv_font_type *nv_fonts;
static unsigned int nv_maxfonts = 0, nv_allocfonts = 0;

nv_2dvertex_type *nv_fontvertexbuf = 0;
unsigned int nv_fontvertexbufsize = 0;

/*
	Функция	: KPairsCompare

	Описание: Сравнивает две кернинговые пары по 2-му символу. Нужна для qsort и bsearch.

	История	: 22.09.12	Создан

*/
int KPairsCompare(const void* p1, const void* p2)
{
	return (int)(((nv_kpair_ingame_type *)p1)->symbol2)-(int)(((nv_kpair_ingame_type *)p2)->symbol2);
}

/*
	Функция	: nvCheckFontArray

	Описание: Проверяет свободное место для шрифта в массиве

	История	: 19.03.23	Создан

*/
static bool nvCheckFontArray(void *array_el, bool set_free)
{
	nv_font_type *el;
	
	el = (nv_font_type *)array_el;
	
	if(set_free) el->used = false;
	
	return (el->used)?true:false;
}

/*
	Функция	: nvCreateFont

	Описание: Создаёт шрифт

	История	: 22.09.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nvCreateFont(const wchar_t *fname)
{
	unsigned int i, j, f, ret;
	//char nek1[4];
	nyan_filetype_type filetype; // Тип файла
	nyan_chunkhead_type chunkhead; // Заголовок чанка
	unsigned int noofallocatedblocks; // Количество блоков, под которые выделена память (т.е. суммарное кол-во эл-тов nv_fonts[i].blockalloc, содержащих true)
	nek1_kpair_type *kpairs = 0; // Кернинговые пары

	if(!nv_isinit) return 0;

	nlPrint(LOG_FDEBUGFORMAT7, F_NVCREATEFONT, N_FNAME, fname); nlAddTab(1);

	// Выделение памяти под шрифты
	if(!nArrayAdd(
		&n_ea, (void **)(&nv_fonts),
		&nv_maxfonts,
		&nv_allocfonts,
		nvCheckFontArray,
		&i,
		NYAN_ARRAY_DEFAULT_STEP,
		sizeof(nv_font_type))
	) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATEFONT, N_FALSE, N_ID, 0);
		return false;
	}

	f = nFileOpen(fname);

	if(f == 0) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATEFONT, ERR_FILENOTFOUNDED);
		return false;
	}

	// Поиск и чтение чанка FILETYPE
	ret = ncfReadAndCheckFiletypeChunk(f, &filetype, "NEK1", NEK1_READERVERSION);
	switch(ret) {
		case NCF_SUCCESS:
			break;
		case NCF_ERROR_WRONGFILETYPE:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATEFONT, ERR_WRONGFILETYPE);
			nFileClose(f);
			return false;
		case NCF_ERROR_CANTFINDFILETYPE:
		default:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATEFONT, ERR_CANTFINDFILETYPE);
			nFileClose(f);
			return false;
		
	}

	// Поиск и чтение чанка FILEHEAD
	ret = ncfSeekForChunkAndCheckMinSize(f, &chunkhead, "FILEHEAD", 4);
	switch(ret) {
		case NCF_SUCCESS:
			nFileRead(f, &nv_fonts[i].noofblocks, 4);
			nFileSeek(f, chunkhead.csize-4, FILE_SEEK_CUR);

			break;
		case NCF_ERROR_DAMAGEDFILE:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATEFONT, ERR_WRONGFILEHEAD);
			nFileClose(f);
			return false;
		case NCF_ERROR_CANTFINDCHUNK:
		default:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATEFONT, ERR_CANTFINDFILEHEAD);
			nFileClose(f);
			return false;
	}

	// Поиск чанка FILEDATA
	if(nv_fonts[i].noofblocks) {
		ret = ncfSeekForChunkAndCheckMinSize(f, &chunkhead, "FILEDATA", nv_fonts[i].noofblocks);
		switch(ret) {
			case NCF_SUCCESS:
				break;
			case NCF_ERROR_DAMAGEDFILE:
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATEFONT, ERR_WRONGFILEDATA);
				nFileClose(f);
				return false;
			case NCF_ERROR_CANTFINDCHUNK:
			default:
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATEFONT, ERR_CANTFINDFILEDATA);
				nFileClose(f);
				return false;
		}

		// Чтение чанка FILEDATA
		nv_fonts[i].blockalloc = nAllocMemory(nv_fonts[i].noofblocks);
		nv_fonts[i].glyphs = nAllocMemory(sizeof(nek1_glyph_type *)*nv_fonts[i].noofblocks);
		if(!nv_fonts[i].blockalloc || !nv_fonts[i].glyphs) {
			if(nv_fonts[i].blockalloc) nFreeMemory(nv_fonts[i].blockalloc);
			if(nv_fonts[i].glyphs) nFreeMemory(nv_fonts[i].glyphs);
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATEFONT, N_FALSE, N_ID, 0);
			nFileClose(f);
			return false;
		}

		nFileRead(f, nv_fonts[i].blockalloc, nv_fonts[i].noofblocks);

		noofallocatedblocks = 0;
		for(j = 0;j < nv_fonts[i].noofblocks;j++)
			if(nv_fonts[i].blockalloc[j])
				noofallocatedblocks++;

		if(chunkhead.csize < nv_fonts[i].noofblocks+noofallocatedblocks*sizeof(nek1_glyph_type)*1024) {
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCREATEFONT, ERR_WRONGFILEDATA);
			nFreeMemory(nv_fonts[i].blockalloc);
			nFreeMemory(nv_fonts[i].glyphs);
			nFileClose(f);
			return false;
		}

		for(j = 0;j < nv_fonts[i].noofblocks;j++)
			if(nv_fonts[i].blockalloc[j]) {
				nv_fonts[i].glyphs[j] = nAllocMemory(sizeof(nek1_glyph_type)*1024);
				if(nv_fonts[i].glyphs[j])
					nFileRead(f, nv_fonts[i].glyphs[j], sizeof(nek1_glyph_type)*1024);
				else { // Не удалось выделить память под 1024 глифа
					unsigned int k;
					// Значит не удастся выделить память и под следующие 1024
					// Все следующие блоки помечаются как false
					for(k = j; k < nv_fonts[i].noofblocks;k++)
						nv_fonts[i].blockalloc[k] = false;
					// Конец чтения блоков
					break;
				}
			}
		// Пропускаем остаток чанка FILEDATA, если он есть
		nFileSeek(f, chunkhead.csize-(nv_fonts[i].noofblocks+noofallocatedblocks*sizeof(nek1_glyph_type)*1024), FILE_SEEK_CUR);
		nv_fonts[i].used = true;
	}

	// Поиск чанка KPRSHEAD (кернинговые пары, заголовок)
	nv_fonts[i].kpairshead.noofkpairs = 0;
	ret = ncfSeekForChunkAndCheckMinSize(f, &chunkhead, "KPRSHEAD", 4);
	switch(ret) {
		case NCF_SUCCESS:
			nFileRead(f, &nv_fonts[i].kpairshead, 4);
			nFileSeek(f, chunkhead.csize-4, FILE_SEEK_CUR);
			kpairs = nAllocMemory(sizeof(nek1_kpair_type)*nv_fonts[i].kpairshead.noofkpairs);
			if(!kpairs) {
				nv_fonts[i].kpairshead.noofkpairs = 0;
				break;
			}
			nv_fonts[i].sortedkpairs = nAllocMemory(sizeof(nv_kpair_ingame_type *)*65536);
			if(!nv_fonts[i].sortedkpairs) {
				nFreeMemory(kpairs);
				nv_fonts[i].kpairshead.noofkpairs = 0;
				break;
			}
			nv_fonts[i].nofsortedkpairs = nAllocMemory(sizeof(unsigned int)*65536);
			if(!nv_fonts[i].nofsortedkpairs) {
				nFreeMemory(nv_fonts[i].sortedkpairs);
				nFreeMemory(kpairs);
				nv_fonts[i].kpairshead.noofkpairs = 0;
				break;
			}
			for(j = 0; j < 65536; j++) {
				nv_fonts[i].sortedkpairs[j] = 0;
				nv_fonts[i].nofsortedkpairs[j] = 0;
			}

			break;
		case NCF_ERROR_DAMAGEDFILE:
			nlPrint(LOG_FDEBUGFORMAT, F_NVCREATEFONT, ERR_WRONGFILEHEAD);
		default:
			break;
	}

	// Поиск чанка KPRSDATA (кернинговые пары, данные)
	if(nv_fonts[i].kpairshead.noofkpairs) {
		ret = ncfSeekForChunkAndCheckMinSize(f, &chunkhead, "KPRSDATA", sizeof(nek1_kpair_type)*nv_fonts[i].kpairshead.noofkpairs);
		switch(ret) {
			case NCF_SUCCESS:
				nFileRead(f, kpairs, sizeof(nek1_kpair_type)*nv_fonts[i].kpairshead.noofkpairs);
				nFileSeek(f, chunkhead.csize-sizeof(nek1_kpair_type)*nv_fonts[i].kpairshead.noofkpairs, FILE_SEEK_CUR);

				// Подсчитываем количества кернинговых пар для каждого первого символа пары
				for(j = 0; j < nv_fonts[i].kpairshead.noofkpairs; j++) {
					if(kpairs[j].symbol1 < 65536)
						nv_fonts[i].nofsortedkpairs[kpairs[j].symbol1]++;
				}

				// Выделяем под них память
				for(j = 0; j < 65536; j++) {
					if(nv_fonts[i].nofsortedkpairs[j]) {
						nv_fonts[i].sortedkpairs[j] = nAllocMemory(nv_fonts[i].nofsortedkpairs[j]*sizeof(nv_kpair_ingame_type));

						if(!nv_fonts[i].sortedkpairs[j]) {
							unsigned int k;

							for(k = 0; k < j; k++) {
								if(nv_fonts[i].sortedkpairs[k])
									nFreeMemory(nv_fonts[i].sortedkpairs[j]);
							}

							nv_fonts[i].kpairshead.noofkpairs = 0;
							nFreeMemory(nv_fonts[i].sortedkpairs);
							nFreeMemory(nv_fonts[i].nofsortedkpairs);

							break;
						}

						// Сбрасываем счётчик пар. Будем заного увеличивать его в следующем цикле
						nv_fonts[i].nofsortedkpairs[j] = 0;
					} else
						nv_fonts[i].sortedkpairs[j] = 0;
				}

				if(nv_fonts[i].kpairshead.noofkpairs) {
					// Сортируем кернинговые пары по массивам по 1-му символу
					for(j = 0; j < nv_fonts[i].kpairshead.noofkpairs; j++) {
						if(kpairs[j].symbol1 < 65536) {
							nv_kpair_ingame_type *kpair;

							kpair = nv_fonts[i].sortedkpairs[kpairs[j].symbol1] + nv_fonts[i].nofsortedkpairs[kpairs[j].symbol1];
							nv_fonts[i].nofsortedkpairs[kpairs[j].symbol1]++;

							kpair->kerning = kpairs[j].kerning;
							kpair->symbol2 = kpairs[j].symbol2;
						}
					}

					// Сортируем кернинговые пары внутри массивов по 2-му символу
					for(j = 0; j < 65536; j++) {
						qsort(nv_fonts[i].sortedkpairs[j], nv_fonts[i].nofsortedkpairs[j], sizeof(nv_kpair_ingame_type), KPairsCompare);
					}
				}

				break;
			case NCF_ERROR_DAMAGEDFILE:
				nlPrint(LOG_FDEBUGFORMAT, F_NVCREATEFONT, ERR_WRONGFILEHEAD);
				// fallthrough
			default:
				nv_fonts[i].kpairshead.noofkpairs = 0;
				nFreeMemory(nv_fonts[i].sortedkpairs);
				nFreeMemory(nv_fonts[i].nofsortedkpairs);
				break;
		}
		nFreeMemory(kpairs);
	}

	i++;

	nFileClose(f);

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT5, F_NVCREATEFONT, N_OK, N_ID, i);

	return (i);
}

/*
	Функция	: nvDestroyFont

	Описание: Уничтожает шрифт

	История	: 22.09.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nvDestroyFont(unsigned int id)
{
	if(!nv_isinit || id > nv_maxfonts || id == 0) return false;

	if(!nv_fonts[id-1].used) return false;

	nlPrint(LOG_FDEBUGFORMAT4, F_NVDESTROYFONT, N_ID, id); nlAddTab(1);

	if(nv_fonts[id-1].noofblocks) {
		unsigned int i;

		for(i = 0;i < nv_fonts[id-1].noofblocks;i++)
			if(nv_fonts[id-1].blockalloc[i])
				nFreeMemory(nv_fonts[id-1].glyphs[i]);

		nFreeMemory(nv_fonts[id-1].blockalloc);
		nFreeMemory(nv_fonts[id-1].glyphs);
	}

	if(nv_fonts[id-1].kpairshead.noofkpairs) {
		unsigned int i;

		for(i = 0; i < 65536; i++) {
			if(nv_fonts[id-1].sortedkpairs[i])
				nFreeMemory(nv_fonts[id-1].sortedkpairs[i]);
		}

		nFreeMemory(nv_fonts[id-1].sortedkpairs);
		nFreeMemory(nv_fonts[id-1].nofsortedkpairs);
	}

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVDESTROYFONT, N_OK);

	nv_fonts[id-1].used = false;

	return true;
}

/*
	Функция	: nvDestroyAllFonts

	Описание: Уничтожает все шрифты

	История	: 22.09.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvDestroyAllFonts(void)
{
	if(!nv_isinit) return;

	if(nv_allocfonts) {
		unsigned int i;
		for(i = 1;i <= nv_maxfonts;i++)
			nvDestroyFont(i);
		nFreeMemory(nv_fonts);
		nv_fonts = 0;
		nv_allocfonts = 0;
		nv_maxfonts = 0;
	}
}

/*
	Функция	: nvDraw2dText

	Описание: Выводит текст

	История	: 22.09.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nvDraw2dText(const wchar_t *text, int posx, int posy, unsigned int fontid, unsigned int texid, float scalex, float scaley, unsigned int color)
{

	unsigned int nglid, textlen;
	float mx, my; // Позиция мыши
	float maxwid, maxhei; // maxwid - Максимальная ширина области текста+posx, maxhei - Максимальная высота области текста+posy
	float cr_sx, cr_sy, cr_ex, cr_ey; // Область отсечения/Clipping region

	cr_sx = (float)(nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONSX));
	cr_sy = (float)(nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONSY));
	cr_ex = (float)(nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONEX));
	cr_ey = (float)(nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONEY));

	if(!nv_isinit || fontid > nv_maxfonts || fontid == 0) return false;

	if(!nv_fonts[fontid-1].used) return false;

	nglid = nvGetNGLTextureId(texid);

	if(!nglid) return false;

	if((float)posx > cr_ex || (float)posy > cr_ey) return false;

	maxwid = (float)posx;
	maxhei = (float)posy;

	textlen = (unsigned int)wcslen(text);

	if(textlen) {
		unsigned int i, ptextlen = 0; // ptextlen - кол-во букв, выведенных на экран
		float curwid, maxghei; // curwid - Текущая ширина строки+posx, maxghei - Максмальная высота глифа в текущей строке
		bool havechar, ignoreline;
		nv_2dvertex_type *pvs;
		unsigned int sym, prevsym; // sym - Текущий символ, prevsym - Предыдущий символ
		const wchar_t *p;
		nv_font_type *font;
		nek1_glyph_type *glyph = 0;

		// Выделяется новый размер буфера (если необходимо)
		if(nv_fontvertexbufsize < textlen) {
			nv_2dvertex_type *_nv_fontvertexbuf;
			int _nv_fontvertexbufsize;

			_nv_fontvertexbufsize = (textlen/1024)*1024+1024;
			_nv_fontvertexbuf = nReallocMemory(nv_fontvertexbuf, sizeof(nv_2dvertex_type)*6*_nv_fontvertexbufsize);

			if(_nv_fontvertexbuf) {
				nv_fontvertexbuf = _nv_fontvertexbuf;
				nv_fontvertexbufsize = _nv_fontvertexbufsize;
			} else
				if(nv_fontvertexbufsize > 0)
					textlen = nv_fontvertexbufsize;
				else
					return false;
		}

		font = &nv_fonts[fontid-1];
		p = text;
		pvs = nv_fontvertexbuf;
		sym = *p;
		curwid = (float)posx;
		maxghei = 0;
		ignoreline = false;

		for(i = 0;i < textlen;i++) {
			if(maxhei >= cr_ey) break; // Изображение вылезло вниз за пределы области отсечения
			if(curwid >= cr_ex) ignoreline = true;
			prevsym = sym;
		#ifdef N_WCHAR32
			sym = *p;
		#else
			if((*p) < 0xD800 || (*p) > 0xDFFF)
				sym = *p;
			else if( ((*p) >= 0xDC00) || (i+2 > textlen)) {
				//nlPrint(L"wrong 2byte sym %d", (*p));
				p++;
				continue;
			} else {
				sym = ((int)(*p) & 0x3FF) << 10;
				i++; p++;
				if( ((*p) < 0xDC00) || ((*p) > 0xDFFF) ) {
					//nlPrint(L"wrong 2byte 2 sym %d", (*p));
					if((*p) < 0xD800 || (*p) > 0xDFFF)
						sym = *p;
					else {
						p++;
						continue;
					}
				} else
					sym = (sym | ((*p) & 0x3FF)) + 0x10000;
				//nlPrint(L"4byte sym %d", sym);
			}
		#endif
			p++;
			switch(sym) {
				// Управляющие символы
				case 0x000A: // LF - line feed — подача строки
				case 0x240A: // LINE FEED
					if(prevsym == 0x000D || prevsym == 0x240D) break;
					// fallthrough
				case 0x000D: // CR - carriage return — возврат каретки
				case 0x240D: // CARRIAGE RETURN
				case 0x0085: // NEL - next line — переход на следующую строку
				case 0x2028: // LS - line separator — разделитель строк
				case 0x2029: // PS - paragraph separator — разделитель абзацев
				case 0x2424: // NEWLINE
					if(maxghei) {
						maxhei += maxghei*scaley;
						maxghei = 0;
					} else if(font->blockalloc[0])
						maxhei += (font->glyphs[0]+32)->hei*scaley;
					if(curwid > maxwid) maxwid = curwid;
					ignoreline = false;
					curwid = (float)posx;
					break;
				// Вывод символа
				default:
					if(sym/1024 < font->noofblocks) {
						if(font->blockalloc[sym/1024]) {
							havechar = true;
							glyph = font->glyphs[sym/1024]+(sym%1024);
							if(glyph->hei > maxghei) maxghei = glyph->hei;
						} else
							havechar = false;
					} else
						havechar = false;

					if(havechar == false && font->noofblocks > 0) {
						if(font->blockalloc[0]) { // Пытаемся прочитать символ 0, если символ sym отсутствует в шрифте 
							havechar = true;
							glyph = font->glyphs[0];
							if(glyph->hei > maxghei) maxghei = glyph->hei;
						}
					}

					if(havechar && !ignoreline) {
						float kpoffx; // Кернинг

						kpoffx = 0;

						// Нахождение кернинговых пар
						if(font->kpairshead.noofkpairs) {
							if(prevsym < 65536) {
								nv_kpair_ingame_type *kpair = 0, kpairkey;

								kpairkey.symbol2 = sym;

								kpair = bsearch(&kpairkey, font->sortedkpairs[prevsym], font->nofsortedkpairs[prevsym], sizeof(nv_kpair_ingame_type), KPairsCompare);

								if(kpair)
									kpoffx = kpair->kerning*scalex;
							}
						}

						// Если символ находится в пределах области отсечения, то его можно вывести
						if(curwid < cr_ex && (curwid + glyph->wid*scalex) > cr_sx && (maxhei+glyph->hei*scaley) > cr_sy) {
							ptextlen++;
							pvs->x = curwid+kpoffx+glyph->offx*scalex;	             pvs->y = maxhei+glyph->offy*scaley;               pvs->z = 0; pvs->colorRGBA = color; pvs->tx = glyph->txstart; pvs->ty = glyph->tyend;   pvs++;
							pvs->x = curwid+kpoffx+(glyph->offx+glyph->twid)*scalex; pvs->y = maxhei+glyph->offy*scaley;               pvs->z = 0; pvs->colorRGBA = color; pvs->tx = glyph->txend;   pvs->ty = glyph->tyend;   pvs++;
							pvs->x = curwid+kpoffx+(glyph->offx+glyph->twid)*scalex; pvs->y = maxhei+(glyph->offy+glyph->thei)*scaley; pvs->z = 0; pvs->colorRGBA = color; pvs->tx = glyph->txend;   pvs->ty = glyph->tystart; pvs++;
							*pvs = *(pvs-3); pvs++;
							*pvs = *(pvs-2); pvs++;
							pvs->x = curwid+kpoffx+glyph->offx*scalex;               pvs->y = maxhei+(glyph->offy+glyph->thei)*scaley; pvs->z = 0; pvs->colorRGBA = color; pvs->tx = glyph->txstart; pvs->ty = glyph->tystart; pvs++;
						}
						curwid += kpoffx+glyph->wid*scalex;
					}
					break;
			}
		}
		//wprintf(L"%d %d, ",maxwid,maxhei);
		if(curwid > maxwid) maxwid = curwid;
		maxhei += maxghei*scaley;
		funcptr_nglBatch2dAdd(NV_DRAWTRIANGLE, nglid, ptextlen*6, nv_fontvertexbuf);
	}

	mx = (float)(nvGetStatusi(NV_STATUS_WINMX));
	my = (float)(nvGetStatusi(NV_STATUS_WINMY));

	//wprintf(L"%d %d %d, %d %d %d\n",posx,mx,maxwid,posy,my,maxhei);
	if(mx>=posx && my>=posy && mx<maxwid  && my<maxhei) {
		if(mx >= cr_sx && my >= cr_sy && mx < cr_ex && my < cr_ey)
			return true;
		else
			return false;
	} else
		return false;
}

/*
	Функция	: nvDraw2dTextbox

	Описание: Рисует textbox
				strlen(textbuf) = размер_текста+1
				status - состояние текстбокса. 1 - текстбокс выбран, 0 - нет

	История	: 06.11.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nvDraw2dTextbox(wchar_t *textbuf, int *status, unsigned int maxsize, wchar_t csym, unsigned int catime, int posx, int posy, unsigned int fontid, unsigned int texid, float scalex, float scaley, unsigned int color, unsigned int color_focused)
{
	bool focus;
	int mbl;
	unsigned int tblen = 0;
	const wchar_t *inpstr;

	if(maxsize < 2) return false;

	inpstr = nvGetStatusw(NV_STATUS_WINTEXTINPUTBUF);

	if(*status == 1) {
		unsigned int i, islen;
		int64_t catime_clocks;
		/*if( (maxsize - wcslen(textbuf) - 2) >= wcslen(inpstr) )
			wcscat(textbuf, inpstr);*/

		tblen = (unsigned int)wcslen(textbuf);
		islen = (unsigned int)wcslen(inpstr);
		for(i = 0; i < islen; i++, inpstr++)
			if(*inpstr == 8) {
				if(tblen > 0) {
					tblen--;
				#ifndef N_WCHAR32
					if(textbuf[tblen] >= 0xDC00 && textbuf[tblen] <= 0xDFFF && tblen > 0) { // Если нижняя суррогатная пара
						if(textbuf[tblen-1] >= 0xD800 && textbuf[tblen-1] < 0xDC00) { // Проверяем, не является ли предыдущий символ верхней суррогатной парой, и если да, удаляем его
							textbuf[tblen] = 0;
							tblen--;
						}
					}
				#endif
					textbuf[tblen] = 0;
				}
			} else if(tblen+2 < maxsize) { // +1 (добавляемый символ) +1 (символ каретки)
			#ifndef N_WCHAR32
				if(*inpstr >= 0xDC00 && *inpstr <= 0xDFFF) { // На вход подали нижнюю суррогатную (без верхней), отбрасываем её
					continue;
				} else if(*inpstr >= 0xD800 && *inpstr < 0xDC00) { // На вход подали верхнюю суррогатную пару
					if(tblen+3 < maxsize && i+1 < islen) { // Файл вмещает ещё и нижнюю суррогатную пару
						if(*(inpstr+1) >= 0xDC00 && *(inpstr+1) <= 0xDFFF) { // Если на входе следующий символ - нижняя, то читаем обе
							textbuf[tblen] = *inpstr;
							tblen++;
							i++; inpstr++;
						} else
							continue;
					} else
						break;
				}
			#endif
				textbuf[tblen] = *inpstr;
				tblen++;
			}
		textbuf[tblen] = 0;

		catime_clocks = catime*N_CLOCKS_PER_SEC/1000;

		if((nFrameStartClock()/catime_clocks)%2)
		{
			textbuf[tblen] = csym;
			textbuf[tblen+1] = 0;
		}
	}

	focus = nvDraw2dText(textbuf, posx, posy, fontid, texid, scalex, scaley, (*status)?color_focused:color);

	if(*status == 1)
		textbuf[tblen] = 0;

	mbl = nvGetStatusi(NV_STATUS_WINMBL);

	if(mbl == 2) {
		if(focus)
			*status = 1;
		else
			*status = 0;
	}

	return focus;
}
