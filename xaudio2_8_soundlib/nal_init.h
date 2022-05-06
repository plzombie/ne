/*
	Файл	: nal_init.h

	Описание: Заголовок для nal_init.c

	История	: 21.12.12	Создан

*/

#include "../nyan/nyan_engapi.h"

extern engapi_type *nal_ea;

extern IXAudio2 *nal_xaudio2;
extern IXAudio2MasteringVoice *nal_masteringvoice;

extern unsigned int nal_isinit;
