/*
	Файл	: nyan_decls_publicapi.h

	Описание: Дефайны для объявления функций

	История	: 28.05.13	Создан

*/

#ifndef NYAN_DECLS_PUBLICAPI_H
#define NYAN_DECLS_PUBLICAPI_H

// Установка N_APIENTRY
#if defined(N_POSIX) || defined(N_ANDROID)
	#define N_APIENTRY
#else
	#define N_APIENTRY __cdecl
#endif

// Установка N_API и N_APIENTRY_EXPORT
#if defined(N_STATIC)
	#define N_API extern
	#define N_APIENTRY_EXPORT N_APIENTRY
#elif defined(N_EXPORT)
	#ifdef N_WINDOWS
		#define N_APIENTRY_EXPORT N_APIENTRY
		#define N_API __declspec(dllexport)
	#elif defined(N_CAUSEWAY)
		#define N_APIENTRY_EXPORT __export N_APIENTRY
		#define N_API
	#elif defined(N_POSIX) || defined(N_ANDROID)
		#define N_APIENTRY_EXPORT N_APIENTRY
		#define N_API __attribute__ ((visibility("default")))
	#else
		#define N_APIENTRY_EXPORT N_APIENTRY
		#define N_API extern
	#endif
#else
	#define N_APIENTRY_EXPORT N_APIENTRY
	#ifdef N_WINDOWS
		#define N_API __declspec(dllimport)
	#elif defined(N_CAUSEWAY)
		#define N_API extern	
	#elif defined(N_POSIX) || defined(N_ANDROID)
		#define N_API extern
	#else
		#define N_API extern
	#endif
#endif

#if defined(N_CAUSEWAY) && !(defined(N_STATIC) || defined(N_EXPORT))
	#if defined(N_CAUSEWAY_IMPLIB)
		#define NYAN_FUNC(ret, name, params) ret (N_APIENTRY_EXPORT *name) params = 0;
	#else
		#define NYAN_FUNC(ret, name, params) N_API ret (N_APIENTRY_EXPORT *name) params;
	#endif
#else
	#define NYAN_FUNC(ret, name, params) N_API ret N_APIENTRY_EXPORT name params;
#endif

#endif
