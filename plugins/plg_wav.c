/*
	Файл	: plg_wav.c

	Описание: Плагин для загрузки wav

	История	: 01.09.12	Создан

*/

#include <stdlib.h>
#include <string.h>

#include "../extclib/_wcsicmp.h"

#include "../nyan/nyan_text.h"
#include "../nyan/nyan_log_publicapi.h"
#include "../nyan/nyan_filesys_publicapi.h"
#include "../nyan/nyan_mem_publicapi.h"
#include "../nyan/nyan_file_publicapi.h"
#include "../nyan/nyan_audiofile_publicapi.h"
#include "../nyan/nyan_plgtypes_publicapi.h"

#include "plg_wav.h"

typedef struct {
	unsigned int offset; // Сколько байт нужно пропустить до начала семпла
	unsigned int compression; // Компрессия. 1 - не сжатый, 6 - A-law
	unsigned int fh; // Хендл файла
} plgwav_type;

typedef struct {
	unsigned short wFormatTag;
	unsigned short nChannels;
	unsigned int nSamplesPerSec;
	unsigned int nAvgBytesPerSec;
	unsigned short nBlockAlign;
	unsigned short wBitsPerSample;
} waveformat_type;

bool N_APIENTRY plgWAVSupportExt(const wchar_t *fname, const wchar_t *fext);
bool N_APIENTRY plgWAVLoad(const wchar_t *fname, na_audiofile_type *aud);
bool N_APIENTRY plgWAVRead(unsigned int offset, unsigned int nofs, void *buf, na_audiofile_type *aud);
void N_APIENTRY plgWAVUnload(na_audiofile_type *aud);

na_audiofile_plugin_type plgWAV = {sizeof(na_audiofile_plugin_type), &plgWAVSupportExt, &plgWAVLoad, &plgWAVRead, &plgWAVUnload};

/*
	Функция	: plgWAVSupportExt

	Описание: Проверяет поддержку типа файла

	История	: 01.09.12	Создан

*/
bool N_APIENTRY plgWAVSupportExt(const wchar_t *fname, const wchar_t *fext)
{
	(void)fname; // Неиспользуемая переменная

	if(_wcsicmp(fext,L"wav") == 0) return true;
	return false;
}

/*
	Функция	: plgWAVLoad

	Описание: Загружает wav файл

	История	: 01.09.12	Создан

*/
bool N_APIENTRY plgWAVLoad(const wchar_t *fname, na_audiofile_type *aud)
{
	unsigned int f, i;
	unsigned int filemaxsize = 0, infsize = 0;
	waveformat_type waveformat;
	plgwav_type *plgdata;
	char sign[5] = {0, 0, 0, 0, 0};

	nlPrint(LOG_FDEBUGFORMAT7, F_PLGWAVLOAD, N_FNAME, fname); nlAddTab(1);

	f = nFileOpen(fname);

	if(f == 0) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGWAVLOAD, ERR_FILENOTFOUNDED);
		nFileClose(f);
		return false;
	}

	nFileRead(f, sign, 4);
	if(strcmp(sign,"RIFF") != 0){
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGWAVLOAD, ERR_WRONGSIGNIN);
		nFileClose(f);
		return false;
	}
	nFileRead(f, &filemaxsize, 4);
	nFileRead(f, sign, 4);
	if(strcmp(sign,"WAVE") != 0){
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGWAVLOAD, ERR_WRONGSIGNIN);
		nFileClose(f);
		return false;
	}
	nFileRead(f, sign, 4);
	if(strcmp(sign,"fmt ") != 0){
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGWAVLOAD, ERR_WRONGSIGNIN);
		nFileClose(f);
		return false;
	}
	nFileRead(f, &infsize, 4);
	nFileRead(f, &waveformat, sizeof(waveformat_type));

	if( (waveformat.wFormatTag != 1) && (waveformat.wFormatTag != 6) ){
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGWAVLOAD, ERR_UNSUPPORTEDDATATYPE);
		nFileClose(f);
		return false;
	}

	if     (waveformat.wFormatTag == 1 && waveformat.nChannels == 1 && waveformat.wBitsPerSample == 8) aud->sf = NA_SOUND_8BIT_MONO;
	else if(waveformat.wFormatTag == 1 && waveformat.nChannels == 2 && waveformat.wBitsPerSample == 8) aud->sf = NA_SOUND_8BIT_STEREO;
	else if(waveformat.nChannels == 1 && (waveformat.wBitsPerSample == 16 || waveformat.wFormatTag == 6) ) aud->sf = NA_SOUND_16BIT_MONO;
	else if(waveformat.nChannels == 2 && (waveformat.wBitsPerSample == 16 || waveformat.wFormatTag == 6) )  aud->sf = NA_SOUND_16BIT_STEREO;
	else {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGWAVLOAD, ERR_UNSUPPORTEDDATATYPE);
		nFileClose(f);
		return false;
	}

	nFileSeek(f, infsize-sizeof(waveformat), FILE_SEEK_CUR);

	i = 0;
	while(!i) {
		nFileRead(f, sign, 4);
		nFileRead(f, &infsize, 4);
		if(!strcmp(sign,"data"))
			i = 1;
		else if(nFileSeek(f, infsize, FILE_SEEK_CUR) == -1L)
			i = 2;
	}

	if(i == 2) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGWAVLOAD, ERR_FILEISDAMAGED);
		nFileClose(f);
		return false;
	}

	plgdata = nAllocMemory(sizeof(plgwav_type));
	if(!plgdata) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGWAVLOAD, N_FALSE);
		nFileClose(f);
		return false;
	}
	aud->plgdata = plgdata;
	plgdata->offset = (unsigned int)nFileTell(f);
	plgdata->compression = waveformat.wFormatTag;
	plgdata->fh = f;
	aud->bps = waveformat.wBitsPerSample>>3; // байт в одном семпле
	aud->nos = infsize/aud->bps/waveformat.nChannels; // Количество семплов
	aud->noc = waveformat.nChannels; // Кол-во каналов
	aud->freq = waveformat.nSamplesPerSec; // Частота
	if(waveformat.wFormatTag == 6)
		aud->bps = 2; // байт в одном семпле
	/*nlPrint(L"aud->bps %d",aud->bps);
	nlPrint(L"aud->nos %d",aud->nos);
	nlPrint(L"aud->noc %d",aud->noc);
	nlPrint(L"aud->freq %d",aud->freq);
	nlPrint(L"infsize %d",infsize);
	nlPrint(L"aud->sf %d",aud->sf);
	nlPrint(L"plgdata->offset %d",plgdata->offset);*/

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGWAVLOAD, N_OK);

	return true;
}

/*
	Функция	: plgWAVRead

	Описание: Читает wav файл
			offset - указывает количество секунд, которые надо пропустить от начала записи
			nofs - Количество семплов, которые надо прочитать (если стерео, то включая семплы для левого и правого каналов)
			aud - Структура, содержащая данные об аудиофайле
			Все три значения __всегда__ правильные, т.е. проверка не требуется
	История	: 01.09.12	Создан

*/
bool N_APIENTRY plgWAVRead(unsigned int offset, unsigned int nofs, void *buf, na_audiofile_type *aud)
{
	unsigned int foffset, i;
	plgwav_type *plgdata;
	plgdata = aud->plgdata;
	foffset = plgdata->offset+offset*aud->freq*aud->noc*aud->bps;
	nFileSeek(plgdata->fh, foffset, FILE_SEEK_SET);
	switch(plgdata->compression) {
		case 1:
			nFileRead(plgdata->fh, buf, nofs*aud->noc*aud->bps);
			break;
		case 6:
			nFileRead(plgdata->fh, (char *)buf+nofs*aud->noc*1, nofs*aud->noc*1);
			for(i = 0; i < nofs*aud->noc; i++) {
				int t, t2, t3;

				t = ((char *)buf+nofs*aud->noc)[i];

				t = t^0x55;

				t2 = (t&15)<<4;

				t3 = (t&112)>>4;

				if(t3) {
					t2 = t2 | 256;
					t3--;
					if(t3)
						t2 = t2<<t3;
				}

				if(t&128) // Старший бит, (t&128)<<8 | t2
					((short *)buf)[i] = t2;
				else
					((short *)buf)[i] = -t2;
			}
			break;
		default:
			return false;
	}

	return true;
}

/*
	Функция	: plgWAVUnload

	Описание: Выгружает wav файл

	История	: 01.09.12	Создан

*/
void N_APIENTRY plgWAVUnload(na_audiofile_type *aud)
{
	plgwav_type *plgdata;
	plgdata = aud->plgdata;
	nFileClose(plgdata->fh);
	nFreeMemory(aud->plgdata);
}
