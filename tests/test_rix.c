
#include <stdio.h>
#include <stdlib.h>

#include "../nyan/nyan_publicapi.h"

const int CWHITE = NV_COLOR(255,255,255,255);

NYAN_MAIN
{
	unsigned int texid;

	NYAN_INIT

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 16);

	nvSetScreen(640, 480, 32, NV_MODE_WINDOWED_FIXED, 0);

	nMountDir(L"media");
	nMountDir(L"../media");
	nMountDir(L"../../media");
	nMountDir(L"../../../media");

	if(!nInit()) return 0;

	nAddPlugin(L"plgrix");

	texid = nvCreateTextureFromFile(L"chihiro.rix", NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(texid);
	while(!nvGetStatusi(NV_STATUS_WIN_EXITMSG) && !nvGetKey(27)) {
		nvBegin2d();
			nvDraw2dPicture(0,0,texid,1.0,1.0,CWHITE);
		nvEnd2d();
		nUpdate();
	}
	nClose();

	NYAN_CLOSE

	return 0;
}
