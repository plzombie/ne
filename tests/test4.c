
#include <stdio.h>
#include <stdlib.h>

#include "../nyan/nyan_publicapi.h"

const int CWHITE = NV_COLOR(255,255,255,255);

NYAN_MAIN
{
	bool gameloop = true;
	unsigned int bufid, srcid = 0, texid, fontid, secs = 0, sec = 0;
	int is_sound_looped = true;
	wchar_t texbuf[80];

	NYAN_INIT


	if(!nvAttachRender(L"ngl")) return 0;
#if defined(_M_ARM) || defined(_M_ARM64)
	if (!naAttachLib(L"xaudio2_8al")) return 0;
#else
	if (!naAttachLib(L"nal")) return 0;
#endif

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

	bufid = naCreateBuffer(L"music.wav");
	if(bufid)
		if(naLoadBuffer(bufid))
			srcid = naCreateSource(bufid, true);

	if(srcid) {
		naPlaySource(srcid);
		naGetSourceLength(srcid, &secs);
		wprintf(L"secs %u\n", secs);
	}

	texid = nvCreateTextureFromFile(L"deffont.tga", NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(texid);
	fontid = nvCreateFont(L"deffont.nek1");

	while(gameloop) {
		unsigned int status;

		if(nvGetStatusi(NV_STATUS_WIN_EXITMSG)) gameloop = false;
		if(nvGetKey(27)) gameloop = false;

		naGetSourceSecOffset(srcid, &sec);
		status = naGetSourceStatus(srcid);

		swprintf(texbuf, 80, L"seconds %u < %u\n", sec, secs);

		nvBegin2d();
			switch(status) {
				case NA_SOURCE_PLAY:
					nvDraw2dText(L">", 8, 8, fontid, texid, 1.0, 1.0, CWHITE);
					break;
				case NA_SOURCE_PAUSE:
					nvDraw2dText(L"||", 8, 8, fontid, texid, 1.0, 1.0, CWHITE);
					break;
				case NA_SOURCE_STOP:
					nvDraw2dText(L"S", 8, 8, fontid, texid, 1.0, 1.0, CWHITE);
					break;
				default:
					nvDraw2dText(L"E", 8, 8, fontid, texid, 1.0, 1.0, CWHITE);
					break;
			}
			nvDraw2dText(texbuf, 40, 8, fontid, texid, 1.0, 1.0, CWHITE);
			if(nvDraw2dText(L"Play", 8, 40, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) naPlaySource(srcid);
			if(nvDraw2dText(L"Pause", 68, 40, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) naPauseSource(srcid);
			if(nvDraw2dText(L"Stop", 148, 40, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) naStopSource(srcid);
			if(nvDraw2dText(L"-10sec", 208, 40, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) naSetSourceSecOffset(srcid, (((int)sec-10)<0)?0:(sec-10));
			if(nvDraw2dText(L"+10sec", 290, 40, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) naSetSourceSecOffset(srcid, ((sec+10)<secs)?(sec+10):secs-1);
			if(nvDraw2dText((is_sound_looped?L"Loop ON":L"Loop OFF"), 378, 40, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) {
				if(is_sound_looped)
					naSetSourceLoop(srcid, false);
				else
					naSetSourceLoop(srcid, true);

				naGetSourceLoop(srcid, &is_sound_looped);
			}
			if(nvDraw2dText(L"Replay", 8, 72, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) naReplaySource(srcid);
		nvEnd2d();

		nUpdate();
	}
	nClose();

	NYAN_CLOSE

	return 0;
}
