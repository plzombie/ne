/*
	Файл	: ngl.h

	Описание: Calling convention и т.д.

	История	: 05.08.12	Создан

*/

#ifdef N_WINDOWS
	#ifndef N_NODYLIB
		#define N_API __declspec(dllexport)
	#else
		#define N_API
	#endif
#elif defined(N_POSIX)
	#ifndef N_NODYLIB
		#define N_API __attribute__ ((visibility("default")))
	#else
		#define N_API
	#endif
#else
	#define N_API
#endif

#if defined(N_POSIX)
	#define N_APIENTRY
#else
	#define N_APIENTRY __cdecl
#endif

#ifdef N_CAUSEWAY
	#define N_APIENTRY_EXPORT __export N_APIENTRY
#else
	#define N_APIENTRY_EXPORT N_APIENTRY
#endif


