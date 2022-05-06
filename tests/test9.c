
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../nyan/nyan_publicapi.h"

const int CWHITE = NV_COLOR(255,255,255,255);
const int CRED = NV_COLOR(255,0,0,255);
const int CGREEN = NV_COLOR(0,255,0,255);
const int CBLUE = NV_COLOR(0,0,255,255);

NYAN_MAIN
{
	unsigned int texid, fontid;
	bool gameloop = true;
	int status = 0;
	wchar_t tbuf[80] = L"desu";
	NYAN_INIT

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 16);

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
		if(nvGetStatusi(NV_STATUS_WIN_EXITMSG)) gameloop = false;
		if(nvGetKey(27)) gameloop = false;
		nvBegin2d();
			nvDraw2dTextbox(tbuf, &status, 80, '_', 500, 32, 32, fontid, texid, 1.0, 1.0, CWHITE, CRED);
		nvEnd2d();
		nUpdate();
	}
	nClose();

	NYAN_CLOSE

	return 0;
}
