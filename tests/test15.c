
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wchar.h>

#include "../nyan/nyan_publicapi.h"

const int CWHITE = NV_COLOR(255,255,255,255);
const int CRED = NV_COLOR(255,0,0,255);
const int CGREEN = NV_COLOR(0,255,0,255);
const int CBLUE = NV_COLOR(0,0,255,255);

NYAN_MAIN
{
	unsigned int texid, fontid;
	int winx = 0, winy = 0, winbpp = 0, winmode = 0, vsync = 0, mbl;
	bool gameloop = true;
	wchar_t fpst[80], screenparamst[512];

	NYAN_INIT

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusi(NV_STATUS_WINMODE, NV_MODE_WINDOWED);

	nMountDir(L"media");
	nMountDir(L"media/fonts");
	nMountDir(L"../media");
	nMountDir(L"../media/fonts");
	nMountDir(L"../../media");
	nMountDir(L"../../media/fonts");
	nMountDir(L"../../../media");
	nMountDir(L"../../../media/fonts");

	if(!nInit()) return 0;
	texid = nvCreateTextureFromFile(L"deffont.tga", NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(texid);
	fontid = nvCreateFont(L"deffont.nek1");

	while(gameloop) {
		mbl = nvGetStatusi(NV_STATUS_WINMBL);
		winx = nvGetStatusi(NV_STATUS_WINX);
		winy = nvGetStatusi(NV_STATUS_WINY);
		winbpp = nvGetStatusi(NV_STATUS_WINBPP);
		winmode = nvGetStatusi(NV_STATUS_WINMODE);
		vsync = nvGetStatusi(NV_STATUS_WINVSYNC);
		if(nvGetStatusi(NV_STATUS_WIN_EXITMSG)) gameloop = false;
		if(nvGetKey(27)) gameloop = false;
		if(nvGetKey(NK_0) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 0);
		if(nvGetKey(NK_1) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 10);
		if(nvGetKey(NK_2) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 20);
		if(nvGetKey(NK_3) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 30);
		if(nvGetKey(NK_4) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 40);
		if(nvGetKey(NK_5) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 50);
		if(nvGetKey(NK_6) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 60);
		if(nvGetKey(NK_7) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 70);
		if(nvGetKey(NK_8) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 80);
		if(nvGetKey(NK_9) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 90);
		if(nvGetKey(NK_F5) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINVSYNC, 1);
		if(nvGetKey(NK_F6) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINVSYNC, 0);
		if(nvGetKey(NK_Q) == NV_KEYSTATUS_RELEASED) nvSetScreen(320, 240, winbpp, winmode, vsync);
		if(nvGetKey(NK_W) == NV_KEYSTATUS_RELEASED) nvSetScreen(640, 480, winbpp, winmode, vsync);
		if(nvGetKey(NK_E) == NV_KEYSTATUS_RELEASED) nvSetScreen(800, 600, winbpp, winmode, vsync);
		if(nvGetKey(NK_R) == NV_KEYSTATUS_RELEASED) nvSetScreen(1024, 768, winbpp, winmode, vsync);
		if(nvGetKey(NK_T) == NV_KEYSTATUS_RELEASED) nvSetScreen(1920, 1080, winbpp, winmode, vsync);
		if(nvGetKey(NK_A) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINMODE, NV_MODE_FULLSCREEN);
		if(nvGetKey(NK_S) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINMODE, NV_MODE_WINDOWED);
		if(nvGetKey(NK_D) == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINMODE, NV_MODE_WINDOWED_FIXED);
		nvBegin2d();
			swprintf(fpst, 80, L"fps %f spf %f afps %f", nGetfps(), nGetspf(), nGetafps()); fpst[79] = 0;
			nvDraw2dText(fpst, 0, 0, fontid, texid, 1.0, 1.0, CWHITE);
			
			
			swprintf(screenparamst, 512, L"screen %dx%dx%d %dx%d (%ls), vsync %d (%ls), update interval %d", 
				winx, winy, winbpp, nvGetStatusi(NV_STATUS_WINDPIX), nvGetStatusi(NV_STATUS_WINDPIY), 
				((winmode == NV_MODE_FULLSCREEN)?(L"Fullscreen"):((winmode == NV_MODE_WINDOWED)?L"Windowed":L"Windowed fixed")),
				vsync, ((nvGetStatusi(NV_STATUS_SUPPORT_WINVSYNC))?L"sup.":L"unsup"),
				nvGetStatusi(NV_STATUS_WINUPDATEINTERVAL)); screenparamst[511] = 0;
			nvDraw2dText(screenparamst, 0, 20, fontid, texid, 1.0, 1.0, CWHITE);
			nvDraw2dText(L"Vsync:", 0, 40, fontid, texid, 1.0, 1.0, CWHITE);
			if(nvDraw2dText(L"ON", 96, 40, fontid, texid, 1.0, 1.0, vsync?CRED:CWHITE) && mbl == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINVSYNC, 1);
			if(nvDraw2dText(L"OFF", 144, 40, fontid, texid, 1.0, 1.0, (!vsync)?CRED:CWHITE) && mbl == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINVSYNC, 0);
			if(nvDraw2dText(L"FULLSCREEN", 0, 60, fontid, texid, 1.0, 1.0, (winmode == NV_MODE_FULLSCREEN)?CRED:CWHITE) && mbl == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINMODE, NV_MODE_FULLSCREEN);
			if(nvDraw2dText(L"WINDOWED", 160, 60, fontid, texid, 1.0, 1.0, (winmode == NV_MODE_WINDOWED)?CRED:CWHITE) && mbl == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINMODE, NV_MODE_WINDOWED);
			if(nvDraw2dText(L"WINDOWED_FIXED", 288, 60, fontid, texid, 1.0, 1.0, (winmode == NV_MODE_WINDOWED_FIXED)?CRED:CWHITE) && mbl == NV_KEYSTATUS_RELEASED) nvSetStatusi(NV_STATUS_WINMODE, NV_MODE_WINDOWED_FIXED);
		nvEnd2d();
		nUpdate();
	}

	nClose();

	NYAN_CLOSE

	return 0;
}
