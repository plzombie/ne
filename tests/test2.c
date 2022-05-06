
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../nyan/nyan_publicapi.h"

const int CWHITE = NV_COLOR(255,255,255,255);

NYAN_MAIN
{
	unsigned int texid, exitid, showid, hideid;
	bool gameloop = true;
	bool zazakas = true;

	NYAN_INIT

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusi(NV_STATUS_WINMODE, NV_MODE_WINDOWED);

	nMountDir(L"media");
	nMountDir(L"media/buttons");
	nMountDir(L"../media");
	nMountDir(L"../media/buttons");
	nMountDir(L"../../media");
	nMountDir(L"../../media/buttons");
	nMountDir(L"../../../media");
	nMountDir(L"../../../media/buttons");

	nAddPlugin(L"plgkplib_pictures");

	if(!nInit()) return 0;
	texid = nvCreateTextureFromFile(L"zazaka.png", NGL_TEX_FLAGS_LINEARMIN | NGL_TEX_FLAGS_LINEARMAG | NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(texid);
	exitid = nvCreateTextureFromFile(L"exit.tga", NGL_TEX_FLAGS_LINEARMIN | NGL_TEX_FLAGS_LINEARMAG | NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(exitid);
	showid = nvCreateTextureFromFile(L"show.tga", NGL_TEX_FLAGS_LINEARMIN | NGL_TEX_FLAGS_LINEARMAG | NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(showid);
	hideid = nvCreateTextureFromFile(L"hide.tga", NGL_TEX_FLAGS_LINEARMIN | NGL_TEX_FLAGS_LINEARMAG | NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(hideid);
	while(gameloop) {
		unsigned int winx, winy;
		winx = nvGetStatusi(NV_STATUS_WINX);
		winy = nvGetStatusi(NV_STATUS_WINY);
		if(nvGetStatusi(NV_STATUS_WIN_EXITMSG)) gameloop = false;
		if(nvGetKey(27)) gameloop = false;
		nvBegin2d();
			if(zazakas) {
				unsigned int i;
				for(i = 0;i<100/*00*/;i++) {
					nv_2dvertex_type vs[6];
					float tx, ty;

					tx = (float)winx*rand()/(float)RAND_MAX-32;
					ty = (float)winy*rand()/(float)RAND_MAX-32;
					vs[0].x = tx;    vs[0].y = ty;    vs[0].z = 0.0; vs[0].colorRGBA = CWHITE; vs[0].tx = 0.0; vs[0].ty = 1.0;
					vs[1].x = tx+64; vs[1].y = ty;    vs[1].z = 0.0; vs[1].colorRGBA = CWHITE; vs[1].tx = 1.0; vs[1].ty = 1.0;
					vs[2].x = tx+64; vs[2].y = ty+64; vs[2].z = 0.0; vs[2].colorRGBA = CWHITE; vs[2].tx = 1.0; vs[2].ty = 0.0;
					vs[3] = vs[0];
					vs[4] = vs[2];
					vs[5].x = tx;    vs[5].y = ty+64; vs[5].z = 0.0; vs[5].colorRGBA = CWHITE; vs[5].tx = 0.0; vs[5].ty = 0.0;
					nvDraw2d(NV_DRAWTRIANGLE, 2, texid, vs);
				}
				if(nvDraw2dPicture(8,8,hideid,1.0,1.0,CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) zazakas = false;
			} else
				if(nvDraw2dPicture(8,8,showid,1.0,1.0,CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) zazakas = true;

			if(nvDraw2dPicture(8,32,exitid,1.0,1.0,CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) gameloop = false;
		nvEnd2d();
		if(!gameloop) { nFileCreateDir(L"Test2/", NF_PATH_PICTURES); nvMakeScreenshot(L"Test2/test2_sshot.tga", NF_PATH_PICTURES); nvMakeScreenshot(L"Test2/test2_sshot.bmp", NF_PATH_PICTURES); /*nvMakeScreenshot(L"Test2/test2_sshot.nek0", NF_PATH_PICTURES);*/ }
		nUpdate();
	}
	nClose();

	NYAN_CLOSE

	return 0;
}
