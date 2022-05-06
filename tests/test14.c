
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "../nyan/nyan_publicapi.h"

#define CWHITE NV_COLOR(255,255,255,255)
#define CRED NV_COLOR(255,0,0,255)
#define CGREEN NV_COLOR(0,255,0,255)
#define CBLUE NV_COLOR(0,0,255,255)
#define MAX_COLORS 4

typedef struct {
	int x;
	int y;
	int pressed;
} touch_type;

NYAN_MAIN
{
	unsigned int texid, texwid, texhei;
	int i, nof_touches, used_touches;
	touch_type *touches;
	const int colors[MAX_COLORS] = { CWHITE, CRED, CGREEN, CBLUE };

	NYAN_INIT

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 16);

	nvSetStatusi(NV_STATUS_WINMODE, NV_MODE_WINDOWED);

	nMountDir(L"media");
	nMountDir(L"../media");
	nMountDir(L"../../media");
	nMountDir(L"../../../media");

	if(!nInit()) return 0;

	if(nvGetStatusi(NV_STATUS_SUPPORT_WINTOUCH)) {
		nof_touches = nvGetStatusi(NV_STATUS_WINTOUCHMAXCOUNT);
		
		touches = nAllocMemory(nof_touches*sizeof(touch_type));
		if(!touches) {
			wprintf(L"%ls", L"Can\'t allocate memory for touches\n");
			nClose();
			return 0;
		}

		memset(touches, 0, nof_touches*sizeof(touch_type));
	} else {
		nof_touches = 0;
		touches = 0;
		wprintf(L"%ls", L"Touches unsupported\n");
	}

	texid = nvCreateTextureFromFile(L"ball.tga", NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(texid);

	nvGetTextureWH(texid, &texwid, &texhei);
	
	nvSetStatusf(NV_STATUS_WINBCRED,0.1f);
	nvSetStatusf(NV_STATUS_WINBCGREEN,0.1f);
	nvSetStatusf(NV_STATUS_WINBCBLUE,0.1f);

	while(!nvGetStatusi(NV_STATUS_WIN_EXITMSG) && !nvGetKey(27)) {
		used_touches = nvGetStatusi(NV_STATUS_WINTOUCHCOUNT);

		if(used_touches > nof_touches) used_touches = nof_touches;

		nvBegin2d();
			for(i = 0; i < used_touches; i++) {
				touches[i].x = nvGetStatusi(NV_STATUS_WINTOUCH0_X+i);
				touches[i].y = nvGetStatusi(NV_STATUS_WINTOUCH0_Y+i);
				touches[i].pressed = nvGetStatusi(NV_STATUS_WINTOUCH0_PRESSED+i);

				if(touches[i].pressed == 1) {
					float scale;

					scale = 3.0f-(float)(touches[i].pressed);

					nvDraw2dPicture(touches[i].x-(int)(texwid*scale)/2, touches[i].y-(int)(texhei*scale)/2, texid, scale, scale, colors[i%MAX_COLORS]);
				}
			}
		nvEnd2d();
		nUpdate();
	}
	nClose();

	if(touches)
		nFreeMemory(touches);

	NYAN_CLOSE

	return 0;
}
