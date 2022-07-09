
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wchar.h>

#include "../nyan/nyan_publicapi.h"

const int CWHITE = NV_COLOR(255,255,255,255);

NYAN_MAIN
{
	const wchar_t *fnames[9] = { L"01.wv",
				L"02.ogg",
				L"03.wv",
				L"04.ogg",
				L"05.wv",
				L"06.ogg",
				L"07.wv",
				L"08.ogg",
				L"09.wv"};
	wchar_t texbuf[80];
	bool gameloop = true;
	int is_sound_looped = true;
	unsigned int srcid, texid, fontid, tracks = 0, track = 0, secs = 0, sec = 0;

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

	nAddPlugin(L"plgwavpack");
	nAddPlugin(L"plgstb_vorbis");

	if(!nInit()) return 0;

	//srcid = naCreateAudioStream(L"music.wav", true);
	srcid = naCreateAudioStreamEx(fnames, 9, true);
	if(srcid) naPlayAudioStream(srcid);

	texid = nvCreateTextureFromFile(L"deffont.tga", NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(texid);
	fontid = nvCreateFont(L"deffont.nek1");

	while(gameloop) {
		unsigned int status;
		if(nvGetStatusi(NV_STATUS_WIN_EXITMSG)) gameloop = false;
		if(nvGetKey(27)) gameloop = false;

		naGetAudioStreamLength(srcid, &secs, &tracks);
		naGetAudioStreamSecOffset(srcid, &sec, &track);
		status = naGetAudioStreamStatus(srcid);

		swprintf(texbuf, 80, L"tracks %u < %u, seconds %u < %u\n", track, tracks, sec, secs);

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
			nvDraw2dText(fnames[track], 40, 8, fontid, texid, 1.0, 1.0, CWHITE);
			nvDraw2dText(texbuf, 8, 40, fontid, texid, 1.0, 1.0, CWHITE);
			if(nvDraw2dText(L"Play", 8, 72, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) naPlayAudioStream(srcid);
			if(nvDraw2dText(L"Pause", 68, 72, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) naPauseAudioStream(srcid);
			if(nvDraw2dText(L"Stop", 148, 72, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) naStopAudioStream(srcid);
			if(nvDraw2dText(L"PrevTrack", 208, 72, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) naSetAudioStreamSecOffset(srcid, 0, (((int)track-1)<0)?(tracks-1):(track-1));
			if(nvDraw2dText(L"NextTrack", 328, 72, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) naSetAudioStreamSecOffset(srcid, 0, ((track+1)<tracks)?(track+1):0);
			if(nvDraw2dText(L"-10sec", 448, 72, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) naSetAudioStreamSecOffset(srcid, (((int)sec-10)<0)?0:(sec-10), track);
			if(nvDraw2dText(L"+10sec", 530, 72, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) naSetAudioStreamSecOffset(srcid, ((sec+10)<secs)?(sec+10):secs-1, track);
			if(nvDraw2dText((is_sound_looped?L"Loop ON":L"Loop OFF"), 8, 104, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL)==2)) {
				if(is_sound_looped)
					naSetAudioStreamLoop(srcid, false);
				else
					naSetAudioStreamLoop(srcid, true);

				naGetAudioStreamLoop(srcid, &is_sound_looped);
			}
			if (nvDraw2dText(L"Replay", 108, 104, fontid, texid, 1.0, 1.0, CWHITE) && (nvGetStatusi(NV_STATUS_WINMBL) == 2)) naReplayAudioStream(srcid);
		nvEnd2d();
		nUpdate();
	}
	nClose();

	NYAN_CLOSE

	return 0;
}
