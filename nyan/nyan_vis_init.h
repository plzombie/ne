/*
	Файл	: nyan_vis_init.h

	Описание: Заголовок для nyan_vis_init.c

	История	: 05.08.12	Создан

*/

extern int nv_isinit;
extern int nv_isrenderattached;

extern int nvInit(void);
extern int nvClose(void);
extern void nvFlip(void);
