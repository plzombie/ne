
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <io.h>

int main(int argc, char **argv)
{
	unsigned short *inpbuf, *sp; unsigned int *outbuf, *ip;
	unsigned int inpsize, outsize = 0, i;
	int f;
	
	if(argc < 3) {
		printf("%s", "  UTF16 to UTF32 converter\n");
		printf("%s", "    utf16toutf32.exe input.txt output.txt\n\n");
		
		if(argc != 1)
			printf("%s","error: need 2 args\n");
		
		return 0;
	}
	
	f = _open(argv[1], O_RDONLY|O_BINARY);
	
	if(!f)
		{ printf("%s", "error: can't open input file\n"); return 0; }
		
	inpsize = _filelength(f)/2;
	
	inpbuf = malloc(inpsize*sizeof(short));
	if(!inpbuf)
		{ printf("%s", "error: can't allocate memory\n"); _close(f); return 0; }
	_read(f, inpbuf, inpsize*sizeof(short)); 
	_close(f);
	
	outbuf = malloc(inpsize*sizeof(int));
	
	if(!outbuf)
		{ printf("%s", "error: can't allocate memory\n"); free(inpbuf); return 0; }

	sp = inpbuf;
	ip = outbuf;
	for(i = 0; i < inpsize; i++) {
		if((*sp) < 0xD800 || (*sp) > 0xDFFF)
			*ip = *sp;
		else if( ((*sp) >= 0xDC00) || (i+2 > inpsize)) {
			sp++;
			continue;
		} else {
			*ip = ((int)(*sp) & 0x3FF) << 10;
			i++; sp++;
			if( ((*sp) < 0xDC00) || ((*sp) > 0xDFFF) ) {
				if((*sp) < 0xD800 || (*sp) > 0xDFFF)
					*ip = *sp;
				else {
					sp++;
					continue;
				}
			} else
				*ip = ((*ip) | ((*sp) & 0x3FF)) + 0x10000;
		}
		
		ip++;
		sp++;
		outsize++;
	}
	
	// Открытие выходного файла
	f = _open(argv[2], _O_CREAT | _O_TRUNC | _O_WRONLY | _O_BINARY, _S_IREAD  | _S_IWRITE);
	if(!f)
		{ printf("%s", "error: can't open output file\n"); free(inpbuf); return 0; }
	
	_write(f, outbuf, outsize*sizeof(int)); 
	
	_close(f);
		
	free(inpbuf);
	free(outbuf);
	
	return 0;
}
