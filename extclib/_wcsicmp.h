/*
	Файл	: _wcsicmp.h

	Описание: Заголовок для _wcsicmp. _wcsicmp - имя в visual c++, wcscasecmp - posix аналог.

	История	: 22.02.16	Создан

*/

#include <string.h>
#include <wchar.h>

#if defined(N_POSIX) || defined(N_ANDROID)
	#if defined(N_ANDROID)
		#include "../androidsupport/wcscasecmp.h"
	#endif

	#define _wcsicmp wcscasecmp
#endif
