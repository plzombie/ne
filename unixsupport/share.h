/*
	Файл	: share.h

	Описание: Параметры доступа для _wsopen и _wfsopen

	История	: 27.06.18	Создан

*/

#ifndef SHARE_H

#ifndef _SH_COMPAT
	#define _SH_COMPAT 0
#endif

#ifndef _SH_DENYRW
	#define _SH_DENYRW 0x10
#endif

#ifndef _SH_DENYWR
	#define _SH_DENYWR 0x20
#endif

#ifndef _SH_DENYRD
	#define _SH_DENYRD 0x30
#endif

#ifndef _SH_DENYNO
	#define _SH_DENYNO 0x40
#endif

#endif
