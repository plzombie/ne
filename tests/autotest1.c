/*
	Тестирование загрузки изображений через встроенные плагины
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <stdbool.h>

#include "../nyan/nyan_publicapi.h"

// Ширина и высота тестового изображения
const unsigned int test_image_width = 123;
const unsigned int test_image_height = 61;

bool TestImageData(nv_texture_type *tex, unsigned char *rawdata, unsigned int bpp)
{
	unsigned int i, w, h;
	unsigned char *p1, *p2;
	unsigned int p1_padding;

	if(!rawdata)
		return false;

	if(tex->nglrowalignment < 1)
		return false;

	w = tex->sizex;
	h = tex->sizey;

	p1 = tex->buffer;
	p2 = rawdata;

	p1_padding = (tex->nglrowalignment - (bpp*w%tex->nglrowalignment)) % tex->nglrowalignment;

	for(i = 0; i < h; i++) {
		if(memcmp(p1, p2, bpp*w)) {
			wprintf(L"\t\tWrong line %u\n", i);
			return false;
		}	

		p1 += bpp*w+p1_padding;
		p2 += bpp*w;
	}

	return true;
}

bool TestImageFile(wchar_t *texfilename, unsigned char *rawdata, unsigned int colorformat, unsigned int bpp)
{
	bool result = true;
	nv_texture_type test_file_data;

	if(nvLoadImageToMemory(texfilename, &test_file_data)) {
		if(test_file_data.sizex != test_image_width) {
			wprintf(L"%ls", L"\tWrong image width\n");
			result = false;
			goto END_OF_TEST_24BIT_RGB_UNCOMPR_BMP;
		}

		if(test_file_data.sizey != test_image_height) {
			wprintf(L"%ls", L"\tWrong image height\n");
			result = false;
			goto END_OF_TEST_24BIT_RGB_UNCOMPR_BMP;
		}

		if(test_file_data.nglcolorformat != colorformat) {
			wprintf(L"%ls", L"\tWrong image color format\n");
			result = false;
			goto END_OF_TEST_24BIT_RGB_UNCOMPR_BMP;
		}

		if(!TestImageData(&test_file_data, rawdata, bpp)) {
			wprintf(L"%ls", L"\tWrong image data\n");
			result = false;
			goto END_OF_TEST_24BIT_RGB_UNCOMPR_BMP;
		}

	END_OF_TEST_24BIT_RGB_UNCOMPR_BMP:
		nFreeMemory(test_file_data.buffer);
	} else {
		wprintf(L"\tCan\'t load \"%ls\"\n", texfilename);
		result = false;
	}

	return result;
}

unsigned char *LoadTestData(wchar_t *test_data_file, unsigned char bpp)
{
	unsigned char *result = 0;
	unsigned int f;

	wprintf(L"\tLoading \"%ls\"\n", test_data_file);
	f = nFileOpen(test_data_file);
	if(f) {
		result = nAllocMemory(test_image_width*test_image_height*bpp);
		if(result) {
			if(nFileRead(f, result, test_image_width*test_image_height*bpp) != (test_image_width*test_image_height*bpp)) {
				nFreeMemory(result);
				result = 0;
			}
		}
		nFileClose(f);
	}
	if(!result)
		wprintf(L"\t\tCan\'t open \"%ls\"\n", test_data_file);

	return result;
}

int main(void)
{
	int fails = 0; // Количество ошибочных тестов
	unsigned char *test_1bit_to_24bit_bgr_bl_raw_data = 0;
	unsigned char *test_4bit_to_24bit_bgr_bl_raw_data = 0;
	unsigned char *test_8bit_to_24bit_bgr_bl_raw_data = 0, *test_8bit_to_24bit_rgb_bl_raw_data = 0;
	unsigned char *test_grayscale_bl_raw_data = 0;
	unsigned char *test_24bit_bgr_bl_raw_data = 0, *test_24bit_rgb_bl_raw_data = 0;
	unsigned char *test_32bit_bgra_bl_raw_data = 0, *test_32bit_rgba_bl_raw_data = 0, *test_32bit_abgr_bl_raw_data = 0;

	NYAN_INIT
	
	nMountDir(L"media/autotests");
	nMountDir(L"../media/autotests");
	nMountDir(L"../../media/autotests");
	nMountDir(L"../../../media/autotests");
	
	if(!nvAttachRender(L"nullgl")) return 6666;
	if(!naAttachLib(L"nullal")) return 7777;

	if(!nInit()) return 8888;

	wprintf(L"%ls", L"Testing image loading\n");

	// Загрузка проверочных данных для тестов
	wprintf(L"%ls", L"Loading test data\n");

	test_1bit_to_24bit_bgr_bl_raw_data = LoadTestData(L"test_1bit_to_24bit_bgr_bl.raw", 3);

	test_4bit_to_24bit_bgr_bl_raw_data = LoadTestData(L"test_4bit_to_24bit_bgr_bl.raw", 3);

	test_8bit_to_24bit_bgr_bl_raw_data = LoadTestData(L"test_8bit_to_24bit_bgr_bl.raw", 3);

	test_8bit_to_24bit_rgb_bl_raw_data = LoadTestData(L"test_8bit_to_24bit_rgb_bl.raw", 3);

	test_grayscale_bl_raw_data = LoadTestData(L"test_grayscale_bl.raw", 1);

	test_24bit_bgr_bl_raw_data = LoadTestData(L"test_24bit_bgr_bl.raw", 3);

	test_24bit_rgb_bl_raw_data = LoadTestData(L"test_24bit_rgb_bl.raw", 3);

	test_32bit_bgra_bl_raw_data = LoadTestData(L"test_32bit_bgra_bl.raw", 4);

	test_32bit_rgba_bl_raw_data = LoadTestData(L"test_32bit_rgba_bl.raw", 4);
	
	if(test_32bit_rgba_bl_raw_data) {
		test_32bit_abgr_bl_raw_data = nAllocMemory(test_image_width*test_image_height*4);
		if(test_32bit_abgr_bl_raw_data) {
			unsigned int i;
			unsigned char *p_in, *p_out, r, g, b, a;
			
			p_in = test_32bit_rgba_bl_raw_data;
			p_out = test_32bit_abgr_bl_raw_data;
			
			for(i = 0; i < test_image_width*test_image_height; i++) {
				r = *p_in; p_in++;
				g = *p_in; p_in++;
				b = *p_in; p_in++;
				a = *p_in; p_in++;
				*p_out = a; p_out++;
				*p_out = b; p_out++;
				*p_out = g; p_out++;
				*p_out = r; p_out++;
			}
		}
	}

	// Проведение тестов

	// Тесты plg_bmp

	// TEST_1BIT_UNCOMPR_BMP
	wprintf(L"%ls", L"TEST_1BIT_UNCOMPR_BMP:\n");
	if(TestImageFile(L"test_1bit_uncompr.bmp", test_1bit_to_24bit_bgr_bl_raw_data, NGL_COLORFORMAT_B8G8R8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_4BIT_UNCOMPR_BMP
	wprintf(L"%ls", L"TEST_4BIT_UNCOMPR_BMP:\n");
	if(TestImageFile(L"test_4bit_uncompr.bmp", test_4bit_to_24bit_bgr_bl_raw_data, NGL_COLORFORMAT_B8G8R8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_8BIT_UNCOMPR_BMP
	wprintf(L"%ls", L"TEST_8BIT_UNCOMPR_BMP:\n");
	if(TestImageFile(L"test_8bit_uncompr.bmp", test_8bit_to_24bit_bgr_bl_raw_data, NGL_COLORFORMAT_B8G8R8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_24BIT_BGR_UNCOMPR_BMP
	wprintf(L"%ls", L"TEST_24BIT_BGR_UNCOMPR_BMP:\n");
	if(TestImageFile(L"test_24bit_bgr_uncompr.bmp", test_24bit_bgr_bl_raw_data, NGL_COLORFORMAT_B8G8R8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_32BIT_ABGR_UNCOMPR_BMP
	wprintf(L"%ls", L"TEST_32BIT_ABGR_UNCOMPR_BMP:\n");
	if(TestImageFile(L"test_32bit_abgr_uncompr.bmp", test_32bit_abgr_bl_raw_data, NGL_COLORFORMAT_A8B8G8R8, 4))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// Тесты plg_tga

	// TEST_8BIT_BL_RLE_TGA
	wprintf(L"%ls", L"TEST_8BIT_BL_RLE_TGA:\n");
	if(TestImageFile(L"test_8bit_bl_rle.tga", test_8bit_to_24bit_bgr_bl_raw_data, NGL_COLORFORMAT_B8G8R8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_8BIT_BL_UNCOMPR_TGA
	wprintf(L"%ls", L"TEST_8BIT_BL_UNCOMPR_TGA:\n");
	if(TestImageFile(L"test_8bit_bl_uncompr.tga", test_8bit_to_24bit_bgr_bl_raw_data, NGL_COLORFORMAT_B8G8R8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_8BIT_TL_RLE_TGA
	wprintf(L"%ls", L"TEST_8BIT_TL_RLE_TGA:\n");
	if(TestImageFile(L"test_8bit_tl_rle.tga", test_8bit_to_24bit_bgr_bl_raw_data, NGL_COLORFORMAT_B8G8R8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_8BIT_TL_UNCOMPR_TGA
	wprintf(L"%ls", L"TEST_8BIT_TL_UNCOMPR_TGA:\n");
	if(TestImageFile(L"test_8bit_tl_uncompr.tga", test_8bit_to_24bit_bgr_bl_raw_data, NGL_COLORFORMAT_B8G8R8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_GRAYSCALE_BL_RLE_TGA
	wprintf(L"%ls", L"TEST_GRAYSCALE_BL_RLE_TGA:\n");
	if(TestImageFile(L"test_grayscale_bl_rle.tga", test_grayscale_bl_raw_data, NGL_COLORFORMAT_L8, 1))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_GRAYSCALE_BL_UNCOMPR_TGA
	wprintf(L"%ls", L"TEST_GRAYSCALE_BL_UNCOMPR_TGA:\n");
	if(TestImageFile(L"test_grayscale_bl_uncompr.tga", test_grayscale_bl_raw_data, NGL_COLORFORMAT_L8, 1))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_GRAYSCALE_TL_RLE_TGA
	wprintf(L"%ls", L"TEST_GRAYSCALE_TL_RLE_TGA:\n");
	if(TestImageFile(L"test_grayscale_tl_rle.tga", test_grayscale_bl_raw_data, NGL_COLORFORMAT_L8, 1))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_GRAYSCALE_TL_UNCOMPR_TGA
	wprintf(L"%ls", L"TEST_GRAYSCALE_TL_UNCOMPR_TGA:\n");
	if(TestImageFile(L"test_grayscale_tl_uncompr.tga", test_grayscale_bl_raw_data, NGL_COLORFORMAT_L8, 1))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_24BIT_BL_RLE_TGA
	wprintf(L"%ls", L"TEST_24BIT_BL_RLE_TGA:\n");
	if(TestImageFile(L"test_24bit_bl_rle.tga", test_24bit_bgr_bl_raw_data, NGL_COLORFORMAT_B8G8R8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_24BIT_BL_UNCOMPR_TGA
	wprintf(L"%ls", L"TEST_24BIT_BL_UNCOMPR_TGA:\n");
	if(TestImageFile(L"test_24bit_bl_uncompr.tga", test_24bit_bgr_bl_raw_data, NGL_COLORFORMAT_B8G8R8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_24BIT_TL_RLE_TGA
	wprintf(L"%ls", L"TEST_24BIT_TL_RLE_TGA:\n");
	if(TestImageFile(L"test_24bit_tl_rle.tga", test_24bit_bgr_bl_raw_data, NGL_COLORFORMAT_B8G8R8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_24BIT_TL_UNCOMPR_TGA
	wprintf(L"%ls", L"TEST_24BIT_TL_UNCOMPR_TGA:\n");
	if(TestImageFile(L"test_24bit_tl_uncompr.tga", test_24bit_bgr_bl_raw_data, NGL_COLORFORMAT_B8G8R8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_32BIT_BL_RLE_TGA
	wprintf(L"%ls", L"TEST_32BIT_BL_RLE_TGA:\n");
	if(TestImageFile(L"test_32bit_bl_rle.tga", test_32bit_bgra_bl_raw_data, NGL_COLORFORMAT_B8G8R8A8, 4))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_32BIT_BL_UNCOMPR_TGA
	wprintf(L"%ls", L"TEST_32BIT_BL_UNCOMPR_TGA:\n");
	if(TestImageFile(L"test_32bit_bl_uncompr.tga", test_32bit_bgra_bl_raw_data, NGL_COLORFORMAT_B8G8R8A8, 4))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_32BIT_TL_RLE_TGA
	wprintf(L"%ls", L"TEST_32BIT_TL_RLE_TGA:\n");
	if(TestImageFile(L"test_32bit_tl_rle.tga", test_32bit_bgra_bl_raw_data, NGL_COLORFORMAT_B8G8R8A8, 4))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;

	// TEST_32BIT_TL_UNCOMPR_TGA
	wprintf(L"%ls", L"TEST_32BIT_TL_UNCOMPR_TGA:\n");
	if(TestImageFile(L"test_32bit_tl_uncompr.tga", test_32bit_bgra_bl_raw_data, NGL_COLORFORMAT_B8G8R8A8, 4))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;
	
	// plg_pcx
	
	// TEST_8BIT_RLE_PCX
	wprintf(L"%ls", L"TEST_8BIT_RLE_PCX:\n");
	if(TestImageFile(L"test_8bit_rle.pcx", test_8bit_to_24bit_rgb_bl_raw_data, NGL_COLORFORMAT_R8G8B8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;
	
	// TEST_24BIT_RGB_RLE_PCX
	wprintf(L"%ls", L"TEST_24BIT_RGB_RLE_PCX:\n");
	if(TestImageFile(L"test_24bit_rgb_rle.pcx", test_24bit_rgb_bl_raw_data, NGL_COLORFORMAT_R8G8B8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;
		
	// plg_nek0
	
	// TEST_GRAYSCALE_NEK0
	wprintf(L"%ls", L"TEST_GRAYSCALE_NEK0:\n");
	if(TestImageFile(L"test_grayscale.nek0", test_grayscale_bl_raw_data, NGL_COLORFORMAT_L8, 1))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;
	
	// TEST_24BIT_RGB_NEK0
	wprintf(L"%ls", L"TEST_24BIT_RGB_NEK0:\n");
	if(TestImageFile(L"test_24bit_rgb.nek0", test_24bit_rgb_bl_raw_data, NGL_COLORFORMAT_R8G8B8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;
	
	// TEST_24BIT_BGR_NEK0
	wprintf(L"%ls", L"TEST_24BIT_BGR_NEK0:\n");
	if(TestImageFile(L"test_24bit_bgr.nek0", test_24bit_bgr_bl_raw_data, NGL_COLORFORMAT_B8G8R8, 3))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;
	
	// TEST_32BIT_RGBA_NEK0
	wprintf(L"%ls", L"TEST_32BIT_RGBA_NEK0:\n");
	if(TestImageFile(L"test_32bit_rgba.nek0", test_32bit_rgba_bl_raw_data, NGL_COLORFORMAT_R8G8B8A8, 4))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;
	
	// TEST_32BIT_BGRA_NEK0
	wprintf(L"%ls", L"TEST_32BIT_BGRA_NEK0:\n");
	if(TestImageFile(L"test_32bit_bgra.nek0", test_32bit_bgra_bl_raw_data, NGL_COLORFORMAT_B8G8R8A8, 4))
		wprintf(L"%ls", L"\tok\n");
	else
		fails++;
	
	wprintf(L"Total %d failed tests\n", fails);

	if(test_1bit_to_24bit_bgr_bl_raw_data)
		nFreeMemory(test_1bit_to_24bit_bgr_bl_raw_data);

	if(test_4bit_to_24bit_bgr_bl_raw_data)
		nFreeMemory(test_4bit_to_24bit_bgr_bl_raw_data);

	if(test_8bit_to_24bit_bgr_bl_raw_data)
		nFreeMemory(test_8bit_to_24bit_bgr_bl_raw_data);

	if(test_8bit_to_24bit_rgb_bl_raw_data)
		nFreeMemory(test_8bit_to_24bit_rgb_bl_raw_data);

	if(test_grayscale_bl_raw_data)
		nFreeMemory(test_grayscale_bl_raw_data);

	if(test_24bit_bgr_bl_raw_data)
		nFreeMemory(test_24bit_bgr_bl_raw_data);

	if(test_24bit_rgb_bl_raw_data)
		nFreeMemory(test_24bit_rgb_bl_raw_data);

	if(test_32bit_bgra_bl_raw_data)
		nFreeMemory(test_32bit_bgra_bl_raw_data);

	if(test_32bit_rgba_bl_raw_data)
		nFreeMemory(test_32bit_rgba_bl_raw_data);
		
	if(test_32bit_abgr_bl_raw_data)
		nFreeMemory(test_32bit_abgr_bl_raw_data);
	
	nClose();
	
	NYAN_CLOSE
	
	return fails;
}
