/*
	Файл	: nyan_entry_publicapi.h

	Описание: Заголовок для точки входа в программу

	История	: 17.08.17	Создан

*/

#ifndef NYAN_ENTRY_PUBLICAPI_H
#define NYAN_ENTRY_PUBLICAPI_H

#include <locale.h>

// Установка NYAN_INIT, NYAN_CLOSE и NYAN_MAIN
#if defined(N_CAUSEWAY) && !defined(N_STATIC) && !defined(N_EXPORT)
	extern void nyanImportCauseWayDll(void);
	extern void nyanUnloadCauseWayDll(void);
	#define NYAN_INIT setlocale(LC_ALL, ""); nyanImportCauseWayDll();
	#define NYAN_CLOSE nyanUnloadCauseWayDll();
#endif
#if defined(N_ANDROID) && !defined(N_EXPORT)
	#include "../nyan_android/nyan_droid_init.h"
	#define NYAN_INIT setlocale(LC_ALL, ""); ndroidInit(state);
	#define NYAN_CLOSE ndroidClose();
	#define NYAN_MAIN void android_main(struct android_app* state)
#endif
#ifndef NYAN_INIT
	#define NYAN_INIT setlocale(LC_ALL, "");
#endif
#ifndef NYAN_CLOSE
	#define NYAN_CLOSE
#endif
#ifndef NYAN_MAIN
	#define NYAN_MAIN int main(void)
#endif

#endif
