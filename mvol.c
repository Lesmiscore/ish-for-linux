/* multi volume sapport
	mvol.c (C) 1992 by taka@exsys.prm.prug.or.jp */
/*    920701    support multi volume file restore (taka) */
/*    930823    modified (aka)		 */

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#ifndef NOINDEX
#include <strings.h>
#endif
#ifdef NDIR
#include <sys/ndir.h>
#define    dirent    direct
#else
#include <dirent.h>
#endif
#include <stdlib.h>
#include "ish.h"

extern unsigned long    fsize;
extern unsigned long    v_offset;
extern unsigned short   max_vol;
extern char   tname[];
extern int    delvol;

#define	ID    "ID block for multi volume ish file"

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
};

struct IDBLOCK    cur_idb;

void
init_mhead(head)
char    *head;
{
char    *p;
int     i;
    /* setup init data */
    cur_idb.id_block_size[0] = 0x00;
    cur_idb.id_block_size[1] = 0x01;
    cur_idb.id_block_version_number[0] = 0x00;
    cur_idb.id_block_version_number[1] = 0x02;
    cur_idb.id_block_id_offset[0] = 0x20;
    cur_idb.id_block_id_offset[1] = 0x00;
    cur_idb.id_block_id_size[0] = 0x60;
    cur_idb.id_block_id_size[1] = 0x00;
    cur_idb.header_copy_offset[0] = 0x80;
    cur_idb.header_copy_offset[1] = 0x00;
    cur_idb.header_copy_size[0] = 0x60;
    cur_idb.header_copy_size[1] = 0x00;
    cur_idb.sequence_number_table_offset[0] = 0xe0;
    cur_idb.sequence_number_table_offset[1] = 0x00;
    cur_idb.sequence_number_table_size[0] = 0x20;
    cur_idb.sequence_number_table_size[1] = 0x00;
    /* clear */
    p = (char *)cur_idb.reserved;
    for (i = 0; i < (16+96+96+32) ; i++)
	*p++ = 0;
    /* set id block id */
    strcpy(cur_idb.id_block_id, ID);
    /* copy header */
    bcopy(head+2, &cur_idb.header_copy, 42);
    /* set file size */
    bcopy(head+2, cur_idb.id_block_offset, 4);
}

int mvol_name(name)
char    *name;
{
char    *p;
char    buf[256];
DIR     *dirp;
struct dirent    *dp;
int     i;
int     n;
int     fd;
    /* set filename "file." */
    if ((p = index(name, '.')) != NULL)
	*p = 0;
    strcat(p, ".#");
    i = 100;
    /* directory serch */
    if ((dirp = opendir(".")) != NULL) {
	while((dp = readdir(dirp)) != NULL) {
	    /* comp file name */
	    if (strncmp(dp->d_name, name, strlen(name)) != 0)
		continue;
	    /* check seq number */
	    p = index(dp->d_name, '.');
	    p += 2;
	    if (!isdigit(*p) || !isdigit(p[1]) || p[2] != 0)
		continue;
	    n = atoi(p);
	    /* compere mvol file */
	    if (mvol_check(name, n)) {
		if (i > n) {
		    i = n;
		}
	    }
	}
	closedir(dirp);
    }
    else {
	fprintf(stderr, "Can't open current directory.\n");
	exit(0);
    }
    if (i != 100) {
	/* make filename */
	sprintf(buf, "%s%02d", name, i);
    }
    else {
	/* serch new file name */
	for (n = 0; n < 100; n++) {
	    sprintf(buf, "%s%02d", name, n);
	    if (access(buf, 0) != 0)
		break;
	}
    }
    strcpy(name, buf);
    umask(n = umask(022));
    if (i != 100) {
	if ((fd = open(name, O_RDWR)) < 0) {
	    return -1;
	}
	if (lseek(fd, -260, 2) == -1) {
	    fprintf(stderr, "Can't seek temp file.(%s)\n", name);
	    exit(0);
	}
	read(fd,&cur_idb, 260);
	delvol=0;
    }
    else {
	if ((fd = open(name, O_RDWR | O_CREAT, 0666 & ~n)) < 0)
	    return -1;
	lseek(fd, fsize, 0);
	write(fd, &cur_idb, 260);
    }
    lseek(fd, v_offset, 0);
    return(fd);
}

int mvol_ok(fp, n)
int    fp;
int    n;
{
int    i,j;
int    m,k;
    /* set done */
    --n;
    i = n >> 3;
    j = n & 0x07;
    cur_idb.sequence_number_table[i] |= (1 << j);
    lseek(fp, -260, 2);
    write(fp, &cur_idb, 260);
    /* check all done */
    m = max_vol -1;
    i = m >> 3;
    for(j = 0; j < i; j++) {
	if (cur_idb.sequence_number_table[j] != 0xff)
	    return(0);
    }
    i = m & 0x07;
    k = 0;
    for (m = 0; m <= i; m++) {
	k |= (1 << m);
    }
    if (cur_idb.sequence_number_table[j] != k)
	return(0);
    /* all dane */
    restore_done(tname, name,fsize);
    return(1);
}

#define    COPY_BUF    1024

restore_done(src, dst,sz)
char    *src;
char    *dst;
long    sz;
{
int     id;
int     od;
char    buf[COPY_BUF];
int     l;
unsigned long    crc32;
unsigned short   crc16;
unsigned long    ncrc32;
unsigned short   ncrc16;
unsigned char    *p;
    if (access(dst,0) == 0) {
	fprintf(stdout, "%s file exist. skip.\n", dst);
	return;
    }
    fprintf(stderr, "%s CRC checking.\n", tname);
    p = cur_idb.header_copy.fcrc32;
    ncrc32 = (long)p[0] + ((long)p[1] << 8) + ((long)p[2] << 16) +
	    ((long)p[3] << 24);
    p = cur_idb.header_copy.fcrc16;
    ncrc16 = (long)p[1] + ((long)p[0] << 8);
    id = open(src, O_RDONLY);
    od = open(dst, O_WRONLY | O_CREAT , 0666);
    crc32 = 0xffffffff;
    crc16 = 0xffff;
    while(sz > 0) {
	if (sz < COPY_BUF)
	    l = read(id, buf, sz);
	else
	    l = read(id, buf, COPY_BUF);
	crc32 = calcrc32(buf, l, crc32);
	crc16 = calcrc(buf, l, crc16);
	write(od, buf, l);
	sz -= l;
    }
    close(id);
    close(od);
    crc32 = ~crc32;
    crc16 = ~crc16;
    if (crc32 == ncrc32 && crc16 == ncrc16) {
	/* checksum OK */
	fprintf(stderr,"Checksum CRC-16 & CRC-32 OK.\n");
	unlink(src);
	fprintf(stderr, "all volume restored %s > %s\n", tname ,name);
    }
    else {
	fprintf(stderr,"Checksum error.\n");
	unlink(dst);
    }
}

int
mvol_check(name, n)
char    *name;
int    n;
{
char    buf[256];
int    fd;
struct IDBLOCK    tmp;
    sprintf(buf, "%s%02d", name, n);
    if ((fd = open(buf, O_RDONLY)) < 0)
	return 0;
    if (lseek(fd, -260, 2) == -1) {
	close(fd);
	return 0;
    }
    read(fd, &tmp, 260);
    close(fd);
    if (strcmp(tmp.id_block_id, ID) != 0)
	return 0;
    if (bcmp(&tmp.header_copy, &cur_idb.header_copy, 42) == 0)
	return 1;
    else
	return 0;
}
