#include <stdio.h>
/*
     "ish.h" ish for UNIX
*/
#define	VERSION	"Ver2.01a5"

#define JIS7_L	63
#define JIS8_L	69
#define SJIS_L	73
#define OJIS_L	69
#define	NJIS_L	69

enum EncDecMode { JIS7, JIS8, SJIS, OJIS, NJIS } ;

#define LBUFLEN	 81
#define NAMLEN	 40

#define MS_DOS	0x00
#define CP_M	0x10
#define OS_9	0x20
#define OS_9EXT 0x21	/* Extended OS9 format */
#define UNIX	0x30	/* UNIX */
#define MAC_OLD	0x40
#define	MAC		0x41
#define OTHER	0xFE
#define ALL_OS	0xFF

#define CREISH	0	/* Create ish file */
#define RESNEW	1
#define RESALL	2
#define RESASK	3

#define ON	0
#define OFF	(-1)

#define ERR	(-1)

#define chkcrc(argv,len)	(0x1d0f==calcrc(argv,len,0xffff))
#define setcrc(argv,len)	(setwrd(argv+len,~calcrc(argv,len,0xffff)))

#define unschr(chr)	(chr)

#ifndef MAIN

/* extern valiable */

extern FILE *opath,*ipath;
extern char name[NAMLEN];

extern short int jis,os,esc,title,mode;
extern int path,size;
extern long lsize;
extern char *filename;

extern unsigned char buff[LBUFLEN];
extern unsigned char obuf[LBUFLEN];
extern unsigned char tatesum[LBUFLEN];
extern unsigned char yokosum[LBUFLEN];
extern unsigned char atrb;

extern enum EncDecMode edmode;
#endif

#ifdef	NOINDEX
#define	index	strchr
#define	rindex	strrchr
#endif

#ifdef NOBCOPY
#define	bcopy(a,b,c)	memcpy(b,a,c)
#define	bcmp(a,b,c)	memcmp((a),(b),(c))
#endif

