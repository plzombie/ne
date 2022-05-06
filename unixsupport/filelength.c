/*
	Файл	: filelength.c

	Описание: Реализация filelength поверх lseek

	История	: 04.04.14	Создан

*/

#include <sys/types.h>
#include <unistd.h>

long filelength(int handle)
{
	off_t filesize, position;

	position = lseek(handle, 0, SEEK_CUR);/*tell(handle)*/; // Узнаём текущую позицию

	filesize = lseek(handle, 0, SEEK_END); // Переходим в конец файла

	lseek(handle, position, SEEK_SET); // Устанавливаем обратно позицию

	return filesize;
}
