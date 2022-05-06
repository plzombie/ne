#ifndef UFMOD_MISC_H
#define UFMOD_MISC_H

#ifdef N_WINDOWS
	#include <windows.h>
	#include "ufmod.h"
#else
	#include "ufmod_unix.h"
#endif

#ifdef N_WINDOWS
extern HWAVEOUT* uFMOD_PlaySong2(wchar_t *fname, int fdwSong);
#else
extern int uFMOD_PlaySong2(wchar_t *fname, int fdwSong);
#endif

#endif
