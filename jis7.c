/*
  "jis7.c" ish for OSK/Human68K(CPU68000)
  JIS 7bit encode & decode
  87/12/16 Ken Miyazaki & Gigo
  88/05/10 for UNIX (kondo)
  93/08/23 -lf (aka)
*/

#include "ish.h"

/* encode & decode jis7 */

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
    0x7c, 0x7d, 0x7e
};

unsigned char dct_j7[] = {
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
    0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0xff
};

static int ej7_m[] = {0, 5, 2, 7, 4, 1, 6, 3};
static int dj7_m[] = {0, 3, 6, 1, 4, 7, 2, 5};

void enc_jis7(argv, len)
unsigned char *argv;
int len;
{
    int k, d, du, dl, i, j;
    int d0, d1, d2;
    register unsigned char *dat;

    argv[len] = '\0';
    dat = argv;
    d0 = unschr(*dat++);
    d1 = unschr(*dat++);
    d2 = unschr(*dat);

    k = 0;

    obuf[k++] = ent_j7[d0 & 0x3f];
    obuf[k++] = ent_j7[d0 >> 6 | d1 << 2 & 0x3f];
    obuf[k++] = ent_j7[d1 >> 4 | d2 << 4 & 0x3f];
    obuf[k++] = ent_j7[d2 >> 2];

    for(i = 0; dat < argv+len; ++i) {
	if((j = ej7_m[i & 8 - 1]) == 0)
	    ++dat;
	d = unschr(*dat) >> j;
	d |= unschr(*++dat) << (8 - j);
	if(j > 3)
	    d |= unschr(*++dat) << (16 - j);
	d &= 0x1fff;
	du = d / 91;
	dl = d - du * 91;
	obuf[k++] = ent_j7[du & 0x7f];
	obuf[k++] = ent_j7[dl & 0x7f];
    }
    obuf[k]='\0';
}

/* decode from jis7 */

dec_jis7(argv, len)
unsigned char *argv;
int len;
{
    int c, c1, c2, c3;
    unsigned i, j, k, d;
    unsigned char *dat;

    if(len > 78)
	len=78;

    argv[len] = '\0';
    dat = obuf;
    if ((c = dct_j7[(int)argv[0]&0xff]) == 0xff)
	return 0;
    if ((c1 = dct_j7[(int)argv[1]&0xff]) == 0xff)
	return 0;
    if ((c2 = dct_j7[(int)argv[2]&0xff]) == 0xff)
	return 0;
    if ((c3 = dct_j7[(int)argv[3]&0xff]) == 0xff)
	return 0;

    *dat++ = c1 << 6 | c;
    *dat++ = c2 << 4 | c1 >> 2;
    *dat = c3 << 2 | c2 >> 4;

    for (i = 0, k = 4; k < len; ++i) {
	if ((c1 = dct_j7[(int)argv[k++]&0xff]) == 0xff)
	    return 0;
	if ((c2 = dct_j7[(int)argv[k++]&0xff]) == 0xff)
	    return 0;

	d = c1 * 91;
	d += c2;
	if (d > 0x3fff)
	    return 0;

	if ((j=dj7_m[i & (8-1)]) != 0)
	    *dat |= d << (8 - j) & 0xff;
	*++dat = d >> j;
	if (j < 5)
	    *++dat = d >> (8 + j);
    }
    return dat-obuf;
}
