/*
	Файл	: ngl_batch.h

	Описание: Заголовок для ngl_batch.c

	История	: 5.10.12	Создан

*/

#define NGL_BATCH_STATE_NO 0
#define NGL_BATCH_STATE_3D 1
#define NGL_BATCH_STATE_2D 2

extern int ngl_batch_state;

N_API bool N_APIENTRY_EXPORT nglBatch2dDraw(void);
N_API void N_APIENTRY_EXPORT nglBatch2dEnd(void);
N_API void N_APIENTRY_EXPORT nglBatch3dEnd(void);
