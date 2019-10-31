#	Makefile for ish201a5

# CFLAGS
#	NOBCOPY = use memcpy() memcpy()
#		default: bcopy() bcmp()
#	NOINDEX = use strchr(), strrchr()
#		default: index() rindex()
#	TZSET = use tzset()
#	FTIME = use ftime()
#		default: gettimeofday()
#	NAMELOWER = convert filename to lower case.
#		default: not convert.
#	NDIR = use <sys/ndir.h> , struct direct
#		default: struct dirent
#

#
#	XENIX-386
#
#CFLAGS = -O -DNOINDEX -DTZSET -DNAMELOWER -DNOBCOPY -DNDIR
#LIBS = -lx

#
#	SCO-UNIX-386
#
#CFLAGS = -O -DNOINDEX -DTZSET -DNAMELOWER -DNOBCOPY

#
#	SUNOS 4.1.1B / NEWS-OS 4.0C
#
#CFLAGS = -g -DNAMELOWER

#
#	SUNOS 5.4(Solaris 2.4), gcc ver 2.6.3
#
#CC = gcc
#CFLAGS = -O -DNOINDEX -DTZSET -DNAMELOWER -DNOBCOPY

#
#	Macintosh A/UX 3.0
#
#CFLAGS = -O -DTZSET -DNAMELOWER -DNOBCOPY

#
#       GNU/Linux x86
#
CFLAGS = -O -DNOINDEX -DTZSET -DNAMELOWER -DNOBCOPY

#
LDFLAGS = -g

#	binaries dir
DEST = /usr/local/bin

SRCS = crc.c crc32.c decode.c encode.c ish.c jis7.c jis8.c sjis.c njis.c mvol.c mencode.c
OBJS = crc.o crc32.o decode.o encode.o ish.o jis7.o jis8.o sjis.o njis.o mvol.o mencode.o
SRCS1 = dispish.c
OBJS1 = dispish.o

HDRS = ish.h
ISH = ish
DISPISH = dispish

all: $(ISH) $(DISPISH)

$(ISH): $(OBJS)
	@echo -n "Loading $(ISH) ... "
	@$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $(ISH)
	@echo "done"

$(DISPISH): $(SRCS1)
	$(CC) $(CFLAGS) -o $(DISPISH) $(SRCS1)

clean:
	@rm -f $(OBJS) $(ISH) $(DISPISH) ish201a5.ish ish201a5.lzh

install: $(ISH)
	@echo Installing $(ISH) in $(DEST)
	cp $(ISH) $(DISPISH) $(DEST)
	cd $(DEST); strip $(ISH) $(DISPISH)

$(OBJS) : $(HDRS)

lha:
	@echo Cleate arcive file.
	@rm -f ish201a5.lzh ish201mr.ish
	@lha a ish201a5.lzh $(SRCS) $(SRCS1) $(HDRS) Makefile *.doc
	@ish -s7 ish201a5.lzh

patch:
	@echo Cleate patch list.
	@rm -f patch.txt
	-@diff -c ./old . > patch.txt
	@cp -p $(SRCS) $(SRCS1) $(HDRS) Makefile *.doc ./old

# If this runs make out of memory, delete /usr/include lines.
crc.o: crc.c
crc32.o: crc32.c
decode.o: decode.c
decode.o: ish.h
dispish.o: dispish.c
encode.o: encode.c
encode.o: ish.h
ish.o: ish.c
ish.o: ish.h
jis7.o: ish.h
jis7.o: jis7.c
jis8.o: ish.h
jis8.o: jis8.c
mencode.o: ish.h
mencode.o: mencode.c
mvol.o: ish.h
mvol.o: mvol.c
njis.o: ish.h
njis.o: njis.c
sjis.o: ish.h
sjis.o: sjis.c
# WARNING: Put nothing here or make depend will gobble it up!
