/*
	Файл	: ngl_batch.с

	Описание: Вывод графики, 2d/3d проекция

	История	: 15.08.12	Создан

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define GL_GLEXT_LEGACY

#ifdef N_WINDOWS
	#include <windows.h>
#endif
#include <GL/glu.h>
#include "../forks/gl/glext.h"

#include "ngl.h"
#include "ngl_text.h"

#include "../nyan/nyan_texformat_publicapi.h"
#include "../nyan/nyan_draw_publicapi.h"

#include "ngl_init.h"
#include "ngl_batch.h"
#include "ngl_texture.h"
#include "ngl_shaders.h"

unsigned int ngl_batch_currenttex = 0; // ngl_textures[ngl_batch_currenttex-1], ngl_batch_currenttex>0
int ngl_batch_state = NGL_BATCH_STATE_NO;

static GLfloat ngl_global_ambient[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

static GLuint ngl_batch2d_vprog_npott_id = 0; // id вертексной программы, обрабатывающей текстурные координаты для текстур со стороной, не кратной двойке

static const char *ngl_batch2d_vprog_npott_text =
	"!!ARBvp1.0\n"
	//"TEMP vertexClip;\n"
	"DP4 result.position.x, state.matrix.mvp.row[0], vertex.position;\n"
	"DP4 result.position.y, state.matrix.mvp.row[1], vertex.position;\n"
	"DP4 result.position.z, state.matrix.mvp.row[2], vertex.position;\n"
	"DP4 result.position.w, state.matrix.mvp.row[3], vertex.position;\n"
	//"MOV result.position, vertexClip;\n"
	"MOV result.color, vertex.color;\n"
	"MUL result.texcoord, vertex.texcoord, program.local[0];\n"
	"END";

#define NGL_BATCH2D_MAXVERTICES 6000 // Должно делиться на 3 и на 2

nv_2dvertex_type *ngl_batch2d_varray = 0;
nv_2dvertex_type *ngl_batch2d_vcurrent = 0;
int ngl_batch2d_vertices = 0;
int ngl_batch2d_type = 0;

N_API void N_APIENTRY_EXPORT nglBatch2dEnd(void);
N_API void N_APIENTRY_EXPORT nglBatch3dEnd(void);

/*
	Функция	: nglBatchInit

	Описание: Инициализирует batching

	История	: 11.06.12	Создан

*/
bool nglBatchInit(void)
{
	if(!ngl_isinit) return false;

	ngl_ea->nlPrint(F_NGLBATCHINIT); ngl_ea->nlAddTab(1);

	ngl_batch_currenttex = 0;
	ngl_batch_state = NGL_BATCH_STATE_NO;
	ngl_batch2d_vertices = 0;
	ngl_batch2d_varray = ngl_ea->nAllocMemory(NGL_BATCH2D_MAXVERTICES*sizeof(nv_2dvertex_type));
	if(!ngl_batch2d_varray) {
		ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLBATCHINIT, N_FALSE);
		return false;
	}
	ngl_batch2d_vcurrent = ngl_batch2d_varray;

	// Загрузка вертексной программы, обрабатывающей текстурные координаты для текстур со стороной, не кратной двойке
	if(ngl_win_vertexprogram_ext)
		ngl_batch2d_vprog_npott_id = nglShaderCreateARBVertexProg(ngl_batch2d_vprog_npott_text);

	nglCatchOpenGLError(F_NGLBATCHINIT);

	ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLBATCHINIT, N_OK);

	return true;
}

/*
	Функция	: nglBatchClose

	Описание: Деинициализирует batching

	История	: 11.06.12	Создан

*/
void nglBatchClose(void)
{
	if(!ngl_isinit) return;

	ngl_ea->nlPrint(F_NGLBATCHCLOSE); ngl_ea->nlAddTab(1);

	if(ngl_batch_state == NGL_BATCH_STATE_2D) nglBatch2dEnd();
	if(ngl_batch_state == NGL_BATCH_STATE_3D) nglBatch3dEnd();

	ngl_batch_state = NGL_BATCH_STATE_NO;

	ngl_ea->nFreeMemory(ngl_batch2d_varray);
	ngl_batch2d_varray = 0;

	if(ngl_win_vertexprogram_ext) funcptr_glDeleteProgramsARB(1, &ngl_batch2d_vprog_npott_id);

	nglCatchOpenGLError(F_NGLBATCHCLOSE);

	ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLBATCHCLOSE, N_OK);
}

/*
	Функция	: nglBatchSetCurrentTexture

	Описание: Устанавливает текущую текстуру

	История	: 23.05.18	Создан

*/
void nglBatchSetCurrentTexture(unsigned int batch_currenttex)
{
	if(nglIsTex(batch_currenttex)) { // Если устанавливаем текстуру
		unsigned int glid;

		if(!ngl_batch_currenttex) { // Текущая текстура не установлена (отключена)
			glEnable(GL_TEXTURE_2D);
			//wprintf(L"glEnable(GL_TEXTURE_2D)\n");
		}

		glid = nglTexGetGLid(batch_currenttex);

		if(ngl_batch_currenttex != batch_currenttex) { // Текущая текстура не равна batch_currenttex
			glBindTexture(GL_TEXTURE_2D, glid);
			//wprintf(L"glBindTexture(%u)\n", glid);
		} else {
			GLint curglid;

			glGetIntegerv(GL_TEXTURE_BINDING_2D, &curglid);

			if(glid != (GLuint)curglid) { // Если текущая текстура OpenGL не соответствует текстуре OpenGL для batch_currenttex
				glBindTexture(GL_TEXTURE_2D, glid);
				//wprintf(L"glBindTexture(%u), curglid %u\n", glid, (unsigned int)curglid);
			}
		}

		ngl_batch_currenttex = batch_currenttex;
	} else { // Если выключаем текстуры
		if(ngl_batch_currenttex) {
			glDisable(GL_TEXTURE_2D);
			//wprintf(L"glDisable(GL_TEXTURE_2D)\n");
		}

		ngl_batch_currenttex = 0;
	}
}

/*
	Функция	: nglBatchGetCurrentTexture

	Описание: Возвращает текущую текстуру

	История	: 23.05.18	Создан

*/
unsigned int nglBatchGetCurrentTexture(void)
{
	return ngl_batch_currenttex;
}


/*
	Функция	: nglBatch2dDraw

	Описание: Рисует 2d

	История	: 11.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch2dDraw(void)
{
	unsigned int flags = 0; float scalex, scaley;
	unsigned int batch_currenttex;
	nv_2dvertex_type *p;

	if(ngl_batch_state != NGL_BATCH_STATE_2D || !ngl_isinit || ngl_batch2d_vertices == 0) return false;

	batch_currenttex = nglBatchGetCurrentTexture();

	if(batch_currenttex) {
		nglTexGetScale(batch_currenttex, &flags, &scalex, &scaley);
		if(flags & NGL_TEX_FLAGS_SCALECOORD) {
			if(ngl_win_vertexprogram_ext && ngl_batch2d_vprog_npott_id) {
				glEnable(GL_VERTEX_PROGRAM_ARB);
				funcptr_glBindProgramARB(GL_VERTEX_PROGRAM_ARB, ngl_batch2d_vprog_npott_id);
				funcptr_glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, scalex, scaley, 1.0, 1.0);
			} else {
				int i;

				p = ngl_batch2d_varray;
				for(i=0;i<ngl_batch2d_vertices;i++) {
					p->tx *= scalex;
					p->ty *= scaley;
					p++;
				}
			}
		}
	}

	//ngl_ea->nlPrint(L"bhavetex %d nglid %d glid %d",ngl_batch_havetex,ngl_batch_currenttex,nglTexGetGLid(ngl_batch_currenttex));

	glInterleavedArrays(GL_T2F_C4UB_V3F, 0, ngl_batch2d_varray);
	switch(ngl_batch2d_type) {
		case NV_DRAWPOINT:
			glDrawArrays(GL_POINTS, 0, ngl_batch2d_vertices);
			break;
		case NV_DRAWLINE:
			glDrawArrays(GL_LINES, 0, ngl_batch2d_vertices);
			break;
		case NV_DRAWTRIANGLE:
			glDrawArrays(GL_TRIANGLES, 0, ngl_batch2d_vertices);
			break;
	}

	//logPrint(L"vis_batch_havetex %d vis_batch_currenttex %d vis_batch2d_vertices %d vis_batch2d_type %d",vis_batch_havetex,vis_batch_currenttex,vis_batch2d_vertices,vis_batch2d_type);
	//for(int i = 0;i < vis_batch2d_vertices;i++)
	//	logPrint(L"tx %f ty %f colorRGBA %d x %f y %f z %f",vis_batch2d_varray[i].tx,vis_batch2d_varray[i].ty,vis_batch2d_varray[i].colorRGBA,vis_batch2d_varray[i].x,vis_batch2d_varray[i].y,vis_batch2d_varray[i].z);

	if((batch_currenttex) && (flags & NGL_TEX_FLAGS_SCALECOORD) && ngl_win_vertexprogram_ext && ngl_batch2d_vprog_npott_id)
		glDisable(GL_VERTEX_PROGRAM_ARB);

	ngl_batch2d_vcurrent = ngl_batch2d_varray;
	ngl_batch2d_vertices = 0;

	nglCatchOpenGLError(F_NGLBATCH2DDRAW);

	return true;
}

/*
	Функция	: nglBatch2dAdd

	Описание: Добавляет примитив

	История	: 11.06.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch2dAdd(int batch2d_type, unsigned int batch_currenttex, unsigned int vertices, nv_2dvertex_type *varray)
{
	if(ngl_batch_state != NGL_BATCH_STATE_2D || !ngl_isinit) return false;

	//if(vertices > NGL_BATCH2D_MAXVERTICES) return false;

	if(ngl_batch2d_type != batch2d_type || nglBatchGetCurrentTexture() != batch_currenttex || (vertices+ngl_batch2d_vertices) > NGL_BATCH2D_MAXVERTICES) nglBatch2dDraw();

	if(vertices > NGL_BATCH2D_MAXVERTICES) {
		if( nglBatch2dAdd(batch2d_type, batch_currenttex, NGL_BATCH2D_MAXVERTICES, varray) )
			return nglBatch2dAdd(batch2d_type, batch_currenttex, vertices-NGL_BATCH2D_MAXVERTICES, varray+NGL_BATCH2D_MAXVERTICES);
		else
			return false;
	} else {

		nglBatchSetCurrentTexture(batch_currenttex);

		ngl_batch2d_type = batch2d_type;
		memcpy(ngl_batch2d_vcurrent,varray,vertices*sizeof(nv_2dvertex_type));
		//memcpy_s(ngl_batch2d_vcurrent,NGL_BATCH2D_MAXVERTICES*sizeof(vis_2dvertex_type)-ngl_batch2d_vertices*sizeof(ngl_2dvertex_type),varray,vertices*sizeof(ngl_2dvertex_type));
		ngl_batch2d_vcurrent += vertices;
		ngl_batch2d_vertices += vertices;

		return true;
	}
}

/*
	Функция	: nglBatch3dDrawMesh

	Описание: Рисует 3d модель

	История	: 29.01.13	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch3dDrawMesh(unsigned int batch_currenttex, unsigned int vertices, nv_3dvertex_type *varray)
{
	if(ngl_batch_state != NGL_BATCH_STATE_3D || !ngl_isinit || vertices == 0) return false;

	nglBatchSetCurrentTexture(batch_currenttex);

	glInterleavedArrays(GL_T2F_C4F_N3F_V3F, 0, varray);
	glEnable(GL_COLOR_MATERIAL);
	glDrawArrays(GL_TRIANGLES, 0, vertices);
	glDisable(GL_COLOR_MATERIAL);

	nglCatchOpenGLError(F_NGLBATCH3DDRAWMESH);

	return true;
}

/*
	Функция	: nglBatch3dDrawIndexedMesh

	Описание: Рисует 3d модель с индексированными вершинами

	История	: 28.06.16	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch3dDrawIndexedMesh(unsigned int batch_currenttex, unsigned int vertices, nv_3dvertex_type *varray, unsigned int triangles, unsigned int *iarray)
{
	if(ngl_batch_state != NGL_BATCH_STATE_3D || !ngl_isinit || vertices == 0 || triangles == 0) return false;
	
	nglBatchSetCurrentTexture(batch_currenttex);

	glInterleavedArrays(GL_T2F_C4F_N3F_V3F, 0, varray);
	glEnable(GL_COLOR_MATERIAL);
	glDrawElements(GL_TRIANGLES, triangles*3, GL_UNSIGNED_INT, iarray);
	glDisable(GL_COLOR_MATERIAL);

	nglCatchOpenGLError(F_NGLBATCH3DDRAWMESH);
	
	return true;
}

/*
	Функция	: nglBatch2dBegin

	Описание: Начало вывода 2d графики

	История	: 05.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch2dBegin(void)
{
	if(ngl_batch_state == NGL_BATCH_STATE_2D || !ngl_isinit) return;

	if(ngl_batch_state == NGL_BATCH_STATE_3D) nglBatch3dEnd();

	ngl_batch_state = NGL_BATCH_STATE_2D;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0,(ngl_winx)?ngl_winx:1,(ngl_winy)?ngl_winy:1,0.0,-1.0,1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	nglBatchSetCurrentTexture(nglBatchGetCurrentTexture());

	nglCatchOpenGLError(F_NGLBATCH2DBEGIN);
}

/*
	Функция	: nglBatch2dEnd

	Описание: Конец вывода 2d графики

	История	: 05.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch2dEnd(void)
{
	if(ngl_batch_state != NGL_BATCH_STATE_2D || !ngl_isinit) return;

	nglBatch2dDraw();

	ngl_batch_state = NGL_BATCH_STATE_NO;

	nglCatchOpenGLError(F_NGLBATCH2DEND);
}

/*
	Функция	: nglBatch3dSetModelviewMatrix

	Описание: Устанавливает матрицу преобразования

	История	: 16.01.15	Создан

*/
N_API bool N_APIENTRY_EXPORT nglBatch3dSetModelviewMatrix(double *matrix)
{
	if(!ngl_isinit) return false;
	if(ngl_batch_state == NGL_BATCH_STATE_NO) return false;

	glLoadIdentity();

	glMultMatrixd(matrix);

	nglCatchOpenGLError(F_NGLBATCHSETMODELVIEWMATRIX);

	return true;
}

/*
	Функция	: nglBatch3dSetAmbientLight

	Описание: Устанавливает глобальное освещение

	История	: 24.04.13	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch3dSetAmbientLight(float r, float g, float b, float a)
{
	if(!ngl_isinit) return;

	ngl_global_ambient[0] = r;
	ngl_global_ambient[1] = g;
	ngl_global_ambient[2] = b;
	ngl_global_ambient[3] = a;

	if(ngl_batch_state == NGL_BATCH_STATE_3D) glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ngl_global_ambient);

	nglCatchOpenGLError(F_NGLBATCH3DSETAMBIENTLIGHT);
}

/*
	Функция	: nglBatch3dBegin

	Описание: Начало вывода 3d графики

	История	: 05.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch3dBegin(void)
{
	//GLfloat mat[4] = {1.0, 1.0, 1.0, 1.0};

	if(ngl_batch_state == NGL_BATCH_STATE_3D || !ngl_isinit) return;

	if(ngl_batch_state == NGL_BATCH_STATE_2D) nglBatch2dEnd();

	ngl_batch_state = NGL_BATCH_STATE_3D;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (ngl_winx != 0 && ngl_winy != 0)?((GLdouble)ngl_winx/(GLdouble)ngl_winy):(1.0), 2.0, 4000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_LIGHTING);

	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ngl_global_ambient);

	glClear(GL_DEPTH_BUFFER_BIT);

	// хз зачем я добавил эту строчку, наверно не нужна
	//ngl_batch_currenttex = 0; // Грязный Хак

	nglBatchSetCurrentTexture(nglBatchGetCurrentTexture());

	nglCatchOpenGLError(F_NGLBATCH3DBEGIN);
}

/*
	Функция	: nglBatch3dEnd

	Описание: Конец вывода 3d графики

	История	: 05.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nglBatch3dEnd(void)
{

	if(ngl_batch_state != NGL_BATCH_STATE_3D || !ngl_isinit) return;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_LIGHTING);

	ngl_batch_state = NGL_BATCH_STATE_NO;

	nglCatchOpenGLError(F_NGLBATCH3DEND);
}

/*
	Функция	: nglReadScreen

	Описание: Читает изображение с экрана

	История	: 24.08.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglReadScreen(unsigned char *buffer)
{
	if(!ngl_isinit) return false;

	if(ngl_batch_state == NGL_BATCH_STATE_2D) nglBatch2dEnd();
	if(ngl_batch_state == NGL_BATCH_STATE_3D) nglBatch3dEnd();

	glFinish();

	if(ngl_windpix == 100 && ngl_windpiy == 100) {
		glReadPixels(0, 0, ngl_winx, ngl_winy, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	} else {
		unsigned char *tempbuffer;

		tempbuffer = ngl_ea->nAllocMemory((ngl_windpix*ngl_winx/100)*(ngl_windpiy*ngl_winy/100)*4);
		if(!tempbuffer)
			return false;

		glReadPixels(0, 0, ngl_windpix*ngl_winx/100, ngl_windpiy*ngl_winy/100, GL_RGBA, GL_UNSIGNED_BYTE, tempbuffer);

		gluScaleImage(GL_RGBA, ngl_windpix*ngl_winx/100, ngl_windpiy*ngl_winy/100, GL_UNSIGNED_BYTE, tempbuffer, ngl_winx, ngl_winy, GL_UNSIGNED_BYTE, buffer);

		ngl_ea->nFreeMemory(tempbuffer);
	}

	nglCatchOpenGLError(F_NGLREADSCREEN);

	return true;
}
