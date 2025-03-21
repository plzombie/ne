/*
	Файл	: nal.h

	Описание: Calling convention и т.д.

	История	: 21.12.12	Создан

*/

#ifdef N_WINDOWS
	#ifndef N_NODYLIB
		#define N_API __declspec(dllexport)
	#else
		#define N_API
	#endif
#elif defined(N_POSIX) || defined(N_ANDROID)
	#ifndef N_NODYLIB
		#define N_API __attribute__ ((visibility("default")))
	#else
		#define N_API
	#endif
#else
	#define N_API
#endif

#if defined(N_POSIX) || defined(N_ANDROID)
	#define N_APIENTRY
#else
	#define N_APIENTRY __cdecl
#endif

#define N_APIENTRY_EXPORT N_APIENTRY
