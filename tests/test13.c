
#include <stdio.h>
#include <stdlib.h>

#include "../nyan/nyan_publicapi.h"

const int CWHITE = NV_COLOR(255,255,255,255);

NYAN_MAIN
{
	unsigned int texid, fontid;
	int winx, winy, mx, my, mbl, mbr, mbm, mwheel;
	wchar_t tempstr[1024];

	NYAN_INIT

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 16);

	nvSetScreen(1016, 480, 32, NV_MODE_WINDOWED, 0);

	nMountDir(L"media");
	nMountDir(L"media/fonts");
	nMountDir(L"../media");
	nMountDir(L"../media/fonts");
	nMountDir(L"../../media");
	nMountDir(L"../../media/fonts");
	nMountDir(L"../../../media");
	nMountDir(L"../../../media/fonts");

	if(!nInit()) return 0;
	texid = nvCreateTextureFromFile(L"deffont.tga", NGL_TEX_FLAGS_LINEARMIN | NGL_TEX_FLAGS_LINEARMAG | NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(texid);
	fontid = nvCreateFont(L"deffont.nek1");

	while(!nvGetStatusi(NV_STATUS_WIN_EXITMSG)) {
		int i;
		winx = nvGetStatusi(NV_STATUS_WINX);
		winy = nvGetStatusi(NV_STATUS_WINY);
		mx = nvGetStatusi(NV_STATUS_WINMX);
		my = nvGetStatusi(NV_STATUS_WINMY);
		mbl = nvGetStatusi(NV_STATUS_WINMBL);
		mbr = nvGetStatusi(NV_STATUS_WINMBR);
		mbm = nvGetStatusi(NV_STATUS_WINMBM);
		mwheel = nvGetStatusi(NV_STATUS_WINMWHEEL);
		nvBegin2d();
			swprintf(tempstr, 1023, L"winx %d winy %d mouse: x %d y %d bl %d br %d bm %d wheel %d", winx, winy, mx, my, mbl, mbr, mbm, mwheel); tempstr[1023] = 0;
			nvDraw2dText(tempstr, 0, 0, fontid, texid, 0.75, 0.75, CWHITE);

			swprintf(tempstr, 1023, L"NK_BACK %d NK_TAB %d NK_RETURN %d NK_SHIFT %d NK_CONTROL %d NK_MENU(NK_ALT) %d NK_PAUSE %d NK_CAPSLOCK %d NK_ESCAPE %d",
				nvGetKey(NK_BACK), nvGetKey(NK_TAB), nvGetKey(NK_RETURN), nvGetKey(NK_SHIFT), nvGetKey(NK_CONTROL), nvGetKey(NK_MENU),
				nvGetKey(NK_PAUSE), nvGetKey(NK_CAPSLOCK), nvGetKey(NK_ESCAPE)); tempstr[1023] = 0;
			nvDraw2dText(tempstr, 0, 12, fontid, texid, 0.75, 0.75, CWHITE);

			swprintf(tempstr, 1023, L"NK_SPACE %d NK_PRIOR(NK_PAGEUP) %d NK_NEXT(NK_PAGEDOWN) %d NK_END %d NK_HOME %d",
				nvGetKey(NK_SPACE), nvGetKey(NK_PRIOR), nvGetKey(NK_NEXT), nvGetKey(NK_END), nvGetKey(NK_HOME)); tempstr[1023] = 0;
			nvDraw2dText(tempstr, 0, 24, fontid, texid, 0.75, 0.75, CWHITE);

			swprintf(tempstr, 1023, L"NK_LEFT %d NK_UP %d NK_RIGHT %d NK_DOWN %d NK_PRINTSCREEN %d NK_INSERT %d NK_DELETE %d",
				nvGetKey(NK_LEFT), nvGetKey(NK_UP), nvGetKey(NK_RIGHT), nvGetKey(NK_DOWN), nvGetKey(NK_PRINTSCREEN), nvGetKey(NK_INSERT), nvGetKey(NK_DELETE)); tempstr[1023] = 0;
			nvDraw2dText(tempstr, 0, 36, fontid, texid, 0.75, 0.75, CWHITE);

			swprintf(tempstr, 1023, L"NK_0 %d NK_1 %d NK_2 %d NK_3 %d NK_4 %d NK_5 %d NK_6 %d NK_7 %d NK_8 %d NK_9 %d",
				nvGetKey(NK_0), nvGetKey(NK_1), nvGetKey(NK_2), nvGetKey(NK_3), nvGetKey(NK_4), nvGetKey(NK_5), nvGetKey(NK_6), nvGetKey(NK_7), nvGetKey(NK_8), nvGetKey(NK_9)); tempstr[1023] = 0;
			nvDraw2dText(tempstr, 0, 48, fontid, texid, 0.75, 0.75, CWHITE);

			swprintf(tempstr, 1023, L"NK_A %d NK_B %d NK_C %d NK_D %d NK_E %d NK_F %d NK_G %d NK_H %d NK_I %d NK_J %d NK_K %d NK_L %d NK_M %d NK_N %d",
				nvGetKey(NK_A), nvGetKey(NK_B), nvGetKey(NK_C), nvGetKey(NK_D), nvGetKey(NK_E), nvGetKey(NK_F), nvGetKey(NK_G), nvGetKey(NK_H), nvGetKey(NK_I), nvGetKey(NK_J)
				, nvGetKey(NK_K), nvGetKey(NK_L), nvGetKey(NK_M), nvGetKey(NK_N)); tempstr[1023] = 0;
			nvDraw2dText(tempstr, 0, 60, fontid, texid, 0.75, 0.75, CWHITE);

			swprintf(tempstr, 1023, L"NK_O %d NK_P %d NK_Q %d NK_R %d NK_S %d NK_T %d NK_U %d NK_V %d NK_W %d NK_X %d NK_Y %d NK_Z %d",
				nvGetKey(NK_O), nvGetKey(NK_P), nvGetKey(NK_Q), nvGetKey(NK_R), nvGetKey(NK_S), nvGetKey(NK_T), nvGetKey(NK_U), nvGetKey(NK_V), nvGetKey(NK_W), nvGetKey(NK_X), nvGetKey(NK_Y), nvGetKey(NK_Z)); tempstr[1023] = 0;
			nvDraw2dText(tempstr, 0, 72, fontid, texid, 0.75, 0.75, CWHITE);

			swprintf(tempstr, 1023, L"NK_LWIN %d NK_RWIN %d NK_APP %d NK_NUMPAD0 %d NK_NUMPAD1 %d NK_NUMPAD2 %d NK_NUMPAD3 %d",
				nvGetKey(NK_LWIN), nvGetKey(NK_RWIN), nvGetKey(NK_APP), nvGetKey(NK_NUMPAD0), nvGetKey(NK_NUMPAD1), nvGetKey(NK_NUMPAD2), nvGetKey(NK_NUMPAD3)); tempstr[1023] = 0;
			nvDraw2dText(tempstr, 0, 84, fontid, texid, 0.75, 0.75, CWHITE);

			swprintf(tempstr, 1023, L"NK_NUMPAD4 %d NK_NUMPAD5 %d NK_NUMPAD6 %d NK_NUMPAD7 %d NK_NUMPAD8 %d NK_NUMPAD9 %d",
				nvGetKey(NK_NUMPAD4), nvGetKey(NK_NUMPAD5), nvGetKey(NK_NUMPAD6), nvGetKey(NK_NUMPAD7), nvGetKey(NK_NUMPAD8), nvGetKey(NK_NUMPAD9)); tempstr[1023] = 0;
			nvDraw2dText(tempstr, 0, 96, fontid, texid, 0.75, 0.75, CWHITE);

			swprintf(tempstr, 1023, L"NK_MULTIPLY %d NK_ADD %d NK_SEPARATOR %d NK_SUBSTRACT %d NK_DECIMAL %d NK_DIVIDE %d",
				nvGetKey(NK_MULTIPLY), nvGetKey(NK_ADD), nvGetKey(NK_SEPARATOR), nvGetKey(NK_SUBSTRACT), nvGetKey(NK_DECIMAL), nvGetKey(NK_DIVIDE)); tempstr[1023] = 0;
			nvDraw2dText(tempstr, 0, 108, fontid, texid, 0.75, 0.75, CWHITE);

			swprintf(tempstr, 1023, L"NK_F1 %d NK_F2 %d NK_F3 %d NK_F4 %d NK_F5 %d NK_F6 %d NK_F7 %d NK_F8 %d NK_F9 %d NK_F10 %d NK_F11 %d NK_F12 %d",
				nvGetKey(NK_F1), nvGetKey(NK_F2), nvGetKey(NK_F3), nvGetKey(NK_F4), nvGetKey(NK_F5), nvGetKey(NK_F6), nvGetKey(NK_F7)
				, nvGetKey(NK_F8), nvGetKey(NK_F9), nvGetKey(NK_F10), nvGetKey(NK_F11), nvGetKey(NK_F12)); tempstr[1023] = 0;
			nvDraw2dText(tempstr, 0, 120, fontid, texid, 0.75, 0.75, CWHITE);

			swprintf(tempstr, 1023, L"NK_NUMLOCK %d NK_SCROLL %d NK_LSHIFT %d NK_RSHIFT %d NK_LCONTROL %d NK_RCONTROL %d NK_LMENU(NK_LALT) %d NK_RMENU(NK_RALT) %d",
				nvGetKey(NK_NUMLOCK), nvGetKey(NK_SCROLL), nvGetKey(NK_LSHIFT), nvGetKey(NK_RSHIFT), nvGetKey(NK_LCONTROL), nvGetKey(NK_RCONTROL), nvGetKey(NK_LMENU), nvGetKey(NK_RMENU)); tempstr[1023] = 0;
			nvDraw2dText(tempstr, 0, 132, fontid, texid, 0.75, 0.75, CWHITE);

			swprintf(tempstr, 1023, L"NV_OEM_1(;) %d NV_OEM_PLUS(=) %d NK_OEM_COMMA(<) %d NK_OEM_MINUS(_) %d NK_OEM_PERIOD(>) %d NK_OEM_2(?) %d NK_OEM_3(~) %d",
				nvGetKey(NK_OEM_1), nvGetKey(NK_OEM_PLUS), nvGetKey(NK_OEM_COMMA), nvGetKey(NK_OEM_MINUS), nvGetKey(NK_OEM_PERIOD), nvGetKey(NK_OEM_2), nvGetKey(NK_OEM_3)); tempstr[1023] = 0;
			nvDraw2dText(tempstr, 0, 144, fontid, texid, 0.75, 0.75, CWHITE);

			swprintf(tempstr, 1023, L"NV_OEM_4([) %d NV_OEM_5(\\|) %d NV_OEM_6(]) %d NV_OEM_7(\") %d",
				nvGetKey(NK_OEM_4), nvGetKey(NK_OEM_5), nvGetKey(NK_OEM_6), nvGetKey(NK_OEM_7)); tempstr[1023] = 0;
			nvDraw2dText(tempstr, 0, 156, fontid, texid, 0.75, 0.75, CWHITE);

			for(i = 1; i <= 12; i++) {
				swprintf(tempstr, 1023, L"NK_F%d %d", 12+i, nvGetKey(NK_F12+i)); tempstr[1023] = 0;
				nvDraw2dText(tempstr, 0, 156+i*12, fontid, texid, 0.75, 0.75, CWHITE);
			}
		nvEnd2d();
		nUpdate();
	}
	nClose();

	NYAN_CLOSE

	return 0;
}
