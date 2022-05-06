#include "softgl.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
	unsigned int sizex, sizey;
	unsigned char *buffer;
	bool isused;
} sgl_texture_type;

static sgl_texture_type *sgl_textures;
static unsigned int sgl_maxtextures;

static unsigned char *sgl_surface[2];
static unsigned int sgl_sx, sgl_sy, sgl_scolortype, sgl_scurrent = 0;
static unsigned int sgl_clippingregion_sx, sgl_clippingregion_sy, sgl_clippingregion_ex, sgl_clippingregion_ey;
static bool sgl_surfinit = false;
static unsigned int sgl_curtexid;

#if defined(_MSC_VER) && !defined(__POCC__)
	#define inline __inline
#endif

#define SOFTGL_FTOUCHAR(v) (unsigned char)( ((v) > 1.0)?255:( ((v) < 0.0)?0:((int)(v*255)) ) )

SGL_API bool SGL_APIENTRY_EXPORT sglCreateSurface(unsigned int sx, unsigned int sy, unsigned int scolortype)
{
	if(sgl_surfinit) {
		free(sgl_surface[0]);
		free(sgl_surface[1]);
		sgl_surfinit = false;
	}

	sgl_clippingregion_sx = 0;
	sgl_clippingregion_sy = 0;
	sgl_clippingregion_ex = sgl_sx = sx;
	sgl_clippingregion_ey = sgl_sy = sy;
	sgl_scolortype = scolortype;

	if(scolortype == SGL_COLORFORMAT_R8G8B8A8) {
		sgl_surface[0] = malloc(sx*sy*4);
		if(!sgl_surface[0]) return false;
		
		sgl_surface[1] = malloc(sx*sy*4);
		if(!sgl_surface[1]) {
			free(sgl_surface[0]);
			return false;
		}

		sgl_surfinit = true;
		
		return true;
	}

	return false;
}

SGL_API bool SGL_APIENTRY_EXPORT sglGetSurface(unsigned int *sx, unsigned int *sy, unsigned int *scolortype, unsigned char **surface)
{
	if(!sgl_surfinit) return false;

	if(sgl_scurrent)
		*surface = sgl_surface[0];
	else
		*surface = sgl_surface[1];
	
	*sx = sgl_sx;
	*sy = sgl_sy;
	*scolortype = sgl_scolortype;
	
	return true;
}

SGL_API bool SGL_APIENTRY_EXPORT sglDestroySurface(void)
{
	if(!sgl_surfinit) return false;

	sgl_surfinit = false;

	free(sgl_surface[0]);
	free(sgl_surface[1]);

	return true;
}

SGL_API bool SGL_APIENTRY_EXPORT sglFlipBuffers(void)
{
	if(!sgl_surfinit) return false;

	if(sgl_scurrent)
		sgl_scurrent = 0;
	else
		sgl_scurrent = 1;

	return true;
}

SGL_API bool SGL_APIENTRY_EXPORT sglClearSurface(float cr, float cg, float cb, float ca)
{
	unsigned char r, g, b, a;
	unsigned int color;

	if(!sgl_surfinit) return false;

	r = SOFTGL_FTOUCHAR(cr);
	g = SOFTGL_FTOUCHAR(cg);
	b = SOFTGL_FTOUCHAR(cb);
	a = SOFTGL_FTOUCHAR(ca);

	color = ((a*256+b)*256+g)*256+r;

	if(sgl_scolortype == SGL_COLORFORMAT_R8G8B8A8) {
		unsigned int i, *p;
		
		p = (unsigned int *)sgl_surface[sgl_scurrent];
		for(i = 0; i < sgl_sx*sgl_sy; i++) {
			*p = color;
			p++;
		}
		return true;
	}

	return false;
}

SGL_API void SGL_APIENTRY_EXPORT sglSetClippingRegion(unsigned int sx, unsigned int sy, unsigned int ex, unsigned int ey)
{
	sgl_clippingregion_sx = sx;
	sgl_clippingregion_sy = sy;
	sgl_clippingregion_ex = ex;
	sgl_clippingregion_ey = ey;
}

#include <stdio.h>

static unsigned int sglGetTexel(float tx, float ty)
{
	int x, y; sgl_texture_type *tex;

	if(sgl_curtexid == 0 || sgl_curtexid > sgl_maxtextures) return 0xFFffFFff;
		
	if(!sgl_textures[sgl_curtexid-1].isused) return 0xFFffFFff;
		
	////if(tx > 1.0 || ty > 1.0) {
		////printf("%f %f\n",tx,ty);
	////}

	tex = sgl_textures+sgl_curtexid-1;

	x = (int)(tx*(tex->sizex-1));
	y = (int)(ty*(tex->sizey-1));

	if(x < 0) x = 0; if(x > (int)(tex->sizex-1)) x = (int)(tex->sizex-1);
	if(y < 0) y = 0; if(y > (int)(tex->sizey-1)) y = (int)(tex->sizey-1);

	return ((unsigned int *)(tex->buffer))[x+y*tex->sizex];
}

static void sglDraw2dPixel(unsigned int *surf, unsigned int color)
{
	unsigned int c1, c2, c3, c4;

	if((color >> 24) < 255) {
		c1 = ( (color&0xFF)*((color>>24)&0xFF)+((*surf)&0xFF)*(255-((color>>24)&0xFF)) )/255;
		c2 = ( ((color>>8)&0xFF)*((color>>24)&0xFF)+(((*surf)>>8)&0xFF)*(255-((color>>24)&0xFF)) )/255;
		c3 = ( ((color>>16)&0xFF)*((color>>24)&0xFF)+(((*surf)>>16)&0xFF)*(255-((color>>24)&0xFF)) )/255;
		c4 = ( ((color>>24)&0xFF)*((color>>24)&0xFF)+(((*surf)>>24)&0xFF)*(255-((color>>24)&0xFF)) )/255;
		*surf = c1+(c2<<8)+(c3<<16)+(c4<<24);
	} else {
		*surf = color;
	}
}


static void sglDraw2dPoint(int x, int y, float tx, float ty, unsigned int color)
{
	if(x < (int)sgl_clippingregion_ex && y < (int)sgl_clippingregion_ey && x >= (int)sgl_clippingregion_sx && y >= (int)sgl_clippingregion_sy) {
		unsigned int *surf;

		if( !(color & 0xFF000000) ) return; // Альфатест ^_^

		surf = (unsigned int *)sgl_surface[sgl_scurrent];

		if(sgl_curtexid) {
			unsigned int texcolor; // Цвет текселя
			unsigned int nc1, nc2, nc3, nc4;

			texcolor = sglGetTexel(tx, ty);

			if( !(texcolor & 0xFF000000) ) return; // Альфатест ^_^

			nc1 = ((color&0xFF)*(texcolor&0xFF))/256;
			nc2 = (((color&0xFF00)>>8)*((texcolor&0xFF00)))/256;
			nc3 = (((color&0xFF0000)>>16)*((texcolor&0xFF0000)>>16))/256;
			nc4 = (((color&0xFF000000)>>24)*((texcolor&0xFF000000)>>24))/256;

			sglDraw2dPixel(surf+sgl_sx*y+x, nc1+(nc2<<8)+(nc3<<16)+(nc4<<24));
		} else {
			sglDraw2dPixel(surf+sgl_sx*y+x, color);
		}
	}
}

static void sglDraw2dLine(int x, int y, int x2, int y2, float tx, float ty, float tx2, float ty2, unsigned int color, unsigned int color2)
{
	if(x == x2 && y == y2) {
		sglDraw2dPoint(x, y, tx, ty, color);

		return;
	}

	if((x < (int)sgl_clippingregion_ex && y < (int)sgl_clippingregion_ey && x2 >= (int)sgl_clippingregion_sx && y2 >= (int)sgl_clippingregion_sy) ||
		(x2 < (int)sgl_clippingregion_ex && y2 < (int)sgl_clippingregion_ey && x >= (int)sgl_clippingregion_sx && y >= (int)sgl_clippingregion_sy)) {

		int deltax, deltay, error, signx, signy, j, k; // Для алгоритма Брезенхема
		double deltaxy; // Длина линии
		unsigned int color_r, color_g, color_b, color_a;
		unsigned int color2_r, color2_g, color2_b, color2_a;
		unsigned int curcolor_r, curcolor_g, curcolor_b, curcolor_a, curtx, curty;
		unsigned int *surf;
		int delta_r, delta_g, delta_b, delta_a, delta_tx, delta_ty;

		if(x < x2) signx = 1; else signx = -1;
		if(y < y2) signy = 1; else signy = -1;

		deltax = abs(x2-x);
		deltay = abs(y2-y);
		deltaxy = (float)sqrt((x2-x)*(x2-x)+(y2-y)*(y2-y));

		surf = (unsigned int *)sgl_surface[sgl_scurrent]+sgl_sx*y+x;

		color_r = color&0xff;
		color_g = (color>>8)&0xff;
		color_b = (color>>16)&0xff;
		color_a = (color>>24)&0xff;

		color2_r = color2&0xff;
		color2_g = (color2>>8)&0xff;
		color2_b = (color2>>16)&0xff;
		color2_a = (color2>>24)&0xff;

		error = 0;

		curcolor_r = color_r*256;
		curcolor_g = color_g*256;
		curcolor_b = color_b*256;
		curcolor_a = color_a*256;

		curtx = (int)(tx*16777216);
		curty = (int)(ty*16777216);

		delta_r = ((int)color2_r-(int)color_r)*256/(int)deltaxy;
		delta_g = ((int)color2_g-(int)color_g)*256/(int)deltaxy;
		delta_b = ((int)color2_b-(int)color_b)*256/(int)deltaxy;
		delta_a = ((int)color2_a-(int)color_a)*256/(int)deltaxy;

		delta_tx = (int)((tx2-tx)*16777216)/(int)deltaxy;
		delta_ty = (int)((ty2-ty)*16777216)/(int)deltaxy;

		if(deltax >= deltay) {
			for(j = x, k = y; j != x2+signx; j += signx) {
				if(j >= (int)sgl_clippingregion_sx && k >= (int)sgl_clippingregion_sy && j < (int)sgl_clippingregion_ex && k < (int)sgl_clippingregion_ey) {
					unsigned int newcolor_r, newcolor_g, newcolor_b, newcolor_a;

					newcolor_r = curcolor_r;
					newcolor_g = curcolor_g;
					newcolor_b = curcolor_b;
					newcolor_a = curcolor_a;

					if(sgl_curtexid) {
						unsigned int texcolor;

						texcolor = sglGetTexel((float)curtx/16777216.0f, (float)curty/16777216.0f);

						newcolor_r = (newcolor_r*(texcolor&0xFF))/256;
						newcolor_g = (newcolor_g*((texcolor&0xFF00)>>8))/256;
						newcolor_b = (newcolor_b*((texcolor&0xFF0000)>>16))/256;
						newcolor_a = (newcolor_a*((texcolor&0xFF000000)>>24))/256;
					}

					if(newcolor_a) sglDraw2dPixel(surf, newcolor_r/256+(newcolor_g&0xff00)+((newcolor_b*256)&0xff0000)+((newcolor_a*65536)&0xff000000)); // Альфатест ^_^
				}

				curcolor_r += delta_r;
				curcolor_g += delta_g;
				curcolor_b += delta_b;
				curcolor_a += delta_a;

				curtx += delta_tx;
				curty += delta_ty;

				if(signx > 0)
					surf++;
				else
					surf--;
				error += deltay;

				if(2*error > deltax) {
					if(signy > 0)
						surf += sgl_sx;
					else
						surf -= sgl_sx;
					k += signy;
					error -= deltax;
				}
			}
		} else {
			for(j = x, k = y; k != y2+signy; k += signy) {
				if(j >= (int)sgl_clippingregion_sx && k >= (int)sgl_clippingregion_sy && j < (int)sgl_clippingregion_ex && k < (int)sgl_clippingregion_ey) {
					unsigned int newcolor_r, newcolor_g, newcolor_b, newcolor_a;

					newcolor_r = curcolor_r;
					newcolor_g = curcolor_g;
					newcolor_b = curcolor_b;
					newcolor_a = curcolor_a;

					if(sgl_curtexid) {
						unsigned int texcolor;

						texcolor = sglGetTexel((float)curtx/16777216.0f, (float)curty/16777216.0f);

						newcolor_r = (newcolor_r*(texcolor&0xFF))/256;
						newcolor_g = (newcolor_g*((texcolor&0xFF00)>>8))/256;
						newcolor_b = (newcolor_b*((texcolor&0xFF0000)>>16))/256;
						newcolor_a = (newcolor_a*((texcolor&0xFF000000)>>24))/256;
					}

					if(newcolor_a) sglDraw2dPixel(surf, newcolor_r/256+(newcolor_g&0xff00)+((newcolor_b*256)&0xff0000)+((newcolor_a*65536)&0xff000000)); // Альфатест ^_^
				}

				curcolor_r += delta_r;
				curcolor_g += delta_g;
				curcolor_b += delta_b;
				curcolor_a += delta_a;

				curtx += delta_tx;
				curty += delta_ty;
				
				if(signy > 0)
					surf += sgl_sx;
				else
					surf -= sgl_sx;
				error += deltax;

				if(2*error > deltay) {
					if(signx > 0)
						surf++;
					else
						surf--;
					j += signx;
					error -= deltay;
				}
			}
		}
	}
}

static void sglDraw2dTriangle(int x, int y, int x2, int y2, int x3, int y3, float tx, float ty, float tx2, float ty2, float tx3, float ty3, unsigned int color, unsigned int color2, unsigned int color3)
{
	int j, k, l;
	unsigned int deltax13, deltax12, deltax23, deltay13, deltay12, deltay23, error1, error2;
	float deltaxy12, deltaxy13, deltaxy23;
	int signx13, signx12, signx23;
	unsigned int c_1, c_2, c_3, c_4, c2_1, c2_2, c2_3, c2_4; // Разложение цвета пикселя по 4-м компонентам
	int dc_1, dc_2, dc_3, dc_4, dc2_1, dc2_2, dc2_3, dc2_4; // Изменение значения цвета следующего пикселя относительно предыдущего
	int tex_x, tex_y, tex2_x, tex2_y, dt_x, dt_y, dt2_x, dt2_y;
	
	unsigned int newcolor, newcolor2; // Итоговый цвет пикселя

	// Если любые 2 координаты совпадают, то рисуем сразу линию
	if(x == x2 && y == y2) {
		sglDraw2dLine(x, y, x3, y3, tx, ty, tx3, ty3, color, color3);

		return;
	}
	if((x == x3 && y == y3) || (x2 == x3 && y2 == y3)) {
		sglDraw2dLine(x, y, x2, y2, tx, ty, tx2, ty2, color, color2);

		return;
	}

	//Упорядочиваем точки
	if(y2 < y) {
		int temp;

		temp = y; y = y2; y2 = temp;
		temp = x; x = x2; x2 = temp;
	}
	if(y3 < y) {
		int temp;

		temp = y; y = y3; y3 = temp;
		temp = x; x = x3; x3 = temp;
	}
	if(y2 > y3) {
		int temp;

		temp = y2; y2 = y3; y3 = temp;
		temp = x2; x2 = x3; x3 = temp;
	}

	error1 = 0; error2 = 0;

	if(x < x2) signx12 = 1; else signx12 = -1;
	if(x < x3) signx13 = 1; else signx13 = -1;
	if(x2 < x3) signx23 = 1; else signx23 = -1;

	deltax12 = abs(x2-x);
	deltay12 = y2-y;
	deltaxy12 = (float)sqrt((x2-x)*(x2-x)+(y2-y)*(y2-y));
	deltax13 = abs(x3-x);
	deltay13 = y3-y;
	deltaxy13 = (float)sqrt((x3-x)*(x3-x)+(y3-y)*(y3-y));
	deltax23 = abs(x3-x2);
	deltay23 = y3-y2;
	deltaxy23 = (float)sqrt((x3-x2)*(x3-x2)+(y3-y2)*(y3-y2));

	c_1 = (color&0xff)<<8;
	c_2 = color&0xff00;
	c_3 = (color>>8)&0xff00;
	c_4 = (color>>16)&0xff00;

	c2_1 = c_1;
	c2_2 = c_2;
	c2_3 = c_3;
	c2_4 = c_4;

	tex_x = (int)(tx*4096);
	tex_y = (int)(ty*4096);
	tex2_x = tex_x;
	tex2_y = tex_y;

	// Цвет, на который изменяется пиксель при переходе от точки 1 к точке 2 по y
	
	if(deltay12 != 0 && deltaxy12 != 0) {
		dc_1 = (int)( sqrt(65536+((deltax12*256)/deltay12)*((deltax12*256)/deltay12))/deltaxy12*(int)((color2&0xff)-(color&0xff)) );
		dc_2 = (int)( sqrt(65536+((deltax12*256)/deltay12)*((deltax12*256)/deltay12))/deltaxy12*(int)(((color2>>8)&0xff)-((color>>8)&0xff)) );
		dc_3 = (int)( sqrt(65536+((deltax12*256)/deltay12)*((deltax12*256)/deltay12))/deltaxy12*(int)(((color2>>16)&0xff)-((color>>16)&0xff)) );
		dc_4 = (int)( sqrt(65536+((deltax12*256)/deltay12)*((deltax12*256)/deltay12))/deltaxy12*(int)(((color2>>24)&0xff)-((color>>24)&0xff)) );
		if(sgl_curtexid) {
			dt_x = (int)( sqrt(16777216+((deltax12*4096)/deltay12)*((deltax12*4096)/deltay12))/deltaxy12*(tx2-tx) );
			dt_y = (int)( sqrt(16777216+((deltax12*4096)/deltay12)*((deltax12*4096)/deltay12))/deltaxy12*(ty2-ty) );
		}
	} else {
		dc_1 = 0;
		dc_2 = 0;
		dc_3 = 0;
		dc_4 = 0;
		dt_x = 0;
		dt_y = 0;
	}

	// Цвет, на который изменяется пиксель при переходе от точки 1 к точке 3 по y
	if(deltay13 != 0 && deltaxy13 != 0) {
		dc2_1 = (int)( sqrt(65536+((deltax13*256)/deltay13)*((deltax13*256)/deltay13))/deltaxy13*(int)((color3&0xff)-(color&0xff)) );
		dc2_2 = (int)( sqrt(65536+((deltax13*256)/deltay13)*((deltax13*256)/deltay13))/deltaxy13*(int)(((color3>>8)&0xff)-((color>>8)&0xff)) );
		dc2_3 = (int)( sqrt(65536+((deltax13*256)/deltay13)*((deltax13*256)/deltay13))/deltaxy13*(int)(((color3>>16)&0xff)-((color>>16)&0xff)) );
		dc2_4 = (int)( sqrt(65536+((deltax13*256)/deltay13)*((deltax13*256)/deltay13))/deltaxy13*(int)(((color3>>24)&0xff)-((color>>24)&0xff)) );
		if(sgl_curtexid) {
			dt2_x = (int)( sqrt(16777216+((deltax13*4096)/deltay13)*((deltax13*4096)/deltay13))/deltaxy13*(tx3-tx) );
			dt2_y = (int)( sqrt(16777216+((deltax13*4096)/deltay13)*((deltax13*4096)/deltay13))/deltaxy13*(ty3-ty) );
		}
	} else {
		dc2_1 = 0;
		dc2_2 = 0;
		dc2_3 = 0;
		dc2_4 = 0;
		dt2_x = 0;
		dt2_y = 0;
	}

	//printf("dx12 %d dy12 %d dx13 %d dy13 %d dx23 %d dy23 %d\n",deltax12,deltay12,deltax13,deltay13,deltax23,deltay23);

	k = x;
	l = x;

	newcolor = color;
	newcolor2 = color;

	if(y == y2) {
		sglDraw2dLine(x, y, x2, y2, tx, ty, tx2, ty2, color, color2);
	} else {
		for(j = y; j < y2; j++) {
			if(sgl_curtexid) {
				sglDraw2dLine(k, j, l, j, tex_x/4096.0f, tex_y/4096.0f, tex2_x/4096.0f, tex2_y/4096.0f, newcolor, newcolor2);

				tex_x += dt_x;
				tex_y += dt_y;
				tex2_x += dt2_x;
				tex2_y += dt2_y;
			} else {
				sglDraw2dLine(k, j, l, j, 1.0, 1.0, 1.0, 1.0, newcolor, newcolor2);
			}
			error1 += deltax12;
			error2 += deltax13;

			c_1 += dc_1;
			c_2 += dc_2;
			c_3 += dc_3;
			c_4 += dc_4;
			newcolor = ((c_4&0xff00)<<16)+((c_3&0xff00)<<8)+(c_2&0xff00)+(c_1>>8);

			c2_1 += dc2_1;
			c2_2 += dc2_2;
			c2_3 += dc2_3;
			c2_4 += dc2_4;
			newcolor2 = ((c2_4&0xff00)<<16)+((c2_3&0xff00)<<8)+(c2_2&0xff00)+(c2_1>>8);

			

			while(error1 >= deltay12 && deltay12 != 0) {
				error1 -= deltay12;
				k += signx12;
			}
			while(error2 >= deltay13 && deltay13 != 0) {
				error2 -= deltay13;
				l += signx13;
			}
		}
	}

	error1 = 0;
	k = x2;

	c_1 = (color2&0xff)<<8;
	c_2 = color2&0xff00;
	c_3 = (color2>>8)&0xff00;
	c_4 = (color2>>16)&0xff00;

	tex_x = (int)(tx2*4096);
	tex_y = (int)(ty2*4096);

	if(deltay23 != 0 && deltaxy23 != 0) {
		dc_1 = (int)( sqrt(65536+((deltax23*256)/deltay23)*((deltax23*256)/deltay23))/deltaxy23*(int)((color3&0xff)-(color2&0xff)) );
		dc_2 = (int)( sqrt(65536+((deltax23*256)/deltay23)*((deltax23*256)/deltay23))/deltaxy23*(int)(((color3>>8)&0xff)-((color2>>8)&0xff)) );
		dc_3 = (int)( sqrt(65536+((deltax23*256)/deltay23)*((deltax23*256)/deltay23))/deltaxy23*(int)(((color3>>16)&0xff)-((color2>>16)&0xff)) );
		dc_4 = (int)( sqrt(65536+((deltax23*256)/deltay23)*((deltax23*256)/deltay23))/deltaxy23*(int)(((color3>>24)&0xff)-((color2>>24)&0xff)) );
		if(sgl_curtexid) {
			dt_x = (int)( sqrt(16777216+((deltax23*4096)/deltay23)*((deltax23*4096)/deltay23))/deltaxy23*(tx3-tx2) );
			dt_y = (int)( sqrt(16777216+((deltax23*4096)/deltay23)*((deltax23*4096)/deltay23))/deltaxy23*(ty3-ty2) );
		}
	} else {
		dc_1 = 0;
		dc_2 = 0;
		dc_3 = 0;
		dc_4 = 0;
		dt_x = 0;
		dt_y = 0;
	}

	newcolor = color2;

	if(y2 == y3) {
		sglDraw2dLine(x2, y2, x3, y3, tx2, ty2, tx3, ty3, color2, color3);
	} else {
		for(j = y2; j < y3; j++) {
			if(sgl_curtexid) {
				sglDraw2dLine(k, j, l, j, tex_x/4096.0f, tex_y/4096.0f, tex2_x/4096.0f, tex2_y/4096.0f, newcolor, newcolor2);
				
				tex_x += dt_x;
				tex_y += dt_y;
				tex2_x += dt2_x;
				tex2_y += dt2_y;
			} else {
				sglDraw2dLine(k, j, l, j, 1.0, 1.0, 1.0, 1.0, newcolor, newcolor2);
			}
			error1 += deltax23;
			error2 += deltax13;

			c_1 += dc_1;
			c_2 += dc_2;
			c_3 += dc_3;
			c_4 += dc_4;
			newcolor = ((c_4&0xff00)<<16)+((c_3&0xff00)<<8)+(c_2&0xff00)+(c_1>>8);

			c2_1 += dc2_1;
			c2_2 += dc2_2;
			c2_3 += dc2_3;
			c2_4 += dc2_4;
			newcolor2 = ((c2_4&0xff00)<<16)+((c2_3&0xff00)<<8)+(c2_2&0xff00)+(c2_1>>8);

			while(error1 >= deltay23 && deltay23 != 0) {
				error1 -= deltay23;
				k += signx23;
			}
			while(error2 >= deltay13 && deltay13 != 0) {
				error2 -= deltay13;
				l += signx13;
			}
		}
	}

	

	//sglDraw2dLine(x, y, x2, y2, NV_COLOR(255,0,255,0), NV_COLOR(255,0,255,0));
	//sglDraw2dLine(x2, y2, x3, y3, NV_COLOR(255,0,255,0), NV_COLOR(255,0,255,0));
	//sglDraw2dLine(x3, y3, x, y, NV_COLOR(255,0,255,0), NV_COLOR(255,0,255,0));
	
	/*unsigned int *surf;

	surf = (unsigned int *)sgl_surface[sgl_scurrent]+sgl_sx*y+x; *surf = color;
	surf = (unsigned int *)sgl_surface[sgl_scurrent]+sgl_sx*y2+x2; *surf = color2;
	surf = (unsigned int *)sgl_surface[sgl_scurrent]+sgl_sx*y3+x3; *surf = color3;*/
}

SGL_API bool SGL_APIENTRY_EXPORT sglDraw2d(int batch2d_type, unsigned int batch_currenttex, unsigned int vertices,sgl_2dvertex_type *varray)
{
	unsigned int i, color, color2, color3;
	int x, y, x2, y2, x3, y3;
	float tx, ty, tx2, ty2, tx3, ty3;

	if(!sgl_surfinit) return false;

	sgl_curtexid = batch_currenttex;

	switch(batch2d_type) {
		case SGL_DRAWPOINT:
			for(i = 0; i < vertices; i++) {
				x = (int)varray->x;
				y = (int)varray->y;
				tx = varray->tx;
				ty = varray->ty;
				color = varray->colorRGBA;
				varray++;

				sglDraw2dPoint(x, y, tx, ty, color);
			}

			return true;
		case SGL_DRAWLINE:
			for(i = 0; i < vertices/2; i++) {
				// Рисование линии алгоритмом брезенхема
				// Получение информации о вершинах из массива
				x = (int)varray->x;
				y = (int)varray->y;
				tx = varray->tx;
				ty = varray->ty;
				color = varray->colorRGBA;
				varray++;
				x2 = (int)varray->x;
				y2 = (int)varray->y;
				tx2 = varray->tx;
				ty2 = varray->ty;
				color2 = varray->colorRGBA;
				varray++;
				
				sglDraw2dLine(x, y, x2, y2, tx, ty, tx2, ty2, color, color2);
			}
			return true;
		case SGL_DRAWTRIANGLE:
			for(i = 0; i < vertices/3; i++) {
				// Рисование треугольников
				// Получение информации о вершинах из массива
				x = (int)varray->x;
				y = (int)varray->y;
				tx = varray->tx;
				ty = varray->ty;
				color = varray->colorRGBA;
				varray++;
				x2 = (int)varray->x;
				y2 = (int)varray->y;
				tx2 = varray->tx;
				ty2 = varray->ty;
				color2 = varray->colorRGBA;
				varray++;
				x3 = (int)varray->x;
				y3 = (int)varray->y;
				tx3 = varray->tx;
				ty3 = varray->ty;
				color3 = varray->colorRGBA;
				varray++;
				
				sglDraw2dTriangle(x, y, x2, y2, x3, y3, tx, ty, tx2, ty2, tx3, ty3, color, color2, color3);
			}
			return true;
		default:
			return false;
	}
}

SGL_API unsigned int SGL_APIENTRY_EXPORT sglCreateTexture(unsigned int sx, unsigned int sy, unsigned int scolortype, unsigned int rowalignment, unsigned char *buffer)
{
	unsigned int curtex;

	if(sgl_scolortype == SGL_COLORFORMAT_R8G8B8A8) {
		if(!( ( (scolortype == SGL_COLORFORMAT_R8G8B8A8 || scolortype == SGL_COLORFORMAT_B8G8R8A8 || scolortype == SGL_COLORFORMAT_A8B8G8R8) && (rowalignment == 4 || rowalignment == 1)) 
			|| ( (scolortype == SGL_COLORFORMAT_R8G8B8 || scolortype == SGL_COLORFORMAT_B8G8R8 || scolortype == SGL_COLORFORMAT_L8 || scolortype == SGL_COLORFORMAT_L8A8) && (rowalignment == 1 || (rowalignment == 4 && ((sx*3)%4) == 0) )) )) return false;
	} else {
		return false;
	}

	if(sx == 0 || sy == 0)
		return false;

	for(curtex = 0; curtex < sgl_maxtextures; curtex++)
		if(!sgl_textures[curtex].isused)
			break;

	if(curtex == sgl_maxtextures) {
		sgl_texture_type *_sgl_textures;

		_sgl_textures = realloc(sgl_textures,(sgl_maxtextures+1024)*sizeof(sgl_texture_type));

		if(_sgl_textures)
			sgl_textures = _sgl_textures;
		else {
			return false;
		}

		for(curtex=sgl_maxtextures; curtex<sgl_maxtextures+1024; curtex++)
			sgl_textures[curtex].isused = false;

		curtex = sgl_maxtextures;

		sgl_maxtextures += 1024;
	}

	sgl_textures[curtex].isused = true;
	sgl_textures[curtex].sizex = sx;
	sgl_textures[curtex].sizey = sy;
	sgl_textures[curtex].buffer = malloc(sx*sy*4);
	if(!sgl_textures[curtex].buffer) {
		sgl_textures[curtex].isused = false;

		return false;
	}
	if(scolortype == SGL_COLORFORMAT_R8G8B8) {
		unsigned int i;
		for(i = 0; i < sx*sy; i++) {
			sgl_textures[curtex].buffer[i*4] = buffer[i*3];
			sgl_textures[curtex].buffer[i*4+1] = buffer[i*3+1];
			sgl_textures[curtex].buffer[i*4+2] = buffer[i*3+2];
			sgl_textures[curtex].buffer[i*4+3] = 255;
		}
	} else if(scolortype == SGL_COLORFORMAT_B8G8R8) {
		unsigned int i;
		for(i = 0; i < sx*sy; i++) {
			sgl_textures[curtex].buffer[i*4] = buffer[i*3+2];
			sgl_textures[curtex].buffer[i*4+1] = buffer[i*3+1];
			sgl_textures[curtex].buffer[i*4+2] = buffer[i*3];
			sgl_textures[curtex].buffer[i*4+3] = 255;
		}
	} else if(scolortype == SGL_COLORFORMAT_B8G8R8A8) {
		unsigned int i;
		for(i = 0; i < sx*sy; i++) {
			sgl_textures[curtex].buffer[i*4] = buffer[i*4+2];
			sgl_textures[curtex].buffer[i*4+1] = buffer[i*4+1];
			sgl_textures[curtex].buffer[i*4+2] = buffer[i*4];
			sgl_textures[curtex].buffer[i*4+3] = buffer[i*4+3];
		}
	} else if(scolortype == SGL_COLORFORMAT_A8B8G8R8) {
		unsigned int i;
		for(i = 0; i < sx*sy; i++) {
			sgl_textures[curtex].buffer[i*4] = buffer[i*4+3];
			sgl_textures[curtex].buffer[i*4+1] = buffer[i*4+2];
			sgl_textures[curtex].buffer[i*4+2] = buffer[i*4+1];
			sgl_textures[curtex].buffer[i*4+3] = buffer[i*4];
		}
	} else if(scolortype == SGL_COLORFORMAT_L8) {
		unsigned int i;
		for(i = 0; i < sx*sy; i++) {
			sgl_textures[curtex].buffer[i*4] = buffer[i];
			sgl_textures[curtex].buffer[i*4+1] = buffer[i];
			sgl_textures[curtex].buffer[i*4+2] = buffer[i];
			sgl_textures[curtex].buffer[i*4+3] = 255;
		}
	} else if(scolortype == SGL_COLORFORMAT_L8A8) {
		unsigned int i;
		for(i = 0; i < sx*sy; i++) {
			sgl_textures[curtex].buffer[i*4] = sgl_textures[curtex].buffer[i*4+1] = sgl_textures[curtex].buffer[i*4+2] = buffer[i*2];
			sgl_textures[curtex].buffer[i*4+3] = buffer[i*2+1];
		}
	} else {
		memcpy(sgl_textures[curtex].buffer, buffer, sx*sy*4);
	}

	return curtex+1;
}

SGL_API bool SGL_APIENTRY_EXPORT sglDestroyTexture(unsigned int texid)
{
	if(texid == 0 || texid > sgl_maxtextures) return false;

	if(!sgl_textures[texid].isused) return false;

	sgl_textures[texid].isused = false;
	free(sgl_textures[texid].buffer);

	return true;
}

SGL_API void SGL_APIENTRY_EXPORT sglDestroyAllTextures(void)
{
	unsigned int i;

	if(!sgl_maxtextures) return;

	for(i = 0; i < sgl_maxtextures; i++)
		if(sgl_textures[i].isused)
			sglDestroyTexture(i+1);

	free(sgl_textures);

	sgl_textures = 0;
	sgl_maxtextures = 0;
}

SGL_API bool SGL_APIENTRY_EXPORT sglIsTexture(unsigned int texid)
{
	if(texid == 0 || texid > sgl_maxtextures) return false;

	if(!sgl_textures[texid].isused) return false;

	return true;
}
