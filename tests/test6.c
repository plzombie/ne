
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
			nvDraw2dText(L"0123456789.,!?-+\\/():;%&`'*#$=[]@^{|}_~><\x262e", 32, 32, fontid, texid, 1.0, 1.0, CWHITE);
			nvDraw2dText(L"abcdefghijklmnopqrstuvwxyz", 32, 80, fontid, texid, 1.0, 1.0, CRED);
			nvDraw2dText(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ", 32, 130, fontid, texid, 1.0, 1.0, CGREEN);
			nvDraw2dText(L"Line\nSeparator\n\nAnother\none", 32, 180, fontid, texid, 1.0, 1.0, CGREEN);
			nvDraw2dText(L"\xC4\xE4 \xC5\xE5 \xC6\xE6 \xD1\xF1 \xD6\xF6 \xD8\xF8 \xDC\xFC \x160\x161 \x17D\x17E \x1E9E\xDF", 32, 280, fontid, texid, 1.0, 1.0, CBLUE);
		nvEnd2d();
		nUpdate();
	}
	nClose();

	NYAN_CLOSE

	return 0;
}
