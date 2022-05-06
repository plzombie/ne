/*
	Файл	: nyan_droid_init.c

	Описание: Процедуры инициализации под андроид

	История	: 19.08.15	Создан

*/

#include "nyan_droid_init.h"

struct android_app* ndroid_state = 0;

N_API struct android_app* N_APIENTRY_EXPORT ndroidGetAndroidApp(void)
{
	return ndroid_state;
}

N_API void N_APIENTRY_EXPORT ndroidInit(struct android_app* state)
{
	ndroid_state = state;
}

N_API void N_APIENTRY_EXPORT ndroidClose(void)
{
	if(!ndroid_state) return;
	
	return;
}
