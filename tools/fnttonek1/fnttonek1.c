#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <string.h> 
#include <io.h>
#include <math.h>

#include "../commonsrc/nek1_save.h"

int main(int argc,char *argv[])
{
	FILE *F;
	int i, newnob;
	nek1_font_type font;
	
	char tempstr[256];
	int twid, thei; // Размер текстуры
	unsigned int Char, Xpos, Ypos, Width, Height, OrigW, OrigH; int Xoffset, Yoffset;
	unsigned int KChar1, KChar2; float Kerning;
	
	if(argc < 5) {
		printf("  UBFG FNT to NEK1 converter\n");
		printf("    fnttonek1.exe input.fnt output.nek1 wid hei\n\n");
		printf("      input.fnt must be created with UNICODE encoding\n\n");
		
		if(argc != 1)
			printf("%s","error: need 4 args\n");
		
		return 0;
	}
	
	twid = atoi(argv[3]);
	thei = atoi(argv[4]);
	//printf("twid %d thei %d\n",twid,thei);
	
	font.noofblocks = 0;
	font.blockalloc = 0;
	font.glyphs = 0;
	font.allockpairs = 0;
	font.kpairshead.noofkpairs = 0;
	font.kpairs = 0;
	
	F = fopen(argv[1],"r");
	if(!F)
		{ printf("%s", "error: can't open input file\n"); return 0; }

	fgets(tempstr, 256, F);
	printf("%s", tempstr);
	fgets(tempstr, 256, F);
	printf("Font name: %s", tempstr);
	while(fgets(tempstr, 256, F)) {
		if(sscanf(tempstr, "%6u %6u %6u %6u %6u %6d %6d %6u %6u", &Char, &Xpos, &Ypos, &Width, &Height, &Xoffset, &Yoffset, &OrigW, &OrigH) != 9) break;
		//printf("char %d %d %d\n",Char,Char/1024,Char%1024);
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
		font.glyphs[Char/1024][Char%1024].txstart = (float)(Xpos)/twid;
		font.glyphs[Char/1024][Char%1024].txend = (float)(Xpos+Width)/twid;
		font.glyphs[Char/1024][Char%1024].tystart = (float)(thei-Ypos-Height)/thei;
		font.glyphs[Char/1024][Char%1024].tyend = (float)(thei-Ypos)/thei;
	}
	
	if(!strncmp(tempstr, "kerning pairs:", 14)) {
		while(fgets(tempstr, 256, F)) {
			if(sscanf(tempstr, "%6u %6u %24f", &KChar1, &KChar2, &Kerning) != 3) break;
			
			if(Kerning == 0) continue;
			
			if(font.kpairshead.noofkpairs == font.allockpairs) {
				nek1_kpair_type *_kpairs;
				_kpairs = realloc(font.kpairs, (font.allockpairs+1024)*sizeof(nek1_kpair_type));
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
		
		printf("Founded %u kerning pairs\n", font.kpairshead.noofkpairs);
	}
	
	fclose(F);
	
	if(!nek1Save(argv[2], &font)) {
		printf("%s", "error: can't save output file\n");
		goto EXIT;
	}
	
EXIT:
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
