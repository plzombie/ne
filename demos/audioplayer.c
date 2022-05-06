/*
	Файл	: audioplayer.с

	Описание: Пример консольного аудиоплеера

	История	: 17.08.17	Создан

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

#include "../nyan/nyan_entry_publicapi.h"
#include "../nyan/nyan_au_init_publicapi.h"
#include "../nyan/nyan_au_main_publicapi.h"
#include "../nyan/nyan_init_publicapi.h"
#include "../nyan/nyan_log_publicapi.h"
#include "../nyan/nyan_plgloader_publicapi.h"
#include "../nyan/nyan_threads_publicapi.h"
#include "../nyan/nyan_vis_init_publicapi.h"

int wmain(int argc, wchar_t **argv)
{
	unsigned int srcid;
	unsigned int old_offset = 0;

	NYAN_INIT

	if(argc == 1) {
		wprintf(L"%ls", L"  Audioplayer\n    usage: audioplayer [files]\n");

		return 0;
	}

	nlEnable(false);

	if(!naAttachLib(L"nal"))
		return 1;

	if(!nvAttachRender(L"nullgl"))
		return 2;

	nAddPlugin(L"plgwavpack");
	nAddPlugin(L"plgstb_vorbis");

	if(!nInit())
		return 3;

	srcid = naCreateAudioStreamEx((const wchar_t **)(argv+1), argc-1, false);
	if(!srcid)
		return 4;

	naPlayAudioStream(srcid);

	while(1) {
		unsigned int offset, track, status;

		nUpdate();

		naGetAudioStreamSecOffset(srcid, &offset, &track);
		status = naGetAudioStreamStatus(srcid);

		if(status != NA_SOURCE_PLAY)
			break;

		if(offset != old_offset) {
			fputwc('.', stdout);
			fflush(stdout);
		}

		old_offset = offset;

		nSleep(50);
	}

	naDestroyAudioStream(srcid);

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
