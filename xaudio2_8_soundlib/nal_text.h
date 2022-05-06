/*
	Файл	: nal_text.h

	Описание: Весь текст

	История	: 21.12.12	Создан

*/

#include "../nyan/nyan_text.h"

// Названия функций

#define F_NALINIT L"nalInit()"
#define F_NALCLOSE L"nalClose()"

#define F_NALCREATEBUFFER L"nalCreateBuffer()"
#define F_NALDESTROYBUFFER L"nalDestroyBuffer()"
#define F_NALLOADBUFFER L"nalLoadBuffer()"
#define F_NALUNLOADBUFFER L"nalUnloadBuffer()"
#define F_NALCREATESOURCE L"nalCreateSource()"
#define F_NALCREATEAUDIOSTREAM L"nalCreateAudioStream()"
#define F_NALDESTROYSOURCE L"nalDestroySource()"
#define F_NALDESTROYAUDIOSTREAM L"nalDestroyAudioStream()"

// Сообщения об ошибках

#define ERR_CANTCREATEXAUDIO2INTERFACE L"Can't create XAudio2 interface"
#define ERR_CANTCREATEMASTERINGVOICE L"Can't create mastering voice"
