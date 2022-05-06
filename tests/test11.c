
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "../nyan/nyan_publicapi.h"

const int CWHITE = NV_COLOR(255,255,255,255);
const int CRED = NV_COLOR(255,0,0,255);
const int CGREEN = NV_COLOR(0,255,0,255);
const int CBLUE = NV_COLOR(0,0,255,255);

NYAN_MAIN
{
	wchar_t *temp;
	
	NYAN_INIT

	if(!nvAttachRender(L"nullgl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	temp = nFileGetAbsoluteFilename(L"", NF_PATH_CURRENTDIR);
	if(temp) {
		nlPrint(L"NF_PATH_CURRENTDIR: \'%ls\'", temp);
		free(temp);
	}

	temp = nFileGetAbsoluteFilename(L"", NF_PATH_USERDIR);
	if(temp) {
		nlPrint(L"NF_PATH_DATA_USERDIR: \'%ls\'", temp);
		free(temp);
	}

	temp = nFileGetAbsoluteFilename(L"", NF_PATH_DATA_ROAMING);
	if(temp) {
		nlPrint(L"NF_PATH_DATA_ROAMING: \'%ls\'", temp);
		free(temp);
	}

	temp = nFileGetAbsoluteFilename(L"", NF_PATH_DATA_LOCAL);
	if(temp) {
		nlPrint(L"NF_PATH_DATA_LOCAL: \'%ls\'", temp);
		free(temp);
	}

	temp = nFileGetAbsoluteFilename(L"", NF_PATH_DOCUMENTS);
	if(temp) {
		nlPrint(L"NF_PATH_DATA_DOCUMENTS: \'%ls\'", temp);
		free(temp);
	}

	temp = nFileGetAbsoluteFilename(L"", NF_PATH_GAMESAVES);
	if(temp) {
		nlPrint(L"NF_PATH_DATA_GAMESAVES: \'%ls\'", temp);
		free(temp);
	}

	temp = nFileGetAbsoluteFilename(L"", NF_PATH_PICTURES);
	if(temp) {
		nlPrint(L"NF_PATH_DATA_PICTURES: \'%ls\'", temp);
		free(temp);
	}

	temp = nFileGetAbsoluteFilename(L"", NF_PATH_MUSIC);
	if(temp) {
		nlPrint(L"NF_PATH_DATA_MUSIC: \'%ls\'", temp);
		free(temp);
	}

	temp = nFileGetAbsoluteFilename(L"", NF_PATH_VIDEOS);
	if(temp) {
		nlPrint(L"NF_PATH_DATA_VIDEOS: \'%ls\'", temp);
		free(temp);
	}

	temp = nFileGetAbsoluteFilename(L"", NF_PATH_DOWNLOADS);
	if(temp) {
		nlPrint(L"NF_PATH_DATA_DOWNLOADS: \'%ls\'", temp);
		free(temp);
	}

	NYAN_CLOSE

	return 0;
}
