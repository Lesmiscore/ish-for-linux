/*
    disp#    Ver. 0.02

    display intermediate ish file

    Copyright (C) 1990  by  M. Ishizuka
    All rights reserved

    920701    on UNIX by taka@exsys.prm.prug.or.jp
    941220    bug FIX. by taka@exsys.prm.prug.or.jp

*/

#include    <stdio.h>
#include    <sys/types.h>
#include    <sys/stat.h>

#define	ID    "ID block for multi volume ish file"

#ifdef NOBCOPY
#include <memory.h>
#define    bcmp    memcmp
#endif

typedef    struct {
	unsigned char    fsize[4];
	unsigned char    line[2];
	unsigned char    block[2];
	unsigned char    byte;
	unsigned char    ibyte;
	unsigned char    fname[11];
	unsigned char    janle;
	unsigned char    os;
	unsigned char    exttype;
	unsigned char    tstamp;
	unsigned char    time[4];
	unsigned char    dtime[3];
	unsigned char    fcrc16[2];
	unsigned char    fcrc32[4];
	unsigned char    bcnt[4];
	unsigned char    reserved[54];
} HEADER_COPY;

struct IDBLOCK {
	unsigned char    id_block_size[2];
	unsigned char    id_block_version_number[2];
	unsigned char    id_block_id_offset[2];
	unsigned char    id_block_id_size[2];
	unsigned char    header_copy_offset[2];
	unsigned char    header_copy_size[2];
	unsigned char    sequence_number_table_offset[2];
	unsigned char    sequence_number_table_size[2];
	unsigned char    reserved[16];
	unsigned char    id_block_id[96];
	HEADER_COPY      header_copy;
	unsigned char    sequence_number_table[32];
	unsigned char    id_block_offset[4];
} idblock;

void disp_fname() {
    int    i;

    for (i=0; i<8; i++)
	if (idblock.header_copy.fname[i] <= ' ')
	    i = 7;
	else
	    putchar(idblock.header_copy.fname[i]);
    putchar('.');
    for (; i<11; i++)
	if (idblock.header_copy.fname[i] <= ' ')
	    break;
	else
	    putchar(idblock.header_copy.fname[i]);
}

void disp_date_time() {
    int    i;
    int    tim;
    unsigned char    *p;
    p = idblock.header_copy.time;
    printf("  (%02u-%02u-%02u %02u:%02u:%02u <difference of time ",
	(p[3]>>1)+1980,
	((p[3]&0x01)<<3) + (p[2]>>5),
	p[2]&0x1f,
	p[1]>>3,
	((p[1]&0x07)<<3) + (p[0]>>5),
	(p[0]&0x1f)<<1);

    if (idblock.header_copy.tstamp & 2 ) {
	tim = idblock.header_copy.dtime[0] +
	    (idblock.header_copy.dtime[1] << 8) +
	    (idblock.header_copy.dtime[2] << 16);
	if (tim & 0x00800000 )
	    tim |= 0xff000000;
	i = tim % 3600L;
	printf("= %ld:%02d:%02d>)", tim/3600, i/60, i%60);
    }
    else
	printf("is not available>)");
}

int calc_max_division() {
    int    i;
    unsigned char    *p;
    int    v;

    p = idblock.header_copy.fsize;
    i = p[0] + ((long)p[1] << 8) + ((long)p[2] << 16) + ((long)p[3] << 24);
    p = idblock.header_copy.bcnt;
    /* bug fix. CEX941220 */
    v = p[0] + ((long)p[1] << 8) + ((long)p[2] << 16) + ((long)p[3] << 24);
    i = i / v +1;
    return i;
}

void disp_volume(max_division,flag)
int max_division;
int flag;
{
/*	int    i;	*/
	int    j;
	int    flag2=0;

	for (j=0; j < max_division; j++)
	    if (((idblock.sequence_number_table[j / 8] >> (j % 8)) & 1)
	      == flag) {
		if (flag2)
		    printf(", ");
		flag2 = 1;
		printf("%u", j+1);
	    }
}

void disp() {
    int    max_division;

    disp_fname();
    disp_date_time();
    if ((max_division = calc_max_division()) > 250 ) {
	printf("over 250 !!!\n");
	return;
    }
    printf("\n     restored volume(s) => ");
    disp_volume(max_division, 1);
    printf("\n non restored volume(s) => ");
    disp_volume(max_division, 0);
    putchar('\n');
}

long filelength(fd)
int    fd;
{
struct stat sb;
    fstat(fd, &sb);
    return sb.st_size;
}

unsigned char main(argc,argv)
int argc;
char **argv;
{
    int     i;
    long    leng;
    FILE    *stream;
static unsigned char h[] = {    0x00,0x01, 0x00,0x02, 0x20,0x00, 0x60,0x00,
		0x80,0x00, 0x60,0x00, 0xe0,0x00, 0x20,0x00,
		0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
		0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00 };
unsigned char    *p;
long    l;
long    m;

    if (argc < 2) {
	printf("%s  Ver. 0.02\n", argv[0]);
	printf("Copyright (C) 1990  by  M. Ishizuka\n");
	printf("Unix supported by taka@exsys.prm.prug.or.jp\n\n");
	printf("Usage:\n");
	printf(" %s filename(s)\n", argv[0]);
	exit(1);
    }
    for (i=1; i<argc; i++) {
	printf("%s: ", argv[i]);
	if ((stream = fopen(argv[i], "rb")) != NULL) {
	    leng = filelength(fileno(stream));
	    leng -= 0x104L;
	    if (leng > 0) {
		if (fseek(stream, leng, 0))
		    printf("seek error.\n");
		else {

		    if (fread((char *)&idblock, sizeof(idblock), 1, stream) == 1) {
			p = idblock.header_copy.fsize;
			l = p[0] + ((long)p[1] << 8) + ((long)p[2] << 16) + ((long)p[3] << 24);
			p = idblock.id_block_offset;
			m = p[0] + ((long)p[1] << 8) + ((long)p[2] << 16) + ((long)p[3] << 24);
			if (bcmp((char *)idblock.id_block_size, (char *)h, 0x20) == 0 &&
			    m == leng &&
			    strncmp(idblock.id_block_id, ID, 96) == 0 &&
			    l == leng) {
				disp();
			}
			else
			    printf("not an intermediate ish file.\n");
		    }
		    else
			printf("file read error.\n");
		}
	    }
	    else
		printf("not an intermediate ish file.\n");
	    if (fclose(stream))
		printf(" file close error !\n");
	}
	else
	    printf("open error.\n");
    }
}
