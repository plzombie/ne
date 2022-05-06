#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h> 
#include <io.h>
#include <math.h>

#include "../../nyan_container/nyan_container.h"

#define RAWTONEK0_READERVERSION 0
#define RAWTONEK0_WRITERVERSION 0

#define NGL_COLORFORMAT_R8G8B8		0x0001
#define NGL_COLORFORMAT_R8G8B8A8	0x0002
#define NGL_COLORFORMAT_B8G8R8		0x0003
#define NGL_COLORFORMAT_B8G8R8A8	0x0004
#define NGL_COLORFORMAT_A8B8G8R8	0x0005
#define NGL_COLORFORMAT_L8			0x0006
#define NGL_COLORFORMAT_L8A8		0x0007
#define NGL_COLORFORMAT_X1R5G5B5	0x0008
#define NGL_COLORFORMAT_R5G6B5		0x0009

typedef struct {
	unsigned int sizex;
	unsigned int sizey;
	unsigned int datatype;
} nek0head_type;

typedef struct {
	nek0head_type base;
	int nglrowalignment; // Выравнивание строки
} nek0headext1_type;

int main(int argc,char *argv[])
{
	int f, f_out;
	int64_t f_len;
	unsigned char *buf;
	unsigned int bpp;
	uint64_t texsize, offset, bytes_left_to_copy;
	size_t bufsize;
	nek0headext1_type head;
	
	nyan_filetypechunk_type nek0; // Чанк, содержащий тип файла
	nyan_chunkhead_type chunkhead; // Заголовок чанка
	
	if(argc < 7) {
		printf("%s", "  RAW to NEK0 converter\n");
		printf("%s", "    rawtonek0.exe input.raw output.nek0 sizex sizey colorformat rowalignment [offset=0]\n\n");
		printf("%s", "      colorformat == 1 - R8G8B8\n");
		printf("%s", "      colorformat == 2 - R8G8B8A8\n");
		printf("%s", "      colorformat == 3 - B8G8R8\n");
		printf("%s", "      colorformat == 4 - B8G8R8A8\n");
		printf("%s", "      colorformat == 5 - A8B8G8R8\n");
		printf("%s", "      colorformat == 6 - L8\n");
		printf("%s", "      colorformat == 7 - L8A8\n");
		printf("%s", "      colorformat == 8 - X1R5G5B5\n");
		printf("%s", "      colorformat == 9 - R5G6B5\n\n");
		printf("%s", "      rowalignment - alignment for bytes in row of pixels data. Usually 1 or 4.\n\n");
		printf("%s", "      offset - offset from the beginning of input.raw, usually 0.\n\n");
		
		if(argc != 1)
			printf("%s","error: need 6 args\n");
		
		return 0;
	}
		
	f = _open(argv[1], O_RDONLY|O_BINARY);
	
	if(!f)
		{ printf("%s", "error: can't open input file\n"); return 0; }
	
	f_len = _filelengthi64(f);
	
	if(f_len == -1)
		{ printf("%s", "error: can't get file length\n"); return 0; }
	
	head.base.sizex = atoi(argv[3]);
	head.base.sizey = atoi(argv[4]);
	head.base.datatype = atoi(argv[5]);

	if(atoi(argv[6]) < 1)
		{ printf("%s", "error: rowalignment must be greater than 0\n"); return 0; }
	head.nglrowalignment = atoi(argv[6]);
	
	if(argc > 7) {
		int ioffset;
		
		ioffset = atoi(argv[7]);
		if(ioffset < 0) {
			printf("%s", "error: offset must be positive value\n");
			return 0;
		} else
			offset = (uint64_t)offset;
	} else
		offset = 0;
	
	switch(head.base.datatype) {
		case NGL_COLORFORMAT_R8G8B8:
		case NGL_COLORFORMAT_B8G8R8:
			bpp = 3;
			break;
		case NGL_COLORFORMAT_R8G8B8A8:
		case NGL_COLORFORMAT_B8G8R8A8:
		case NGL_COLORFORMAT_A8B8G8R8:
			bpp = 4;
			break;
		case NGL_COLORFORMAT_L8A8:
		case NGL_COLORFORMAT_X1R5G5B5:
		case NGL_COLORFORMAT_R5G6B5:
			bpp = 2;
			break;
		case NGL_COLORFORMAT_L8:
			bpp = 1;
			break;
		default:
			bpp = 0;
	}
	
	if(bpp) {
		uint64_t rowsize;
		
		rowsize = bpp*(uint64_t)head.base.sizex;
		if(rowsize%head.nglrowalignment > 0)
			rowsize += head.nglrowalignment-(rowsize%head.nglrowalignment);
		texsize = head.base.sizey*rowsize;
		if(texsize > f_len)
			printf("%s", "warning: input file is probably damaged");
	} else
		texsize = (uint64_t)f_len;
	
	if(texsize > 0x1000000)
		bufsize = 0x1000000;
	else
		bufsize = (size_t)texsize;
	
	buf = malloc(bufsize);
	if(!buf)
		{ printf("%s", "error: can't allocate memory\n"); _close(f); return 0; } 

	// Открытие выходного файла
	f_out = _open(argv[2], _O_CREAT | _O_TRUNC | _O_WRONLY | _O_BINARY, _S_IREAD  | _S_IWRITE);
	if(!f_out)
		{ printf("%s", "error: can't open output file\n"); _close(f); free(buf); return 0; }

	// Запись чанка, содержащего тип файла
	ncfSetFileType(&nek0, "NEK0", RAWTONEK0_READERVERSION, RAWTONEK0_WRITERVERSION);
	_write(f_out, &nek0, sizeof(nek0));
	
	// Запись чанка, содержащего заголовок файла
	memcpy(chunkhead.cname, "FILEHEAD", 8);
	chunkhead.csize = sizeof(head);
	_write(f_out, &chunkhead, sizeof(chunkhead));
	_write(f_out, &head, sizeof(head));
	
	// Запись чанка, содержащего данные файла
	memcpy(chunkhead.cname, "FILEDATA", 8);
	chunkhead.csize = texsize;
	_write(f_out, &chunkhead, sizeof(chunkhead));
	bytes_left_to_copy = texsize;
	while(bytes_left_to_copy) {
		int bytes_readed;
		unsigned bytes_to_read;
		
		if(bytes_left_to_copy > 0x1000000)
			bytes_to_read = 0x1000000;
		else
			bytes_to_read = bytes_left_to_copy;
		
		bytes_readed = _read(f, buf, bytes_to_read);
		if(bytes_readed != bytes_to_read) {
			printf("warning: can't read full input file, bytes %d/%u\n", bytes_readed, bytes_to_read);
			break;
		}
		
		if(_write(f_out, buf, bytes_to_read) != bytes_to_read) {
			printf("%s", "warning: can't write full output file\n");
			break;
		}
		
		bytes_left_to_copy -= bytes_to_read;
	}
	
	_close(f_out);
	_close(f);
	free(buf);
	
	return 0;
}
