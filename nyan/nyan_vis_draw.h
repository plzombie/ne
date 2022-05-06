/*
	Файл	: nyan_vis_draw.h

	Описание: Заголовок для nyan_vis_draw.c

	История	: 16.08.12	Создан

*/

#define NV_DRAW_STATE_NO 0
#define NV_DRAW_STATE_3D 1
#define NV_DRAW_STATE_2D 2

extern int nv_draw_state;

extern float nv_global_ambient[4];

extern void nvDrawInit(void);
extern void nvDrawClose(void);
