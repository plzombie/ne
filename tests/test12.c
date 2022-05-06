
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wchar.h>

#include "../nyan/nyan_publicapi.h"

const int CWHITE = NV_COLOR(255,255,255,255);
const int CORANGE = NV_COLOR(255,164,0,255);

NYAN_MAIN
{
	unsigned int texid, fonttexid, fontid, ssheetid;
	bool gameloop = true;

	NYAN_INIT

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 16);

	nvSetStatusi(NV_STATUS_WINMODE, NV_MODE_WINDOWED);

	nMountDir(L"media");
	nMountDir(L"media/buttons");
	nMountDir(L"media/fonts");
	nMountDir(L"../media");
	nMountDir(L"../media/buttons");
	nMountDir(L"../media/fonts");
	nMountDir(L"../../media");
	nMountDir(L"../../media/buttons");
	nMountDir(L"../../media/fonts");
	nMountDir(L"../../../media");
	nMountDir(L"../../../media/buttons");
	nMountDir(L"../../../media/fonts");

	if(!nInit()) return 0;
	texid = nvCreateTextureFromFile(L"uberbutton.tga", NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(texid);
	fonttexid = nvCreateTextureFromFile(L"deffont.tga", NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(fonttexid);
	fontid = nvCreateFont(L"deffont.nek1");
	ssheetid = nvCreateSpriteSheetFromTileset(340, 111, 2, 3, L"uberbutton");
	while(gameloop) {
		unsigned int i, j;

		if(nvGetStatusi(NV_STATUS_WIN_EXITMSG)) gameloop = false;
		if(nvGetKey(27)) gameloop = false;

		nvBegin2d();
			for(i = 0; i < 3; i++) {
				for(j = 0; j < 2; j++) {
					unsigned int sid;

					sid = i*2+j+1;
					if(nvDrawSprite(ssheetid, sid, 10+200*j, 10+64*i, texid, 1.0, 1.0, CWHITE))
						nvDraw2dText(nvGetSpriteNameById(ssheetid, sid), 10+200*j, 50+64*i, fontid, fonttexid, 1.0, 1.0, CORANGE);
					else
						nvDraw2dText(nvGetSpriteNameById(ssheetid, sid), 10+200*j, 50+64*i, fontid, fonttexid, 1.0, 1.0, CWHITE);
				}
			}
		nvEnd2d();
		nUpdate();
	}
	nClose();

	NYAN_CLOSE

	return 0;
}
