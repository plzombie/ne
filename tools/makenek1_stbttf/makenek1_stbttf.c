
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <string.h> 
#include <io.h>
#include <math.h>

#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

#include "../../forks/stb/stb_rect_pack.h"
#include "../../forks/stb/stb_truetype.h"
#include "../../nyan_container/nyan_container_strings.h"

#include "../commonsrc/nek1_save.h"

int BakeFontBitmapAdv(unsigned char *data, int offset,
	float pixel_height, unsigned char *pixels, int pw, int ph, unsigned int h_oversample, unsigned int v_oversample,
	unsigned int *char_list, int num_chars, stbtt_packedchar *chardata);

int main(int argc,char *argv[])
{
	FILE *F;
	int f, i, j, newnob;
	nek1_font_type font;
	int sizex, sizey; // Размер текстуры
	unsigned int Char, Width, Height, OrigW, OrigH; int Xoffset, Yoffset;
	unsigned int oversample_w, oversample_h;
	unsigned int KChar1, KChar2; float Kerning;
	unsigned char *ttf_buffer = 0;
	size_t ttf_size;
	unsigned char *tga_buffer = 0;
	stbtt_packedchar *cdata = 0;
	unsigned int *char_list = 0;
	int num_chars;
	float pixelheight; // Высота шрифта в пикселях
#if 0
	int fontbb_x0, fontbb_y0, fontbb_x1, fontbb_y1;
#else
	int font_ascent, font_descent, font_linegap;
#endif
	float font_baseline;
	stbtt_fontinfo stbttfont;
	float scale;

	if(argc < 4) {
		printf("%s", "  Truetype font maker (via stb_truetype)\n");
		printf("%s", "    makenek1_stbttf.exe fontname.ttf fontname.tga fontname.nek1 [glyphheight=32 [sizex=512 [sizey=512 [char_list.txt [ovw=1 ovh=1]]]]]\n\n");
		printf("%s", "    char_list.txt is a list of symbols in utf16. Default is symbols from 32 to 127\n");
		printf("%s", "    ovw and ovh is oversampling values for width and height\n\n");
		if(argc != 1)
			printf("%s", "error: need 3 args\n");
		
		return 0;
	}

	if(argc > 4)
		pixelheight = (float)atoi(argv[4]);
	else
		pixelheight = 32.f;

	printf("Glyph height = %f\n", pixelheight);

	if(argc > 5)
		sizex = atoi(argv[5]);
	else
		sizex = 512;

	if(argc > 6)
		sizey = atoi(argv[6]);
	else
		sizey = 512;

	printf("Texture size is %dx%d\n", sizex, sizey);

	if(argc > 7) {
		unsigned short *char_list_utf16;
		size_t num_chars_utf16;
		int i;
		
		F = fopen(argv[7], "rb");
		if(!F) { printf("%s", "error: can't open characher list file\n"); return 0; }

		if(fseek(F, 0, SEEK_END)) {
			fclose(F);
			printf("%s", "error: can't seek characher list file\n");
			return 0;
		}

		num_chars_utf16 = ftell(F)/sizeof(unsigned short);
		if(!num_chars_utf16) {
			fclose(F);
			printf("%s", "error: characher list is empty\n");
			goto EXIT;
		}
		char_list_utf16 = malloc(num_chars_utf16*sizeof(unsigned short));
		if(!char_list_utf16) {
			fclose(F);
			printf("%s", "error: can't allocate memory for characher list\n");
			return 0;
		}
		
		num_chars = ncfCalculateUTF16ToUTF32StringSize(char_list_utf16, num_chars_utf16);
		if(!num_chars) {
			fclose(F);
			printf("%s", "error: characher list is empty\n");
			goto EXIT;
		}
		char_list = malloc(num_chars*sizeof(unsigned int));
		if(!char_list) {
			fclose(F);
			printf("%s", "error: can't allocate memory for characher list\n");
			return 0;
		}

		fseek(F, 0, SEEK_SET);
		fread(char_list_utf16, 1, num_chars_utf16*sizeof(unsigned short), F);
		fclose(F);
		
		if(ncfUTF16ToUTF32(char_list, num_chars, char_list_utf16, num_chars_utf16) != NCF_SUCCESS) {
			free(char_list_utf16);
			fclose(F);
			printf("%s", "error: can't convert characters list to utf32\n");
			return 0;
		}
		
		printf("Chars list contains %d charachers (including repetitions)\n", num_chars);
		
		for(i = 1; i < num_chars; ) {
			int j;
			
			for(j = 0; j < i; j++) {
				if(char_list[j] == char_list[i]) {
					num_chars--;
					char_list[i] = char_list[num_chars];
					break;
				}
			}
			
			if(j == i)
				i++;
		}
		
		free(char_list_utf16);
	} else {
		num_chars = 96;

		char_list = malloc(num_chars*sizeof(unsigned int));
		if(!char_list) 
			{ printf("%s", "error: can't allocate memory for characher list\n"); return 0; }

		for(i = 0; i < 96; i++)
			char_list[i] = i + 32;
	}

	printf("Total chars: %d\n", num_chars);
	/*printf("%s", "Chars list: ");
	for(i = 0; i < num_chars; i++)
		fputc(char_list[i], stdout);
	printf("\n");*/
	
	if(argc > 9) {
		oversample_w = atoi(argv[8]);
		oversample_h = atoi(argv[9]);
	} else {
		oversample_w = 1;
		oversample_h = 1;
	}
	
	printf("Oversampling: %ux%u\n", oversample_w, oversample_h);

	cdata = malloc(num_chars*sizeof(stbtt_packedchar));
	if(!cdata) {
		printf("%s", "error: can't allocate memory for characher data\n"); goto EXIT;
	}

	font.noofblocks = 0;
	font.blockalloc = 0;
	font.glyphs = 0;
	font.allockpairs = 0;
	font.kpairshead.noofkpairs = 0;
	font.kpairs = 0;
	
	// Читаем файл шрифта
	F = fopen(argv[1], "rb");
	if(!F)
		{ printf("%s", "error: can't open input file\n"); goto EXIT; }

	if(fseek(F, 0, SEEK_END)) {
		fclose(F);
		printf("%s", "error: can't seek input file\n");
		goto EXIT;
	}

	ttf_size = ftell(F);
	if(!ttf_size) {
		fclose(F);
		printf("%s", "error: ttf font file is empty\n");
		goto EXIT;
	}
	ttf_buffer = malloc(ttf_size);
	if(!ttf_buffer) {
		fclose(F);
		printf("%s", "error: can't allocate memory for ttf font\n");
		goto EXIT;
	}

	fseek(F, 0, SEEK_SET);

	fread(ttf_buffer, 1, ttf_size, F);
	
	fclose(F);

	if(!stbtt_InitFont(&stbttfont, ttf_buffer, 0)) {
		printf("%s", "error: can't init font structure\n");
		goto EXIT;
	}
	
	scale = stbtt_ScaleForPixelHeight(&stbttfont, pixelheight);
#if 0
	stbtt_GetFontBoundingBox(&stbttfont, &fontbb_x0, &fontbb_y0, &fontbb_x1, &fontbb_y1); // Выдаёт хрень
	font_baseline = -(float)fontbb_y0*scale;
	OrigH = (unsigned int)(pixelheight+0.5);
	//printf("fontbb_x0 %d fontbb_y0 %d fontbb_x1 %d fontbb_y1 %d fontbb_baseline %f scale %f\n", fontbb_x0, fontbb_y0, fontbb_x1, fontbb_y1, font_baseline, scale);
#else
	stbtt_GetFontVMetrics(&stbttfont, &font_ascent, &font_descent, &font_linegap);
	font_baseline = font_ascent*scale;
	OrigH = (unsigned int)((font_ascent-font_descent+font_linegap)*scale+0.5);
	//printf("font_ascent %d font_descent %d, font_linegap %d font_baseline %f y advance %u\n", font_ascent, font_descent, font_linegap, font_baseline, OrigH);
#endif
	
	// Читаем глифы, формируем текстуру
	tga_buffer = malloc(18 + sizex * sizey);
	if(!tga_buffer) {
		printf("%s", "error: can't allocate memory for font texture\n");
		goto EXIT;
	}

	tga_buffer[0] = 0;
	tga_buffer[1] = 0;
	tga_buffer[2] = 3;
	tga_buffer[3] = 0;
	tga_buffer[4] = 0;
	tga_buffer[5] = 0;
	tga_buffer[6] = 0;
	tga_buffer[7] = 0;
	*(short *)(tga_buffer + 8) = 0;
	*(short *)(tga_buffer + 10) = 0;
	*(short *)(tga_buffer + 12) = sizex;
	*(short *)(tga_buffer + 14) = sizey;
	tga_buffer[16] = 8;
	tga_buffer[17] = 0x20; // Отзеркалено по y

	if(BakeFontBitmapAdv(ttf_buffer, 0, pixelheight, tga_buffer+18, sizex, sizey, oversample_w, oversample_h, char_list, num_chars, cdata)) {
		printf("%s", "error: can't bake font\n"); goto EXIT;
	}

	// Запись текстуры
	f = _open(argv[2], _O_CREAT | _O_TRUNC | _O_WRONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
	if(!f)
	{
		printf("%s", "error: can't open output file\n"); goto EXIT;
	}

	_write(f, tga_buffer, 18 + sizex*sizey);

	_close(f);

	free(tga_buffer);
	tga_buffer = 0;

	// Составление глифов шрифта
	for(j = 0; j < num_chars; j++) {
		stbtt_packedchar *curchar;

		Char = char_list[j];

		curchar = cdata+j;

		OrigW = (unsigned int)(curchar->xadvance+0.5);
		Width = (unsigned int)(curchar->xoff2-curchar->xoff+0.5);
		Height = (unsigned int)(curchar->yoff2-curchar->yoff+0.5);
		Xoffset = (int)(curchar->xoff+0.5);
		Yoffset = (int)(font_baseline+curchar->yoff+0.5);
		
		/*printf("char %hu x0 %hu y0 %hu x1 %hu y1 %hu xoff %f yoff %f xadv %f xoff2 %f yoff2 %f\n", char_list[j],
			curchar->x0, curchar->y0, curchar->x1, curchar->y1,
			(double)curchar->xoff, (double)curchar->yoff, (double)curchar->xadvance, (double)curchar->xoff2, (double)curchar->yoff2);*/

		if(Char/1024 >= font.noofblocks) {
			char *_blockalloc;
			nek1_glyph_type **_glyphs;
			
			newnob = Char/1024+1;
			
			_blockalloc = realloc(font.blockalloc, newnob*sizeof(char));
			if(!_blockalloc){
				printf("Can't allocate more memory for font.blockalloc\n");
				break;
			}
			font.blockalloc = _blockalloc;
			
			_glyphs = realloc(font.glyphs, newnob*sizeof(nek1_glyph_type *));
			if(!_glyphs){
				printf("Can't allocate more memory for font.glyphs\n");
				break;
			}
			font.glyphs = _glyphs;
			
			for(i = font.noofblocks;i < newnob;i++)
				font.blockalloc[i] = false;
			font.noofblocks = newnob;
			//printf("new font.noofblocks size %d\n",font.noofblocks);
		}
		if(font.blockalloc[Char/1024] == false) {
			font.glyphs[Char/1024] = malloc(sizeof(nek1_glyph_type)*1024);
			if(!font.glyphs[Char/1024]) { // Проверяем, действительно ли выделена память
				printf("Can't allocate more memory for glyphs\n");
				break;
			}

			memset(font.glyphs[Char/1024], 0, sizeof(nek1_glyph_type)*1024);
			font.blockalloc[Char/1024] = true;
		}
		font.glyphs[Char/1024][Char%1024].wid = OrigW;
		font.glyphs[Char/1024][Char%1024].hei = OrigH;
		font.glyphs[Char/1024][Char%1024].twid = Width;
		font.glyphs[Char/1024][Char%1024].thei = Height;
		font.glyphs[Char/1024][Char%1024].offx = Xoffset;
		font.glyphs[Char/1024][Char%1024].offy = Yoffset;
		font.glyphs[Char/1024][Char%1024].txstart = curchar->x0/(float)sizex;
		font.glyphs[Char/1024][Char%1024].txend = curchar->x1/(float)sizex;
		font.glyphs[Char/1024][Char%1024].tystart = 1.0f-curchar->y1/(float)sizey;
		font.glyphs[Char/1024][Char%1024].tyend = 1.0f-curchar->y0/(float)sizey;
	}

	// Нахождение кернинговых пар
	if(1) {
		// Перебираем все возможные пары символов
		for(i = 0; i < num_chars; i++) {
			KChar1 = char_list[i];
			
			if(KChar1 > 65535) continue;
			
			for(j = 0; j < num_chars; j++) {
				KChar2 = char_list[j];
				
				if(KChar2 > 65535) continue;

				Kerning = scale*stbtt_GetCodepointKernAdvance(&stbttfont, KChar1, KChar2);

				if(Kerning == 0) continue;
				
				//printf("kerning %f\n", Kerning);

				if(font.kpairshead.noofkpairs == font.allockpairs) {
					nek1_kpair_type *_kpairs;
					_kpairs = realloc(font.kpairs, (font.allockpairs + 1024) * sizeof(nek1_kpair_type));
					if(_kpairs) {
						font.kpairs = _kpairs;
						font.allockpairs += 1024;
					} else {
						printf("Can't allocate more memory for kerning pairs\n");
						break;
					}
				}
				font.kpairs[font.kpairshead.noofkpairs].symbol1 = KChar1;
				font.kpairs[font.kpairshead.noofkpairs].symbol2 = KChar2;
				font.kpairs[font.kpairshead.noofkpairs].kerning = Kerning;
				font.kpairs[font.kpairshead.noofkpairs].reserved = 0;
				font.kpairshead.noofkpairs++;
			}
		}

		printf("Founded %u kerning pairs\n", font.kpairshead.noofkpairs);
	}

	if(!nek1Save(argv[3], &font)) {
		printf("%s", "error: can't save output file\n");
		goto EXIT;
	}
	
EXIT:	
	if(char_list)
		free(char_list);
	
	if(cdata)
		free(cdata);
	
	if(tga_buffer)
		free(tga_buffer);
		
	if(ttf_buffer)
		free(ttf_buffer);
	
	if(font.noofblocks) {
		for(i = 0;i <= font.noofblocks;i++)
			if(font.blockalloc[i])
				free(font.glyphs[i]);
				
		free(font.blockalloc);
		free(font.glyphs);
	}
	
	if(font.kpairshead.noofkpairs)
		free(font.kpairs);

	return 0;
}

int BakeFontBitmapAdv(unsigned char *data, int offset,
	float pixel_height, unsigned char *pixels, int pw, int ph, unsigned int h_oversample, unsigned int v_oversample,
	unsigned int *char_list, int num_chars, stbtt_packedchar *chardata)
{
	int i;
	stbtt_pack_context pc;
	stbtt_pack_range range;

	range.first_unicode_codepoint_in_range = 0;
	range.array_of_unicode_codepoints = malloc(num_chars*sizeof(int));
	range.num_chars = num_chars;
	range.chardata_for_range = chardata;
	range.font_size = pixel_height;
	if(!range.array_of_unicode_codepoints)
		return -1;

	for(i = 0; i < num_chars; i++)
		range.array_of_unicode_codepoints[i] = char_list[i];

	stbtt_PackBegin(&pc, pixels, pw, ph, 0, 1, NULL);
	stbtt_PackSetOversampling(&pc, h_oversample, v_oversample);
	stbtt_PackFontRanges(&pc, data, 0, &range, 1);
	stbtt_PackEnd(&pc);

	free(range.array_of_unicode_codepoints);

	return 0;
}
