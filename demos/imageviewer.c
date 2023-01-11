/*
	Файл	: imageviewer.с

	Описание: Пример программы загрузки изображений

	История	: 20.06.18	Создан

*/

#include <math.h>

#include "../nyan/nyan_publicapi.h"

const int CWHITE = NV_COLOR(255,255,255,255);

enum {
	STATE_LOADING,
	STATE_LOADED
};

typedef struct {
	wchar_t *name;
	unsigned int id;
	unsigned int w, h;
	uintptr_t mutex;
	bool is_finished;
} image_type;


void N_APIENTRY LoadingTask(void *param)
{
	image_type image;
	
	image = *((image_type *)param);

	image.id = nvCreateTextureFromFile(image.name, NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_FOR2D);
	if(image.id) {
		if(nvLoadTexture(image.id))
			nvGetTextureWH(image.id, &(image.w), &(image.h));
	}

	image.is_finished = true;

	nLockMutex(image.mutex);
		*((image_type *)param) = image;
	nUnlockMutex(image.mutex);
}

int wmain(int argc, wchar_t **argv)
{
	double secs = 0;
	unsigned int state = STATE_LOADING;
	image_type image;
	
	NYAN_INIT
	
	if(argc > 1) {
		image.name = argv[1];
		image.id = 0;
		image.w = image.h = 0;
		image.is_finished = false;
	} else {
		wprintf(L"%ls", L"  Image viewer\n    usage: imageviewer [files]\n");
		return 0;
	}
	
	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;
	
	nvSetStatusw(NV_STATUS_WINTITLE, L"Loading picture");
	
	nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 32);
	
	nvSetScreen(320, 160, 32, NV_MODE_WINDOWED, 0);
	
	if(!nInit()) return 0;
	
	// Add plugins after init to not interact with standard plugins
	nAddPlugin(L"plgrix");
	nAddPlugin(L"plgqoi");
	nAddPlugin(L"plgstb_image");
	
	image.mutex = nCreateMutex();
	if(!image.mutex) {
		nClose();
		return 0;
	}
	
	if(!nCreateTask(LoadingTask, &image, 0, 1)) {
		nDestroyMutex(image.mutex);
		nClose();
		return 0;
	}
	
	while(!nvGetStatusi(NV_STATUS_WIN_EXITMSG) && !nvGetKey(27)) {
		int w, h, i;
		
		secs += nGetspf();
		
		w = nvGetStatusi(NV_STATUS_WINX);
		h = nvGetStatusi(NV_STATUS_WINY);
		
		nvBegin2d();
		switch(state) {
			case STATE_LOADING:
				for(i = 0; i < 10; i++)
					nvJustDraw2dPoint((float)((w/20)*(2*i+1)), (float)((h/2)+(h/4)*cos((double)i*0.628+secs)), CWHITE);

				nLockMutex(image.mutex);
					if(image.is_finished) {
						state = STATE_LOADED;

						if(nvGetTextureStatus(image.id) != NV_TEX_STATUS_LOADED)
							nvSetStatusw(NV_STATUS_WINTITLE, L"Error loading picture");
						else
							nvSetStatusw(NV_STATUS_WINTITLE, image.name);
					}
				nUnlockMutex(image.mutex);

				break;
			case STATE_LOADED:
				if(!image.w || !image.h)
					break;
				if((float)w/(float)h > (float)image.w/(float)image.h) {
					float scale;

					scale = (float)h/(float)image.h;

					nvDraw2dPicture((w-(int)(image.w*scale))/2, 0, image.id, scale, scale, CWHITE);
				} else {
					float scale;

					scale = (float)w/(float)image.w;

					nvDraw2dPicture(0, (h-(int)(image.h*scale))/2, image.id, scale, scale, CWHITE);
				}
				break;
			default:
				break;
		}
		nvEnd2d();
		nUpdate();
	}

	nDestroyMutex(image.mutex);
	
	nClose();
	
	NYAN_CLOSE
	
	return 0;
}

#if defined(__GNUC__) && defined(__MINGW32__)

#include <windows.h>

int main(int argc, char **argv)
{
	LPWSTR cmdline;
	int wargc;
	LPWSTR *wargv;
	int ret;

	(void)argc;
	(void)argv;

	cmdline = GetCommandLineW();

	wargv = CommandLineToArgvW(cmdline, &wargc);

	if(!wargv)
		return -1;

	ret = wmain(wargc, wargv);

	LocalFree(wargv);

	return ret;
}

#elif defined(__linux__) || defined(__GNUC__) || defined(__FreeBSD__)

#include <string.h>
#include <locale.h>

int main(int argc, char **argv)
{
	int ret, i;
	wchar_t **wargv;

	setlocale(LC_ALL, "");

	wargv = malloc(argc * sizeof(wchar_t *));
	if(!wargv)
		return -1;

	for(i = 0; i < argc; i++) {
		int str_size;

		str_size = strlen(argv[i])+1;

		wargv[i] = malloc(str_size*sizeof(wchar_t));
		if(!(wargv[i])) {
			int j;
			
			for(j = 0; j < i; j++)
				free(wargv[j]);

			free(wargv);

			return -1;
		}

		if(mbstowcs(wargv[i], argv[i], str_size) == (size_t)(-1))
			wargv[i][0] = 0;
		else
			wargv[i][str_size - 1] = 0;
	}

	setlocale(LC_ALL, "C");

	ret = wmain(argc, wargv);

	for(i = 0; i < argc; i++)
		free(wargv[i]);

	free(wargv);

	return ret;
}
#endif
