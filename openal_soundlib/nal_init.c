/*
	Файл	: nal_init.c

	Описание: Инициализация аудио библиотеки

	История	: 21.12.12	Создан

*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../extclib/mbstowcsl.h"

#include "nal.h"
#include "nal_text.h"

#include "../forks/al/al.h"
#include "../forks/al/alc.h"

#include "nal_init.h"
#include "nal_main.h"

N_API bool N_APIENTRY_EXPORT nalClose(void);

static ALCcontext *nal_alcontext;
static ALCdevice *nal_aldevice;

engapi_type *nal_ea = 0;

unsigned int nal_isinit = false;

/*
	Функция	: nalSetupDll

	Описание: Настройка dll аудио библиотеки

	История	: 21.12.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nalSetupDll(engapi_type *engapi)
{
	if(engapi) {
		if(engapi->mysize == sizeof(engapi_type)) {
			nal_ea = engapi;

			return true;
		}
	}

	return false;
}

/*
	Функция	: nalInit

	Описание: Инициализация аудио движка

	История	: 25.21.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nalInit(void)
{
	wchar_t *tempt = 0, *_tempt = 0;
	
	if(nal_isinit || !nal_ea) return false;
	
	nal_ea->nlPrint(F_NALINIT); nal_ea->nlAddTab(1);

	nal_aldevice = alcOpenDevice(0);
	if(!nal_aldevice) {
		nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT,F_NALINIT, ERR_CANTCREATEDEVICE);
		
		return false;
	}
	nal_alcontext = alcCreateContext(nal_aldevice, 0);
	if(!nal_alcontext) {
		nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT,F_NALINIT, ERR_CANTCREATECONTEXT);
		alcCloseDevice(nal_aldevice);	

		return false;
	}
	alcMakeContextCurrent(nal_alcontext);

	tempt = nal_ea->nAllocMemory(sizeof(wchar_t)*(strlen(alGetString(AL_RENDERER))+1));
	if(tempt) {
		mbstowcsl(tempt,alGetString(AL_RENDERER),strlen(alGetString(AL_RENDERER))+1);
		nal_ea->nlPrint(LOG_FDEBUGFORMAT2,F_NALINIT,L"AL_RENDERER", tempt);
	}
	
	_tempt = nal_ea->nReallocMemory(tempt,sizeof(wchar_t)*(strlen(alGetString(AL_VENDOR))+1));
	if(_tempt) {
		tempt = _tempt;
		mbstowcsl(tempt,alGetString(AL_VENDOR),strlen(alGetString(AL_VENDOR))+1);
		nal_ea->nlPrint(LOG_FDEBUGFORMAT2,F_NALINIT,L"AL_VENDOR", tempt);
	}
	
	_tempt = nal_ea->nReallocMemory(tempt,sizeof(wchar_t)*(strlen(alGetString(AL_VERSION))+1));
	if(_tempt) {
		tempt = _tempt;
		mbstowcsl(tempt,alGetString(AL_VERSION),strlen(alGetString(AL_VERSION))+1);
		nal_ea->nlPrint(LOG_FDEBUGFORMAT2,F_NALINIT,L"AL_VERSION", tempt);
	}
	
	_tempt = nal_ea->nReallocMemory(tempt,sizeof(wchar_t)*(strlen(alGetString(AL_EXTENSIONS))+1));
	if(_tempt) {
		tempt = _tempt;
		mbstowcsl(tempt,alGetString(AL_EXTENSIONS),strlen(alGetString(AL_EXTENSIONS))+1);
		nal_ea->nlPrint(LOG_FDEBUGFORMAT2,F_NALINIT,L"AL_EXTENSIONS", tempt);
	}
	if(tempt)
		nal_ea->nFreeMemory(tempt);

	nal_isinit = true;
	
	if(!nalCreateAST()) {
		nalClose();
		nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT, F_NALINIT, N_FALSE);
		return false;
	}
	
	nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT, F_NALINIT, N_OK);
	
	return true;
}

/*
	Функция	: nalClose

	Описание: Завершает работу аудиодвижка

	История	: 25.12.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nalClose(void)
{
	if(!nal_isinit) return false;

	nal_ea->nlPrint(F_NALCLOSE); nal_ea->nlAddTab(1);
	
	nalDestroyAllAudioStreams();
	nalDestroyAllSources();
	nalDestroyAllBuffers();
	
	nalDestroyAST(); // Вызывается до "na_isinit = false;"

	nal_isinit = false;
	
	alcMakeContextCurrent(0);
	alcDestroyContext(nal_alcontext);
	alcCloseDevice(nal_aldevice);	

	nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT, F_NALCLOSE, N_OK);
	
	return true;
}
