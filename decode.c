/*
  "decode.c" ish for Human68
  ken.Miyazaki & o.imaizumi
  1987/12/05 OS9ext support
  1988/02/01 same as OSK version.
  1988/02/06 for Human68K. (Gigo)
  1988/05/10 for UNIX (kondo)
  1990/07/13 non-kana (keizo)

  1993/08/23 lf (aka)
  1993/11/25 byte count in volume BUG FIX (aka)
*/

#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#ifdef TZSET
#include <time.h>
#else
#include <sys/time.h>
#endif
#ifdef FTIME
#include <sys/timeb.h>
#endif
#ifdef __TURBOC__
#include <time.h>
#include <io.h>
#define index   strchr
#endif
#include <fcntl.h>
#include "ish.h"

#define isleapyear(y)   (((y)%4 == 0 && (y)%100 != 0 || (y)%400 == 0))

void hedprn();

struct tm tm;
time_t timep[2], seconds();

int  type,dline;
unsigned char decbuf[LBUFLEN][LBUFLEN];
char	tname[20];
char	l2buf[LBUFLEN],l3buf[LBUFLEN];
int	decok,delvol;
static int	seq = 0;

extern int errno,lfflag;
extern unsigned char ent_j8[];
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

decode()
{
    int l, skip,fc;
    skip = 0;
    fc = 0;

    l2buf[0]=l3buf[0]=0;
    decok=0xff;
    while((l = readln(ipath, buff, LBUFLEN)) > 0) {
	if (l > 60) {
	    switch(*buff) {
	    case '!':
		l = dec_jis7(buff, l);
		break;
	    case '#':
		l = dec_jis8(buff, l);
		break;
/*
	    case '@':
		l = dec_sjis(buff, l);
		break;
*/
	    default:
		l = 0;
		skip=0;
		break;
	    }

	    if (l > 60) {
		if (chkcrc(obuf, l)) {
		    if (obuf[0] == '\0') {
			if (skip == 0) {
			    strcpy(l3buf,buff);
			    fc++;
			    hedprn(obuf);
			    delvol=1;
			    skip = decish();
			    l3buf[0]=0;
			    decok=0xff;
			}
			continue;
		    }
		    else {
			fprintf(stderr, "Can't Found Header\n");
			skip = 0;
			continue;
		    }
		}
		skip=0;
	    }
	}
	skip = 0;
    } /* EOF */
    return(fc);
}

/* add by CEX */
unsigned short	crc_check;	/* crc-16 */
unsigned long	fsize;		/* total file size */
unsigned short	max_vol;	/* max volume no */
unsigned long	v_offset;	/* volume offset */
unsigned short	this_vol;	/* volume no */
/* end CEX */

typedef struct {
    unsigned char sinc;
    unsigned char itype;
    unsigned char fsize[4];
    unsigned char line[2];
    unsigned short block;
    unsigned char byte;
    unsigned char ibyte;
    unsigned char iname[11];
    unsigned char janle;
    unsigned char os;
    unsigned char exttype;
    unsigned char tstamp;
    unsigned char time[4];
	/* add by CEX */
    unsigned char dtime[3];	/* difference time */
    unsigned char fcrc16[2];	/* file CRC-16 */
    unsigned char fcrc32[4];	/* file CRC-32 */
    unsigned char bcnt[4];	/* byte count */
    unsigned char vcrc16[2];	/* volume CRC-16 */
	/* end CEX */
#if 0
    unsigned char atr;		/* 32:    OS-9Ext only */
    unsigned char ename[29];	/* 33-61: OS-9Ext only */
#endif
} ish_head;

void hedprn(head)
ish_head *head;
{
    int i;
    unsigned char *p, *s;
    /* add by CEX */
    extern void init_mhead();

    this_vol = head->itype;
    /* set crc_cehck */
    if (head->sinc == 0 && head->itype == 0)
	p = head->fcrc16;
    else
	p = head->vcrc16;
    crc_check = (long)p[1] + ((long)p[0] << 8);
    /* end CEX */

    p = head->fsize;
    lsize = (long)p[0]+((long)p[1]<<8)+((long)p[2]<<16)+((long)p[3]<<24);
    /* add by CEX */
    if (head->itype != 0) {
	/* copy file size */
	fsize = lsize;
	/* volume size */
	p = head->bcnt;
    lsize = (long)p[0]+((long)p[1]<<8)+((long)p[2]<<16)+((long)p[3]<<24);
/*	lsize = (long)p[0] + ((long)p[1]<<8);	*/
	max_vol = fsize / lsize + 1;	/* max volume */
	v_offset = lsize * (head->itype-1);	/* volume offset */
	if (head->itype == max_vol) {
	    /* last volume */
	    lsize = fsize - lsize * (max_vol-1);
	}
	init_mhead((char *)head);
    }
    /* end CEX */
    p = head->line;
    jis = (long)p[0]+((long)p[1]<<8);
    type = head->ibyte;
    os = head->os;
    for(p = head->iname, s = (unsigned char *)name, i = 0; i < 8; i++) {
#ifdef	NAMELOWER
	if (isupper(*p))
	    *p = tolower(*p);
#endif
	*s++ = *p++;
	if (*p == ' ')
	    break;
    }
    for(p = head->iname+8, i = 0; i < 3; i++) {
#ifdef	NAMELOWER
	if (isupper(*p))
	    *p = tolower(*p);
#endif
	if (*p == ' ')
	    break;
	if (i == 0)
	    *s++ = '.';
	*s++ = *p++;
    }
    *s = '\0';
    fprintf(stderr, "<<< %s for ",name);
    switch(os) {
    case MS_DOS:
	fprintf(stderr, "MS-DOS");
	break;
    case CP_M:
	fprintf(stderr, "CP/M or MSX-DOS");
	break;
    case OS_9:
	fprintf(stderr, "OS-9");
	break;
    case OS_9EXT:
	fprintf(stderr, "OS-9Ext");
	break;
    case UNIX:
	fprintf(stderr, "UNIX");
	break;
    case OTHER:
	fprintf(stderr, "OTHER");
	break;
    case MAC_OLD:
	fprintf(stderr, "Mac (old)");
	break;
    case MAC:
	fprintf(stderr, "Mac");
	break;
    case ALL_OS :
	fprintf(stderr, "ALL-OS");
	break;
    default:
	fprintf(stderr,"?????");
    }
    fprintf(stderr, " ( use ");
    switch(type) {
    case 13:
	fprintf(stderr, "jis7");
	break;
    case 8:
	fprintf(stderr, "jis8");
	break;
    case 14:
	fprintf(stderr, "shift_jis non-kana");
	break;
    case 15:
	fprintf(stderr, "shift_jis");
	break;
    }
    fprintf(stderr, " ) ");
    if (head->tstamp != 0) {
	p = head->time;
	tm.tm_year = (p[3]>>1)+80;
	tm.tm_mon = (p[3]&0x01)<<3;
	tm.tm_mon += (p[2]>>5) - 1;
	tm.tm_mday = p[2]&0x1f;
	tm.tm_hour = p[1]>>3;
	tm.tm_min = (p[1]&0x07)<<3;
	tm.tm_min += p[0]>>5;
	tm.tm_sec = (p[0]&0x1f)<<1;
	fprintf(stderr,"%02d/%02d/%02d %02d:%02d:%02d ",
	    tm.tm_year,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
#ifdef __TURBOC__
	memcpy( &timep[0], head->time, 4);
	timep[1] = timep[0];
#else
	timep[0] = timep[1] = seconds(&tm);
#endif
    }
    /* add by CEX */
    if (this_vol != 0)
	fprintf(stderr, "%ld/%ld byte >>>\n",lsize , fsize);
    else
    /* end CEX */
	fprintf(stderr, "%ld byte >>>\n", lsize);

    switch(type) {		/* 90-7-13 */
    case 13:
		edmode = JIS7;
		break;
    case 8:
		edmode = JIS8;
		break;
    case 14:
		edmode = NJIS;
		break;
    case 15:
		edmode = SJIS;
		break;
    }
}

decish()
{
    int fp, i, j;
    unsigned char *p;
    unsigned short crc, ncrc;

    if (this_vol != 0) {
	strcpy(tname, name);
	if ((fp = mvol_name(tname)) == -1) {
	    fprintf(stderr, "ish: Can't open:%s\n", tname);
	/*	exit(errno);	*/
	    return errno;
	}
    }
    else {
	if (access(name,0) == 0) {
	    switch(mode) {
	    case RESASK:
		fprintf(stderr, "'%s' already exist. replace ? (y/[n]/q):", name);
		fflush(stderr);
		if (read(0, buff, LBUFLEN) <= 0)
		    buff[0] = 'n';
		buff[0] = isalpha(buff[0]) ? toupper(buff[0]) : buff[0];
		if (buff[0] == 'Q')
		    exit(0);
		if (buff[0] == 'Y') {
		    if(unlink(name) != ERR)
		        break;
		    fprintf(stderr, "'%s' - can't replace.\n", name);
		}
		return -1;
	    case RESNEW:
		fprintf(stderr, "'%s' - file already exist. Skip it.\n", name);
		return -1;
	    case RESALL:
		i = 0;
		fprintf(stderr, "file name change '%s'", name);
		strcpy(buff, name);
		if((p = (unsigned char *)index(name, '.')) != NULL)
		    *p = NULL;
		do {
		    sprintf(buff, "%s.%03d", name, i++);
		} while(access(buff,0) == 0);
		fprintf(stderr, " to '%s'\n", buff);
		strcpy(name, buff);
		break;
	    }
	}
	umask(i = umask(022));
#ifdef __TURBOC__
	if ((fp = _creat(name, 0x20)) == ERR) {
#else
	if ((fp = creat(name, 0666&~i)) == ERR) {
#endif
	    fprintf(stderr, "ish: Can't open:%s\n", name);
/*	exit(errno);	*/
	    return errno;
	}
    }
    crc = 0xffff;
    lsize += 2;
    dline = lsize/(jis-3) + 1;
    fprintf(stderr, "%s : ", name);
    /* add by CEX */
    if (this_vol != 0) {
        /* multi volume */
        fprintf(stderr, "<%d/%d> > %s :", this_vol, max_vol, tname);
    }
    /* end CEX */
    seq = 0;
    while (lsize > 0) {
	if (decblk() != 0)
	{
	    if(delvol == 0)
		fprintf(stderr,"Error!! \007\n");
	    else
	    	fprintf(stderr, " Error!!\007 File delete.\n");
	    fflush(stderr);
	    close(fp);
	    if(delvol != 0)
	    {
		if(this_vol == 0)
		    unlink(name);
		else
		    unlink(tname);
	    }
	    return 0;
	}
	fflush(stderr);
	for (i = 1; i < jis-2; ++i) {
	    if (lsize < (jis-3))
		j = lsize;
	    else
		j = jis-3;
	    lsize -= j;
	    if (lsize >= 2) {
		crc = calcrc(&decbuf[i][1], j, crc);
		write(fp, &decbuf[i][1], j);
	    }
	    else if (lsize == 1) {
		crc = calcrc(&decbuf[i][1], j-1, crc);
		write(fp, &decbuf[i][1], j-1);
		ncrc = decbuf[i][j] << 8;
	    }
	    else {
		if (j >= 2) {
		    if (j > 2) {
			crc = calcrc(&decbuf[i][1], j-2, crc);
			write(fp, &decbuf[i][1], j-2);
		    }
		    ncrc = decbuf[i][j-1] << 8 | unschr(decbuf[i][j]);
		}
		else {
		    ncrc |= unschr(decbuf[i][1]);
		}
		break;
	    }
	}
    }
    if ((crc=~crc) != ncrc)
	fprintf(stderr, " CRC Error : %04X(%04X)\007\n", crc, ncrc);
    else {
	fprintf(stderr, " Finished.\n");
	if (this_vol != 0)
	    if (mvol_ok(fp, this_vol) != 0)
		utime(name, timep);
    }
#ifdef __TURBOC__
	setftime(fp, timep);
	close(fp);
#else
    if (this_vol == 0)
        utime(name, timep);
    close(fp);
#endif
    return 0;
}

decblk()
{
#if 0
    static int seq = 0; /* for out of sequence check */
#endif
    int i, j, k, n, err, bad[3];

    /* reset decode buffer */

    for (i = 0; i <= jis; decbuf[i++][0] = '\0');
    if (seq)    /* put back ln */
	for (i=0; i < jis-2; i++)
	    decbuf[seq][i] = obuf[i];
    seq = 0;
    decok=0;
    while ((i = readln(ipath, buff, LBUFLEN)) > 0) {
	j=-1;
	decok=seq | 0x80;
	if (i < 60)
	    continue;
	if((buff[0] == '!') || (buff[0] == '#'))
	{
	    if(strcmp(l3buf,buff) != 0)
	    {
	    	if(buff[0] == '!')
		    k=dec_jis7(buff,i);
		else
		    k=dec_jis8(buff,i);
		if(k > 60)
		    if(chkcrc(obuf,k) != 0)
		    {
			j=0;
			break;
		    }
	    }
	}		    	
	if(j)
	{	    
	    if ((k = dec_j(buff, i)) < jis)
		continue;
	    if (chkcrc(obuf,k) == 0)
		continue;
	    if ((j = obuf[0]) > jis)
		continue;
	}
	if (j < seq) {
	    seq = j;
	    break;
	}
	else {
	    seq = j;
	    decok=seq;
	    for (n=0; n < jis-2; ++n)
		decbuf[j][n] = obuf[n];
	    if (j == jis){
		seq = 0;
		break;
	    }
	}
    }
    if(j == 0)
	return 1;

    k = (dline < jis-3) ? dline : jis-3;
    dline -= k;
    if (i <= 0 && dline > 2) {
	fprintf(stderr, " Warning - unexpected end of file");
	return -1;
    }
    for (i=1, err=0; i < jis-2; i++) {
	if (decbuf[i][0] == '\0') {
	    decbuf[i][0] = i;
	    for(j = 1; j < jis-2; decbuf[i][j++] = 0);
	    if (i <= k) {
		if (err < 3)
		    bad[err] = i;
		err++;
	    }
	}
    }
    if (err == 0) {
	fprintf(stderr, "o");
	return(0);
    }
    if (err <= 2)
	if (ecc(err, bad[0], bad[1]) == 0){
	    fprintf(stderr, "%d",err);
	    return(0);
	}
	else
	    err = 3;

    fprintf(stderr, "\n%s : ", name);
    if (this_vol != 0) {
        /* multi volume */
        fprintf(stderr, "<%d/%d> > %s :", this_vol, max_vol, tname);
    }
    for(i=0;i<3;i++)
    	fprintf(stderr,"<%d> ",bad[i]);

    fprintf(stderr, "%d lines", err);
    return ERR;
}

dec_j(argv, len)
unsigned char *argv;
int len;
{
    switch(type) {
    case 13:
	return dec_jis7(argv, len);
    case 8 :
	return dec_jis8(argv, len);
    case 14:
	return dec_njis(argv, len);
    case 15:
	return dec_sjis(argv, len);
    default:
	return 0;
    }
}

/*
  "ecc.c" ish for OS9/OSK
  Error collection routine
  87/12/17,88/01/30 By Gigo
  (original idea by M.ishizuka)
*/

ecc(err, e1, e2)
int err, e1, e2;
{
    register unsigned sum;
    register int i, j, k, x;

    if (err == 1) {     /* 1 line error correct (easy!) */
	if (decbuf[jis-2][0] != '\0') {
	    for (j = 1; j < jis-2; j++) {
		for(i=1, sum=0; i < jis-1; sum -= decbuf[i++][j]);
		decbuf[e1][j] = sum;
	    }
	    return 0;
	}
	e2 = jis-2;     /* oh! checksum(V) error increase */
	decbuf[e2][0] = e2;
	for(j = 1; j < jis-2; decbuf[e2][j++] = 0);
    }

    /* correct 2 line error */

    if (decbuf[jis][0] == '\0')      /* no checksum(T) line, give up */
	return 3;

    /* calc hidden byte */

    switch(edmode) {
    case JIS7:
	sum = 0x9d;
	break;
    case JIS8:
    case NJIS:
	sum = 0x1a;
	break;
    case SJIS:
	sum = 0x04;
	break;
    }

    for(j = 1; j < jis-2; j++)
	sum -= decbuf[jis][j];
    decbuf[jis][0] = sum;

    /* clear & set */

    for(j = 0; j < jis; j++)
	decbuf[jis-1][j] = 0;

    x = 0;

    for(k = 0; k < jis-3 ;k++) {     /* search scanning pass */
	if (x <= e1)               /* use '?' oprater, clash !! Oops! */
	    j = jis-1-e1+x;
	else
	    j = x-e1+1;

	/* travase */

	for(i = 1, sum = 0;i <= jis ;j++, i++) {
	    j=j % (jis-2);
	    sum -= decbuf[i][j];
	}
	decbuf[e2][x=((x+e2-e1)%(jis-2))] = sum;

	/* vartical checksum */

	for(i=1, sum=0; i < jis-1; i++)
	    sum -= decbuf[i][x];
	decbuf[e1][x] = sum;
    }
    return 0;
}

int readln(ipath, buff, maxlen)
FILE *ipath;
int maxlen;
char *buff;
{
    int c, i;

    if(lfflag)
    	return readln2(ipath,buff,maxlen);
	
    i = 0;

    while((c = fgetc(ipath)) != EOF && maxlen > i) {
	if((c > 0x1f) && (c != 0x7f)) {
	    buff[i++] = c;
	}
	if ((c == '\n') && i)
	    break;
    }

    buff[i] = NULL;

    return i;
}

int     readln2(ipath,buff,maxlen)	/* readln for -lf option */
FILE    *ipath;
int     maxlen;
char    *buff;
{
    int     i,j;
    char    c;

    if((decok & 0x80) == 0)
        j=78;
    else
        j=1;

    for(i=0;i<LBUFLEN;i++)
    	buff[i]=0;
	
    for(i=j;l2buf[i] != 0;i++)
        if(topchar(l2buf[i]))
            break;
    j=0;
    if(l2buf[i])
    {
	while(l2buf[i])
            buff[j++]=l2buf[i++];
    }
    else
    {
        while((c=fgetc(ipath)) != EOF)
            if(topchar(c))
            {
                buff[j++]=c;
                break;
            }
    }
    
    while(j<79)
    {
        if((c=fgetc(ipath)) == EOF)
            break;
        else if((c > 0x20) || (c < 0))
            buff[j++]=c;
    }

    strcpy(l2buf,buff);

    return j;
}

#ifndef __TURBOC__
int days(t)
struct tm *t;
{
    int y = t->tm_year+1900-1, m = t->tm_mon, d = t->tm_mday-1;
    static unsigned short month[] = {
	0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };

    d += 365*y - 718685;    /* (1970-1)*365 */
    d += y/4 - 492;         /* (1970-1)/4 */
    d -= y/100 - 19;        /* (1970-1)/100 */
    d += (y++)/400 - 4;     /* (1970-1)/400 */
    d += (long)month[m];
    if (isleapyear(y) && m > 1)
	d++;
    return d;
}

time_t seconds(t)
struct tm *t;
{
    long s;
#ifdef TZSET
    extern long timezone;
    tzset();
#else
#ifdef FTIME
    struct timeb buf;
    long timezone;

    ftime(&buf);
    timezone = buf.timezone * 60L;
#else
    struct timeval tv;
    struct timezone tz;
    long timezone;

    gettimeofday(&tv, &tz);
    timezone = tz.tz_minuteswest * 60;
#endif
#endif
    s = (long)days(t)*86400+t->tm_hour*3600+t->tm_min*60+t->tm_sec;
    s += timezone;

#ifdef DST
    t = localtime(&s);
    if (t->t_isdst)
	s -= 3600;
#endif
    return s;
}

#endif

int     topchar(c)
unsigned char c;
{
    char    c2;
    int	i;
    
    if((c == '!') || (c == '#'))
	return 1;
    
    c2=(decok & 0x7f);
    if(c2 > 80)
	return 0;

    switch(type)
    {
    case 8:
	if((c == ent_j8[67]) || (c == ent_j8[69]))
	    return 1;
	for(i=0;i<3;i++)
	{
	    c2++;
	    if(c2 > 67)
		c2=1;
	    if(c == ent_j8[c2])
		return 1;
	}
	return 0;
    case 13:
	if((c == ent_j7[61]) || (c == ent_j7[63]))
	    return 1;
	for(i=0;i<3;i++)
	{
	    c2++;
	    if(c2 > 61)
		c2=1;
	    if(c == ent_j7[c2])
		return 1;
	}
	return 0;
    case 14:
	if((c == ent_j7[67]) || (c == ent_j7[69]))
	    return 1;
	for(i=0;i<3;i++)
	{
	    c2++;
	    if(c2 > 67)
		c2=1;
	    if(c == ent_j7[c2])
		return 1;
	}
	return 0;
    case 15:
	if((c == ent_j8[71]) || (c == ent_j8[73]))
	    return 1;
	for(i=0;i<3;i++)
	{
	    c2++;
	    if(c2 > 72)
		c2=1;
	    if(c == ent_j8[c2])
		return 1;
	}
	return 0;
    default:
	return 0;
    }
}
