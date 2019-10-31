/*
  "ish.c"	    Version 1.11

  ish file converter for Human68K

       (C) Pekin
      Public Domain Software

  1987.11.11 ken Miyazaki(ken)
  1987.11.28 O Imaizumi(Gigo)
  1987.12.05 Bug Fix
  1987.12.06 OS9Ext support.
  1988.01.31 V.1.11
  1988.02.06 for Human68K(Gigo)
  1988.05.10 for UNIX (kondo)
  1990.07.12 non-kana (keizo)

  1993.09.25 -mv & -lf (aka)

  special thanks for M.ishizuka(ish)
*/

#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#ifndef NOINDEX
#include <strings.h>
#endif
#define MAIN
#include "ish.h"

#ifdef __TURBOC__
#define    rindex    strrchr
#endif

extern int errno;

void help(), puttitle();

enum EncDecMode edmode = JIS7;
short int jis = JIS7_L,  esc = OFF, title = 50, mode = RESNEW;
#ifdef __TURBOC__
short int os = MS_DOS;
#else
short int os = UNIX;
#endif
int  path = 0, size;
long lsize;
char *filename = "file.ish";
/* CEX */
int    mline = 0;
int    lfflag = 0;

FILE *opath = stdout;
FILE *ipath;
char name[NAMLEN];

unsigned char buff[LBUFLEN];
unsigned char obuf[LBUFLEN];
unsigned char tatesum[LBUFLEN];
unsigned char yokosum[LBUFLEN];
unsigned char atrb;

static int ishargc=0;
static char *ishargv;

void main(argc, argv)
int argc;
char **argv;
{
    int oflg = 0, n = 0, fc = 0;
    char *p;
    FILE *fopen();
    int ac;
    char **av;

    if (argc < 2)
	help();
    ac = argc -1;
    av = argv;
    av++;
    while(--argc > 0) {
	if (**++argv == '-') {
	    switch(isalpha(argv[0][1])?toupper(argv[0][1]):argv[0][1]) {
	    case 'S' :
		switch(isalpha(argv[0][2])?toupper(argv[0][2]):argv[0][2]) {
		case '\0':
		case '7':
		    jis = JIS7_L;
		    edmode = JIS7;
		    break;
		case '8':
		    jis = JIS8_L;
		    edmode = JIS8;
		    break;
		case 'S':
		    jis = SJIS_L;
		    edmode = SJIS;
		    break;
		case 'N':
		    jis = NJIS_L;
		    edmode = NJIS;
		    break;
		default:
		    help();
		}
		mode = CREISH;
		break;
	    case 'O':
		jis = OJIS;
		printf("\nSorry, -o not supported.\n\n");
		exit(225); /* bad para err */
	    case 'M':
	    /* CEX */
		if (toupper(argv[0][2]) == 'V') {
		    mline = atoi(&argv[0][3]);
		}
		else if(toupper(argv[0][2]) == 'A')
		    os = MAC;
		else
		    os = MS_DOS;
		break;
	    case 'L':
		if (toupper(argv[0][2]) == 'F')
		    lfflag=1;
		break;
	    case 'C':
		os = CP_M;
		break;
	    case '9':
		os = OS_9;
		break;
	    case 'K':
		os = OS_9EXT;
		break;
	    case 'U':
		os = UNIX;
		break;
	    case '?':
		os = OTHER;
		break;
	    case '*':
		os = ALL_OS;
		break;
	    case 'N':
		esc = OFF;
		break;
	    case 'E':
		esc = ON;
		break;
	    case 'T':
		title = atoi(&argv[0][2]);
		break;
	    case 'R':
		mode = RESNEW;
		break;
	    case 'A':
		mode = RESALL;
		break;
	    case 'Q':
		mode = RESASK;
		break;
	    case 'F':
		if (argv[0][2] == '=') {
		    oflg = -1;
		    strcpy(name, &argv[0][3]);
		}
		else
		    oflg = 1;
		break;
	    default:
		help();
	    }
	}
	else {
	/* set first filename */
	    if (ishargc == 0)
		ishargv = argv[0];
	    filename = argv[0];
	    ishargc++;
	}
    }
/*
    if ((p = rindex(filename, '.')) != NULL)
	if (tolower(*++p) == 'i' && tolower(*++p) == 's' 
	&& tolower(*++p) == 'h' && *++p == '\0' && mode == CREISH)
	    mode = RESNEW;
*/
    if (mode == CREISH) {
	puttitle();
	n = 0;
	if (oflg == -1) {
	    if (ishargc) {
		if (oflg != -1)
		    strcat(name, ishargv);
		if ((p = rindex(name, '.')) != 0 
		&& tolower(*++p) == 'i' 
		&& tolower(*++p) == 's'
		&& tolower(*++p) == 'h'
		&& *++p == '\0' ) {
		    if (oflg != -1)
			n++;
	    }
	    else {
		if((p = rindex(name, '.')) != NULL)
		    strcpy(p, ".ish");
		else
		    strcat(name, ".ish");
	    }
	    if ((filename = rindex(name, '/')) == 0)
		filename = name;
	    else
		filename++;
	    if ((opath=fopen(filename, "w")) == NULL)
		exit(errno);
	    fprintf(stderr, "< Create to %s >\n", filename);
	}
    }
    clearerr(opath);
    for (; ac > 0; --ac, ++n) {
	if (av[n][0] == '-')
	    continue;
#ifdef __TURBOC__
	if ((path = open(av[n], O_RDONLY | O_BINARY)) == ERR)
#else
	if ((path = open(av[n], O_RDONLY)) == ERR)
#endif
	if (n)
	    exit(errno);
	else
	    continue;
	if ((filename = rindex(av[n],'/')) == 0)
	    filename = av[n];
	else
	    filename++;
	if(encode(oflg) == ERR)
	    exit(errno);
	close(path);
	fc++;
    }
    if (fc == 0)
	fprintf(stderr, "\nNone of terget file(s)\n\n");
    }
    else {	    /* Restore */
	if (ishargc == 0) {
	    /* from stdin */
	    ipath = stdin;
	    puttitle();
	    fprintf(stderr, "< Restore from stdin >\n");
	    if (decode() == 0)
		fprintf(stderr,"ish: can't find ish header.\n");
	}
	else {
	    /* restore from file */
	    for (n = 0; ac > 0; --ac, ++n) {
		if (av[n][0] == '-')
		    continue;
		filename = av[n];
		if ((ipath = fopen(filename, "r")) == NULL) {
		    strcpy(name, filename);
		    strcat(name, ".ish");
		    if ((ipath = fopen(name, "r")) == NULL) {
			fprintf(stderr, "ish: %s not found.\n", filename);
			exit(errno);
		    }
		}
		if (fc == 0)
		    puttitle();
		fprintf(stderr, "< Restore from %s >\n", filename);
		if (decode() == 0)
		    fprintf(stderr, "ish: can't find ish header.\n");
		fclose(ipath);
		++fc;
	    }
	}
    }
    exit(0);
}

void help()
{
    printf("ish %s\n",VERSION);
    printf("\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n\n",
    "Restore : ish <path> [-{r|a|q}] [-{m|c|9|k|u|?|*}] [-lf]",
    "Store   : ish <path> {-s[8|s|n|7]} [-{m|c|9|k|u|?|*}] [-{n|e}] [-tn] [-mvn]\n",
    "-r : restore new file(s)                -a : restore all file(s)",
    "-q : restore with question",
    "-s : store a JIS7 format ish file       -s8: store a JIS8 format ish file",
    "-ss: store a shift JIS format ish file  -s7: same as -s",
    "-sn: store a shift JIS non-kana format ish file",
    "-m : MS-DOS & Human68K                  -? : other OS(s)",
    "-c : CP/M and/or MSX-DOS                -9 : OS-9",
    "-k : OS-9/68000 and/or OS-9Ext          -u : UNIX",
    "-ma: Macintosh                          -* : all OS(s)",
    "-n : no ESC sequence in TITLE line      -e : ESC sequence in TITLE line",
    "-tn: title in n line(default 50 line)   -f=<path> : Output to <path>",
    "-mvn: multi volume limit n lines        -lf: ignore CR and LF"
    );
    exit(0);
}

void puttitle()
{
    fprintf(stderr, "ish file converter for UNIX %s\n",VERSION);
    fprintf(stderr, "original sources are Human68K Ver1.11\n");
    fprintf(stderr, "original idea By M.ishizuka(for MS-DOS)\n");
    fprintf(stderr, "copyright 1988 - Pekin - (by Ken+Gigo)\n");
    fprintf(stderr, "    non-kana by keizo   July 12, 1990\n");
    fprintf(stderr, "multi-vol restore supported by taka@exsys.prm.prug.or.jp 1992\n");
    fprintf(stderr, "multi-vol store   supported by aka@redcat.niiza.saitama.jp 1993\n\n");
}
