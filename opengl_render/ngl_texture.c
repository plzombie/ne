/*
	Файл	: ngl_texture.c

	Описание: Работа с текстурами

	История	: 14.08.12	Создан

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h> // Temp

#define GL_GLEXT_LEGACY

#ifdef N_WINDOWS
	#include <windows.h>
#endif
#include <GL/glu.h>
#include "../forks/gl/glext.h"

#include "ngl.h"
#include "ngl_text.h"

#include "../nyan/nyan_texformat_publicapi.h"

#include "ngl_init.h"
#include "ngl_batch.h"
#include "ngl_texture.h"

// Число не в степени двойки, если результат не ноль
#define NOTPOW2(num) ((num) & ((num) - 1))

#if (defined(_MSC_VER) && !defined(__POCC__)) || defined(__BORLANDC__)
	#define log2(a) (log(a)/log(2))
#endif

// Параметры текстуры
unsigned int ngl_tex_maxsize = 65536;
int ngl_tex_maxanis = 0; // Максимальный уровень анизотропии
int ngl_tex_anis = 666; // Уровень анизотропии
bool ngl_tex_s_npot = false; // Поддержка текстур не в степени двойки
bool ngl_tex_s_bgra_ext = false;
bool ngl_tex_s_abgr_ext = false;
bool ngl_tex_s_cmyka_ext = false;
bool ngl_tex_s_packed_pixels = false;
int ngl_tex_enabledflags = NGL_TEX_FLAGS_ALL; // Разрешённые флаги текстур

// Массивы текстур
ngl_texture_type *ngl_textures = 0;
unsigned int ngl_maxtextures = 0;

uintptr_t ngl_textures_sync_mutex = 0; // Мьютекс для синхронизации работы с текстурами

/*
	Функция	: nglIsTex

	Описание: Возвращает true, если id - текстура

	История	: 02.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglIsTex(unsigned int id)
{
	bool result = true;

	if(ngl_isinit == false || id == 0) return false;

	ngl_ea->nLockMutex(ngl_textures_sync_mutex);
		if(id > ngl_maxtextures)
			result = false;
		else if(ngl_textures[id-1].status != NGL_TEX_STATUS_LOADED)
			result = false;
	ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);

	return result;
}

/*
	Функция	: nglGetGLid

	Описание: Возвращает id opengl текстуры

	История	: 02.06.12	Создан

*/
unsigned int nglTexGetGLid(unsigned int id)
{
	unsigned int glid;

	if(ngl_isinit == false || id == 0) return 0;

	ngl_ea->nLockMutex(ngl_textures_sync_mutex);
		if(id > ngl_maxtextures) {
			ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
			return 0;
		}

		if(ngl_textures[id-1].status != NGL_TEX_STATUS_LOADED) {
			ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
			return 0;
		}

		glid = ngl_textures[id-1].glid;
	ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);

	return glid;
}

/*
	Функция	: nglTexGetScale

	Описание: Возвращает числа, на которые надо домножить координаты текстуры

	История	: 25.06.12	Создан

*/
void nglTexGetScale(unsigned int id,unsigned int *flags, float *scalex, float *scaley)
{
	if(ngl_isinit == false || id > ngl_maxtextures || id == 0) { *flags = 0; *scalex = 1.0; *scaley = 1.0; return; }

	if(ngl_textures[id-1].status != NGL_TEX_STATUS_LOADED) { *flags = 0; *scalex = 1.0; *scaley = 1.0; return; }

	*flags = ngl_textures[id-1].flags;
	*scalex = ngl_textures[id-1].cscalex;
	*scaley = ngl_textures[id-1].cscaley;
}

/*
	Функция	: nglTexGetBytesPerPixel

	Описание: Количество байт на пиксель

	История	: 17.06.12	Создан

*/
int nglTexGetBytesPerPixel(unsigned int nglcolorformat)
{
	switch(nglcolorformat) {
		case NGL_COLORFORMAT_R8G8B8A8:
		case NGL_COLORFORMAT_B8G8R8A8:
		case NGL_COLORFORMAT_A8B8G8R8:
			return 4;
		case NGL_COLORFORMAT_R8G8B8:
		case NGL_COLORFORMAT_B8G8R8:
			return 3;
		case NGL_COLORFORMAT_L8A8:
		case NGL_COLORFORMAT_X1R5G5B5:
		case NGL_COLORFORMAT_R5G6B5:
			return 2;
		case NGL_COLORFORMAT_L8:
			return 1;
		default:
			return 0;
	}
}

/*
	Функция	: nglTexGetgltype

	Описание: Возвращает gltype

	История	: 26.08.12	Создан

*/
GLenum nglTexGetgltype(unsigned int nglcolorformat)
{
	switch(nglcolorformat) {
		case NGL_COLORFORMAT_R8G8B8:
		case NGL_COLORFORMAT_R8G8B8A8:
		case NGL_COLORFORMAT_B8G8R8:
		case NGL_COLORFORMAT_B8G8R8A8:
		case NGL_COLORFORMAT_A8B8G8R8:
		case NGL_COLORFORMAT_L8:
		case NGL_COLORFORMAT_L8A8:
			return GL_UNSIGNED_BYTE;
		case NGL_COLORFORMAT_X1R5G5B5:
			return GL_UNSIGNED_SHORT_1_5_5_5_REV;
		case NGL_COLORFORMAT_R5G6B5:
			return GL_UNSIGNED_SHORT_5_6_5;
		default:
			return 0;
	}
}

/*
	Функция	: nglTexGetglformat

	Описание: Возвращает glformat

	История	: 26.08.12	Создан

*/
GLenum nglTexGetglformat(unsigned int nglcolorformat)
{
	switch(nglcolorformat) {
		case NGL_COLORFORMAT_R8G8B8:
			return GL_RGB;
		case NGL_COLORFORMAT_R8G8B8A8:
			return GL_RGBA;
		case NGL_COLORFORMAT_B8G8R8:
			return GL_BGR;
		case NGL_COLORFORMAT_B8G8R8A8:
			return GL_BGRA;
		case NGL_COLORFORMAT_A8B8G8R8:
			return GL_ABGR_EXT;
		case NGL_COLORFORMAT_L8:
			return GL_LUMINANCE;
		case NGL_COLORFORMAT_L8A8:
			return GL_LUMINANCE_ALPHA;
		case NGL_COLORFORMAT_X1R5G5B5:
			return GL_BGRA;
		case NGL_COLORFORMAT_R5G6B5:
			return GL_RGB;
		default:
			return 0;
	}
}

/*
	Функция	: nglTexGetglinternalFormat

	Описание: Возвращает glinternalFormat

	История	: 26.08.12	Создан

*/
GLenum nglTexGetglinternalFormat(unsigned int nglcolorformat)
{
	switch(nglcolorformat) {
		case NGL_COLORFORMAT_R8G8B8:
			return GL_RGB;
		case NGL_COLORFORMAT_R8G8B8A8:
			return GL_RGBA;
		case NGL_COLORFORMAT_B8G8R8:
			return GL_RGB8;
		case NGL_COLORFORMAT_B8G8R8A8:
			return GL_RGBA8;
		case NGL_COLORFORMAT_A8B8G8R8:
			return GL_RGBA8;
		case NGL_COLORFORMAT_L8:
			return GL_LUMINANCE;
		case NGL_COLORFORMAT_L8A8:
			return GL_LUMINANCE_ALPHA;
		case NGL_COLORFORMAT_X1R5G5B5:
			return GL_RGB5;
		case NGL_COLORFORMAT_R5G6B5:
			return GL_RGB16;
		default:
			return 0;
	}
}

/*
	Функция	: nglTexGetRowSize

	Описание: Возвращает размер строки текстуры. НЕ проверяет поддержку формата движком (это должно делаться в nglTexConvertToSupported)

	История	: 25.06.12	Создан

*/
int nglTexGetRowSize(unsigned int nglcolorformat,int sizex,int packrow)
{
	int bpp; // Байт на пиксель

	bpp = nglTexGetBytesPerPixel(nglcolorformat);

	if(((sizex*bpp) % packrow) == 0) return (sizex*bpp); else
		return ((sizex*bpp/packrow)*packrow+packrow);
}

/*
	Функция	: nglTexGetSize

	Описание: Возвращает размер текстуры. НЕ проверяет поддержку формата движком (это должно делаться в nglTexConvertToSupported)

	История	: 17.06.12	Создан

*/
size_t nglTexGetSize(unsigned int nglcolorformat,int sizex,int sizey,int packrow)
{
	return (size_t)nglTexGetRowSize(nglcolorformat, sizex, packrow)*(size_t)sizey;
}


/*
	Функция	: nglFreeTexture

	Описание: Освобождает текстуру

	История	: 14.08.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglFreeTexture(unsigned int id)
{
	ngl_texture_type texture;

	if(!ngl_isinit) return false;

	ngl_ea->nLockMutex(ngl_textures_sync_mutex);
		if(id < 1 || id > ngl_maxtextures) {
			ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NGLFREETEXTURE, N_FALSE, N_ID, id);
			return false;
		}
		if(ngl_textures[id-1].status != NGL_TEX_STATUS_LOADED) {
			ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLFREETEXTURE, N_FALSE, N_ID, id);
			return false;
		}
		texture = ngl_textures[id-1];
		ngl_textures[id-1].status = NGL_TEX_STATUS_FREE;
	ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);

	glDeleteTextures(1, (const GLuint *)&texture.glid);

	nglCatchOpenGLError(F_NGLFREETEXTURE);

	ngl_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NGLFREETEXTURE, N_OK, N_ID, id);

	return true;
}

/*
	Функция	: nglFreeAllTextures

	Описание: Освобождает текстуру

	История	: 14.08.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglFreeAllTextures(void)
{
	unsigned int i;

	if(!ngl_isinit) return false;

	ngl_ea->nLockMutex(ngl_textures_sync_mutex);
		if(ngl_maxtextures) {
			for(i = 0;i<ngl_maxtextures;i++)
				if(ngl_textures[i].status == NGL_TEX_STATUS_LOADED)
					nglFreeTexture(i+1);

			ngl_maxtextures = 0;
			ngl_ea->nFreeMemory(ngl_textures);
			ngl_textures = 0;
		}
	ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);

	return true;
}

/*
	Функция	: nglTexScale2to1ub

	Описание: Уменьшает размер текстуры в 2 раза. epp - кол-во элементов. Тип элемента - GL_UNSIGNED_BYTE.

	История	: 25.06.12	Создана (и закомментирована :)
			30.07.12	Раскомментирована. Первая работающая версия

*/
//#include <io.h>
//#include <fcntl.h>
//#include <sys/stat.h>
//#include <mmintrin.h>
bool nglTexScale2to1ub(unsigned int sizex, unsigned int sizey, unsigned int epp, unsigned int packrow, unsigned char *dst, unsigned char *src)
{
	size_t oldrow; // Старый размер строки
	//size_t newrow; // Новый размер строки
	size_t oldrowadd, newrowadd; // Количество байт, которые необходимо добавить к строке для выравнивания
	size_t i, j, k;
	//int f, l;

	if(epp == 0) return false;

	if(((sizex*epp) % packrow) == 0) oldrowadd = 0; else
		oldrowadd = packrow- (size_t)sizex*epp%packrow;

	oldrow = (size_t)sizex*epp+oldrowadd;

	if(((sizex/2*epp) % packrow) == 0) newrowadd = 0; else
		newrowadd = packrow- (size_t)sizex/2*epp%packrow;

	//newrow = (size_t)sizex/2*epp+newrowadd;

//for(l = 0; l < 100; l++) {
#if 1
/*if(epp == 4) {
	__m64 v, v1, v2, v3, v4;
	unsigned char *p, *p1, *p2, *p3, *p4;
	p = dst;

	p1 = src;
	p2 = src+oldrow;
	p3 = src+4;
	p4 = src+oldrow+4;

#ifdef __WATCOMC__
	v = _m_from_int(0);
#else
	v = _mm_setzero_si64();
#endif

	for(i=0;i<sizey/2;i++) {
		for(j=0;j<sizex/2;j++) {
			v1 = _m_from_int(*(int *)p1);
			v1 = _m_punpcklbw(v1, v);
			v2 = _m_from_int(*(int *)p2);
			v2 = _m_punpcklbw(v2, v);
			v3 = _m_from_int(*(int *)p3);
			v3 = _m_punpcklbw(v3, v);
			v4 = _m_from_int(*(int *)p4);
			v4 = _m_punpcklbw(v4, v);
		

			v1 = _m_paddw(v1, v2);
			v3 = _m_paddw(v3, v4);
			v1 = _m_paddw(v1, v3);

			v1 = _m_psrlwi(v1, 2);

		#ifdef __WATCOMC__
			*p = v1._8[0]; p++;
			*p = v1._8[2]; p++;
			*p = v1._8[4]; p++;
			*p = v1._8[6]; p++;
		#else
			*p = v1.m64_u8[0]; p++;
			*p = v1.m64_u8[2]; p++;
			*p = v1.m64_u8[4]; p++;
			*p = v1.m64_u8[6]; p++;
		#endif

			p1 += 8;
			p2 += 8;
			p3 += 8;
			p4 += 8;
		}
		p += newrowadd;
		p1 += oldrowadd+oldrow;
		p2 += oldrowadd+oldrow;
		p3 += oldrowadd+oldrow;
		p4 += oldrowadd+oldrow;
	}
	_m_empty();
} else */{
	unsigned char *p, *p1, *p2;

	p = dst;

	p1 = src;
	p2 = src+oldrow;

	for(i=0; i<sizey/2; i++) {
		for(j=0; j<sizex/2; j++) {
			for(k=0; k<epp; k++) {
				*p = (*p1+*(p1+epp)+*p2+*(p2+epp))/4;
				p++;
				p1++;
				p2++;
			}
			p1 += epp;
			p2 += epp;
		}
		p += newrowadd;
		p1 += oldrowadd+oldrow;
		p2 += oldrowadd+oldrow;
	}
}
#else
	//Неоптимизированная версия
	for(i=0; i<sizey/2; i++)
		for(j=0; j<sizex/2; j++)
			for(k=0; k<epp; k++)
				dst[i*newrow+epp*j+k] = (src[(i*2)*oldrow+epp*(j*2)+k]+src[(i*2)*oldrow+epp*(j*2+1)+k]+
										src[(i*2+1)*oldrow+epp*(j*2)+k]+src[(i*2+1)*oldrow+epp*(j*2+1)+k])/4;
#endif
//}

	//f = _open("gleng.raw",_O_CREAT|_O_WRONLY|_O_BINARY, _S_IREAD | _S_IWRITE);
	//_write(f,dst,newrow*sizey/2);
	//_close(f);

	return true;
}

/*
	Функция	: nglTexScale2to1ub

	Описание: Изменяет размер текстуры методом ближайшего соседа. epp - кол-во элементов. Тип элемента - GL_UNSIGNED_BYTE.

	История	: 02.06.18	Создана

*/
bool nglTexScaleNearestub(unsigned int src_sizex, unsigned int src_sizey, unsigned int dst_sizex, unsigned int dst_sizey, unsigned int epp, unsigned int packrow, unsigned char *dst, unsigned char *src)
{
	size_t i, j, k;
	size_t src_rowadd, dst_rowadd; // Количество байт, которые необходимо добавить к строке для выравнивания

	if(epp == 0) return false;

	if(((src_sizex*epp) % packrow) == 0) src_rowadd = 0; else
		src_rowadd = packrow-(size_t)src_sizex*epp%packrow;

	if(((dst_sizex*epp) % packrow) == 0) dst_rowadd = 0; else
		dst_rowadd = packrow-(size_t)dst_sizex*epp%packrow;

	for(i = 0; i < dst_sizey; i++) {
		size_t src_i;
		size_t src_row_offset, dst_row_offset;

		src_i = i*src_sizey/dst_sizey;
		src_row_offset = src_i*src_sizex*epp+src_rowadd;
		dst_row_offset = i*dst_sizex*epp+dst_rowadd;

		for(j = 0; j < dst_sizex; j++) {
			size_t src_j;
			size_t src_pixel_offset, dst_pixel_offset;

			src_j = j*src_sizex/dst_sizex;

			src_pixel_offset = src_row_offset+src_j*epp;
			dst_pixel_offset = dst_row_offset+j*epp;

			for(k = 0; k < epp; k++) {
				/*if(src_pixel_offset+k >= src_sizey*(size_t)(epp*src_sizex+src_rowadd))
					wprintf(L"error1\n");
				if(dst_pixel_offset+k >= dst_sizey*(size_t)(epp*dst_sizex+dst_rowadd))
					wprintf(L"error2\n");*/
				dst[dst_pixel_offset+k] = src[src_pixel_offset+k];
			}
		}
	}

	return true;
}

/*
	Функция	: nglTexScale

	Описание: Изменяет размер текстуры. НЕ проверяет поддержку формата движком (это должно делаться в nglTexConvertToSupported)

	История	: 17.06.12	Создан

*/
bool nglTexScale(ngl_texture_type *tex, unsigned int sizex, unsigned int sizey)
{
	unsigned char *buffer; // Картинка до преобразования
	bool success = true;
	GLenum gltype, glformat;
	int64_t delta;

	if(tex->nglrowalignment != ngl_glrowalignment) { // Для gluScaleImage
		ngl_glrowalignment = tex->nglrowalignment;
		glPixelStorei(GL_UNPACK_ALIGNMENT, ngl_glrowalignment);
		glPixelStorei(GL_PACK_ALIGNMENT, ngl_glrowalignment);
	}

	gltype = nglTexGetgltype(tex->nglcolorformat);
	glformat = nglTexGetglformat(tex->nglcolorformat);

	buffer = tex->buffer;
	tex->buffer = ngl_ea->nAllocMemory(nglTexGetSize(tex->nglcolorformat, sizex, sizey, tex->nglrowalignment));
	if(!tex->buffer) {
		tex->buffer = buffer;
		return false;
	}

	delta = ngl_ea->nClock();

	if((gltype == GL_UNSIGNED_BYTE) && (tex->sizex == sizex*2) && (tex->sizey == sizey*2))
		success = nglTexScale2to1ub(tex->sizex, tex->sizey, nglTexGetBytesPerPixel(tex->nglcolorformat), tex->nglrowalignment, tex->buffer, buffer);
	else if (tex->sizex > 16384 || tex->sizey > 16384) { // Приблизительное значение, при котором крашится gluScaleImage
		if(gltype == GL_UNSIGNED_BYTE)
			success = nglTexScaleNearestub(tex->sizex, tex->sizey, sizex, sizey, nglTexGetBytesPerPixel(tex->nglcolorformat), tex->nglrowalignment, tex->buffer, buffer);
		else
			success = false;
	} else {
		if(gluScaleImage(glformat, tex->sizex, tex->sizey, gltype, buffer, sizex, sizey, gltype, tex->buffer)) {
			if(gltype == GL_UNSIGNED_BYTE)
				success = nglTexScaleNearestub(tex->sizex, tex->sizey, sizex, sizey, nglTexGetBytesPerPixel(tex->nglcolorformat), tex->nglrowalignment, tex->buffer, buffer);
			else
				success = false;
		}

	}

	delta = ngl_ea->nClock() - delta;
	ngl_ea->nlPrint(L"CTS2(resize only) %lld", delta);

	if(tex->bufmayfree)
		ngl_ea->nFreeMemory(buffer);
	else
		tex->bufmayfree = true;

	tex->sizex = sizex;
	tex->sizey = sizey;

	return success;
}

/*
	Функция	: nglTexScaleCanvas

	Описание: !!!УВЕЛИЧИТЬ!!! размер холста. НЕ проверяет поддержку формата движком (это должно делаться в visTexConvertToSupported)
				sizex,sizey - Новый размер для tex->sizex, tex->sizey

	История	: 17.06.12	Создан

*/
bool nglTexScaleCanvas(ngl_texture_type *tex, unsigned int sizex, unsigned int sizey)
{
	unsigned int bpp, linsize, newrow, rowal, newrowal;
	unsigned char *buffer, *p, *p2; // Картинка до преобразования
	unsigned int i, j;
	size_t texsize;

	if((tex->sizex > sizex) || (tex->sizey > sizey) || (tex->sizex == 0) || (tex->sizey == 0)) return false;

	texsize = nglTexGetSize(tex->nglcolorformat, sizex, sizey, tex->nglrowalignment);
	if(texsize == 0) return false;

	bpp = nglTexGetBytesPerPixel(tex->nglcolorformat);

	linsize = tex->sizex*nglTexGetBytesPerPixel(tex->nglcolorformat); // Старый размер строки (без выравнивания)

	if(((tex->sizex*bpp) % tex->nglrowalignment) == 0) rowal = 0; else // Выравнивание старой строки
		rowal = tex->nglrowalignment-tex->sizex*bpp%tex->nglrowalignment;

	if(((sizex*bpp) % tex->nglrowalignment) == 0) newrowal = 0; else // Выравнивание новой строки
		newrowal = tex->nglrowalignment-sizex*bpp%tex->nglrowalignment;

	newrow = nglTexGetRowSize(tex->nglcolorformat, sizex, tex->nglrowalignment); // Новый размер строки

	buffer = tex->buffer;
	tex->buffer = ngl_ea->nAllocMemory(texsize);
	if(!tex->buffer) {
		tex->buffer = buffer;
		return false;
	}

	// Перенос информации из старой текстуры в новую
	p = buffer;
	p2 = tex->buffer;
	for(i=0;i<tex->sizey;i++) {
		memcpy(p2, p, linsize);
		p += linsize; p2 += linsize;
		for(j=0;j<(sizex-tex->sizex);j++) { // Добавляю границу справа
			memcpy(p2, p-bpp, bpp);
			p2 += bpp;
		}
		p += rowal; p2 += newrowal;
	}
	p = p2-newrow;
	for(i=0;i<(sizey-tex->sizey);i++) { // Добавляю границу сверху
		memcpy(p2, p, newrow);
		p2 += newrow;
	}

	if(tex->bufmayfree)
		ngl_ea->nFreeMemory(buffer);
	else
		tex->bufmayfree = true;

	tex->osizex = (tex->osizex*sizex)/tex->sizex;
	tex->osizey = (tex->osizey*sizey)/tex->sizey;
	tex->sizex = sizex;
	tex->sizey = sizey;
	tex->cscalex = (float)tex->oasizex/tex->osizex;
	tex->cscaley = (float)tex->oasizey/tex->osizey;
	tex->flags |= NGL_TEX_FLAGS_SCALECOORD;

	return true;//nglTexScale(tex, sizex, sizey);
}

/*
	Функция	: nglTexConvertToSupported

	Описание: Конвертирует текстуру в поддерживаемый формат

	История	: 17.06.12	Создан

*/
bool nglTexConvertToSupported(ngl_texture_type *tex)
{
	//int64_t delta;
	//ngl_ea->nlPrint(L"CTS0");
	//delta = ngl_ea->nClock();

	switch(tex->nglcolorformat) {
		case NGL_COLORFORMAT_R8G8B8:
		case NGL_COLORFORMAT_R8G8B8A8:
			break;
		case NGL_COLORFORMAT_B8G8R8:
		case NGL_COLORFORMAT_B8G8R8A8:
			if(!ngl_tex_s_bgra_ext) {
				unsigned char *p1, *p2, *p11, *p12;
				unsigned int al, bpp, i, j;
				size_t texsize;
				bpp = (tex->nglcolorformat == NGL_COLORFORMAT_B8G8R8) ? 3 : 4;
				if(((tex->sizex*bpp) % tex->nglrowalignment) == 0) al = 0; else // Выравнивание строки
					al = tex->nglrowalignment-tex->sizex*bpp%tex->nglrowalignment;
				p1 = tex->buffer; p2 = tex->buffer+2;
				texsize = nglTexGetSize(tex->nglcolorformat, tex->sizex, tex->sizey, tex->nglrowalignment);
				tex->buffer = ngl_ea->nAllocMemory(texsize);
				if(tex->buffer)
					tex->bufmayfree = true;
				else
					return false;
				memcpy(tex->buffer, p1, texsize);
				p11 = tex->buffer; p12 = tex->buffer+2;
				for(i = 0;i<tex->sizex;i++) {
					for(j = 0;j<tex->sizey;j++) {
						*p12 = *p1;
						*p11 = *p2;
						p1  += bpp; p2  += bpp;
						p11 += bpp; p12 += bpp;
					}
					p1 += al; p2 += al; p11 += al; p12 += al;
				}
				tex->nglcolorformat = (tex->nglcolorformat==NGL_COLORFORMAT_B8G8R8)?NGL_COLORFORMAT_R8G8B8:NGL_COLORFORMAT_R8G8B8A8;
			}
			break;
		case NGL_COLORFORMAT_A8B8G8R8:
			if(!ngl_tex_s_abgr_ext) {
				unsigned char *p1, *p2;
				unsigned int i;
				size_t texsize;

				p1 = tex->buffer;

				texsize = nglTexGetSize(tex->nglcolorformat, tex->sizex, tex->sizey, tex->nglrowalignment);
				tex->buffer = ngl_ea->nAllocMemory(texsize);
				if(tex->buffer)
					tex->bufmayfree = true;
				else
					return false;

				p2 = tex->buffer;

				for(i = 0; i < tex->sizex*tex->sizey; i++) {
					unsigned char a, b, g, r;
					a = *(p1++);
					b = *(p1++);
					g = *(p1++);
					r = *(p1++);
					*(p2++) = r;
					*(p2++) = g;
					*(p2++) = b;
					*(p2++) = a;
				}
				tex->nglcolorformat = NGL_COLORFORMAT_R8G8B8A8;
			}
			break;
		case NGL_COLORFORMAT_L8:
		case NGL_COLORFORMAT_L8A8:
			break;
		case NGL_COLORFORMAT_X1R5G5B5:
		case NGL_COLORFORMAT_R5G6B5:
			if(!ngl_tex_s_packed_pixels || !ngl_tex_s_bgra_ext) {
				unsigned int rowal, i, j;
				unsigned short *p;
				unsigned char *p2;
				p = (unsigned short *)tex->buffer;
				tex->buffer = ngl_ea->nAllocMemory(tex->sizex*tex->sizey*3);
				if(tex->buffer)
					tex->bufmayfree = true;
				else
					return false;
				p2 = tex->buffer;
				rowal = (tex->sizex*2)%4;
				if((tex->sizex*3)%4 == 0) tex->nglrowalignment = 4; else tex->nglrowalignment = 1;
				for(i=0;i<tex->sizey;i++) {
					for(j=0;j<tex->sizex;j++) {
						if(tex->nglcolorformat == NGL_COLORFORMAT_X1R5G5B5) {
							*p2 = (((*p)>>10)&0x001F)<<3; p2++;
							*p2 = (((*p)>>5)&0x001F)<<3; p2++;
							*p2 = ((*p)&0x001F)<<3; p2++;
						} else {
							*p2 = (((*p)>>11)&0x001F)<<3; p2++;
							*p2 = (((*p)>>5)&0x003F)<<2; p2++;
							*p2 = ((*p)&0x001F)<<3; p2++;
						}
						p++;
					}
					p += rowal;
				}
				tex->nglcolorformat = NGL_COLORFORMAT_R8G8B8;
			}
			break;
		/*case GL_CMYK_EXT:
		case GL_CMYKA_EXT:
			if(!ngl_tex_s_cmyka_ext) {
				ngl_ea->nlPrint(LOG_FDEBUGFORMAT3, F_NGLTEXCONVERTTOSUPPORTED, ERR_UNSUPPORTEDNGLCOLORFORMAT, tex->nglcolorformat);
				return false;
			}
			break;*/
		default:
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT3, F_NGLTEXCONVERTTOSUPPORTED, ERR_UNSUPPORTEDNGLCOLORFORMAT, tex->nglcolorformat);
			return false;
	}
	//delta = ngl_ea->nClock()-delta;
	//ngl_ea->nlPrint(L"CTS1 %lld",delta);
	//delta = ngl_ea->nClock();
	if(tex->sizex > ngl_tex_maxsize || tex->sizey > ngl_tex_maxsize) {
		unsigned int newx, newy;
		if(tex->sizex > ngl_tex_maxsize) newx = ngl_tex_maxsize; else newx = tex->sizex;
		if(tex->sizey > ngl_tex_maxsize) newy = ngl_tex_maxsize; else newy = tex->sizey;

		if(!nglTexScale(tex, newx, newy)) return false;
	}
	//delta = ngl_ea->nClock()-delta;
	//ngl_ea->nlPrint(L"CTS2 %lld",delta);
	//delta = ngl_ea->nClock();
	if((NOTPOW2(tex->sizex) || NOTPOW2(tex->sizey)) && !ngl_tex_s_npot) {
		int newx, newy;

		if(NOTPOW2(tex->sizex)) newx = 1 << ((int)log2(tex->sizex)+1); else newx = tex->sizex;
		if(NOTPOW2(tex->sizey)) newy = 1 << ((int)log2(tex->sizey)+1); else newy = tex->sizey;
		if(tex->flags & NGL_TEX_FLAGS_FOR2D) {
			ngl_ea->nlPrint(L"CTS3 npot scalecanvas %d", tex->flags);
			if(!nglTexScaleCanvas(tex, newx, newy)) return false;
		} else {
			ngl_ea->nlPrint(L"CTS3 npot scaleimg %d", tex->flags);
			if(!nglTexScale(tex, newx, newy)) return false;
		}
	}
	//delta = ngl_ea->nClock()-delta;
	//ngl_ea->nlPrint(L"CTS3 %lld",delta);
	return true;
}

/*
	Функция	: nglGLBuild2DMipmaps

	Описание: Задаёт мипмап-уровни для OpenGL-текстуры

	История	: 26.02.17	Создан

*/
static void nglGLBuild2DMipmaps(GLenum target, GLsizei sizex, GLsizei sizey, unsigned int nglcolorformat, const void *buffer)
{
	GLint glinternalFormat;
	GLenum glformat;
	GLenum gltype;
	unsigned int new_nglcolorformat; // Нужен для передачи glinternalFormat в старых имплементациях glu

	// Старые некоторые имплементации glu не поддерживают internalFormat=GL_RGB8, например.
	// Для этого определяем совместимый с ними new_nglcolorformat.
	switch(nglcolorformat) {
		case NGL_COLORFORMAT_B8G8R8:
			new_nglcolorformat = NGL_COLORFORMAT_R8G8B8;
			break;
		case NGL_COLORFORMAT_B8G8R8A8:
		case NGL_COLORFORMAT_A8B8G8R8:
			new_nglcolorformat = NGL_COLORFORMAT_R8G8B8A8;
			break;
		default:
			new_nglcolorformat = nglcolorformat;
	}

	gltype = nglTexGetgltype(nglcolorformat);
	glformat = nglTexGetglformat(nglcolorformat);
	glinternalFormat = nglTexGetglinternalFormat(new_nglcolorformat);

	gluBuild2DMipmaps(target, glinternalFormat, sizex, sizey, glformat, gltype, buffer);
}

/*
	Функция	: nglTexLoadToGL

	Описание: Загружает текстуру в OpenGL. Данная функция не проверяет правильность формата текстуры

	История	: 15.06.12	Создан

*/
bool nglTexLoadToGL(ngl_texture_type *tex)
{
	int minfilter, magfilter;
	GLenum gltype, glformat, glinternalFormat;

	nglCatchOpenGLError(0);

	if(nglIsMainRContext()) {
		if(tex->nglrowalignment != ngl_glrowalignment) {
			ngl_glrowalignment = tex->nglrowalignment;
			glPixelStorei(GL_UNPACK_ALIGNMENT, ngl_glrowalignment);
			glPixelStorei(GL_PACK_ALIGNMENT, ngl_glrowalignment);
		}
	} else {
		glPixelStorei(GL_UNPACK_ALIGNMENT, tex->nglrowalignment);
		glPixelStorei(GL_PACK_ALIGNMENT, tex->nglrowalignment);
	}

	gltype = nglTexGetgltype(tex->nglcolorformat);
	glformat = nglTexGetglformat(tex->nglcolorformat);
	glinternalFormat = nglTexGetglinternalFormat(tex->nglcolorformat);

	if(nglCatchOpenGLError(F_NGLTEXLOADTOGL)) // Ошибка установки GL_(UN)PACK_ALIGNMENT
		return false;

	glGenTextures(1, &tex->glid);

	if(nglCatchOpenGLError(F_NGLTEXLOADTOGL)) // Ошибка создания id текстуры
		return false;

	glBindTexture(GL_TEXTURE_2D, tex->glid);

	if(nglCatchOpenGLError(F_NGLTEXLOADTOGL)) { // Ошибка выбора только что созданной текстуры
		glDeleteTextures(1, &tex->glid);
		return false;
	}

	if((tex->flags & NGL_TEX_FLAGS_ANISOTROPY) && (ngl_tex_maxanis > 0))
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_ANISOTROPY_EXT,ngl_tex_anis);

	if(tex->flags & NGL_TEX_FLAGS_LINEARMAG)
		magfilter = GL_LINEAR;
	else
		magfilter = GL_NEAREST;

	if(nglCatchOpenGLError(F_NGLTEXLOADTOGL)) { // Ошибка установки анизотропной фильтрации
		glDeleteTextures(1, &tex->glid);
		return false;
	}

	if(tex->flags & NGL_TEX_FLAGS_MIPMAP) {
		nglGLBuild2DMipmaps(GL_TEXTURE_2D, tex->sizex, tex->sizey, tex->nglcolorformat, tex->buffer);

		if(tex->flags & NGL_TEX_FLAGS_LINEARMIN)
			if(tex->flags & NGL_TEX_FLAGS_LINEARMIPMAP)
				minfilter = GL_LINEAR_MIPMAP_LINEAR;
			else
				minfilter = GL_LINEAR_MIPMAP_NEAREST;
		else
			if(tex->flags & NGL_TEX_FLAGS_LINEARMIPMAP)
				minfilter = GL_NEAREST_MIPMAP_LINEAR;
			else
				minfilter = GL_NEAREST_MIPMAP_NEAREST;
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, glinternalFormat, tex->sizex, tex->sizey, 0, glformat, gltype, tex->buffer);
		if(tex->flags & NGL_TEX_FLAGS_LINEARMIN)
			minfilter = GL_LINEAR;
		else
			minfilter = GL_NEAREST;
	}

	if(nglCatchOpenGLError(F_NGLTEXLOADTOGL)) { // Ошибка загрузки данных текстуры
		glDeleteTextures(1, &tex->glid);
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
	if(tex->flags & (NGL_TEX_FLAGS_FOR2D | NGL_TEX_FLAGS_CLAMP_TO_EDGE)) {
		if(ngl_win_texture_edge_clamp) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}
	}

	if(nglCatchOpenGLError(F_NGLTEXLOADTOGL)) { // Ошибка установки параметров/фильтрации
		glDeleteTextures(1, &tex->glid);
		return false;
	}

	glFlush(); // Нужно при загрузке текстуры из отдельного потока

	if(nglIsMainRContext()) {
		if(ngl_batch_currenttex)
			glBindTexture(GL_TEXTURE_2D,nglTexGetGLid(ngl_batch_currenttex)); // Устанавливаю старую текстуру
	}

	nglCatchOpenGLError(F_NGLTEXLOADTOGL); // Ошибка установки предыдущей текстуры. Новая уже загружена, так что возвращать false нет смысла

	return true;
}

/*
	Функция	: nglLoadTexture

	Описание: Загружает текстуру

	История	: 14.08.12	Создан

*/
N_API unsigned int N_APIENTRY_EXPORT nglLoadTexture(unsigned int flags, unsigned int sizex, unsigned int sizey, unsigned int nglcolorformat, unsigned int nglrowalignment, unsigned char *buffer)
{
	unsigned int curtex; // Текущая текстура (id которой вернут пользователю)
	ngl_texture_type texture;
	bool success = true;

	if(!ngl_isinit) return 0;

	ngl_ea->nlPrint(F_NGLLOADTEXTURE); ngl_ea->nlAddTab(1);

	// Проверка наличия и создание контекста OpenGL в случае необходимости
	if(!nglAddSeparateRContextIfNeeded()) {
		ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLLOADTEXTURE, N_FALSE);
		return 0;
	}

	// Выделение памяти под текстуры
	ngl_ea->nLockMutex(ngl_textures_sync_mutex);
		for(curtex = 0; curtex < ngl_maxtextures; curtex++)
			if(ngl_textures[curtex].status == NGL_TEX_STATUS_FREE)
				break;

		if(curtex == ngl_maxtextures) {
			ngl_texture_type *_ngl_textures;

			_ngl_textures = ngl_ea->nReallocMemory(ngl_textures,(ngl_maxtextures+1024)*sizeof(ngl_texture_type));

			if(_ngl_textures)
				ngl_textures = _ngl_textures;
			else {
				ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
				ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NGLLOADTEXTURE, N_FALSE, N_ID, 0);
				return 0;
			}

			for(curtex=ngl_maxtextures;curtex<ngl_maxtextures+1024;curtex++)
				ngl_textures[curtex].status = NGL_TEX_STATUS_FREE;

			curtex = ngl_maxtextures;

			ngl_maxtextures += 1024;
		}
		ngl_textures[curtex].status = NGL_TEX_STATUS_PENDING;
	ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);

	texture.bufmayfree = false;
	texture.flags = flags;
	texture.oasizex = sizex;
	texture.oasizey = sizey;
	texture.osizex = sizex;
	texture.osizey = sizey;
	texture.sizex = sizex;
	texture.sizey = sizey;
	texture.nglcolorformat = nglcolorformat;
	texture.nglrowalignment = nglrowalignment;
	texture.buffer = buffer;

	success = nglTexConvertToSupported(&texture);
	//ngl_ea->nlPrint(L"nglTexConvertToSupported %d",(int)success);

	if(success) success = nglTexLoadToGL(&texture);
	//ngl_ea->nlPrint(L"nglTexLoadToGL %d",(int)success);

	if(texture.bufmayfree)
		ngl_ea->nFreeMemory(texture.buffer);

	ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT5, F_NGLLOADTEXTURE, success?N_OK:N_FALSE, N_ID, success?curtex+1:0);

	if(success) {
		texture.status = NGL_TEX_STATUS_LOADED;
		ngl_ea->nLockMutex(ngl_textures_sync_mutex);
		ngl_textures[curtex] = texture;
		ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
		return (curtex+1);
	} else {
		ngl_ea->nLockMutex(ngl_textures_sync_mutex);
		ngl_textures[curtex].status = NGL_TEX_STATUS_FREE;
		ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
		return 0;
	}
}

/*
	Функция	: nglTexUpdateToGL

	Описание: Загружает текстуру в OpenGL. Данная функция не проверяет правильность формата текстуры

	История	: 12.02.13	Создан

*/
bool nglTexUpdateToGL(ngl_texture_type *tex)
{
	GLenum gltype, glformat;

	nglCatchOpenGLError(NULL);

	if(nglIsMainRContext()) {
		if(tex->nglrowalignment != ngl_glrowalignment) {
			ngl_glrowalignment = tex->nglrowalignment;
			glPixelStorei(GL_UNPACK_ALIGNMENT, ngl_glrowalignment);
			glPixelStorei(GL_PACK_ALIGNMENT, ngl_glrowalignment);
		}
	} else {
		glPixelStorei(GL_UNPACK_ALIGNMENT, ngl_glrowalignment);
		glPixelStorei(GL_PACK_ALIGNMENT, ngl_glrowalignment);
	}

	if(nglCatchOpenGLError(F_NGLTEXUPDATETOGL)) // Ошибка установки GL_(UN)PACK_ALIGNMENT
		return false;

	gltype = nglTexGetgltype(tex->nglcolorformat);
	glformat = nglTexGetglformat(tex->nglcolorformat);

	glBindTexture(GL_TEXTURE_2D, tex->glid);

	if(nglCatchOpenGLError(F_NGLTEXUPDATETOGL)) // Ошибка выбора текстуры
		return false;

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->sizex, tex->sizey, glformat, gltype, tex->buffer);

	if(nglCatchOpenGLError(F_NGLTEXUPDATETOGL)) // Ошибка обновления текстуры
		return false;

	glFlush(); // Нужно при обновлении текстуры из отдельного потока

	if(nglIsMainRContext()) {
		if(ngl_batch_currenttex)
			glBindTexture(GL_TEXTURE_2D,nglTexGetGLid(ngl_batch_currenttex)); // Устанавливаю старую текстуру
	}

	nglCatchOpenGLError(F_NGLTEXUPDATETOGL);

	return true;
}

/*
	Функция	: nglUpdateTexture

	Описание: Загружает текстуру

	История	: 12.02.13	Создан

*/
N_API bool N_APIENTRY_EXPORT nglUpdateTexture(unsigned int texid, unsigned char *buffer)
{
	bool success = true;

	if(!ngl_isinit) return false;

	// Проверка наличия и создание контекста OpenGL в случае необходимости
	if(!nglAddSeparateRContextIfNeeded()) {
		ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLUPDATETEXTURE, N_FALSE);
		return 0;
	}

	ngl_ea->nLockMutex(ngl_textures_sync_mutex);
		if((texid == 0) || (texid > ngl_maxtextures)) {
			ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
			return false;
		}

		if(ngl_textures[texid-1].status != NGL_TEX_STATUS_LOADED) {
			ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);
			return false;
		}

		ngl_textures[texid-1].bufmayfree = false;
		ngl_textures[texid-1].buffer = buffer;

		success = nglTexConvertToSupported(&ngl_textures[texid-1]);
		//ngl_ea->nlPrint(L"nglTexConvertToSupported %d",(int)success);

		if(success) success = nglTexUpdateToGL(&ngl_textures[texid-1]);
		//ngl_ea->nlPrint(L"nglTexLoadToGL %d",(int)success);

		if(ngl_textures[texid-1].bufmayfree)
			ngl_ea->nFreeMemory(ngl_textures[texid-1].buffer);
	ngl_ea->nUnlockMutex(ngl_textures_sync_mutex);

	if(success) {
		return true;
	} else
		return false;
}
