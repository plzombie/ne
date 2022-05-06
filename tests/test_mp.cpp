
#include <stdio.h>
#include <stdlib.h>

#include "../nyan/nyan_publicapi.h"

#include "../forks/mparticles/platform_win_posix.h"
#include "../forks/mparticles/mp_wrap.h"

const int CWHITE = NV_COLOR(255,255,255,255);

NYAN_MAIN
{
	int client_wi = 800;
	int client_he = 600;
	int cur = 0;

	NYAN_INIT

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetScreen(800, 600, 32, NV_MODE_WINDOWED_FIXED, 0);

	nvSetStatusw(NV_STATUS_WINTITLE, L"Magic Particles (www.astralax.com) wrapper");

	nMountDir(L"media");
	nMountDir(L"../media");
	nMountDir(L"../../media");
	nMountDir(L"../../../media");

#if defined(_N_POSIX)
	nAddPlugin(L"/home/miha/libplgkplib_pictures.so");
#else
	nAddPlugin(L"plgkplib_pictures.dll");
#endif

	if(!nInit()) return 0;

	MP_Device_WRAP device(client_wi, client_he);
	device.Create();

	MP_Manager& MP = MP_Manager::GetInstance();

	MP_Platform* platform = new MP_Platform_WIN_POSIX;
	MP.Initialization(MAGIC_pXpY, platform, MAGIC_INTERPOLATION_ENABLE, MAGIC_CHANGE_EMITTER_DEFAULT, 1024, 1024, 1, 1.f, 0.1f, true);

	// eng: find of all ptc-files in folder
	// rus: поиск всех ptc-файлов в папке
	MP.LoadAllEmitters();

	MP.RefreshAtlas();

	MP.CloseFiles();

	MP.Stop();

	cur = MP.GetFirstEmitter();

	MP_Emitter* emitter = MP.GetEmitter(cur);
	emitter->SetState(MAGIC_STATE_UPDATE);

	// eng: locate emitters the same as editor
	// rus: расставляем эмиттеры также, как они стояли в редакторе
	HM_EMITTER hmEmitter = MP.GetFirstEmitter();
	while(hmEmitter)
	{
		Magic_CorrectEmitterPosition(hmEmitter, client_wi, client_he);
		hmEmitter = MP.GetNextEmitter(hmEmitter);
	}

	while(!nvGetStatusi(NV_STATUS_WIN_EXITMSG) && !nvGetKey(27)) {
		MP.UpdateByTimer();

		MP_Emitter* emitter = MP.GetEmitter(cur);
		if(emitter->GetState()==MAGIC_STATE_STOP)
		{
			cur = MP.GetNextEmitter(cur);
			if(!cur)
				cur = MP.GetFirstEmitter();

			emitter = MP.GetEmitter(cur);
			emitter->SetState(MAGIC_STATE_UPDATE);
		}

		MP.Render();

		nUpdate();
	}

	MP.Destroy();
	device.Destroy();

	nClose();

	NYAN_CLOSE

	return 0;
}
