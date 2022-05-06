
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wchar.h>

#include "../nyan/nyan_publicapi.h"

const int CGREY = NV_COLOR(128,128,128,255);
const int CDARKGREY = NV_COLOR(48,48,48,255);
const int CWHITE = NV_COLOR(255,255,255,255);

NYAN_MAIN
{
	unsigned int texid, texid2, texid3;
	bool gameloop = true;

	NYAN_INIT

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 16);

	nvSetStatusi(NV_STATUS_WINMODE, NV_MODE_WINDOWED);

	nMountDir(L"media");
	nMountDir(L"media/buttons");
	nMountDir(L"../media");
	nMountDir(L"../media/buttons");
	nMountDir(L"../../media");
	nMountDir(L"../../media/buttons");
	nMountDir(L"../../../media");
	nMountDir(L"../../../media/buttons");

	if(!nInit()) return 0;
	texid = nvCreateTextureFromFile(L"uberbutton.tga", NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_FOR2D);
	texid2 = nvCreateTextureFromFile(L"uberbutton2.tga", NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_FOR2D);
	texid3 = nvCreateTextureFromFile(L"uberbutton3.tga", NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(texid);
	nvLoadTexture(texid2);
	nvLoadTexture(texid3);
	while(gameloop) {
		if(nvGetStatusi(NV_STATUS_WIN_EXITMSG)) gameloop = false;
		if(nvGetKey(27)) gameloop = false;

		nvBegin2d();
			nvDraw2dButton(10, 10, 2, 500, 3, texid, 1.0, 1.0, CWHITE, CWHITE, CWHITE);
			nvDraw2dButton(190, 10, 2, 500, 2, texid2, 1.0, 1.0, CWHITE, CWHITE, CWHITE);
			nvDraw2dButton(370, 10, 2, 500, 1, texid3, 1.0, 1.0, CWHITE, CGREY, CDARKGREY);
		nvEnd2d();
		nUpdate();
	}
	nClose();

	NYAN_CLOSE

	return 0;
}
