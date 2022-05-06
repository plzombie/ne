/*
	Тестировани nyan_container_strings
*/

#include "../nyan_container/nyan_container_strings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define UTF16_TEST_SIZE 8
#ifdef N_WCHAR32
	#define WCHAR_TEST_SIZE 5
#else
	#define WCHAR_TEST_SIZE 8
#endif

int main(void)
{
	int fails = 0; // Количество ошибочных тестов
	unsigned char utf16_test[UTF16_TEST_SIZE*2] = {/*0*/0xA9, 0x00, /*1*/0x3C, 0xD8, 0x08, 0xDF, /*2*/0x3D, 0xD8, 0xA2, 0xDC, /*3*/0x39, 0x26, /*4*/0x3D, 0xD8, 0x03, 0xDE};
#ifdef N_WCHAR32
	wchar_t wchar_test[WCHAR_TEST_SIZE] = {0x00A9, 0x1F308, 0x1F4A2, 0x2639, 0x1F603};
#else
	wchar_t wchar_test[WCHAR_TEST_SIZE] = {/*0*/0x00A9, /*1*/0xD83C, 0xDF08, /*2*/0xD83D, 0xDCA2, /*3*/0x2639, /*4*/0xD83D, 0xDE03};
#endif

	unsigned short utf16_test_converted[UTF16_TEST_SIZE];
	wchar_t wchar_test_converted[UTF16_TEST_SIZE];
	
	printf("%s", "1.1 ncfCalculateWideCharToUTF16StringSize calculate correct size for correct widechar string\n");
	if(ncfCalculateWideCharToUTF16StringSize(wchar_test, WCHAR_TEST_SIZE) != UTF16_TEST_SIZE) {
		fails++;
		printf("%s", "\tfailed\n");
	} else
		printf("%s", "\tok\n");


	printf("%s", "1.2 ncfCalculateUTF16ToWideCharStringSize calculate correct size for correct utf16 string\n");
	if(ncfCalculateUTF16ToWideCharStringSize((unsigned short *)utf16_test, UTF16_TEST_SIZE) != WCHAR_TEST_SIZE) {
		fails++;
		printf("%s", "\tfailed\n");
	} else
		printf("%s", "\tok\n");
		
	printf("%s", "1.3 ncfUTF16ToWideChar converts correct uft16 string to widechar string\n");
	if(ncfUTF16ToWideChar(wchar_test_converted, UTF16_TEST_SIZE, (unsigned short *)utf16_test, UTF16_TEST_SIZE) != NCF_SUCCESS) {
		fails++;
		printf("%s", "\tfailed\n");
	} else {
		if(memcmp(wchar_test_converted, wchar_test, WCHAR_TEST_SIZE*sizeof(wchar_t)) != 0) {
			fails++;
			printf("%s", "\tfailed\n");
		} else {
			size_t i;
			
			for(i = WCHAR_TEST_SIZE; i < UTF16_TEST_SIZE; i++) {
				if(wchar_test_converted[i] != 0)
					break;
			}
			
			if(i == UTF16_TEST_SIZE)
				printf("%s", "\tok\n");
			else {
				fails++;
				printf("%s", "\tfailed\n");
			}
		}
	}
	
	printf("%s", "1.4 ncfWideCharToUTF16 converts correct widechar string to utf16 string\n");
	if(ncfWideCharToUTF16(utf16_test_converted, UTF16_TEST_SIZE, wchar_test, WCHAR_TEST_SIZE) != NCF_SUCCESS) {
		fails++;
		printf("%s", "\tfailed\n");
	} else {
		if(memcmp(utf16_test_converted, (unsigned short *)utf16_test, UTF16_TEST_SIZE*sizeof(unsigned short)) != 0) {
			fails++;
			printf("%s", "\tfailed\n");
		} else {
			printf("%s", "\tok\n");
		}
	}
	
	// ТУДУ: добавить тесты для ошибочных преобразований, или когда out_size < inp_size (> для ncfWideCharToUTF16)
	
	printf("Total %d failed tests\n", fails);
	
	return fails;
}
