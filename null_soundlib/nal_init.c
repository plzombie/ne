/*
	Файл	: nal_init.c

	Описание: Инициализация аудио библиотеки

	История	: 21.12.12	Создан

*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "nal.h"
#include "nal_text.h"

#include "nal_init.h"
#include "nal_main.h"

N_API bool N_APIENTRY_EXPORT nalClose(void);

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
	if(nal_isinit || !nal_ea) return false;
	
	nal_ea->nlPrint(F_NALINIT); nal_ea->nlAddTab(1);

	nal_isinit = true;
	
	// Инициализация audio api

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
	
	nal_isinit = false;
	
	nalDestroyAST(); // Вызывается после "na_isinit = false;"
	
	// Деинициализация audio api	

	nal_ea->nlAddTab(-1); nal_ea->nlPrint(LOG_FDEBUGFORMAT, F_NALCLOSE, N_OK);
	
	return true;
}

#if defined(N_CAUSEWAY) && !defined(N_NODYLIB)
int main(int reason)
{
	int result;

	if (!reason) {

		/*
		** DLL initialisation.
		*/
		wprintf(L"DLL startup...\n");

		/* return zero to let the load continue */
		result=0;

	} else {

		/*
		** DLL clean up.
		*/
		wprintf(L"DLL shutdown...\n");

		result=1;
	}

	return(result);
}
#endif
