
#include "../nyan/nyan_entry_publicapi.h"
#include "../nyan/nyan_au_init_publicapi.h"
#include "../nyan/nyan_vis_init_publicapi.h"
#include "../nyan/nyan_init_publicapi.h"
#include "../nyan/nyan_filesys_publicapi.h"
#include "../nyan/nyan_filesys_dirpaths_publicapi.h"

#if defined(__DOS__)
int mywmain(int argc, wchar_t **argv)
#else
int wmain(int argc, wchar_t **argv)
#endif
{
	wchar_t *inputfile, *outputfile;
	unsigned int ifhandle = 0, ofhandle = 0;
	unsigned char *tempbuf = 0; size_t tempsize;
	long long filesize, filepos = 0;

	NYAN_INIT

	if(argc < 3) {
		wprintf(L"%ls", L"  File copy\n    usage: filecopy inputfile outputfile\n");

		goto FINAL;
	}

	inputfile = argv[1];
	outputfile = argv[2];

	if(!nvAttachRender(L"nullgl")) goto FINAL;
	if(!naAttachLib(L"nullal")) goto FINAL;

	if(!nInit()) goto FINAL;

	wprintf(L"Input file: '%ls'\nOutput file: '%ls'\n", inputfile, outputfile);

	if(!nFileCreate(outputfile, true, NF_PATH_CURRENTDIR)) {
		wprintf(L"%ls", L"error: can't create output file\n");

		goto FINAL;
	}

	ifhandle = nFileOpen(inputfile);
	if(!ifhandle) {
		wprintf(L"%ls", L"error: can't open input file\n");

		goto FINAL;
	}

	ofhandle = nFileOpen(outputfile);
	if(!ofhandle) {
		wprintf(L"%ls", L"error: can't open output file\n");

		goto FINAL;
	}

	filesize = nFileLength(ifhandle);
	if(filesize == -1) {
		wprintf(L"%ls", L"error: can't get file size\n");
		goto FINAL;
	}
	wprintf(L"Input file size: %lld\n", filesize);


	if(!filesize)
		goto FINAL;

	if(filesize > 0x1000000) // 16MiB
		tempsize = 0x1000000;
	else
		tempsize = (size_t)filesize;

	tempbuf = malloc(tempsize);
	while(!tempbuf) {
		if(tempsize <= 0x400) // 4KiB
			break;

		wprintf(L"warning: can't allocate %u bytes", (unsigned int)tempsize);
		tempsize /= 2;
		wprintf(L", trying to allocate %u bytes\n", (unsigned int)tempsize);
		tempbuf = malloc(tempsize);
	}
	if(!tempbuf) {
		wprintf(L"%ls", L"error: can't allocate temporary buffer\n");

		goto FINAL;
	}

	while(filepos < filesize) {
		size_t bytestoread;

		if((filesize-filepos) < (long long)tempsize)
			bytestoread = (size_t)(filesize-filepos);
		else
			bytestoread = tempsize;

		// Тест на возможность вызова nFileSeek, можно закомментировать
		if(nFileSeek(ifhandle, filepos, FILE_SEEK_SET) != filepos) {
			wprintf(L"%ls", L"error: can't seek to current position\n");

			goto FINAL;
		}

		if(nFileRead(ifhandle, tempbuf, bytestoread) != (long long)bytestoread) {
			wprintf(L"%ls", L"error: can't read from input file\n");

			goto FINAL;
		}

		if(nFileWrite(ofhandle, tempbuf, bytestoread) != (long long)bytestoread) {
			wprintf(L"%ls", L"error: can't write to output file\n");

			goto FINAL;
		}

		filepos += bytestoread;

		// Тест на возможнось вызова nFileTell, можно закомментировать
		if(nFileTell(ifhandle) != filepos || nFileTell(ofhandle) != filepos) {
			wprintf(L"%ls", L"error: file positions differ from expected\n");

			goto FINAL;
		}

		wprintf(L"copied %lld of %lld\n", filepos, filesize);

	}

	if(filepos == filesize && nFileLength(ifhandle) != nFileLength(ofhandle)) {
		wprintf(L"warning: input file length not equal to output file length after copy procedure (%lld/%lld)\n", nFileLength(ifhandle), nFileLength(ofhandle));
	}


FINAL:
	if(ofhandle)
		nFileClose(ofhandle);
	if(ifhandle)
		nFileClose(ifhandle);
	if(tempbuf)
		free(tempbuf);

	if(nIsInit())
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

#elif defined(__linux__) || defined(__GNUC__) || defined(__FreeBSD__) || defined(__DOS__)

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

#if defined(__DOS__)
	ret = mywmain(argc, wargv);
#else
	ret = wmain(argc, wargv);
#endif

	for(i = 0; i < argc; i++)
		free(wargv[i]);

	free(wargv);

	return ret;
}
#endif
