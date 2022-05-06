
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wchar.h>

#include "../nyan/nyan_publicapi.h"

#ifndef N_WINDOWS
	#define LOWORD(l)  ((short)((l)&0xFFFF))
	#define HIWORD(l)  ((short)((l)>>16))
#endif
#include "../forks/ufmod/ufmod_misc.h"

const int CWHITE = NV_COLOR(255,255,255,255);

NYAN_MAIN
{
	bool gameloop = true;

	NYAN_INIT

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 16);

	nvSetStatusi(NV_STATUS_WINMODE, NV_MODE_WINDOWED);
	nvSetStatusf(NV_STATUS_WINBCGREEN, 0.0);

	nMountDir(L"media");
	nMountDir(L"../media");
	nMountDir(L"../../media");
	nMountDir(L"../../../media");

	if(!nInit()) return 0;

	if(uFMOD_PlaySong2(L"mini.xm", 0))
		printf("Title: \'%s\'\n", (char *)uFMOD_GetTitle());
	else
		printf("Can't open file\n");

	while(gameloop) {
		unsigned int vol = 0;

		if(nvGetStatusi(NV_STATUS_WIN_EXITMSG)) gameloop = false;
		if(nvGetKey(27)) gameloop = false;

		vol = uFMOD_GetStats();
		nvSetStatusf(NV_STATUS_WINBCRED, LOWORD(vol)/(float)0x7FFF);
		nvSetStatusf(NV_STATUS_WINBCBLUE, HIWORD(vol)/(float)0x7FFF);
		nUpdate();
	}
	nClose();

	NYAN_CLOSE

	return 0;
}
