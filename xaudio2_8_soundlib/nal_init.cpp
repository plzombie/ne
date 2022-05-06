/*
	Файл	: nal_init.c

	Описание: Инициализация аудио библиотеки

	История	: 21.12.12	Создан

*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <xaudio2.h>

#include "nal.h"
#include "nal_text.h"

#include "nal_init.h"
#include "nal_main.h"

extern "C" N_API bool N_APIENTRY_EXPORT nalClose(void);

engapi_type *nal_ea;

IXAudio2 *nal_xaudio2;
IXAudio2MasteringVoice *nal_masteringvoice;

unsigned int nal_isinit = false;

/*
	Функция	: nalSetupDll

	Описание: Настройка dll аудио библиотеки

	История	: 21.12.12	Создан

*/
extern "C" N_API bool N_APIENTRY_EXPORT nalSetupDll(engapi_type *engapi)
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
extern "C" N_API bool N_APIENTRY_EXPORT nalInit(void)
{
	HRESULT coinit_result;

	if(nal_isinit || !nal_ea) return false;
	
	nal_ea->nlPrint(F_NALINIT); nal_ea->nlAddTab(1);
	
	coinit_result = CoInitializeEx(0, COINIT_MULTITHREADED);

	if(coinit_result != S_OK && coinit_result != S_FALSE) {
		nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT, F_NALINIT, ERR_CANTCREATEXAUDIO2INTERFACE);

		return false;
	}

	if(XAudio2Create(&nal_xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR) != S_OK) {
		nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT, F_NALINIT, ERR_CANTCREATEXAUDIO2INTERFACE);

		CoUninitialize();

		return false;
	}

	if(nal_xaudio2->CreateMasteringVoice(&nal_masteringvoice, 2, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL) != S_OK) {
		nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT, F_NALINIT, ERR_CANTCREATEMASTERINGVOICE);

		nal_xaudio2->Release();

		CoUninitialize();

		return false;
	}

	// Инициализация audio api

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
extern "C" N_API bool N_APIENTRY_EXPORT nalClose(void)
{
	if(!nal_isinit) return false;

	nal_ea->nlPrint(F_NALCLOSE); nal_ea->nlAddTab(1);
	
	nalDestroyAllAudioStreams();
	nalDestroyAllSources();
	nalDestroyAllBuffers();
	
	nal_isinit = false;
	
	nalDestroyAST(); // Вызывается после "na_isinit = false;"
	
	nal_masteringvoice->DestroyVoice();

	nal_xaudio2->Release();

	CoUninitialize();

	nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT, F_NALCLOSE, N_OK);
	
	return true;
}
