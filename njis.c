/*
  "njis.c"
  "Shift-JIS without kana" encode & decode
  90/07/12 for UNIX (keizo)
  93/09/25 -lf (aka)
*/

#include "ish.h"

/* encode & decode njis */

static unsigned char ent_j7[] = {
 0x21, 0x22, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
 0x2a, 0x2b, 0x2c, 0x2d, 0x2f, 0x30, 0x31, 0x32,
 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,
 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42,
 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52,
 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,
 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x61, 0x62, 0x63,
 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73,
 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b,
 0x7c, 0x7d, 0x7e,

/* ent_sjn */
	0x89, 0x8a, 0x8b, 0x8c, 0x8d,
 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95,
 0x96, 0x97, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e,
 0x9f, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
 0xe7, 0xe8, 0xe9,   0,   0,   0,   0,   0
};



static unsigned char ent_sjn[] = {
 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90,
 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x99,
 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xe0, 0xe1,
 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9
};

extern unsigned char dct_j7[];

extern unsigned char ent_sj[];
extern unsigned char ent_sj2[];
extern unsigned char dct_sj[];
extern unsigned char dct_sj2[];

/* decode_table_non-kana */
static unsigned char dct_sjn[] = {
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0x00, 0x01, 0xff, 0x02, 0x03, 0x04, 0x05,
 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0xff, 0x0c,
 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,
 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24,
 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c,
 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34,
 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c,
 0xff, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43,
 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b,
 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53,
 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0xff,
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61,
 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
 0xff, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
 0x79, 0x7a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static unsigned char *dat;
static int scount;

void enc_njis(argv, len)
unsigned char *argv;
int len;
{
    int i, k, d, du, dl, c;

    argv[len] = '\0';
    dat = argv;
    k = 0;
    obuf[k++] = ent_j7[*dat];
    i = 75;
    scount  = 0;

    while (i >= 0) {
	if (scount==0) dat++;
	d = *dat++ >> scount;
	d |= *dat << (8-scount);
	if (scount > 2) {
	    d |= *++dat << (16-scount);
	}
	scount = (scount-2) & 7;
	/* extract lower 14 bits */
	d &= 0x3fff;
	/* split into 2 chars */
	if (d < 11193) {
	    du = (d / 123) & 0xff;
	    dl = (d % 123) & 0xff;
	    obuf[k++] = ent_j7[du];
	    c = ent_j7[dl];
	    obuf[k++] = c;
	    i -= 2;
	    if (c >= 128) {
		if (scount==0) dat++;
		d = *dat >> scount;
		if (scount>1) {
		    d |= *++dat << (8-scount);
		}
		obuf[k++] = ent_sj2[d & 0x7f];
		scount = (scount-1) & 7;
		i--;
	    }
	}
	else {
	    d -= 11193;
	    du = (d / 182) & 0xff;
	    dl = (d % 182) & 0xff;
	    obuf[k++] = ent_sjn[du];
	    obuf[k++] = ent_sj2[dl];
	    i -= 2;
	}
    }

    if (i<0) {
	if (scount==0) dat++;
	d = *dat >> scount;
	if (i == -2)
	    obuf[k++] = ent_j7[d];
	else {
	    d |= *++dat << (8-scount);
	    if (scount > 3) {
		d |= *++dat << (16-scount);
	    }
	    d &= 0x1fff;
	    du = d / 91;
	    dl = d % 91;
	    obuf[k++] = ent_j7[du];
	    obuf[k++] = ent_j7[dl];
	}
    }
    obuf[79] = 0;
}

/* decode from njis */


ins14(d)
unsigned d;
{
    d &= 0x3fff;
    if (scount) *dat |= (d << (8-scount)) & 0xff;
    *++dat = d >> scount;
    if (scount < 6) {
	*++dat = d >> (8+scount);
    }
    scount = (scount + 2) & 7;
}

dec_njis(argv, len)
unsigned char *argv;
int len;
{
    int c1,m;
    unsigned d;

    if(len > 79)
	len=79;

    argv[len] = '\0';
    dat = obuf;

    *dat = dct_j7[*argv++];
    scount = 0;
    m = 75;

    while ( m >= 0) {
	c1 = dct_sjn[*argv++];
	if (c1==0xff)
	    return 0;
	m--;
	if (c1 < 91) {
	    d = c1 * 123;
	    c1 = dct_sjn[*argv++];
	    m--;
	    d += c1;
	    ins14(d);
	    if (c1 >= 91) {
		d = dct_sj2[*argv++];
		m--;
		if (scount)
		    *dat |= (d << (8-scount)) & 0xff;
		if (scount < 7) {
		    *++dat = d >> scount;
		}
		scount = (scount + 1) & 7;
	    }
	}
	else {
	    c1 -= 91;
	    d = c1 * 182 + 11193;
	    c1 = dct_sj2[*argv++];
	    m--;
	    d += c1;
	    ins14(d);
	}
    }
    if (m < 0) {
	c1 = dct_j7[*argv++];
	if (m==-2) {
	    d = c1 & 0x3f;
	    if (scount) *dat |= (d << (8-scount)) & 0xff;
	    dat++;
	    if (scount < 6)
		*dat = d >> scount;
	}
	else if (m == -1) {
	    d = c1 * 91;
	    d = (d + dct_j7[*argv]) & 0x1fff;
	    ins14(d);
	}
    }
    obuf[NJIS_L] = 0;    /* for debug */
    return dat-obuf;
}
