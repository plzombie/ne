/*
	Файл	: ngl_batch.с

	Описание: Вывод графики, 2d/3d проекция

	История	: 09.05.14	Создан

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "ngl.h"
#include "ngl_text.h"

#include "../forks/tinygl/src/zgl.h"
#include "../forks/tinygl/examples/glu.h"

#include "../nyan/nyan_texformat_publicapi.h"
#include "../nyan/nyan_draw_publicapi.h"

#include "ngl_init.h"
#include "ngl_batch.h"
#include "ngl_texture.h"

static float ngl_global_ambient[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

int ngl_batch_state;

N_API void N_APIENTRY_EXPORT nglBatch2dEnd(void);
N_API void N_APIENTRY_EXPORT nglBatch3dEnd(void);

/*
	Функция	: nglBatch2dDraw

	Описание: Рисует 2d

	История	: 09.05.14	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch2dDraw(void)
{
	if(ngl_batch_state != NGL_BATCH_STATE_2D || !ngl_isinit) return false;
	
	return true;
}

/*
	Функция	: nglBatch2dAdd

	Описание: Добавляет примитив

	История	: 09.05.14	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch2dAdd(int batch2d_type, unsigned int batch_currenttex, unsigned int vertices, nv_2dvertex_type *varray)
{
	unsigned int i;

	if(ngl_batch_state != NGL_BATCH_STATE_2D || !ngl_isinit) return false;

	if(batch_currenttex) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, batch_currenttex);
	} else
		glDisable(GL_TEXTURE_2D);

	glPushMatrix();
		switch(batch2d_type) {
			case NV_DRAWPOINT:
				glBegin(GL_POINTS);
				break;
			case NV_DRAWLINE:
				glBegin(GL_LINES);
				break;
			case NV_DRAWTRIANGLE:
				glBegin(GL_TRIANGLES);
				break;
			default:
				return false;
		}
		for(i = 0; i < vertices; i++) {
			glColor4f((varray[i].colorRGBA&0xff)/255.0f, ((varray[i].colorRGBA>>8)&0xff)/255.0f, ((varray[i].colorRGBA>>16)&0xff)/255.0f, ((varray[i].colorRGBA>>24)&0xff)/255.0f);
			glTexCoord2f(varray[i].tx, varray[i].ty);
			glVertex3f(varray[i].x, varray[i].y, varray[i].z);
		}
		glEnd();
	glPopMatrix();
	
	return true;
}

/*
	Функция	: nglBatch3dDrawMesh

	Описание: Рисует 3d модель

	История	: 09.05.14	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch3dDrawMesh(unsigned int batch_currenttex, unsigned int vertices, nv_3dvertex_type *varray)
{
	unsigned int i;

	if(ngl_batch_state != NGL_BATCH_STATE_3D || !ngl_isinit || vertices == 0) return false;
	
	if(batch_currenttex) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, batch_currenttex);
	} else
		glDisable(GL_TEXTURE_2D);

	glBegin(GL_TRIANGLES);
	for(i = 0; i < vertices; i++) {
		glColor4f(varray[i].cr, varray[i].cg, varray[i].cb, varray[i].ca);
		glTexCoord2f(varray[i].tx, varray[i].ty);
		glNormal3f(varray[i].nx, varray[i].ny, varray[i].nz);
		glVertex3f(varray[i].x, varray[i].y, varray[i].z);
	}
	glEnd();
	
	return true;
}

/*
	Функция	: nglBatch3dDrawIndexedMesh

	Описание: Рисует 3d модель с индексированными вершинами

	История	: 28.06.16	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch3dDrawIndexedMesh(unsigned int batch_currenttex, unsigned int vertices, nv_3dvertex_type *varray, unsigned int triangles, unsigned int *iarray)
{
	unsigned int i;
	
	if(ngl_batch_state != NGL_BATCH_STATE_3D || !ngl_isinit || vertices == 0 || triangles == 0) return false;
	
	if(batch_currenttex) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, batch_currenttex);
	} else
		glDisable(GL_TEXTURE_2D);

	glBegin(GL_TRIANGLES);
	for(i = 0; i < triangles; i++) {
		unsigned int i1, i2, i3;
		
		i1 = iarray[i*3];
		
		glColor4f(varray[i1].cr, varray[i1].cg, varray[i1].cb, varray[i1].ca);
		glTexCoord2f(varray[i1].tx, varray[i1].ty);
		glNormal3f(varray[i1].nx, varray[i1].ny, varray[i1].nz);
		glVertex3f(varray[i1].x, varray[i1].y, varray[i1].z);
		
		i2 = iarray[i*3+1];
		
		glColor4f(varray[i2].cr, varray[i2].cg, varray[i2].cb, varray[i2].ca);
		glTexCoord2f(varray[i2].tx, varray[i2].ty);
		glNormal3f(varray[i2].nx, varray[i2].ny, varray[i2].nz);
		glVertex3f(varray[i2].x, varray[i2].y, varray[i2].z);
		
		i3 = iarray[i*3+2];
		
		glColor4f(varray[i3].cr, varray[i3].cg, varray[i3].cb, varray[i3].ca);
		glTexCoord2f(varray[i3].tx, varray[i3].ty);
		glNormal3f(varray[i3].nx, varray[i3].ny, varray[i3].nz);
		glVertex3f(varray[i3].x, varray[i3].y, varray[i3].z);
	}
	glEnd();
	
	return true;
}

/*
	Функция	: gluOrtho2d

	Описание: Реализация gluOrtho2d
		Формулы см https://msdn.microsoft.com/en-us/library/windows/desktop/dd373965(v=vs.85).aspx

	История	: 02.06.18	Создан
	
*/
void gluOrtho2d(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top)
{
	GLdouble n_e_a_r = -1, f_a_r = 1;
	GLfloat view_matrix[16];

	view_matrix[0] = (GLfloat)(2 / (right - left));
	view_matrix[1] = 0;
	view_matrix[2] = 0;
	view_matrix[3] = 0;

	view_matrix[4] = 0;
	view_matrix[5] = (GLfloat)(2 / (top - bottom));
	view_matrix[6] = 0;
	view_matrix[7] = 0;

	view_matrix[8] = 0;
	view_matrix[9] = 0;
	view_matrix[10] = (GLfloat)(-2 / (f_a_r - n_e_a_r));
	view_matrix[11] = 0;

	view_matrix[12] = (GLfloat)(-(right + left) / (right - left));
	view_matrix[13] = (GLfloat)(-(top + bottom) / (top - bottom));
	view_matrix[14] = (GLfloat)(-(f_a_r + n_e_a_r) / (f_a_r - n_e_a_r));
	view_matrix[15] = 1;

	glMultMatrixf(view_matrix);
}

/*
	Функция	: nglBatch2dBegin

	Описание: Начало вывода 2d графики

	История	: 09.05.14	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch2dBegin(void)
{
	if(ngl_batch_state == NGL_BATCH_STATE_2D || !ngl_isinit) return;

	if(ngl_batch_state == NGL_BATCH_STATE_3D) nglBatch3dEnd();

	ngl_batch_state = NGL_BATCH_STATE_2D;

	glDisable(GL_DEPTH_TEST);

	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2d(0, (ngl_winx)?ngl_winx:1, (ngl_winy)?ngl_winy:1, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/*
	Функция	: nglBatch2dEnd

	Описание: Конец вывода 2d графики

	История	: 09.05.14	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch2dEnd(void)
{
	if(ngl_batch_state != NGL_BATCH_STATE_2D || !ngl_isinit) return;

	nglBatch2dDraw();

	ngl_batch_state = NGL_BATCH_STATE_NO;
}

/*
Функция	: nglBatch3dSetModelviewMatrix

Описание: Устанавливает матрицу преобразования

История	: 16.01.15	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch3dSetModelviewMatrix(double *matrix)
{
	float fmatrix[16];
	unsigned int i;

	if (!ngl_isinit) return false;
	if (ngl_batch_state == NGL_BATCH_STATE_NO) return false;

	glLoadIdentity();
	
	for(i = 0; i < 16; i++)
		fmatrix[i] = (float)matrix[i];

	glMultMatrixf(fmatrix);

	return true;
}

/*
	Функция	: nglBatch3dSetAmbientLight

	Описание: Устанавливает глобальное освещение
	
	История	: 09.05.14	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch3dSetAmbientLight(float r, float g, float b, float a)
{
	if(!ngl_isinit) return;
	
	ngl_global_ambient[0] = r;
	ngl_global_ambient[1] = g;
	ngl_global_ambient[2] = b;
	ngl_global_ambient[3] = a;
	
	if(ngl_batch_state == NGL_BATCH_STATE_3D) glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ngl_global_ambient);
}

/*
	Функция	: nglBatch3dBegin

	Описание: Начало вывода 3d графики

	История	: 09.05.14	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch3dBegin(void)
{
	if(ngl_batch_state == NGL_BATCH_STATE_3D || !ngl_isinit) return;

	if(ngl_batch_state == NGL_BATCH_STATE_2D) nglBatch2dEnd();

	ngl_batch_state = NGL_BATCH_STATE_3D;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (ngl_winx != 0 && ngl_winy != 0)?((GLdouble)ngl_winx/(GLdouble)ngl_winy):(1.0), 2.0, 4000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ngl_global_ambient);

	glClear(GL_DEPTH_BUFFER_BIT);
}

/*
	Функция	: nglBatch3dEnd

	Описание: Конец вывода 3d графики

	История	: 09.05.14	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch3dEnd(void)
{
	if(ngl_batch_state != NGL_BATCH_STATE_3D || !ngl_isinit) return;

	ngl_batch_state = NGL_BATCH_STATE_NO;
}

/*
	Функция	: nglReadScreen

	Описание: Читает изображение с экрана

	История	: 09.05.14	Создан

*/
N_API bool N_APIENTRY_EXPORT nglReadScreen(unsigned char *buffer)
{
	unsigned int i, j;
	
	if (!ngl_isinit) return false;

	if(ngl_batch_state == NGL_BATCH_STATE_2D) nglBatch2dEnd();
	if(ngl_batch_state == NGL_BATCH_STATE_3D) nglBatch3dEnd();
	
	// Вывод в buffer содержимого экрана. Формат ngl_winx x ngl_winy x RGBA
	for(i = 0; i < ngl_winy; i++) {
		unsigned int k;
		
		k = (ngl_winy-i-1)*ngl_glvpy/ngl_winy;
		for(j = 0; j < ngl_winx; j++) {
			unsigned int l;
			unsigned char *pin, *pout;
			unsigned char b, g, r, a;
			
			l = j*ngl_glvpx/ngl_winx;
			pin = buffer+i*4*ngl_winx+j*4;
			pout = (unsigned char *)(ngl_tinygl_context->zb->pbuf)+k*4*ngl_glvpx+l*4;
			b = *pout; pout++;
			g = *pout; pout++;
			r = *pout; pout++;
			a = *pout; 
			*pin = r; pin++;
			*pin = g; pin++;
			*pin = b; pin++;
			*pin = a;
		}
	}
	
	return true;
}
