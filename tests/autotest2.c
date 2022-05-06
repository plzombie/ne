/*
	Тестирование mbstowcsl и wcstombsl
*/

#include "../extclib/mbstowcsl.h"
#include "../extclib/wcstombsl.h"

#include <stdio.h>
#include <string.h>
#include <wchar.h>

const char test_s1[] = "Test Str";
const wchar_t test_ws1[] = L"Test Str";

const size_t temp_size = 10;

int main(void)
{
	int fails = 0; // Количество ошибочных тестов

	char temps[10];
	wchar_t tempws[10];

	// 1. wcstombsl

	// 1.1. Реагирует на dest == NULL
	printf("%s", "1.1. wcstombsl checks for dest == NULL\n");
	if(wcstombsl(NULL, test_ws1, temp_size) != (size_t)-1) {
		fails++;
		printf("%s", "\tfailed\n");
	} else
		printf("%s", "\tok\n");

	// 1.2. Реагирует на src == NULL
	printf("%s", "1.2. wcstomsl checks for src == NULL\n");
	if(wcstombsl(temps, NULL, temp_size) != (size_t)-1) {
		fails++;
		printf("%s", "\tfailed\n");
	} else
		printf("%s", "\tok\n");

	// 1.3 Реагирует на max == 0
	printf("%s", "1.3. wcstombsl checks for max == 0\n");
	if(wcstombsl(temps, test_ws1, 0) != 0) {
		fails++;
		printf("%s", "\tfailed\n");
	} else
		printf("%s", "\tok\n");

	// 1.4 Конвертирует строку полностью
	printf("%s", "1.4 wcstombsl converts full string\n");
	if(wcstombsl(temps, test_ws1, temp_size) != strlen(test_s1)) {
		fails++;
		printf("%s", "\tfailed\n");
	} else if(strcmp(temps, test_s1))  {
		fails++;
		printf("%s", "\tfailed\n");
	} else
		printf("%s", "\tok\n");

	// 1.5 Конвертирует строку частично
	printf("%s", "1.5 wcstombsl converts part of string\n");
	if(wcstombsl(temps, test_ws1, 5) != 4)  {
		fails++;
		printf("%s", "\tfailed\n");
	} else if(strncmp(temps, test_s1, 4) != 0 || temps[4] != 0) {
		fails++;
		printf("%s", "\tfailed\n");
	} else
		printf("%s", "\tok\n");


	// 2. mbstowcsl

	// 2.1. Реагирует на dest == NULL
	printf("%s", "2.1. mbstowcsl checks for dest == NULL\n");
	if(mbstowcsl(NULL, test_s1, temp_size) != (size_t)-1) {
		fails++;
		printf("%s", "\tfailed\n");
	} else
		printf("%s", "\tok\n");

	// 2.2. Реагирует на src == NULL
	printf("%s", "2.2. mbstowcsl checks for src == NULL\n");
	if(mbstowcsl(tempws, NULL, temp_size) != (size_t)-1) {
		fails++;
		printf("%s", "\tfailed\n");
	} else
		printf("%s", "\tok\n");

	// 2.3 Реагирует на max == 0
	printf("%s", "2.3. mbstowcsl checks for max == 0\n");
	if(mbstowcsl(tempws, test_s1, 0) != 0) {
		fails++;
		printf("%s", "\tfailed\n");
	} else
		printf("%s", "\tok\n");

	// 2.4 Конвертирует строку полностью
	printf("%s", "2.4 mbstowcsl converts full string\n");
	if(mbstowcsl(tempws, test_s1, temp_size) != wcslen(test_ws1)) {
		fails++;
		printf("%s", "\tfailed\n");
	} else if(wcscmp(tempws, test_ws1))  {
		fails++;
		printf("%s", "\tfailed\n");
	} else
		printf("%s", "\tok\n");

	// 2.5 Конвертирует строку частично
	printf("%s", "2.5 mbstowcsl converts part of string\n");
	if(mbstowcsl(tempws, test_s1, 5) != 4)  {
		fails++;
		printf("%s", "\tfailed\n");
	} else if(wcsncmp(tempws, test_ws1, 4) != 0 || tempws[4] != 0) {
		fails++;
		printf("%s", "\tfailed\n");
	} else
		printf("%s", "\tok\n");

	printf("Total %d failed tests\n", fails);

	return fails;
}
