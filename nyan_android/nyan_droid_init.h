/*
	Файл	: nyan_droid_init.h

	Описание: Заголовок для nyan_droid_init.c

	История	: 19.08.15	Создан

*/

#ifndef NYAN_DROID_INIT_H
#define NYAN_DROID_INIT_H

#include "../android_native_app_glue/android_native_app_glue.h"

#include "../nyan/nyan_decls.h"

#ifdef __cplusplus
extern "C" {
#endif

N_API struct android_app* N_APIENTRY_EXPORT ndroidGetAndroidApp(void);
N_API void N_APIENTRY_EXPORT ndroidInit(struct android_app* state);
N_API void N_APIENTRY_EXPORT ndroidClose(void);

#ifdef __cplusplus
}
#endif

#endif
