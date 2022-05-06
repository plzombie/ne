
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wchar.h>

#include "../nyan/nyan_publicapi.h"

const int CWHITE = NV_COLOR(255,255,255,255);
const int CRED = NV_COLOR(255,0,0,255);
const int CGREEN = NV_COLOR(0,255,0,255);
const int CBLUE = NV_COLOR(0,0,255,255);

wchar_t *ReadUCS2FromFile(wchar_t *filename)
{
	unsigned int f;
	size_t fsize, i;
	wchar_t *text;
	unsigned short *data;


	f = nFileOpen(filename);
	if(!f) return 0;

	fsize = (size_t)nFileLength(f);

	data = nAllocMemory(fsize);
	if(!data) {
		nFileClose(f);

		return 0;
	}

	text = nAllocMemory((fsize/2+1)*sizeof(wchar_t));
	if(!text) {
		nFileClose(f);
		nFreeMemory(data);
		return 0;
	}

	nFileRead(f, data, fsize);

	for(i = 0; i < fsize/2; i++) {
		text[i] = data[i];
	}
	text[fsize/2] = 0;

	nFreeMemory(data);

	nFileClose(f);

	return text;
}

NYAN_MAIN
{
	unsigned int texid, fontid, mx = 0, my = 0, mbl = 0, oldmx, oldmy, oldmbl, tx = 0, ty = 0;
	bool gameloop = true;
	wchar_t *text, fpst[16];
	float scale = 0.5f;

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
	texid = nvCreateTextureFromFile(L"testfont.tga", NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(texid);
	fontid = nvCreateFont(L"testfont.nek1");
	text = ReadUCS2FromFile(L"test.txt");
	if(!text) goto EXIT;
	wprintf(L"size of text %d\n", (int)wcslen(text));

	while(gameloop) {
		oldmx = mx; oldmy = my; oldmbl = mbl;
		mx = nvGetStatusi(NV_STATUS_WINMX);
		my = nvGetStatusi(NV_STATUS_WINMY);
		mbl = nvGetStatusi(NV_STATUS_WINMBL);
		if(nvGetStatusi(NV_STATUS_WIN_EXITMSG)) gameloop = false;
		if(nvGetKey(NK_ESCAPE)) gameloop = false;
		if(mbl && oldmbl) {
			tx += mx-oldmx;
			ty += my-oldmy;
		}
		if(nvGetKey(NK_CONTROL) == NK_STATUS_PRESSED)
			scale *= 1.0f+0.125f*nvGetStatusi(NV_STATUS_WINMWHEEL);
		nvBegin2d();
			swprintf(fpst, 16, L"fps %f", nGetafps());
			nvDraw2dText(text, tx, ty, fontid, texid, scale, scale, CWHITE);
			nvDraw2dText(fpst, 0, 0, fontid, texid, 0.5f, 0.5f, CBLUE);
		nvEnd2d();
		nUpdate();
	}

	nFreeMemory(text);

EXIT:
	nClose();

	NYAN_CLOSE

	return 0;
}
