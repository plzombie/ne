/*
	Файл	: nyan_container_defs.h

	Описание: Общие перечисления и типы данных, используемые вспомогательными функциями для работы с потоками

	История	: 04.06.18	Создан

*/

#ifndef NYAN_CONTAINER_DEFS_H
#define NYAN_CONTAINER_DEFS_H

enum {
	NCF_SUCCESS = 0,
	NCF_ERROR_WRONGFILETYPE,
	NCF_ERROR_CANTFINDFILETYPE,
	NCF_ERROR_CANTFINDCHUNK,
	NCF_ERROR_DAMAGEDFILE,
	NCF_FAIL
};

#endif
